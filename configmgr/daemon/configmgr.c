/**
 *  @file configmgr.c
 *
 *  @brief Config management implementation file.
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
#include "jf_process.h"
#include "jf_file.h"
#include "jf_config.h"
#include "jf_jiukun.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"

#include "configmgr.h"
#include "configtree.h"
#include "configmgrcommon.h"
#include "configmgrmsg.h"
#include "configmgrsetting.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum connection in async server socket.
 */
#define CONFIG_MGR_DEFAULT_NUM_OF_CONN                (3)

/** Default config manager setting file.
 */
#define CONFIG_MGR_DEFAULT_SETTING_FILE             "../config/configmgr.setting"

/** Default number of transaction.
 */
#define CONFIG_MGR_DEFAULT_NUM_TRANSACTION          (10)

/** Define the internal config manager data type.
 */
typedef struct
{
    boolean_t icm_bInitialized;
    u8 icm_u8Reserved[7];

    u32 icm_u32Reserved[8];

    jf_network_chain_t * icm_pjncConfigMgrChain;
    jf_network_assocket_t * icm_pjnaConfigMgrAssocket;

    internal_config_mgr_setting_t icm_icmsSetting;

} internal_config_mgr_t;

/** The internal config manager instance.
 */
static internal_config_mgr_t ls_icmConfigMgr;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onConfigMgrConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("new connection");

    return u32Ret;
}

static u32 _onConfigMgrDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onConfigMgrSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("sbuf: %d", sBuf);

    return u32Ret;
}

static u32 _initConfigMgrRespMsgHeader(
    config_mgr_msg_header_t * pHeader, config_mgr_msg_header_t * pReqHeader, u8 u8MsgId, u32 u32MsgSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->cmmh_u8MsgId = u8MsgId;
    pHeader->cmmh_u32MagicNumber = CONFIG_MGR_MSG_MAGIC_NUMBER;
    pHeader->cmmh_u32SeqNum = pReqHeader->cmmh_u32SeqNum;
    pHeader->cmmh_u32PayloadSize = u32MsgSize - sizeof(config_mgr_msg_header_t);
    pHeader->cmmh_u32TransactionId = pReqHeader->cmmh_u32TransactionId;

    return u32Ret;
}

static u32 _procesConfigMgrMsgGetConfig(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_get_config_req_t * pReq = (config_mgr_get_config_req_t *)pu8Buffer;
    u8 u8Resp[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_get_config_resp_t * pResp = (config_mgr_get_config_resp_t *)u8Resp;
    olchar_t * pstrName = NULL, * pstrValue = NULL;
    olsize_t sName = 0, sValue = 0;

    JF_LOGGER_DEBUG("get config");

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(config_mgr_get_config_req_t) + pReq->cmgcr_u16NameLen)
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(u8Resp, sizeof(u8Resp));
        if (pReq->cmgcr_u16NameLen == 0)
        {
            pResp->cmgcr_cmmhHeader.cmmh_u32Result = JF_ERR_INVALID_PARAM;
        }
        else
        {
            pstrName = (olchar_t *)(pReq + 1);
            sName = pReq->cmgcr_u16NameLen;
            pstrValue = (olchar_t *)(pResp + 1);
            sValue = sizeof(u8Resp) - sizeof(*pResp);

            pResp->cmgcr_cmmhHeader.cmmh_u32Result = getConfigFromConfigTree(
                pReq->cmgcr_cmmhHeader.cmmh_u32TransactionId, pstrName, sName, pstrValue, &sValue);
            pResp->cmgcr_u16ValueLen = (u16)sValue;
        }

        _initConfigMgrRespMsgHeader(
            (config_mgr_msg_header_t *)pResp, (config_mgr_msg_header_t *)pReq,
            CMMI_GET_CONFIG_RESP, sizeof(*pResp) + pResp->cmgcr_u16ValueLen);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(
            pAssocket, pAsocket, (u8 *)pResp, sizeof(*pResp) + pResp->cmgcr_u16ValueLen);
    }

    return u32Ret;
}

static u32 _procesConfigMgrMsgSetConfig(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_set_config_req_t * pReq = (config_mgr_set_config_req_t *)pu8Buffer;
    config_mgr_set_config_resp_t resp;
    olchar_t * pstrName = NULL, * pstrValue = NULL;
    olsize_t sName = 0, sValue = 0;

    JF_LOGGER_DEBUG("set config");

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(config_mgr_set_config_req_t) + pReq->cmscr_u16NameLen + pReq->cmscr_u16ValueLen)
        u32Ret = JF_ERR_INCOMPLETE_DATA;
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        if ((pReq->cmscr_u16NameLen == 0) || (pReq->cmscr_u16ValueLen == 0))
        {
            resp.cmscr_cmmhHeader.cmmh_u32Result = JF_ERR_INVALID_PARAM;
        }
        else
        {
            pstrName = (olchar_t *)(pReq + 1);
            sName = pReq->cmscr_u16NameLen;
            pstrValue = pstrName + sName;
            sValue = pReq->cmscr_u16ValueLen;

            resp.cmscr_cmmhHeader.cmmh_u32Result = setConfigIntoConfigTree(
                pReq->cmscr_cmmhHeader.cmmh_u32TransactionId, pstrName, sName, pstrValue, sValue);
        }

        _initConfigMgrRespMsgHeader(
            (config_mgr_msg_header_t *)&resp, (config_mgr_msg_header_t *)pReq,
            CMMI_SET_CONFIG_RESP, sizeof(resp));

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }
    
    return u32Ret;
}

static u32 _procesConfigMgrMsgStartTransaction(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_start_transaction_req_t * pReq = (config_mgr_start_transaction_req_t *)pu8Buffer;
    config_mgr_start_transaction_resp_t resp;

    JF_LOGGER_DEBUG("start transaction");

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(config_mgr_start_transaction_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initConfigMgrRespMsgHeader(
            (config_mgr_msg_header_t *)&resp, (config_mgr_msg_header_t *)pReq,
            CMMI_START_TRANSACTION_RESP, sizeof(resp));
        /*Start transaction.*/
        resp.cmstr_cmmhHeader.cmmh_u32Result = startConfigTreeTransaction(
            &resp.cmstr_cmmhHeader.cmmh_u32TransactionId);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _procesConfigMgrMsgCommitTransaction(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_commit_transaction_req_t * pReq = (config_mgr_commit_transaction_req_t *)pu8Buffer;
    config_mgr_commit_transaction_resp_t resp;

    jf_logger_logDebugMsg("commit transaction");

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(config_mgr_commit_transaction_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initConfigMgrRespMsgHeader(
            (config_mgr_msg_header_t *)&resp, (config_mgr_msg_header_t *)pReq,
            CMMI_COMMIT_TRANSACTION_RESP, sizeof(resp));
        /*Commit transaction.*/
        resp.cmctr_cmmhHeader.cmmh_u32Result = commitConfigTreeTransaction(
            pReq->cmctr_cmmhHeader.cmmh_u32TransactionId);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _procesConfigMgrMsgRollbackTransaction(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t u32Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_rollback_transaction_req_t * pReq = (config_mgr_rollback_transaction_req_t *)pu8Buffer;
    config_mgr_rollback_transaction_resp_t resp;

    JF_LOGGER_DEBUG("rollback transaction");

    /*Check the size of the specified message.*/
    if (u32Size < sizeof(config_mgr_rollback_transaction_req_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&resp, sizeof(resp));
        _initConfigMgrRespMsgHeader(
            (config_mgr_msg_header_t *)&resp, (config_mgr_msg_header_t *)pReq,
            CMMI_ROLLBACK_TRANSACTION_RESP, sizeof(resp));
        /*Rollback transaction.*/
        resp.cmrtr_cmmhHeader.cmmh_u32Result = rollbackConfigTreeTransaction(
            pReq->cmrtr_cmmhHeader.cmmh_u32TransactionId);

        /*Send the response.*/
        u32Ret = jf_network_sendAssocketData(pAssocket, pAsocket, (u8 *)&resp, sizeof(resp));
    }

    return u32Ret;
}

static u32 _validateConfigMgrReqMsg(
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;
    u32 u32Size = u32EndPointer - u32Begin;
    config_mgr_msg_header_t * pHeader = (config_mgr_msg_header_t *)(pu8Buffer + u32Begin);

    /*The size should more than header size.*/
    if (u32Size < sizeof(config_mgr_msg_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check the magic number.*/
        if (pHeader->cmmh_u32MagicNumber != CONFIG_MGR_MSG_MAGIC_NUMBER)
        {
            u32Ret = JF_ERR_INVALID_DATA;
        }
    }

    return u32Ret;
}

static u32 _procesConfigMgrMsg(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = 0, u32Size = 0;
    config_mgr_msg_header_t * pHeader = NULL;

    /*Validate the config manager message.*/
    u32Ret = _validateConfigMgrReqMsg(pu8Buffer, pu32BeginPointer, u32EndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pHeader = (config_mgr_msg_header_t *)pu8Buffer;
        u32Begin = *pu32BeginPointer;
        pu8Buffer += u32Begin;
        u32Size = u32EndPointer - u32Begin;

        /*Process the message according to the message id.*/
        switch (pHeader->cmmh_u8MsgId)
        {
        case CMMI_GET_CONFIG_REQ:
            u32Ret = _procesConfigMgrMsgGetConfig(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case CMMI_SET_CONFIG_REQ:
            u32Ret = _procesConfigMgrMsgSetConfig(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case CMMI_START_TRANSACTION_REQ:
            u32Ret = _procesConfigMgrMsgStartTransaction(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case CMMI_COMMIT_TRANSACTION_REQ:
            u32Ret = _procesConfigMgrMsgCommitTransaction(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        case CMMI_ROLLBACK_TRANSACTION_REQ:
            u32Ret = _procesConfigMgrMsgRollbackTransaction(pAssocket, pAsocket, pu8Buffer, u32Size);
            break;
        default:
            jf_logger_logInfoMsg("received unkwown msg, id: %u", pHeader->cmmh_u8MsgId);
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

static u32 _onConfigMgrData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;

    JF_LOGGER_DEBUG("begin: %d, end: %d", u32Begin, u32EndPointer);

    /*Process the config  manager message.*/
    u32Ret = _procesConfigMgrMsg(pAssocket, pAsocket, pu8Buffer, pu32BeginPointer, u32EndPointer);

    return u32Ret;
}

static u32 _createConfigMgrAssocket(internal_config_mgr_t * picm, config_mgr_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;

    /*Set the parameter for creating async server socket.*/
    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = CONFIG_MGR_MAX_MSG_SIZE;
    jnacp.jnacp_u32MaxConn = picm->icm_icmsSetting.icms_u16MaxNumOfTransaction;
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, CONFIG_MGR_SERVER_ADDR);
    jnacp.jnacp_fnOnConnect = _onConfigMgrConnect;
    jnacp.jnacp_fnOnDisconnect = _onConfigMgrDisconnect;
    jnacp.jnacp_fnOnSendData = _onConfigMgrSendData;
    jnacp.jnacp_fnOnData = _onConfigMgrData;
    jnacp.jnacp_pstrName = "configmgr";

    /*Creat the async server socket.*/
    u32Ret = jf_network_createAssocket(
        picm->icm_pjncConfigMgrChain, &picm->icm_pjnaConfigMgrAssocket, &jnacp);

    return u32Ret;
}

static void _configMgrSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

    /*Stop config manager service.*/
    stopConfigMgr();
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initConfigMgr(config_mgr_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_mgr_t * picm = &ls_icmConfigMgr;
    config_tree_init_param_t ctip;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];

    assert(pcmp != NULL);
    assert(! picm->icm_bInitialized);
    
    JF_LOGGER_DEBUG(
        "cmd line: %s, setting file: %s", pcmp->cmp_pstrCmdLine, pcmp->cmp_pstrSettingFile);

    ol_bzero(picm, sizeof(internal_config_mgr_t));

    /*Change the working directory.*/
    jf_file_getDirectoryName(
        strExecutablePath, sizeof(strExecutablePath), pcmp->cmp_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    /*Parse setting file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strncpy(
            picm->icm_icmsSetting.icms_strSettingFile, pcmp->cmp_pstrSettingFile, JF_LIMIT_MAX_PATH_LEN - 1);

        u32Ret = readConfigMgrSetting(&picm->icm_icmsSetting);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (picm->icm_icmsSetting.icms_u16MaxNumOfTransaction == 0)
            picm->icm_icmsSetting.icms_u16MaxNumOfTransaction = CONFIG_MGR_DEFAULT_NUM_TRANSACTION;
        if (picm->icm_icmsSetting.icms_u16MaxNumOfConnection == 0)
            picm->icm_icmsSetting.icms_u16MaxNumOfTransaction = CONFIG_MGR_DEFAULT_NUM_OF_CONN;
    }

    /*Initialize the config tree module.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&ctip, sizeof(ctip));

        ctip.ctip_picmsSetting = &picm->icm_icmsSetting;

        u32Ret = initConfigTree(&ctip);
    }

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_registerSignalHandlers(_configMgrSignalHandler);

    /*Create the basic chain.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createChain(&picm->icm_pjncConfigMgrChain);

    /*Create the async server socket for message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createConfigMgrAssocket(picm, pcmp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        picm->icm_bInitialized = TRUE;
    else
        finiConfigMgr();

    return u32Ret;
}

u32 finiConfigMgr(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_mgr_t * picm = &ls_icmConfigMgr;

    JF_LOGGER_DEBUG("fini");

    /*Destroy the async server socket.*/
    if (picm->icm_pjnaConfigMgrAssocket != NULL)
        u32Ret = jf_network_destroyAssocket(&picm->icm_pjnaConfigMgrAssocket);

    /*Destroy the network chain.*/
    if (picm->icm_pjncConfigMgrChain != NULL)
        u32Ret = jf_network_destroyChain(&picm->icm_pjncConfigMgrChain);

    /*Finalize the config tree module.*/
    finiConfigTree();

    /*Free the resource in setting.*/
    freeConfigMgrSetting(&picm->icm_icmsSetting);

    picm->icm_bInitialized = FALSE;

    return u32Ret;
}

u32 startConfigMgr(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_mgr_t * picm = &ls_icmConfigMgr;

    JF_LOGGER_DEBUG("start");
    
    if (! picm->icm_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_startChain(picm->icm_pjncConfigMgrChain);

    return u32Ret;
}

u32 stopConfigMgr(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_mgr_t * picm = &ls_icmConfigMgr;

    if (! picm->icm_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_stopChain(picm->icm_pjncConfigMgrChain);

    }

    return u32Ret;
}

u32 setDefaultConfigMgrParam(config_mgr_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pcmp, sizeof(config_mgr_param_t));
    pcmp->cmp_pstrSettingFile = CONFIG_MGR_DEFAULT_SETTING_FILE;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
