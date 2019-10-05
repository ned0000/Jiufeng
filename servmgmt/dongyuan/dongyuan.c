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
#include "jf_mem.h"
#include "jf_network.h"
#include "jf_ipaddr.h"

#include "dongyuan.h"
#include "servmgmt.h"
#include "servmgmtcommon.h"
#include "servmgmtmsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_DONGYUAN_ASSOCKET_BUF_SIZE  (1024)

#define MAX_DONGYUAN_ASSOCKET_CONN      (3)

typedef struct
{
    boolean_t id_bInitialized;
    u8 id_u8Reserved[7];

    olchar_t * id_pstrSettingFile;
    u32 id_u32Reserved[8];

    jf_network_chain_t * id_pjncDongyuanChain;
    jf_network_assocket_t * id_pjnaDongyuanAssocket;

} internal_dongyuan_t;

static internal_dongyuan_t ls_idDongyuan;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDongyuanConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("on dongyuan connect, new connection");

    return u32Ret;
}

static u32 _onDongyuanDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg(
        "on dongyuan disconnect, reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onDongyuanSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("on dongyuan send data, len: %d", sBuf);

    return u32Ret;
}

static u32 _procesServMgmtMsgGetInfo(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_get_info_req_t * pReq = (servmgmt_get_info_req_t *)pu8Buffer;
    servmgmt_get_info_resp_t inforesp;

    if (u32Size < sizeof(servmgmt_get_info_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        inforesp.sgir_u32RetCode = getServMgmtServInfo(pReq->sgir_strName, &inforesp.sgir_jsiInfo);

        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)&inforesp, sizeof(inforesp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgGetInfoList(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_get_info_list_req_t * pReq = (servmgmt_get_info_list_req_t *)pu8Buffer;
    servmgmt_get_info_list_resp_t inforesp;
    jf_serv_info_list_t * pList = NULL;

    if (u32Size < sizeof(servmgmt_get_info_list_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_sharedmemory_attach(pReq->sgilr_jsiShmId, (void **)&pList);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        inforesp.sgilr_u32RetCode = getServMgmtServInfoList(pList);

        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)&inforesp, sizeof(inforesp));
    }

    if (pList != NULL)
        jf_sharedmemory_detach((void **)&pList);
    
    return u32Ret;
}

static u32 _procesServMgmtMsgStartServ(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_start_serv_req_t * pReq = (servmgmt_start_serv_req_t *)pu8Buffer;
    servmgmt_start_serv_resp_t servresp;

    if (u32Size < sizeof(servmgmt_start_serv_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        servresp.sssr_u32RetCode = startServMgmtServ(pReq->sssr_strName);

        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)&servresp, sizeof(servresp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgStopServ(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_stop_serv_req_t * pReq = (servmgmt_stop_serv_req_t *)pu8Buffer;
    servmgmt_stop_serv_resp_t servresp;

    if (u32Size < sizeof(servmgmt_stop_serv_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        servresp.sssr_u32RetCode = stopServMgmtServ(pReq->sssr_strName);

        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)&servresp, sizeof(servresp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsgSetStartupType(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    servmgmt_set_startup_type_req_t * pReq = (servmgmt_set_startup_type_req_t *)pu8Buffer;
    servmgmt_set_startup_type_resp_t servresp;

    if (u32Size < sizeof(servmgmt_set_startup_type_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        servresp.ssstr_u32RetCode = setServMgmtServStartupType(
            pReq->ssstr_strName, pReq->ssstr_u8StartupType);

        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)&servresp, sizeof(servresp));
    }

    return u32Ret;
}

static u32 _procesServMgmtMsg(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;
    u32 u32Size = u32EndPointer - u32Begin;
    servmgmt_msg_header_t * pHeader = NULL;

    if (u32Size < sizeof(servmgmt_msg_header_t))
        return u32Ret;

    pHeader = (servmgmt_msg_header_t *)pu8Buffer;
    switch (pHeader->smh_u8MsgId)
    {
    case SERVMGMT_MSG_ID_GET_INFO_REQ:
        u32Ret = _procesServMgmtMsgGetInfo(pAssocket, pAsocket, pu8Buffer + u32Begin, u32Size);
        break;
    case SERVMGMT_MSG_ID_GET_INFO_LIST_REQ:
        u32Ret = _procesServMgmtMsgGetInfoList(pAssocket, pAsocket, pu8Buffer + u32Begin, u32Size);
        break;
    case SERVMGMT_MSG_ID_START_SERV_REQ:
        u32Ret = _procesServMgmtMsgStartServ(pAssocket, pAsocket, pu8Buffer + u32Begin, u32Size);
        break;
    case SERVMGMT_MSG_ID_STOP_SERV_REQ:
        u32Ret = _procesServMgmtMsgStopServ(pAssocket, pAsocket, pu8Buffer + u32Begin, u32Size);
        break;
    case SERVMGMT_MSG_ID_SET_STARTUP_TYPE_REQ:
        u32Ret = _procesServMgmtMsgSetStartupType(
            pAssocket, pAsocket, pu8Buffer + u32Begin, u32Size);
        break;
    default:
        break;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *pu32BeginPointer = u32EndPointer;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _onDongyuanData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;

    jf_logger_logDebugMsg("on dongyuan data, begin: %d, end: %d", u32Begin, u32EndPointer);

    u32Ret = _procesServMgmtMsg(pAssocket, pAsocket, pu8Buffer, pu32BeginPointer, u32EndPointer);

    return u32Ret;
}

static u32 _createDongyuanAssocket(internal_dongyuan_t * pid, dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;

    u32Ret = jf_network_createChain(&pid->id_pjncDongyuanChain);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&jnacp, 0, sizeof(jnacp));

        jnacp.jnacp_sInitialBuf = MAX_DONGYUAN_ASSOCKET_BUF_SIZE;
        jnacp.jnacp_u32MaxConn = MAX_DONGYUAN_ASSOCKET_CONN;
        jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiAddr, SERVMGMT_SERVER_ADDR);
        jnacp.jnacp_fnOnConnect = _onDongyuanConnect;
        jnacp.jnacp_fnOnDisconnect = _onDongyuanDisconnect;
        jnacp.jnacp_fnOnSendData = _onDongyuanSendData;
        jnacp.jnacp_fnOnData = _onDongyuanData;

        u32Ret = jf_network_createAssocket(
            pid->id_pjncDongyuanChain, &pid->id_pjnaDongyuanAssocket, &jnacp);
    }

    return u32Ret;
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
    
    jf_logger_logDebugMsg("init dongyuan");

    ol_bzero(pid, sizeof(internal_dongyuan_t));

    pid->id_pstrSettingFile = pdp->dp_pstrSettingFile;

    /*change the working directory*/
    jf_file_getDirectoryName(
        strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&smip, 0, sizeof(smip));

        smip.smip_pstrSettingFile = pid->id_pstrSettingFile;

        u32Ret = initServMgmt(&smip);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createDongyuanAssocket(pid, pdp);
    }
    
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

    if (pid->id_pjnaDongyuanAssocket != NULL)
        u32Ret = jf_network_destroyAssocket(&pid->id_pjnaDongyuanAssocket);

    if (pid->id_pjncDongyuanChain != NULL)
        u32Ret = jf_network_destroyChain(pid->id_pjncDongyuanChain);

    finiServMgmt();

    pid->id_bInitialized = FALSE;

    return u32Ret;
}

u32 startDongyuan(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pid = &ls_idDongyuan;

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

    ol_memset(pdp, 0, sizeof(dongyuan_param_t));


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


