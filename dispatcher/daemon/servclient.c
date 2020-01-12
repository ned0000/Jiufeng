/**
 *  @file servclient.c
 *
 *  @brief Implementation file for message client of dispather service.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"
#include "jf_jiukun.h"
#include "jf_listhead.h"
#include "jf_queue.h"
#include "jf_hlisthead.h"
#include "jf_hashtable.h"

#include "servconfig.h"
#include "servclient.h"
#include "dispatcherxfer.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather service client data type.
 */
typedef struct
{
    dispatcher_serv_config_t * dsc_pdscConfig;

    dispatcher_xfer_t * dsc_pdxXfer;

    jf_listhead_t dsc_jlServ;

    boolean_t dsc_bLogin;
    u8 dsc_u8Reserved[7];

    /**The process id of the service.*/
    pid_t dsc_piServId;
} dispatcher_serv_client_t;

/** Define the dispatcher subscribed message data type.
 */
typedef struct
{
    /**The pointer to the message config.*/
    dispatcher_msg_config_t * dsm_pdmcMsgConfig;
    /**The pointer to the service client who subscribe the message.*/
    dispatcher_serv_client_t * dsm_pdscClient;
    /**Double linked list node for hash.*/
    jf_hlisthead_node_t dsm_jhnHash;

} dispatcher_subscribed_msg_t;

/** The chain for service clients. 
 */
static jf_network_chain_t * ls_pjncServClientChain = NULL;

/** The dispather client list.
 */
static jf_listhead_t ls_jlServClientList;

/** The shift for the subscribed message hash table.
 */
#define DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SHIFT     (8)

/** The size of the subscribed message hash table.
 */
#define DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SIZE  (1 << DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SHIFT)

/** The name of subscribed message cache.
 */
#define DISPATCHER_SUBSCRIBED_MSG_CACHE                "dispatcher_sub_msg"

/** The subscribed message hash table.
 */
static jf_hlisthead_t ls_jhMsgHashTable[DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SIZE];

/** The jiukun cache for dispatcher subscribed message.
 */
static jf_jiukun_cache_t * ls_pjjcSubscribedMsg = NULL;


/* --- private routine section ------------------------------------------------------------------ */

static u32 _createDispatcherServClientXfer(
    dispatcher_serv_client_t * pdsc, create_dispatcher_serv_client_param_t * pcdscp,
    jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_xfer_create_param_t dxcp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];
    jf_ipaddr_t jiRemote;

    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdscp->cdscp_pstrSocketDir,
        pdsc->dsc_pdscConfig->dsc_strMessagingIn);
    jf_ipaddr_setUdsAddr(&jiRemote, sockfile);

    ol_bzero(&dxcp, sizeof(dxcp));
    dxcp.dxcp_sMaxMsg = (olsize_t)pdsc->dsc_pdscConfig->dsc_u32MaxMsgSize;
    dxcp.dxcp_u32MaxNumMsg = pdsc->dsc_pdscConfig->dsc_u32MaxNumMsg;
    dxcp.dxcp_pjiRemote = &jiRemote;

    u32Ret = dispatcher_xfer_create(pChain, &pdsc->dsc_pdxXfer, &dxcp);

    return u32Ret;
}

static u32 _destroyDispatcherServClient(dispatcher_serv_client_t ** ppClient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = *ppClient;

    if (pdsc->dsc_pdxXfer != NULL)
        dispatcher_xfer_destroy(&pdsc->dsc_pdxXfer);

    jf_jiukun_freeMemory((void **)ppClient);

    return u32Ret;
}

static u32 _destroyDispatcherServClients(jf_listhead_t * pjlServClientList)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL, * temp = NULL;
    dispatcher_serv_client_t * pdsc = NULL;

    jf_listhead_forEachSafe(&ls_jlServClientList, pjl, temp)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        jf_listhead_del(&pdsc->dsc_jlServ);

        _destroyDispatcherServClient(&pdsc);
    }

    return u32Ret;
}

static u32 _createDispatcherServClient(
    dispatcher_serv_client_t ** ppClient, dispatcher_serv_config_t * pdsc,
    create_dispatcher_serv_client_param_t * pcdscp, jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pClient = NULL;

    jf_logger_logDebugMsg("create serv client: %s", pdsc->dsc_strName);

    u32Ret = jf_jiukun_allocMemory((void **)&pClient, sizeof(*pClient));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pClient, sizeof(*pClient));
        pClient->dsc_pdscConfig = pdsc;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherServClientXfer(pClient, pcdscp, pChain);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppClient = pClient;
    else if (pClient != NULL)
        _destroyDispatcherServClient(&pClient);

    return u32Ret;
}

static u32 _createDispatcherServClients(
    jf_linklist_t * pjlServConfig, jf_listhead_t * pjlServClientList,
    create_dispatcher_serv_client_param_t * pcdscp, jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode = NULL;
    dispatcher_serv_config_t * pdsc = NULL;
    dispatcher_serv_client_t * pClient = NULL;

    jf_logger_logDebugMsg("create serv clients");

    pNode = jf_linklist_getFirstNode(pjlServConfig);
    while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pdsc = jf_linklist_getDataFromNode(pNode);

        u32Ret = _createDispatcherServClient(&pClient, pdsc, pcdscp, pChain);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_listhead_addTail(pjlServClientList, &pClient->dsc_jlServ);

            pNode = jf_linklist_getNextNode(pNode);
        }
    }

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _servClientThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    jf_logger_logInfoMsg("enter serv client thread");

    /*Start the client chain.*/
    u32Ret = jf_network_startChain(pChain);

    jf_logger_logInfoMsg("quit serv client thread");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _dispatchMsgToServ(dispatcher_serv_client_t * pdsc, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Send the message to service.*/
    u32Ret = dispatcher_xfer_sendMsg(pdsc->dsc_pdxXfer, pdm);

    return u32Ret;
}

static u32 _addSubscribedMsgToTable(
    jf_jiukun_cache_t * pjjcSubscribedMsg, dispatcher_serv_client_t * pdsc,
    dispatcher_msg_config_t * pMsgConfig, jf_hlisthead_t * pjhMsgHashTable, u32 u32Shift)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_subscribed_msg_t * pdsm = NULL;
    u32 u32Index = 0;

    jf_logger_logDebugMsg("add msg to hash table, msg id: %u", pMsgConfig->dmc_u32MsgId);

    u32Ret = jf_jiukun_allocObject(pjjcSubscribedMsg, (void **)&pdsm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pdsm->dsm_pdmcMsgConfig = pMsgConfig;
        pdsm->dsm_pdscClient = pdsc;
        JF_HLISTHEAD_INIT_NODE(&pdsm->dsm_jhnHash);

        u32Index = jf_hashtable_hashU32(pMsgConfig->dmc_u32MsgId, u32Shift);
        jf_logger_logDebugMsg("add msg to hash table, index: %u", u32Index);
        jf_hlisthead_addHead(&pjhMsgHashTable[u32Index], &pdsm->dsm_jhnHash);
    }

    return u32Ret;
}

static u32 _addSubscribedMsgListToTable(
    jf_jiukun_cache_t * pjjcSubscribedMsg, dispatcher_serv_client_t * pdsc,
    jf_hlisthead_t * pjhMsgHashTable, u32 u32Shift)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pjln = NULL;
    dispatcher_msg_config_t * pMsgConfig = NULL;

    jf_logger_logDebugMsg(
        "add msg list to hash table, serv: %s", pdsc->dsc_pdscConfig->dsc_strName);

	pjln = jf_linklist_getFirstNode(&pdsc->dsc_pdscConfig->dsc_jlSubscribedMsg);
    while ((pjln != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pMsgConfig = jf_linklist_getDataFromNode(pjln);

        u32Ret = _addSubscribedMsgToTable(
            pjjcSubscribedMsg, pdsc, pMsgConfig, pjhMsgHashTable, u32Shift);

        if (u32Ret == JF_ERR_NO_ERROR)
            pjln = jf_linklist_getNextNode(pjln);
    }

    return u32Ret;
}

static u32 _buildDispatcherMsgHashTable(
    jf_jiukun_cache_t * pjjcSubscribedMsg, jf_listhead_t * pjlClient,
    jf_hlisthead_t * pjhMsgHashTable, u16 u16Count, u32 u32Shift)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL;
    dispatcher_serv_client_t * pdsc = NULL;
    u16 u16Index = 0;

    jf_logger_logDebugMsg("build sub msg hash table");

    /*Initialize the hash table.*/
    for (u16Index = 0; u16Index < u16Count; u16Index ++)
    {
        JF_HLISTHEAD_INIT(&pjhMsgHashTable[u16Index]);
    }

    /*Iterate the client list.*/
    jf_listhead_forEach(pjlClient, pjl)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        u32Ret = _addSubscribedMsgListToTable(
            pjjcSubscribedMsg, pdsc, pjhMsgHashTable, u32Shift);
    }

    return u32Ret;
}

static u32 _destroyDispatcherSubscribedMsgHashTable(
    jf_jiukun_cache_t * pCache, jf_hlisthead_t * pjhMsgHashTable, u16 u16Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hlisthead_node_t * pjhn = NULL, * temp = NULL;
    dispatcher_subscribed_msg_t * pdsm = NULL;
    u16 u16Index = 0;

    for (u16Index = 0; u16Index < u16Count; u16Index ++)
    {
        jf_hlisthead_forEachSafe(&pjhMsgHashTable[u16Index], pjhn, temp)
        {
            pdsm = jf_listhead_getEntry(pjhn, dispatcher_subscribed_msg_t, dsm_jhnHash);

            jf_hlisthead_del(&pdsm->dsm_jhnHash);

            jf_jiukun_freeObject(pCache, (void **)&pdsm);
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherServClients(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_client_param_t * pcdscp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("create dispatcher serv client");

    jf_listhead_init(&ls_jlServClientList);

    u32Ret = jf_network_createChain(&ls_pjncServClientChain);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createDispatcherServClients(
            pjlServConfig, &ls_jlServClientList, pcdscp, ls_pjncServClientChain);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_jiukun_cache_create_param_t jjccp;

        ol_bzero(&jjccp, sizeof(jjccp));
        jjccp.jjccp_pstrName = DISPATCHER_SUBSCRIBED_MSG_CACHE;
        jjccp.jjccp_sObj = sizeof(dispatcher_subscribed_msg_t);
        JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

        u32Ret = jf_jiukun_createCache(&ls_pjjcSubscribedMsg, &jjccp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _buildDispatcherMsgHashTable(
            ls_pjjcSubscribedMsg, &ls_jlServClientList, ls_jhMsgHashTable,
            DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SIZE, DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SHIFT);
    }

    return u32Ret;
}

u32 destroyDispatcherServClients(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("destroy dispatcher serv client");

    _destroyDispatcherSubscribedMsgHashTable(
        ls_pjjcSubscribedMsg, ls_jhMsgHashTable, DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SIZE);
    
    if (ls_pjjcSubscribedMsg != NULL)
        jf_jiukun_destroyCache(&ls_pjjcSubscribedMsg);

    _destroyDispatcherServClients(&ls_jlServClientList);

    if (ls_pjncServClientChain != NULL)
        u32Ret = jf_network_destroyChain(&ls_pjncServClientChain);

    return u32Ret;
}

u32 startDispatcherServClients(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to run the chain.*/
    u32Ret = jf_thread_create(NULL, NULL, _servClientThread, ls_pjncServClientChain);

    return u32Ret;
}

u32 stopDispatcherServClients(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_network_stopChain(ls_pjncServClientChain);

    return u32Ret;
}

u32 dispatchMsgToServ(dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL;
    dispatcher_serv_client_t * pdsc = NULL;

    jf_logger_logDebugMsg("dispatch msg to serv");

    jf_listhead_forEach(&ls_jlServClientList, pjl)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        u32Ret = _dispatchMsgToServ(pdsc, pdm);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


