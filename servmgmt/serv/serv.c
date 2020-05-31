/**
 *  @file servmgmt/serv/serv.c
 *
 *  @brief Implementation file for service library.
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
#include "jf_serv.h"
#include "jf_mutex.h"
#include "jf_ipaddr.h"
#include "jf_network.h"

#include "servmgmtcommon.h"
#include "servmgmtmsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The default timeout value.
 */
#define SERV_SEND_RECV_DATA_TIMEOUT   (5)

/** Define the internal service control data type.
 */
typedef struct
{
    /**Flag for the library initialization status. The library is initialized if it's TRUE.*/
    boolean_t is_bInitialized;
    u8 is_u8Reserved[7];

    u32 is_u32Reserved[4];
    /**The timeout value for sending and receiving message.*/
    u32 is_u32Timeout;

    /**Server address of service management daemon.*/
    jf_ipaddr_t is_jiServer;

    /**The mutex lock for the sequence number.*/
    jf_mutex_t is_jmLock;
    /**The sequence number.*/
    u32 is_u32SeqNum;

} internal_serv_t;

/** The service control instance.
 */
static internal_serv_t ls_isServ;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initServMgmtReqMsgHeader(
    servmgmt_msg_header_t * pHeader, internal_serv_t * pis, u8 u8MsgId, u32 u32MsgSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pHeader->smh_u8MsgId = u8MsgId;
    pHeader->smh_u32MagicNumber = SERVMGMT_MSG_MAGIC_NUMBER;
    pHeader->smh_u32PayloadSize = u32MsgSize - sizeof(servmgmt_msg_header_t);

    /*Set the transaction id and increase the id with 1.*/
    jf_mutex_acquire(&pis->is_jmLock);
    pHeader->smh_u32SeqNum = pis->is_u32SeqNum;
    pis->is_u32SeqNum ++;
    jf_mutex_release(&pis->is_jmLock);

    return u32Ret;
}

static u32 _sendRecvServMgmtMsg(
    internal_serv_t * pis, u8 * pSendMsg, olsize_t sSendMsg, u8 * pRecvMsg, olsize_t sRecvMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * pSocket = NULL;
    olsize_t sMsg = 0;
    servmgmt_msg_header_t * pHeader = NULL;

    /*Create the UDS socket.*/
    u32Ret = jf_network_createSocket(AF_UNIX, SOCK_STREAM, 0, &pSocket);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("socket created");

        /*Connect to the remote server.*/
        u32Ret = jf_network_connect(pSocket, &pis->is_jiServer, 0);
    }

    /*Send the request message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("send data, size: %d", sSendMsg);

        sMsg = sSendMsg;
        u32Ret = jf_network_sendnWithTimeout(pSocket, (void *)pSendMsg, &sMsg, pis->is_u32Timeout);
    }

    /*Receive the header of the response message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sMsg = sizeof(servmgmt_msg_header_t);

        JF_LOGGER_DEBUG("recv header, size: %d", sMsg);

        u32Ret = jf_network_recvnWithTimeout(pSocket, (void *)pRecvMsg, &sMsg, pis->is_u32Timeout);
    }

    /*Receive the payload of the response message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pHeader = (servmgmt_msg_header_t *)pRecvMsg;
        pRecvMsg = (u8 *)pRecvMsg + sMsg;
        sMsg = pHeader->smh_u32PayloadSize;

        JF_LOGGER_DEBUG("recv payload, size: %d", sMsg);

        if (sMsg != 0)
            u32Ret = jf_network_recvnWithTimeout(
                pSocket, (void *)pRecvMsg, &sMsg, pis->is_u32Timeout);
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

u32 jf_serv_init(jf_serv_init_param_t * pjsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;

    assert(pjsip != NULL);

    JF_LOGGER_INFO("init serv");

    pis->is_u32Timeout = SERV_SEND_RECV_DATA_TIMEOUT;
    jf_ipaddr_setUdsAddr(&pis->is_jiServer, SERVMGMT_SERVER_ADDR);

    u32Ret = jf_mutex_init(&pis->is_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        pis->is_bInitialized = TRUE;
    else
        jf_serv_fini();

    return u32Ret;
}

u32 jf_serv_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;

    JF_LOGGER_INFO("fini serv");

    u32Ret = jf_mutex_fini(&pis->is_jmLock);
    
    pis->is_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_serv_getInfoList(jf_serv_info_list_t * pjsil)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_get_info_list_req_t req;
    u8 u8Buffer[2048];
    servmgmt_get_info_list_resp_t * pResp = (servmgmt_get_info_list_resp_t *)u8Buffer;

    assert(pjsil != NULL);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initServMgmtReqMsgHeader(
        (servmgmt_msg_header_t *)&req, pis, SERVMGMT_MSG_ID_GET_INFO_LIST_REQ, sizeof(req));

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvServMgmtMsg(
        pis, (u8 *)&req, sizeof(req), u8Buffer, sizeof(u8Buffer));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = pResp->sgilr_smhHeader.smh_u32Result;

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_memcpy(pjsil, &pResp->sgilr_jsilList, pResp->sgilr_smhHeader.smh_u32PayloadSize);

    return u32Ret;
}

u32 jf_serv_getInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_get_info_req_t req;
    servmgmt_get_info_resp_t resp;

    assert((pstrName != NULL) && (pjsi != NULL));

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initServMgmtReqMsgHeader(
        (servmgmt_msg_header_t *)&req, pis, SERVMGMT_MSG_ID_GET_INFO_REQ, sizeof(req));
    ol_strncpy(req.sgir_strName, pstrName, JF_SERV_MAX_SERV_NAME_LEN - 1);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvServMgmtMsg(pis, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.sgir_smhHeader.smh_u32Result;

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_memcpy(pjsi, &resp.sgir_jsiInfo, resp.sgir_smhHeader.smh_u32PayloadSize);

    return u32Ret;
}

u32 jf_serv_stopServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_stop_serv_req_t req;
    servmgmt_stop_serv_resp_t resp;

    assert(pstrName != NULL);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initServMgmtReqMsgHeader(
        (servmgmt_msg_header_t *)&req, pis, SERVMGMT_MSG_ID_STOP_SERV_REQ, sizeof(req));
    ol_strncpy(req.sssr_strName, pstrName, JF_SERV_MAX_SERV_NAME_LEN - 1);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvServMgmtMsg(pis, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.sssr_smhHeader.smh_u32Result;

    return u32Ret;
}

u32 jf_serv_startServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_start_serv_req_t req;
    servmgmt_start_serv_resp_t resp;

    assert(pstrName != NULL);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initServMgmtReqMsgHeader(
        (servmgmt_msg_header_t *)&req, pis, SERVMGMT_MSG_ID_START_SERV_REQ, sizeof(req));
    ol_strncpy(req.sssr_strName, pstrName, JF_SERV_MAX_SERV_NAME_LEN - 1);

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvServMgmtMsg(pis, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.sssr_smhHeader.smh_u32Result;

    return u32Ret;
}

u32 jf_serv_setServStartupType(const olchar_t * pstrName, const u8 u8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_set_startup_type_req_t req;
    servmgmt_set_startup_type_resp_t resp;

    assert(pstrName != NULL);

    /*Initialize the request message.*/
    ol_bzero(&req, sizeof(req));
    _initServMgmtReqMsgHeader(
        (servmgmt_msg_header_t *)&req, pis, SERVMGMT_MSG_ID_SET_STARTUP_TYPE_REQ, sizeof(req));
    ol_strncpy(req.ssstr_strName, pstrName, JF_SERV_MAX_SERV_NAME_LEN - 1);
    req.ssstr_u8StartupType = u8StartupType;

    /*Send the request message and receive the respond message.*/
    u32Ret = _sendRecvServMgmtMsg(pis, (u8 *)&req, sizeof(req), (u8 *)&resp, sizeof(resp));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = resp.ssstr_smhHeader.smh_u32Result;

    return u32Ret;
}

const olchar_t * jf_serv_getStringServStatus(const u8 u8Status)
{
    return getStringServStatus(u8Status);
}

const olchar_t * jf_serv_getStringServStartupType(const u8 u8StartupType)
{
    return getStringServStartupType(u8StartupType);
}

u32 jf_serv_getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = getServStartupTypeFromString(pstrType, pu8StartupType);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


