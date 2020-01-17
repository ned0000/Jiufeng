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
    /**The service config.*/
    dispatcher_serv_config_t * dsc_pdscConfig;
    /**The xfer object for sending message.*/
    dispatcher_xfer_t * dsc_pdxXfer;
    /**The linked list for service client.*/
    jf_listhead_t dsc_jlServ;

    u8 dsc_u8Reserved[8];

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

/** Create dispatcher xfer.
 */
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
    dxcp.dxcp_pstrName = pdsc->dsc_pdscConfig->dsc_strName;

    u32Ret = dispatcher_xfer_create(pChain, &pdsc->dsc_pdxXfer, &dxcp);

    return u32Ret;
}

/** Destroy one service client.
 */
static u32 _destroyDispatcherServClient(dispatcher_serv_client_t ** ppClient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = *ppClient;

    /*Destroy dispatcher xfer.*/
    if (pdsc->dsc_pdxXfer != NULL)
        dispatcher_xfer_destroy(&pdsc->dsc_pdxXfer);

    /*Free the service client.*/
    jf_jiukun_freeMemory((void **)ppClient);

    return u32Ret;
}

/** Destroy all the service clients.
 */
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

/** Create one service client.
 */
static u32 _createDispatcherServClient(
    dispatcher_serv_client_t ** ppClient, dispatcher_serv_config_t * pdsc,
    create_dispatcher_serv_client_param_t * pcdscp, jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pClient = NULL;

    JF_LOGGER_DEBUG("serv client: %s", pdsc->dsc_strName);

    /*Allocate memory for service client data type.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pClient, sizeof(*pClient));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pClient, sizeof(*pClient));
        pClient->dsc_pdscConfig = pdsc;
    }

    /*Create dispatcher xfer.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherServClientXfer(pClient, pcdscp, pChain);

    /*Pause dispatcher xfer until service active message is received from service. The service may
      be not started ever. It's a waste of resource if we keep trying sending message to the not
      running service.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = dispatcher_xfer_pause(pClient->dsc_pdxXfer);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppClient = pClient;
    else if (pClient != NULL)
        _destroyDispatcherServClient(&pClient);

    return u32Ret;
}

/** Create all service clients.
 */
static u32 _createDispatcherServClients(
    jf_linklist_t * pjlServConfig, jf_listhead_t * pjlServClientList,
    create_dispatcher_serv_client_param_t * pcdscp, jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode = NULL;
    dispatcher_serv_config_t * pdsc = NULL;
    dispatcher_serv_client_t * pClient = NULL;

    JF_LOGGER_DEBUG("create serv clients");

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

/** Service client thread is to send message to service.
 */
static JF_THREAD_RETURN_VALUE _servClientThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    JF_LOGGER_INFO("enter serv client thread");

    /*Start the client chain.*/
    u32Ret = jf_network_startChain(pChain);

    JF_LOGGER_INFO("quit serv client thread");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _dispatchMsgToServ(dispatcher_serv_client_t * pdsc, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Increase the reference number in message.*/
    incDispatcherMsgRef(pdm);

    /*Send the message to service.*/
    u32Ret = dispatcher_xfer_sendMsg(pdsc->dsc_pdxXfer, pdm);

    return u32Ret;
}

/** Hash message id to key.
 */
static u32 _hashMsgIdToKey(u32 u32MsgId, u32 u32Shift)
{
    return jf_hashtable_hashU32(u32MsgId, u32Shift);
}

/** Add subscribed message to hash table.
 */
static u32 _addSubscribedMsgToTable(
    jf_jiukun_cache_t * pjjcSubscribedMsg, dispatcher_serv_client_t * pdsc,
    dispatcher_msg_config_t * pMsgConfig, jf_hlisthead_t * pjhMsgHashTable, u32 u32Shift)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_subscribed_msg_t * pdsm = NULL;
    u32 u32Index = 0;

    JF_LOGGER_DEBUG("msg id: %u", pMsgConfig->dmc_u32MsgId);

    /*Allocate dispatcher subscribed message from cache.*/
    u32Ret = jf_jiukun_allocObject(pjjcSubscribedMsg, (void **)&pdsm);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pdsm->dsm_pdmcMsgConfig = pMsgConfig;
        pdsm->dsm_pdscClient = pdsc;
        JF_HLISTHEAD_INIT_NODE(&pdsm->dsm_jhnHash);

        /*Hash message id to key.*/
        u32Index = _hashMsgIdToKey(pMsgConfig->dmc_u32MsgId, u32Shift);
        JF_LOGGER_DEBUG("index: %u", u32Index);
        /*Add dispatcher subscribed message data type to hash table.*/
        jf_hlisthead_addHead(&pjhMsgHashTable[u32Index], &pdsm->dsm_jhnHash);
    }

    return u32Ret;
}

/** Add subscribed message linked list to hash table.
 */
static u32 _addSubscribedMsgListToTable(
    jf_jiukun_cache_t * pjjcSubscribedMsg, dispatcher_serv_client_t * pdsc,
    jf_hlisthead_t * pjhMsgHashTable, u32 u32Shift)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pjln = NULL;
    dispatcher_msg_config_t * pMsgConfig = NULL;

    JF_LOGGER_DEBUG("serv: %s", pdsc->dsc_pdscConfig->dsc_strName);

    /*Subscribed message list is in service config of service client.*/
	pjln = jf_linklist_getFirstNode(&pdsc->dsc_pdscConfig->dsc_jlSubscribedMsg);
    while ((pjln != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pMsgConfig = jf_linklist_getDataFromNode(pjln);

        /*Add one subscribed message to hash table.*/
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

    JF_LOGGER_DEBUG("build hash table");

    /*Initialize the hash table.*/
    for (u16Index = 0; u16Index < u16Count; u16Index ++)
    {
        JF_HLISTHEAD_INIT(&pjhMsgHashTable[u16Index]);
    }

    /*Iterate the client list.*/
    jf_listhead_forEach(pjlClient, pjl)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        /*Add subscribed message list in service config to hash table.*/
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

    /*Free the hash table.*/
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

/** Find dispatcher service client by pid.
 */
static u32 _findDispatcherServClientByPid(
    jf_listhead_t * pjlServClientList, pid_t servPid, dispatcher_serv_client_t ** ppdsc)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    jf_listhead_t * pjl = NULL;
    dispatcher_serv_client_t * pdsc = NULL;

    jf_listhead_forEach(pjlServClientList, pjl)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        if (pdsc->dsc_pdscConfig->dsc_piServPid == servPid)
        {
            u32Ret = JF_ERR_NO_ERROR;
            *ppdsc = pdsc;
            break;
        }
    }

    return u32Ret;
}

/** Check if the message is matching to the subscribed msg.
 *
 *  @return The matching status.
 *  @retval TRUE The message is matching.
 *  @retval FALSE The message is not matching.
 */
static boolean_t _isMatchingDispatcherSubscribedMsg(
    dispatcher_subscribed_msg_t * pdsm, dispatcher_msg_t * pdm)
{
    boolean_t bRet = FALSE;
    u32 u32MsgId = getDispatcherMsgId(pdm);
    pid_t destPid = getDispatcherMsgDestinationId(pdm);

    /*Message id is not match, return FALSE.*/
    if (pdsm->dsm_pdmcMsgConfig->dmc_u32MsgId != u32MsgId)
        return bRet;

    /*If the destination id is not 0, the message should send to specified service.*/
    if (destPid != 0)
    {
        /*If the destination id in message is not equal to the saved id in service config, return
          FALSE.*/
        if (pdsm->dsm_pdscClient->dsc_pdscConfig->dsc_piServPid != destPid)
            return bRet;
    }
    
    return TRUE;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherServClients(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_client_param_t * pcdscp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("create serv client");

    jf_listhead_init(&ls_jlServClientList);

    /*Create the network chain.*/
    u32Ret = jf_network_createChain(&ls_pjncServClientChain);

    /*Create all the service client.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createDispatcherServClients(
            pjlServConfig, &ls_jlServClientList, pcdscp, ls_pjncServClientChain);
    }

    /*Create the cache for hash table entry.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_jiukun_cache_create_param_t jjccp;

        ol_bzero(&jjccp, sizeof(jjccp));
        jjccp.jjccp_pstrName = DISPATCHER_SUBSCRIBED_MSG_CACHE;
        jjccp.jjccp_sObj = sizeof(dispatcher_subscribed_msg_t);
        JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

        u32Ret = jf_jiukun_createCache(&ls_pjjcSubscribedMsg, &jjccp);
    }

    /*Build the hash table for all subscribed message, so we can send the message to all clients
      quickly*/
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

    JF_LOGGER_DEBUG("destroy serv clients");

    /*Destroy dispatcher subscribed message hash table.*/
    _destroyDispatcherSubscribedMsgHashTable(
        ls_pjjcSubscribedMsg, ls_jhMsgHashTable, DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SIZE);
    
    /*Destroy dispatcher subscribed message(dispatcher_subscribed_msg_t) cache.*/
    if (ls_pjjcSubscribedMsg != NULL)
        jf_jiukun_destroyCache(&ls_pjjcSubscribedMsg);

    /*Destroy service client linked list.*/
    _destroyDispatcherServClients(&ls_jlServClientList);

    /*Destroy the network chain.*/
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

    /*Stop the network chain.*/
    u32Ret = jf_network_stopChain(ls_pjncServClientChain);

    return u32Ret;
}

u32 pauseDispatcherServClient(pid_t servPid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = NULL;

    JF_LOGGER_INFO("servPid: %u", servPid);

    u32Ret = _findDispatcherServClientByPid(&ls_jlServClientList, servPid, &pdsc);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = dispatcher_xfer_pause(pdsc->dsc_pdxXfer);

    return u32Ret;
}

u32 resumeDispatcherServClient(pid_t servPid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = NULL;

    JF_LOGGER_INFO("servPid: %u", servPid);

    u32Ret = _findDispatcherServClientByPid(&ls_jlServClientList, servPid, &pdsc);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = dispatcher_xfer_resume(pdsc->dsc_pdxXfer);

    return u32Ret;
}

u32 dispatchMsgToServClients(dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hlisthead_node_t * pjhn = NULL;
    dispatcher_subscribed_msg_t * pdsm = NULL;
    u32 u32Index = 0;
    u32 u32MsgId = getDispatcherMsgId(pdm);

    JF_LOGGER_DEBUG("msg id: %u", u32MsgId);

    u32Index = _hashMsgIdToKey(u32MsgId, DISPATCHER_SUBSCRIBED_MSG_HASH_TABLE_SHIFT);
    JF_LOGGER_DEBUG("index: %u", u32Index);

    jf_hlisthead_forEach(&ls_jhMsgHashTable[u32Index], pjhn)
    {
        pdsm = jf_hlisthead_getEntry(pjhn, dispatcher_subscribed_msg_t, dsm_jhnHash);

        /*Need to double check the message id, incase different message id has the same hash key.*/
        if (_isMatchingDispatcherSubscribedMsg(pdsm, pdm))
            u32Ret = _dispatchMsgToServ(pdsm->dsm_pdscClient, pdm);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


