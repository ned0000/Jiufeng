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

#include "dispatchercommon.h"
#include "servconfig.h"
#include "servclient.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather service client data type.
 */
typedef struct
{
    dispatcher_serv_config_t * dsc_pdscConfig;

    jf_network_chain_t * dsc_pjncChain;
    jf_network_acsocket_t * dsc_pjnaAcsocket;

    jf_listhead_t dsc_jlServ;

    boolean_t dsc_bLogin;
    u8 dsc_u8Reserved[7];

    /**The process id of the service.*/
    pid_t dsc_piServId;
} dispatcher_serv_client_t;

/** The chain for service clients. 
 */
static jf_network_chain_t * ls_pjncServClientChain = NULL;

/** The dispather client list.
 */
static jf_listhead_t ls_jlServClientList;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDispatcherServClientConnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_serv_client_t * pdsc = jf_network_getTagOfAcsocket(pAcsocket);

    jf_logger_logDebugMsg("on serv client connect, connected");


    return u32Ret;
}

static u32 _onDispatcherServClientDisconnect(
    jf_network_acsocket_t * pAcsocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = jf_network_getTagOfAcsocket(pAcsocket);

    jf_logger_logDebugMsg(
        "on serv client disconnect, reason: %s", jf_err_getDescription(u32Status));

    pdsc->dsc_bLogin = FALSE;

    return u32Ret;
}

static u32 _onDispatcherServClientSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("on serv client send data");

    return u32Ret;
}

static u32 _onDispatcherServClientData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_serv_client_t * pdsc = jf_network_getTagOfAcsocket(pAcsocket);

    jf_logger_logInfoMsg("on serv client data, no data is expected");

    return u32Ret;
}

static u32 _createDispatcherServClientAcsocket(
    dispatcher_serv_client_t * pdsc, create_dispatcher_serv_client_param_t * pcdscp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];

    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = (olsize_t)pdsc->dsc_pdscConfig->dsc_u32MaxMsgSize;
    jnacp.jnacp_u32MaxConn = pcdscp->cdscp_u32MaxConnInClient;
    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdscp->cdscp_pstrSocketDir,
        pdsc->dsc_pdscConfig->dsc_strMessagingIn);
//    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiClient, sockfile);
    jnacp.jnacp_fnOnConnect = _onDispatcherServClientConnect;
    jnacp.jnacp_fnOnDisconnect = _onDispatcherServClientDisconnect;
    jnacp.jnacp_fnOnSendData = _onDispatcherServClientSendData;
    jnacp.jnacp_fnOnData = _onDispatcherServClientData;
    jnacp.jnacp_pstrName = pdsc->dsc_pdscConfig->dsc_strName;

    u32Ret = jf_network_createAcsocket(pdsc->dsc_pjncChain, &pdsc->dsc_pjnaAcsocket, &jnacp);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_network_setTagOfAcsocket(pdsc->dsc_pjnaAcsocket, pdsc);

    return u32Ret;
}

static u32 _destroyDispatcherServClient(dispatcher_serv_client_t ** ppClient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_client_t * pdsc = *ppClient;

    if (pdsc->dsc_pjnaAcsocket != NULL)
        jf_network_destroyAcsocket(&pdsc->dsc_pjnaAcsocket);

    jf_jiukun_freeMemory((void **)ppClient);

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
        pClient->dsc_pjncChain = pChain;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherServClientAcsocket(pClient, pcdscp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppClient = pClient;
    else if (pClient != NULL)
        _destroyDispatcherServClient(&pClient);

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

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherServClients(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_client_param_t * pcdscp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_config_t * pdsc = NULL;
    dispatcher_serv_client_t * pClient = NULL;
    jf_linklist_node_t * pNode = NULL;

    jf_logger_logDebugMsg("create dispatcher serv client");

    jf_listhead_init(&ls_jlServClientList);

    u32Ret = jf_network_createChain(&ls_pjncServClientChain);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pNode = jf_linklist_getFirstNode(pjlServConfig);
        while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            pdsc = jf_linklist_getDataFromNode(pNode);

            u32Ret = _createDispatcherServClient(&pClient, pdsc, pcdscp, ls_pjncServClientChain);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_listhead_addTail(&ls_jlServClientList, &pClient->dsc_jlServ);

                pNode = jf_linklist_getNextNode(pNode);
            }
        }
    }

    return u32Ret;
}

u32 destroyDispatcherServClients(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL, * temp = NULL;
    dispatcher_serv_client_t * pdsc = NULL;

    jf_logger_logDebugMsg("destroy dispatcher serv client");

    jf_listhead_forEachSafe(&ls_jlServClientList, pjl, temp)
    {
        pdsc = jf_listhead_getEntry(pjl, dispatcher_serv_client_t, dsc_jlServ);

        jf_listhead_del(&pdsc->dsc_jlServ);

        _destroyDispatcherServClient(&pdsc);
    }

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

/*------------------------------------------------------------------------------------------------*/


