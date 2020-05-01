/**
 *  @file servserver.c
 *
 *  @brief Implementation file for message server of dispather service.
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
#include "servserver.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather service server data type.
 */
typedef struct
{
    dispatcher_serv_config_t * dss_pdscConfig;
    /**The network chain.*/
    jf_network_chain_t * dss_pjncChain;
    jf_network_assocket_t * dss_pjnaAssocket;
    /**The linked list of the service server.*/
    jf_listhead_t dss_jlServ;

    boolean_t dss_bLogin;
    u8 dss_u8Reserved[7];
    /**The callback function to queue the message.*/
    fnQueueServServerMsg_t dss_fnQueueMsg;
} dispatcher_serv_server_t;

/** The chain for service servers. 
 */
static jf_network_chain_t * ls_pjncServServerChain = NULL;

/** The dispather server list.
 */
static JF_LISTHEAD(ls_jlServServerList);

/* --- private routine section ------------------------------------------------------------------ */

static u32 _checkServCredential(dispatcher_serv_server_t * pdss, jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct ucred credential;
    olsize_t len = sizeof(credential);

    /*Get the peer credential.*/
    u32Ret = jf_network_getAsocketOption(pAsocket, SOL_SOCKET, SO_PEERCRED, &credential, &len);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check the user id.*/
        if (pdss->dss_pdscConfig->dsc_uiUser != credential.uid)
            u32Ret = JF_ERR_DISPATCHER_UNAUTHORIZED_USER;
    }

    return u32Ret;
}

static u32 _onDispatcherServServerConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_server_t * pdss = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG("connect");

    pdss->dss_bLogin = TRUE;

    /*The following operation are available only for UDS.*/
    u32Ret = _checkServCredential(pdss, pAsocket);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        JF_LOGGER_INFO("security check failed");
        /*Security check is not passed, close the connection.*/
        u32Ret = jf_network_disconnectAssocket(pAssocket, pAsocket);
    }


    return u32Ret;
}

static u32 _onDispatcherServServerDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_server_t * pdss = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG("reason: %s", jf_err_getDescription(u32Status));

    pdss->dss_bLogin = FALSE;

    return u32Ret;
}

static u32 _onDispatcherServServerSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("no data is expected");

    return u32Ret;
}

static u32 _validateServServerMsg(
    u8 * pu8Buffer, olsize_t sBeginPointer, olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBuf = sEndPointer - sBeginPointer;
    olsize_t sMsg = 0;

    if (sBuf < sizeof(jf_messaging_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The full message size is header size plus payload size.*/
        sMsg = getMessagingSize(pu8Buffer + sBeginPointer);

        if (sBuf < sMsg)
            u32Ret = JF_ERR_INCOMPLETE_DATA;
    }

    return u32Ret;
}

static u32 _findServServerMsgId(jf_linklist_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16MsgId = *((u16 *)pArg);
    dispatcher_msg_config_t * pdmc = jf_linklist_getDataFromNode(pNode);

    /*Terminate the iteration if the message id is found.*/
    if (pdmc->dmc_u16MsgId == u16MsgId)
        u32Ret = JF_ERR_TERMINATED;

    return u32Ret;
}

static u32 _isServServerMsgAllowed(dispatcher_serv_server_t * pdss, u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16MsgId = getMessagingMsgId(pu8Msg, sMsg);

    /*The reserved message is used in dispatcher, it's always allowed.*/
    if (isReservedMessagingMsg(pu8Msg, sMsg))
        return u32Ret;
    
    if (pdss->dss_pdscConfig->dsc_u32ServId == DISPATCHER_INVALID_SERVICE_ID)
    {
        JF_LOGGER_INFO("invalid service id, msgid: %u", u16MsgId);
        u32Ret = JF_ERR_INVALID_MESSAGE;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_linklist_iterate(
            &pdss->dss_pdscConfig->dsc_jlPublishedMsg, _findServServerMsgId, (void **)&u16MsgId);

        /*Iteration is terminated, the message id is in the published list.*/
        if (u32Ret == JF_ERR_TERMINATED)
        {
            u32Ret = JF_ERR_NO_ERROR;
        }
        else
        {
            /*The message id is not in the published list.*/
            u32Ret = JF_ERR_MSG_NOT_IN_PUBLISHED_LIST;
            JF_LOGGER_INFO("message is not in published list, msg id: %u", u16MsgId);
        }
    }

    return u32Ret;
}

static u32 _processServServerReservedMsg(dispatcher_serv_server_t * pdss, u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16MsgId = getMessagingMsgId(pu8Msg, sMsg);
    dispatcher_serv_active_msg * pdsam = (dispatcher_serv_active_msg *)pu8Msg;

    if (u16MsgId == DISPATCHER_MSG_ID_SERV_ACTIVE)
    {
        if (pdsam->dsam_jmhHeader.jmh_u32SourceId != pdsam->dsam_dsampPayload.dsamp_u32ServId)
            u32Ret = JF_ERR_INVALID_MESSAGE;

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Save the service id.*/
            pdss->dss_pdscConfig->dsc_u32ServId = pdsam->dsam_dsampPayload.dsamp_u32ServId;
            JF_LOGGER_INFO("receive serv active msg, serv id: %u", pdss->dss_pdscConfig->dsc_u32ServId);
        }
    }

    return u32Ret;
}

static u32 _processServServerMsg(
    dispatcher_serv_server_t * pdss, jf_network_assocket_t * pAssocket,
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBegin = *psBeginPointer;
    olsize_t sMsg = 0;

    /*Validate the message.*/
    u32Ret = _validateServServerMsg(pu8Buffer, *psBeginPointer, sEndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Receive full message.*/
        sMsg = getMessagingSize(pu8Buffer + sBegin);

        u32Ret = _processServServerReservedMsg(pdss, pu8Buffer + sBegin, sMsg);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _isServServerMsgAllowed(pdss, pu8Buffer + sBegin, sMsg);
    }

    /*Invoke the callback function to queue the message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        pdss->dss_fnQueueMsg(pu8Buffer + sBegin, sMsg);

    if (u32Ret == JF_ERR_MSG_NOT_IN_PUBLISHED_LIST)
        /*Message is right, but it's not in the published list, the message is discarded.*/
        *psBeginPointer = sMsg;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        /*Message is not complete, need next round to get more data.*/
        u32Ret = JF_ERR_NO_ERROR;
    else
        /*For other else, discard the message.*/
        *psBeginPointer = sMsg;

    return u32Ret;
}

static u32 _onDispatcherServServerData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_server_t * pdss = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG("begin: %d, end: %d", *psBeginPointer, sEndPointer);

    u32Ret = _processServServerMsg(
        pdss, pAssocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer);

    return u32Ret;
}

static u32 _createDispatcherServServerAssocket(
    dispatcher_serv_server_t * pdss, create_dispatcher_serv_server_param_t * pcdssp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];

    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = (olsize_t)pdss->dss_pdscConfig->dsc_u32MaxMsgSize;
    jnacp.jnacp_u32MaxConn = pcdssp->cdssp_u32MaxConnInServer;
    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdssp->cdssp_pstrSocketDir,
        pdss->dss_pdscConfig->dsc_strMessagingOut);
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, sockfile);
    jnacp.jnacp_fnOnConnect = _onDispatcherServServerConnect;
    jnacp.jnacp_fnOnDisconnect = _onDispatcherServServerDisconnect;
    jnacp.jnacp_fnOnSendData = _onDispatcherServServerSendData;
    jnacp.jnacp_fnOnData = _onDispatcherServServerData;
    jnacp.jnacp_pstrName = pdss->dss_pdscConfig->dsc_strName;

    u32Ret = jf_network_createAssocket(pdss->dss_pjncChain, &pdss->dss_pjnaAssocket, &jnacp);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_network_setTagOfAssocket(pdss->dss_pjnaAssocket, pdss);

    return u32Ret;
}

static u32 _destroyDispatcherServServer(dispatcher_serv_server_t ** ppServ)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_server_t * pdss = *ppServ;

    if (pdss->dss_pjnaAssocket != NULL)
        jf_network_destroyAssocket(&pdss->dss_pjnaAssocket);

    jf_jiukun_freeMemory((void **)ppServ);

    return u32Ret;
}

static u32 _createDispatcherServServer(
    dispatcher_serv_server_t ** ppServ, dispatcher_serv_config_t * pdsc,
    create_dispatcher_serv_server_param_t * pcdssp, jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_server_t * pdss = NULL;

    JF_LOGGER_DEBUG("serv server: %s", pdsc->dsc_strName);

    u32Ret = jf_jiukun_allocMemory((void **)&pdss, sizeof(*pdss));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdss, sizeof(*pdss));
        pdss->dss_pdscConfig = pdsc;
        pdss->dss_pjncChain = pChain;
        pdss->dss_fnQueueMsg = pcdssp->cdssp_fnQueueMsg;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherServServerAssocket(pdss, pcdssp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppServ = pdss;
    else if (pdss != NULL)
        _destroyDispatcherServServer(&pdss);

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _servServerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    JF_LOGGER_INFO("enter serv server thread");

    /*Start the server chain.*/
    u32Ret = jf_network_startChain(pChain);

    JF_LOGGER_INFO("quit serv server thread");

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherServServers(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_server_param_t * pcdssp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_serv_config_t * pdsc = NULL;
    dispatcher_serv_server_t * pdss = NULL;
    jf_linklist_node_t * pNode = NULL;

    JF_LOGGER_DEBUG(
        "max conn: %u, socket dir: %s", pcdssp->cdssp_u32MaxConnInServer,
        pcdssp->cdssp_pstrSocketDir);

    jf_listhead_init(&ls_jlServServerList);

    u32Ret = jf_network_createChain(&ls_pjncServServerChain);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pNode = jf_linklist_getFirstNode(pjlServConfig);
        while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            pdsc = jf_linklist_getDataFromNode(pNode);

            u32Ret = _createDispatcherServServer(&pdss, pdsc, pcdssp, ls_pjncServServerChain);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_listhead_addTail(&ls_jlServServerList, &pdss->dss_jlServ);

                pNode = jf_linklist_getNextNode(pNode);
            }
        }
    }

    return u32Ret;
}

u32 destroyDispatcherServServers(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL, * temp = NULL;
    dispatcher_serv_server_t * pdss = NULL;

    JF_LOGGER_DEBUG("destroy serv servers");

    jf_listhead_forEachSafe(&ls_jlServServerList, pjl, temp)
    {
        pdss = jf_listhead_getEntry(pjl, dispatcher_serv_server_t, dss_jlServ);

        jf_listhead_del(&pdss->dss_jlServ);

        _destroyDispatcherServServer(&pdss);
    }

    if (ls_pjncServServerChain != NULL)
        u32Ret = jf_network_destroyChain(&ls_pjncServServerChain);

    return u32Ret;
}

u32 startDispatcherServServers(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to run the chain.*/
    u32Ret = jf_thread_create(NULL, NULL, _servServerThread, ls_pjncServServerChain);

    return u32Ret;
}

u32 stopDispatcherServServers(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_network_stopChain(ls_pjncServServerChain);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


