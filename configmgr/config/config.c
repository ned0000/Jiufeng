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
    config_mgr_msg_header_t * pHeader, internal_config_t * pic, u8 u8MsgId, u32 u32MsgSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->cmmh_u8MsgId = u8MsgId;
    pHeader->cmmh_u32MagicNumber = CONFIG_MGR_MSG_MAGIC_NUMBER;
    pHeader->cmmh_u32PayloadSize = u32MsgSize - sizeof(config_mgr_msg_header_t);

    /*Set the transaction id and increase the id with 1.*/
    jf_mutex_acquire(&pic->ic_jmLock);
    pHeader->cmmh_u32SeqNum = pic->ic_u32SeqNum;
    pic->ic_u32SeqNum ++;
    jf_mutex_release(&pic->ic_jmLock);

    return u32Ret;
}

static u32 _sendRecvConfigMgrMsg(
    internal_config_t * pic, u8 * pSendMsg, olsize_t sSendMsg, u8 * pRecvMsg, olsize_t sRecvMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * pSocket = NULL;
    olsize_t sMsg = 0;
    config_mgr_msg_header_t * pHeader = NULL;

    /*Create the socket.*/
    u32Ret = jf_network_createTypeStreamSocket(pic->ic_jiServer.ji_u8AddrType, &pSocket);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("socket created");

        /*Connect to the remote server.*/
        u32Ret = jf_network_connect(pSocket, &pic->ic_jiServer, 0);
    }

    /*Send the request message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("send data, size: %d", sSendMsg);

        sMsg = sSendMsg;
        u32Ret = jf_network_sendnWithTimeout(pSocket, (void *)pSendMsg, &sMsg, pic->ic_u32Timeout);
    }

    /*Receive the header of the response message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sMsg = sizeof(config_mgr_msg_header_t);

        JF_LOGGER_DEBUG("recv header, size: %d", sMsg);

        u32Ret = jf_network_recvnWithTimeout(pSocket, (void *)pRecvMsg, &sMsg, pic->ic_u32Timeout);
    }

    /*Receive the payload of the response message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pHeader = (config_mgr_msg_header_t *)pRecvMsg;
        pRecvMsg = (u8 *)pRecvMsg + sMsg;
        sMsg = pHeader->cmmh_u32PayloadSize;

        JF_LOGGER_DEBUG("recv payload, size: %d", sMsg);

        if (sMsg != 0)
            u32Ret = jf_network_recvnWithTimeout(
                pSocket, (void *)pRecvMsg, &sMsg, pic->ic_u32Timeout);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        JF_LOGGER_ERR(u32Ret, "send recv msg");
    }

    /*Destroy the socket.*/
    if (pSocket != NULL)
    {
        jf_network_destroySocket(&pSocket);
    }

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

u32 jf_config_get(const olchar_t * name, olchar_t * value, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    u8 u8Req[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_get_config_req_t * pReq = (config_mgr_get_config_req_t *)u8Req;
    olsize_t sReq = 0;
    u8 u8Resp[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_get_config_resp_t * pResp = (config_mgr_get_config_resp_t *)u8Resp;
    olchar_t * pstr = NULL;

    assert((name != NULL) && (value != NULL) && (sValue > 0));
    JF_LOGGER_DEBUG("name: %s, sValue: %d", name, sValue);

    /*Initialize the request message.*/
    sReq = sizeof(*pReq) + ol_strlen(name);
    ol_bzero(pReq, sReq);
    _initConfigMgrReqMsgHeader((config_mgr_msg_header_t *)pReq, pic, CMMI_GET_CONFIG_REQ, sReq);
    pReq->cmgcr_u16NameLen = ol_strlen(name);
    pstr = (olchar_t *)(pReq + 1);
    ol_memcpy(pstr, name, pReq->cmgcr_u16NameLen);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)pReq, sReq, u8Resp, sizeof(u8Resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = pResp->cmgcr_cmmhHeader.cmmh_u32Result;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(value, sValue);
        if (sValue > (olsize_t)pResp->cmgcr_u16ValueLen)
            sValue = (olsize_t)pResp->cmgcr_u16ValueLen;
        pstr = (olchar_t *)(pResp + 1);
        ol_memcpy(value, pstr, sValue);
        JF_LOGGER_DEBUG("value: %s(%d)", value, sValue);
    }

    return u32Ret;
}

u32 jf_config_set(const olchar_t * name, const olchar_t * value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    u8 u8Req[CONFIG_MGR_MAX_MSG_SIZE];
    config_mgr_set_config_req_t * pReq = (config_mgr_set_config_req_t *)u8Req;
    olsize_t sReq = 0;
    config_mgr_set_config_resp_t resp;
    u8 * pu8Mem = NULL;

    assert((name != NULL) && (value != NULL));
    JF_LOGGER_DEBUG("name: %s, value: %s", name, value);

    /*Initialize the request message.*/
    sReq = sizeof(*pReq) + ol_strlen(name) + ol_strlen(value);
    ol_bzero(pReq, sReq);
    _initConfigMgrReqMsgHeader((config_mgr_msg_header_t *)pReq, pic, CMMI_SET_CONFIG_REQ, sReq);
    pReq->cmscr_u16NameLen = ol_strlen(name);
    pReq->cmscr_u16ValueLen = ol_strlen(value);
    pu8Mem = (u8 *)(pReq + 1);
    ol_memcpy(pu8Mem, name, pReq->cmscr_u16NameLen);
    pu8Mem += pReq->cmscr_u16NameLen;
    ol_memcpy(pu8Mem, value, pReq->cmscr_u16ValueLen);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)pReq, sReq, (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmscr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

u32 jf_config_startTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_start_transaction_req_t req;
    config_mgr_start_transaction_resp_t resp;

    JF_LOGGER_DEBUG("start");

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_START_TRANSACTION_REQ, sizeof(req));

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmstr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

u32 jf_config_commitTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_commit_transaction_req_t req;
    config_mgr_commit_transaction_resp_t resp;

    JF_LOGGER_DEBUG("commit");

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_COMMIT_TRANSACTION_REQ, sizeof(req));

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmctr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

u32 jf_config_rollbackTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_t * pic = &ls_icConfig;
    config_mgr_rollback_transaction_req_t req;
    config_mgr_rollback_transaction_resp_t resp;

    JF_LOGGER_DEBUG("rollback");

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initConfigMgrReqMsgHeader(
        (config_mgr_msg_header_t *)&req, pic, CMMI_ROLLBACK_TRANSACTION_REQ, sizeof(req));

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvConfigMgrMsg(pic, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.cmrtr_cmmhHeader.cmmh_u32Result;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
