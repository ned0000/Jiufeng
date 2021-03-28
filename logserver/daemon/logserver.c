/**
 *  @file logserver/daemon/logserver.c
 *
 *  @brief Log server implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"

/*Defined in logger library.*/
#include "common.h"

#include "logsave.h"
#include "logserver.h"
#include "logservermsg.h"
#include "logservercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum number of log client supported.
 */
#define LOG_SERVER_DEFAULT_MAX_LOG_CLIENT                (10)

/** Default log file name.
 */
#define LOG_SERVER_DEFAULT_LOG_FILE_NAME                 "jiufeng.log"

/** Define the internal log server data type.
 */
typedef struct
{
    /**Log server is initialized if it's TRUE.*/
    boolean_t ils_bInitialized;
    u8 ils_u8Reserved[7];

    u32 ils_u32Reserved[8];

    /**Network chain.*/
    jf_network_chain_t * ils_pjncLogServerChain;
    /**Async server socket.*/
    jf_network_assocket_t * ils_pjnaLogServerAssocket;

} internal_log_server_t;

/** The internal log server instance.
 */
static internal_log_server_t ls_ilsLogServer;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onLogServerConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("new connection");

    return u32Ret;
}

static u32 _onLogServerDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onLogServerSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("sbuf: %d", sBuf);

    return u32Ret;
}

static u32 _initLogServerRespMsgHeader(
    log_2_server_msg_header_t * pHeader, log_2_server_msg_header_t * pReqHeader, u8 u8MsgId, u16 u16MsgSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->l2smh_u8MsgId = u8MsgId;
    pHeader->l2smh_u16Signature = LOG_2_SERVER_MSG_SIGNATURE;
    pHeader->l2smh_u32SeqNum = pReqHeader->l2smh_u32SeqNum;
    pHeader->l2smh_u16PayloadSize = u16MsgSize - sizeof(log_2_server_msg_header_t);

    return u32Ret;
}

static u32 _procesLogServerMsgSaveLog(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_2_server_save_log_svc_t * pReq = (log_2_server_save_log_svc_t *)pu8Buffer;

    JF_LOGGER_DEBUG("msg size: %d", sMsg);

    /*Add the message to log save queue.*/
    u32Ret = saveLogToQueue(pReq->l2ssls_u64Time, pReq->l2ssls_strSource, pReq->l2ssls_strLog);

    return u32Ret;
}

static u32 _procesLogServerMsgGetSetting(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_server_get_setting_req_t * pReq = (log_server_get_setting_req_t *)pu8Buffer;
    log_server_get_setting_resp_t resp;

    JF_LOGGER_DEBUG("get setting");

    ol_bzero(&resp, sizeof(resp));

    _initLogServerRespMsgHeader(
        (log_2_server_msg_header_t *)&resp, (log_2_server_msg_header_t *)pReq,
        LSMI_GET_SETTING_RESP, sizeof(resp));

    /*Fill in the setting.*/

    /*Send the response.*/
    u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));

    return u32Ret;
}

static u32 _validateLogServerReqMsg(
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBegin = *psBeginPointer;
    olsize_t sMsg = sEndPointer - sBegin;
    log_2_server_msg_header_t * pHeader = (log_2_server_msg_header_t *)(pu8Buffer + sBegin);

    /*The size should more than header size.*/
    if (sMsg < sizeof(log_2_server_msg_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    /*Check the size of the message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sMsg < getLogServerMsgSize(pHeader))
            u32Ret = JF_ERR_INCOMPLETE_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check the message signature.*/
        if (pHeader->l2smh_u16Signature != LOG_2_SERVER_MSG_SIGNATURE)
        {
            u32Ret = JF_ERR_INVALID_DATA;
        }
    }

    return u32Ret;
}

static u32 _procesLogServerMsg(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sMsg = 0;
    log_2_server_msg_header_t * pHeader = NULL;

    /*Validate the log server message.*/
    u32Ret = _validateLogServerReqMsg(pu8Buffer, psBeginPointer, sEndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pu8Buffer += *psBeginPointer;
        pHeader = (log_2_server_msg_header_t *)pu8Buffer;
        sMsg = getLogServerMsgSize(pHeader);

        /*Process the message according to the message id.*/
        switch (pHeader->l2smh_u8MsgId)
        {
        case L2SMI_SAVE_LOG_SVC:
            _procesLogServerMsgSaveLog(pAssocket, pAsocket, pu8Buffer, sMsg);
            break;
        case LSMI_GET_SETTING_REQ:
            _procesLogServerMsgGetSetting(pAssocket, pAsocket, pu8Buffer, sMsg);
            break;
        default:
            JF_LOGGER_INFO("received unkwown msg, id: %u", pHeader->l2smh_u8MsgId);
            break;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        /*The message is handled successfully, discard the message.*/
        *psBeginPointer += sMsg;
    else if (u32Ret == JF_ERR_INVALID_DATA)
        /*Invalid data, discard all data.*/
        *psBeginPointer = sEndPointer;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        /*Incomplete data, waiting for more data.*/
        ;
    else
        /*For other errors, discard all data.*/
        *psBeginPointer = sEndPointer;

    return u32Ret;
}

static u32 _onLogServerData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("begin: %d, end: %d", *psBeginPointer, sEndPointer);

    /*Process the log server message. Quit until no more data can be processed, in case one
      data buffer has several messages.*/
    while (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _procesLogServerMsg(
            pAssocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer, pUser);

    return u32Ret;
}

static u32 _createLogServerAssocket(internal_log_server_t * pils, log_server_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;

    /*Set the parameter for creating async server socket.*/
    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = LOG_2_SERVER_MAX_MSG_SIZE;
    jnacp.jnacp_u32MaxConn = (u32)plsip->lsip_u16MaxLogClient + 3;
    /*Use INADDR_ANY if server address is not specified.*/
    if (plsip->lsip_pstrServerAddress == NULL)
        jf_ipaddr_setIpV4AddrToInaddrAny(&jnacp.jnacp_jiServer);
    else
        jf_ipaddr_getIpAddrFromString(
            plsip->lsip_pstrServerAddress, JF_IPADDR_TYPE_V4, &jnacp.jnacp_jiServer);
    jnacp.jnacp_u16ServerPort = plsip->lsip_u16ServerPort;
    jnacp.jnacp_fnOnConnect = _onLogServerConnect;
    jnacp.jnacp_fnOnDisconnect = _onLogServerDisconnect;
    jnacp.jnacp_fnOnSendData = _onLogServerSendData;
    jnacp.jnacp_fnOnData = _onLogServerData;
    jnacp.jnacp_pstrName = LOG_SERVER_NAME;

    /*Creat the async server socket.*/
    u32Ret = jf_network_createAssocket(
        pils->ils_pjncLogServerChain, &pils->ils_pjnaLogServerAssocket, &jnacp);

    return u32Ret;
}

static void _logServerSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

    /*Stop log server service.*/
    stopLogServer();
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initLogServer(log_server_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_server_t * pils = &ls_ilsLogServer;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];

    assert(plsip != NULL);
    assert(! pils->ils_bInitialized);
    
    JF_LOGGER_DEBUG(
        "cmd line: %s, max log client: %u, server: %s:%u", plsip->lsip_pstrCmdLine,
        plsip->lsip_u16MaxLogClient, plsip->lsip_pstrServerAddress, plsip->lsip_u16ServerPort);

    ol_bzero(pils, sizeof(internal_log_server_t));

    /*Change the working directory.*/
    jf_file_getDirectoryName(
        strExecutablePath, sizeof(strExecutablePath), plsip->lsip_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_registerSignalHandlers(_logServerSignalHandler);

    /*Initialize the log save module.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {    
        log_save_init_param_t lsip;
        ol_bzero(&lsip, sizeof(lsip));
        lsip.lsip_bLogToStdout = plsip->lsip_bLogToStdout;
        lsip.lsip_bLogToFile = plsip->lsip_bLogToFile;
        lsip.lsip_bLogToTty = plsip->lsip_bLogToTty;
        lsip.lsip_pstrLogFile = plsip->lsip_pstrLogFile;
        lsip.lsip_sLogFile = plsip->lsip_sLogFile;
        lsip.lsip_pstrTtyFile = plsip->lsip_pstrTtyFile;

        u32Ret = initLogSave(&lsip);
    }

    /*Create the basic chain.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createChain(&pils->ils_pjncLogServerChain);

    /*Create the async server socket for message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createLogServerAssocket(pils, plsip);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        pils->ils_bInitialized = TRUE;
    else
        finiLogServer();

    return u32Ret;
}

u32 finiLogServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_server_t * pils = &ls_ilsLogServer;

    JF_LOGGER_DEBUG("fini");

    /*Destroy the async server socket.*/
    if (pils->ils_pjnaLogServerAssocket != NULL)
        u32Ret = jf_network_destroyAssocket(&pils->ils_pjnaLogServerAssocket);

    /*Destroy the network chain.*/
    if (pils->ils_pjncLogServerChain != NULL)
        u32Ret = jf_network_destroyChain(&pils->ils_pjncLogServerChain);

    /*Finalize the log save.*/
    finiLogSave();

    pils->ils_bInitialized = FALSE;

    return u32Ret;
}

u32 startLogServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_server_t * pils = &ls_ilsLogServer;

    JF_LOGGER_INFO("start");
    
    if (! pils->ils_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startLogSave();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_startChain(pils->ils_pjncLogServerChain);

    return u32Ret;
}

u32 stopLogServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_server_t * pils = &ls_ilsLogServer;

    if (! pils->ils_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Stop the chain.*/
        jf_network_stopChain(pils->ils_pjncLogServerChain);

        /*Stop log save.*/
        stopLogSave();
    }

    return u32Ret;
}

u32 setDefaultLogServerParam(log_server_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(plsip, sizeof(log_server_init_param_t));
    plsip->lsip_u16MaxLogClient = LOG_SERVER_DEFAULT_MAX_LOG_CLIENT;
    plsip->lsip_u16ServerPort = JF_LOGGER_DEFAULT_SERVER_PORT;
    plsip->lsip_pstrLogFile = LOG_SERVER_DEFAULT_LOG_FILE_NAME;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
