/**
 *  @file dongyuan.c
 *
 *  @brief software management implementation file
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_serv.h"
#include "jf_jiukun.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"

#include "dongyuan.h"
#include "servmgmt.h"
#include "servmgmtcommon.h"
#include "servmgmtmsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The buffer should be large enough to hold all sevice information
 *  sizeof(jf_serv_info_t) * JF_SERV_MAX_NUM_OF_SERV
 */
#define MAX_DONGYUAN_ASSOCKET_BUF_SIZE         (2048)

/** Maximum connection in async server socket.
 */
#define MAX_DONGYUAN_ASSOCKET_CONN             (3)

/** Default donyuan setting file.
 */
#define DONGYUAN_DEFAULT_SETTING_FILE          "../config/servmgmt.setting"


/** Define the internal dongyuan data type.
 */
typedef struct
{
    boolean_t id_bInitialized;
    u8 id_u8Reserved[7];

    u32 id_u32Reserved[8];

    jf_network_chain_t * id_pjncDongyuanChain;
    jf_network_assocket_t * id_pjnaDongyuanAssocket;

} internal_dongyuan_t;

/** The internal dongyuan instance.
 */
static internal_dongyuan_t ls_idDongyuan;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDongyuanConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("on dongyuan connect, new connection");

    return u32Ret;
}

static u32 _onDongyuanDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onDongyuanSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("sbuf: %d", sBuf);

    return u32Ret;
}

static u32 _initServMgmtRespMsgHeader(
    servmgmt_msg_header_t * pHeader, servmgmt_msg_header_t * pReqHeader, u8 u8MsgId, u32 u32MsgSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->smh_u8MsgId = u8MsgId;
    pHeader->smh_u32MagicNumber = SERVMGMT_MSG_MAGIC_NUMBER;
    pHeader->smh_u32SeqNum = pReqHeader->smh_u32SeqNum;
    pHeader->smh_u32PayloadSize = u32MsgSize - sizeof(servmgmt_msg_header_t);

    return u32Ret;
}

static u32 _procesServMgmtMsgGetInfo(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_get_info_req_t * pReq = (servmgmt_get_info_req_t *)pu8Buffer;
    servmgmt_get_info_resp_t resp;

    JF_LOGGER_DEBUG("process msg, get info");
    
    /*Check the size of the specified message.*/
    if (u32Size < sizeof(servmgmt_get_info_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initServMgmtRespMsgHeader(
            (servmgmt_msg_header_t *)&resp, (servmgmt_msg_header_t *)pReq,
            SERVMGMT_MSG_ID_GET_INFO_RESP, sizeof(resp));
        resp.sgir_smhHeader.smh_u32Result = getServMgmtServInfo(
            pReq->sgir_strName, &resp.sgir_jsiInfo);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgGetInfoList(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_get_info_list_req_t * pReq = (servmgmt_get_info_list_req_t *)pu8Buffer;
    u8 u8Buffer[2048];
    servmgmt_get_info_list_resp_t * pResp = (servmgmt_get_info_list_resp_t *)u8Buffer;

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(servmgmt_get_info_list_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    JF_LOGGER_DEBUG("process msg, get info list");
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pResp, sizeof(*pResp));
        _initServMgmtRespMsgHeader(
            (servmgmt_msg_header_t *)pResp, (servmgmt_msg_header_t *)pReq,
            SERVMGMT_MSG_ID_GET_INFO_LIST_RESP, sizeof(*pResp));
        /*Get the service information list.*/
        pResp->sgilr_smhHeader.smh_u32Result = getServMgmtServInfoList(&pResp->sgilr_jsilList);
        /*Calculate the payload size.*/
        pResp->sgilr_smhHeader.smh_u32PayloadSize = sizeof(jf_serv_info_list_t) +
            sizeof(jf_serv_info_t) * pResp->sgilr_jsilList.jsil_u16NumOfService;

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)pResp,
            sizeof(servmgmt_msg_header_t) + pResp->sgilr_smhHeader.smh_u32PayloadSize);
    }
    
    return u32Ret;
}

static u32 _procesServMgmtMsgStartServ(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_start_serv_req_t * pReq = (servmgmt_start_serv_req_t *)pu8Buffer;
    servmgmt_start_serv_resp_t resp;

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(servmgmt_start_serv_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    JF_LOGGER_DEBUG("process msg, start serv");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initServMgmtRespMsgHeader(
            (servmgmt_msg_header_t *)&resp, (servmgmt_msg_header_t *)pReq,
            SERVMGMT_MSG_ID_START_SERV_RESP, sizeof(resp));
        /*Start the service.*/
        resp.sssr_smhHeader.smh_u32Result = startServMgmtServ(pReq->sssr_strName);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgStopServ(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_stop_serv_req_t * pReq = (servmgmt_stop_serv_req_t *)pu8Buffer;
    servmgmt_stop_serv_resp_t resp;

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(servmgmt_stop_serv_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    JF_LOGGER_DEBUG("process msg, stop serv");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initServMgmtRespMsgHeader(
            (servmgmt_msg_header_t *)&resp, (servmgmt_msg_header_t *)pReq,
            SERVMGMT_MSG_ID_STOP_SERV_RESP, sizeof(resp));
        /*Stop the service.*/
        resp.sssr_smhHeader.smh_u32Result = stopServMgmtServ(pReq->sssr_strName);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgSetStartupType(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_set_startup_type_req_t * pReq = (servmgmt_set_startup_type_req_t *)pu8Buffer;
    servmgmt_set_startup_type_resp_t resp;

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(servmgmt_set_startup_type_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    JF_LOGGER_DEBUG("process msg, set startup type");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initServMgmtRespMsgHeader(
            (servmgmt_msg_header_t *)&resp, (servmgmt_msg_header_t *)pReq,
            SERVMGMT_MSG_ID_SET_STARTUP_TYPE_RESP, sizeof(resp));
        /*Set the startup type.*/
        resp.ssstr_smhHeader.smh_u32Result = setServMgmtServStartupType(
            pReq->ssstr_strName, pReq->ssstr_u8StartupType);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _validateServMgmtReqMsg(
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;
    u32 u32Size = u32EndPointer - u32Begin;
    servmgmt_msg_header_t * pHeader = (servmgmt_msg_header_t *)(pu8Buffer + u32Begin);

    /*The size should more than header size.*/
    if (u32Size < sizeof(servmgmt_msg_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check the magic number.*/
        if (pHeader->smh_u32MagicNumber != SERVMGMT_MSG_MAGIC_NUMBER)
        {
            u32Ret = JF_ERR_INVALID_DATA;
        }
    }

    return u32Ret;
}

static u32 _procesServMgmtMsg(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = 0, u32Size = 0;
    servmgmt_msg_header_t * pHeader = NULL;

    /*Validate the service management message.*/
    u32Ret = _validateServMgmtReqMsg(pu8Buffer, pu32BeginPointer, u32EndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pHeader = (servmgmt_msg_header_t *)pu8Buffer;
        u32Begin = *pu32BeginPointer;
        pu8Buffer += u32Begin;
        u32Size = u32EndPointer - u32Begin;

        /*Process the message according to the message id.*/
        switch (pHeader->smh_u8MsgId)
        {
        case SERVMGMT_MSG_ID_GET_INFO_REQ:
            u32Ret = _procesServMgmtMsgGetInfo(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case SERVMGMT_MSG_ID_GET_INFO_LIST_REQ:
            u32Ret = _procesServMgmtMsgGetInfoList(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case SERVMGMT_MSG_ID_START_SERV_REQ:
            u32Ret = _procesServMgmtMsgStartServ(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case SERVMGMT_MSG_ID_STOP_SERV_REQ:
            u32Ret = _procesServMgmtMsgStopServ(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case SERVMGMT_MSG_ID_SET_STARTUP_TYPE_REQ:
            u32Ret = _procesServMgmtMsgSetStartupType(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        default:
            jf_logger_logInfoMsg("received unkwown msg, id: %u", pHeader->smh_u8MsgId);
            break;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        /*The message is handled without error, clear the data.*/
        *pu32BeginPointer = u32EndPointer;
    else if (u32Ret == JF_ERR_INVALID_DATA)
        /*Invalid data, discard the data.*/
        *pu32BeginPointer = u32EndPointer;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        /*Incomplete data, waiting for more data.*/
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _onDongyuanData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;

    JF_LOGGER_DEBUG("begin: %d, end: %d", u32Begin, u32EndPointer);

    /*Process the service management message.*/
    u32Ret = _procesServMgmtMsg(pAssocket, pAsocket, pu8Buffer, pu32BeginPointer, u32EndPointer);

    return u32Ret;
}

static u32 _createDongyuanAssocket(internal_dongyuan_t * pid, dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;

    /*Set the parameter for creating async server socket.*/
    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = MAX_DONGYUAN_ASSOCKET_BUF_SIZE;
    jnacp.jnacp_u32MaxConn = MAX_DONGYUAN_ASSOCKET_CONN;
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, SERVMGMT_SERVER_ADDR);
    jnacp.jnacp_fnOnConnect = _onDongyuanConnect;
    jnacp.jnacp_fnOnDisconnect = _onDongyuanDisconnect;
    jnacp.jnacp_fnOnSendData = _onDongyuanSendData;
    jnacp.jnacp_fnOnData = _onDongyuanData;
    jnacp.jnacp_pstrName = "dongyuan";

    /*Creat the async server socket.*/
    u32Ret = jf_network_createAssocket(
        pid->id_pjncDongyuanChain, &pid->id_pjnaDongyuanAssocket, &jnacp);

    return u32Ret;
}

static void _dongyuanSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

    /*Handle the SIGCHID for forked children process.*/
    if (signal == SIGCHLD)
        handleServMgmtSignal(signal);
    else /*Stop dongyuan service.*/
        stopDongyuan();
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initDongyuan(dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pid = &ls_idDongyuan;
    serv_mgmt_init_param_t smip;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];

    assert(pdp != NULL);
    assert(! pid->id_bInitialized);
    
    JF_LOGGER_DEBUG("cmd line: %s, setting file: %s", pdp->dp_pstrCmdLine, pdp->dp_pstrSettingFile);

    ol_bzero(pid, sizeof(internal_dongyuan_t));

    /*Change the working directory.*/
    jf_file_getDirectoryName(strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    /*Create the basic chain.*/
    u32Ret = jf_network_createChain(&pid->id_pjncDongyuanChain);
    
    /*Initialize the service management module.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&smip, sizeof(smip));

        smip.smip_pstrSettingFile = pdp->dp_pstrSettingFile;
        smip.smip_pjncChain = pid->id_pjncDongyuanChain;

        u32Ret = initServMgmt(&smip);
    }

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_registerSignalHandlers(_dongyuanSignalHandler);

    /*Create the async server socket for service control.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDongyuanAssocket(pid, pdp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        pid->id_bInitialized = TRUE;
    else
        finiDongyuan();

    return u32Ret;
}

u32 finiDongyuan(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pid = &ls_idDongyuan;

    JF_LOGGER_DEBUG("fini");

    /*Destroy the async server socket.*/
    if (pid->id_pjnaDongyuanAssocket != NULL)
        u32Ret = jf_network_destroyAssocket(&pid->id_pjnaDongyuanAssocket);

    /*Destroy the network chain.*/
    if (pid->id_pjncDongyuanChain != NULL)
        u32Ret = jf_network_destroyChain(&pid->id_pjncDongyuanChain);

    /*Finalize the service management module.*/
    finiServMgmt();

    pid->id_bInitialized = FALSE;

    return u32Ret;
}

u32 startDongyuan(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pid = &ls_idDongyuan;

    JF_LOGGER_DEBUG("start");
    
    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startServMgmt();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_startChain(pid->id_pjncDongyuanChain);

    return u32Ret;
}

u32 stopDongyuan(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pid = &ls_idDongyuan;

    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_stopChain(pid->id_pjncDongyuanChain);

        stopServMgmt();
    }

    return u32Ret;
}

u32 setDefaultDongyuanParam(dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pdp, sizeof(dongyuan_param_t));
    pdp->dp_pstrSettingFile = DONGYUAN_DEFAULT_SETTING_FILE;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
