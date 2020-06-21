/**
 *  @file config.c
 *
 *  @brief Implementation file for config library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_time.h"
#include "jf_config.h"
#include "jf_mutex.h"
#include "jf_ipaddr.h"
#include "jf_network.h"

#include "configmgrcommon.h"
#include "configmgrmsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The default timeout value.
 */
#define CONFIG_SEND_RECV_DATA_TIMEOUT   (5)

/** Define the internal config control type.
 */
typedef struct
{
    /**Flag for the library initialization status. The library is initialized if it's TRUE.*/
    boolean_t ic_bInitialized;
    u8 ic_u8Reserved[7];

    u32 ic_u32Reserved[4];
    /**The timeout value for sending and receiving message.*/
    u32 ic_u32Timeout;

    /**Server address of config management daemon.*/
    jf_ipaddr_t ic_jiServer;

    /**The mutex lock for the sequence number.*/
    jf_mutex_t ic_jmLock;
    /**The sequence number.*/
    u32 ic_u32SeqNum;

} internal_config_t;

/** The config control instance.
 */
static internal_config_t ls_icConfig;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initConfigMgrReqMsgHeader(
    config_mgr_msg_header_t * pHeader, internal_config_t * pic, u8 u8MsgId, u32 u32MsgSize,
    u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->cmmh_u8MsgId = u8MsgId;
    pHeader->cmmh_u32MagicNumber = CONFIG_MGR_MSG_MAGIC_NUMBER;
    pHeader->cmmh_u32PayloadSize = u32MsgSize - sizeof(config_mgr_msg_header_t);
    pHeader->cmmh_u32TransactionId = u32TransactionId;

    /*Set the transaction id and increase the id with 1.*/
    jf_mutex_acquire(&pic->ic_jmLock);
    pHeader->cmmh_u32SeqNum = pic->ic_u32SeqNum;
    pic->ic_u32SeqNum ++;
    jf_mutex_release(&pic->ic_jmLock);

    return u32Ret;
}

static olsize_t _getFullConfigMgrMsgSize(void * pHeader, olsize_t sHeader)
{
    olsize_t size = 0;
    config_mgr_msg_header_t * header = pHeader;

    size = sizeof(*header) + header->cmmh_u32PayloadSize;

    return size;
}

static u32 _sendRecvConfigMgrMsg(
    internal_config_t * pic, u8 * pSendMsg, olsize_t sSendMsg, u8 * pRecvMsg, olsize_t sRecvMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_transfer_data_param_t jntdp;

    ol_bzero(&jntdp, sizeof(jntdp));

    jntdp.jntdp_bReply = TRUE;
    jntdp.jntdp_pjiServer = &pic->ic_jiServer;
    jntdp.jntdp_u32Timeout = pic->ic_u32Timeout;
    jntdp.jntdp_pSendBuf = pSendMsg;
    jntdp.jntdp_sSendBuf = sSendMsg;
    jntdp.jntdp_pRecvBuf = pRecvMsg;
    jntdp.jntdp_sRecvBuf = sRecvMsg;
    jntdp.jntdp_sHeader = sizeof(config_mgr_msg_header_t);
    jntdp.jntdp_fnGetFullDataSize = _getFullConfigMgrMsgSize;

    u32Ret = jf_network_transferData(&jntdp);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_config_init(jf_config_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;

    assert(pjcip != NULL);

    JF_LOGGER_INFO("init config");

    pic->ic_u32Timeout = CONFIG_SEND_RECV_DATA_TIMEOUT;
    jf_ipaddr_setUdsAddr(&pic->ic_jiServer, CONFIG_MGR_SERVER_ADDR);

    u32Ret = jf_mutex_init(&pic->ic_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        pic->ic_bInitialized = TRUE;
    else
        jf_config_fini();

    return u32Ret;
}

u32 jf_config_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;

    JF_LOGGER_INFO("fini config");

    u32Ret = jf_mutex_fini(&pic->ic_jmLock);
    
    pic->ic_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_config_get(
    u32 u32TransactionId, const olchar_t * pstrName, olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    u8 u8Req[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_get_config_req_t * pReq = (config_mgr_get_config_req_t *)u8Req;
    olsize_t sReq = 0;
    u8 u8Resp[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_get_config_resp_t * pResp = (config_mgr_get_config_resp_t *)u8Resp;
    olchar_t * pstr = NULL;

    assert((pstrName != NULL) && (pstrValue != NULL) && (sValue > 0));
    JF_LOGGER_DEBUG("tran id: %u, name: %s, sValue: %d", u32TransactionId, pstrName, sValue);

    /*Initialize the request message.*/
    sReq = sizeof(*pReq) + ol_strlen(pstrName);
    ol_bzero(pReq, sReq);
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)pReq, pic, CMMI_GET_CONFIG_REQ, sReq, u32TransactionId);
    pReq->cmgcr_u16NameLen = ol_strlen(pstrName);
    pstr = (olchar_t *)(pReq + 1);
    ol_memcpy(pstr, pstrName, pReq->cmgcr_u16NameLen);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)pReq, sReq, u8Resp, sizeof(u8Resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = pResp->cmgcr_cmmhHeader.cmmh_u32Result;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pstrValue, sValue);
        if (sValue > (olsize_t)pResp->cmgcr_u16ValueLen)
            sValue = (olsize_t)pResp->cmgcr_u16ValueLen;
        pstr = (olchar_t *)(pResp + 1);
        ol_memcpy(pstrValue, pstr, sValue);
        JF_LOGGER_DEBUG("value: %s(%d)", pstrValue, sValue);
    }

    return u32Ret;
}

u32 jf_config_set(
    u32 u32TransactionId, const olchar_t * pstrName, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    u8 u8Req[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_set_config_req_t * pReq = (config_mgr_set_config_req_t *)u8Req;
    olsize_t sReq = 0;
    config_mgr_set_config_resp_t resp;
    u8 * pu8Mem = NULL;

    assert((pstrName != NULL) && (pstrValue != NULL));
    JF_LOGGER_DEBUG("tran id: %u, name: %s, value: %s", u32TransactionId, pstrName, pstrValue);

    /*Initialize the request message.*/
    sReq = sizeof(*pReq) + ol_strlen(pstrName) + ol_strlen(pstrValue);
    ol_bzero(pReq, sReq);
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)pReq, pic, CMMI_SET_CONFIG_REQ, sReq, u32TransactionId);
    pReq->cmscr_u16NameLen = ol_strlen(pstrName);
    pReq->cmscr_u16ValueLen = ol_strlen(pstrValue);
    pu8Mem = (u8 *)(pReq + 1);
    ol_memcpy(pu8Mem, pstrName, pReq->cmscr_u16NameLen);
    pu8Mem += pReq->cmscr_u16NameLen;
    ol_memcpy(pu8Mem, pstrValue, pReq->cmscr_u16ValueLen);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)pReq, sReq, (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmscr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

u32 jf_config_startTransaction(u32 * pu32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_start_transaction_req_t req;
    config_mgr_start_transaction_resp_t resp;

    JF_LOGGER_DEBUG("start");

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_START_TRANSACTION_REQ, sizeof(req),
        JF_CONFIG_INVALID_TRANSACTION_ID);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmstr_cmmhHeader.cmmh_u32Result;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *pu32TransactionId = resp.cmstr_cmmhHeader.cmmh_u32TransactionId;
        JF_LOGGER_DEBUG("tran id: %u", *pu32TransactionId);
    }

    return u32Ret;
}

u32 jf_config_commitTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_commit_transaction_req_t req;
    config_mgr_commit_transaction_resp_t resp;

    JF_LOGGER_DEBUG("tran id: %u", u32TransactionId);
    assert(u32TransactionId != JF_CONFIG_INVALID_TRANSACTION_ID);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_COMMIT_TRANSACTION_REQ, sizeof(req),
        u32TransactionId);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmctr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

u32 jf_config_rollbackTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_rollback_transaction_req_t req;
    config_mgr_rollback_transaction_resp_t resp;

    JF_LOGGER_DEBUG("tran id: %u", u32TransactionId);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_ROLLBACK_TRANSACTION_REQ, sizeof(req),
        u32TransactionId);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmrtr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
