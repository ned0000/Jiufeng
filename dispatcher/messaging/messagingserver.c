/**
 *  @file messagingserver.c
 *
 *  @brief Implementation file for dispatcher messaging server of messaging library.
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
#include "messagingserver.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather messaging server data type.
 */
typedef struct
{
    /**The network chain.*/
    jf_network_chain_t * dms_pjncChain;
    jf_network_assocket_t * dms_pjnaAssocket;

    u8 dms_u8Reserved[8];
    /**The callback function to process the message.*/
    jf_messaging_fnProcessMsg_t dms_fnProcessMsg;
    /**The process id of the service.*/
    pid_t dms_piServId;
} dispatcher_messaging_server_t;

/** The chain for service servers. 
 */
static jf_network_chain_t * ls_pjncMessagingServerChain = NULL;

/** The dispather messaging server.
 */
static dispatcher_messaging_server_t * ls_pdmsMessagingServer;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDispatcherMessagingServerConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    jf_logger_logDebugMsg("on messaging server connect, new connection");

    return u32Ret;
}

static u32 _onDispatcherMessagingServerDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    jf_logger_logDebugMsg(
        "on messaging server disconnect, reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onDispatcherMessagingServerSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("on messaging server send data, no data is expected");

    return u32Ret;
}

static u32 _validateMessagingServerMsg(
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

static u32 _isMessagingServerMsgAllowed(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _processMessagingServerMsg(
    dispatcher_messaging_server_t * pdms, jf_network_assocket_t * pAssocket,
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBegin = *psBeginPointer;
    olsize_t sMsg = 0;

    u32Ret = _validateMessagingServerMsg(pu8Buffer, *psBeginPointer, sEndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Receive full message.*/
        sMsg = getMessagingSize(pu8Buffer + sBegin);

        u32Ret = _isMessagingServerMsgAllowed(pu8Buffer + sBegin, sMsg);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pdms->dms_fnProcessMsg(pu8Buffer + sBegin, sMsg);

    if (u32Ret == JF_ERR_NO_ERROR)
        /*Message is processed, discard the message.*/
        *psBeginPointer = sMsg;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        /*Incomplete data.*/
        u32Ret = JF_ERR_NO_ERROR;
    else  /*For other errors, just discard the data.*/
        *psBeginPointer = sMsg;

    return u32Ret;
}

static u32 _onDispatcherMessagingServerData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    jf_logger_logDebugMsg(
        "on messaging server data, begin: %d, end: %d", *psBeginPointer, sEndPointer);

    u32Ret = _processMessagingServerMsg(
        pdms, pAssocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer);

    return u32Ret;
}

static u32 _createDispatcherMessagingServerAssocket(
    dispatcher_messaging_server_t * pdms, create_dispatcher_messaging_server_param_t * pcdmsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];

    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = pcdmsp->cdmsp_sMaxMsg;
    jnacp.jnacp_u32MaxConn = pcdmsp->cdmsp_u32MaxConnInServer;
    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdmsp->cdmsp_pstrSocketDir,
        pcdmsp->cdmsp_pstrMessagingIn);
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, sockfile);
    jnacp.jnacp_fnOnConnect = _onDispatcherMessagingServerConnect;
    jnacp.jnacp_fnOnDisconnect = _onDispatcherMessagingServerDisconnect;
    jnacp.jnacp_fnOnSendData = _onDispatcherMessagingServerSendData;
    jnacp.jnacp_fnOnData = _onDispatcherMessagingServerData;
    jnacp.jnacp_pstrName = pcdmsp->cdmsp_pstrName;

    u32Ret = jf_network_createAssocket(pdms->dms_pjncChain, &pdms->dms_pjnaAssocket, &jnacp);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_network_setTagOfAssocket(pdms->dms_pjnaAssocket, pdms);

    return u32Ret;
}

static u32 _destroyDispatcherMessagingServer(dispatcher_messaging_server_t ** ppServ)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = *ppServ;

    if (pdms->dms_pjnaAssocket != NULL)
        jf_network_destroyAssocket(&pdms->dms_pjnaAssocket);

    jf_jiukun_freeMemory((void **)ppServ);

    return u32Ret;
}

static u32 _createDispatcherMessagingServer(
    dispatcher_messaging_server_t ** ppServ, create_dispatcher_messaging_server_param_t * pcdmsp,
    jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = NULL;

    jf_logger_logDebugMsg("create dispatcher messaging server: %s", pcdmsp->cdmsp_pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&pdms, sizeof(*pdms));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdms, sizeof(*pdms));
        pdms->dms_pjncChain = pChain;
        pdms->dms_fnProcessMsg = pcdmsp->cdmsp_fnProcessMsg;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherMessagingServerAssocket(pdms, pcdmsp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppServ = pdms;
    else if (pdms != NULL)
        _destroyDispatcherMessagingServer(&pdms);

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _messagingServerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    JF_LOGGER_DEBUG("enter");

    /*Start the server chain.*/
    u32Ret = jf_network_startChain(pChain);

    JF_LOGGER_DEBUG("quit");

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherMessagingServer(create_dispatcher_messaging_server_param_t * pcdmsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("create dispatcher messaging server");

    u32Ret = jf_network_createChain(&ls_pjncMessagingServerChain);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherMessagingServer(
            &ls_pdmsMessagingServer, pcdmsp, ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 destroyDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("destroy dispatcher messaging server");

    if (ls_pdmsMessagingServer != NULL)
        _destroyDispatcherMessagingServer(&ls_pdmsMessagingServer);

    if (ls_pjncMessagingServerChain != NULL)
        jf_network_destroyChain(&ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 startDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to run the chain.*/
    u32Ret = jf_thread_create(NULL, NULL, _messagingServerThread, ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 stopDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_network_stopChain(ls_pjncMessagingServerChain);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


