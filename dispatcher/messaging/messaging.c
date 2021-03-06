/**
 *  @file messaging.c
 *
 *  @brief The implementation file for the messaging library.
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
#include "jf_messaging.h"
#include "jf_ipaddr.h"
#include "jf_time.h"
#include "jf_mutex.h"

#include "dispatchercommon.h"
#include "messagingserver.h"
#include "messagingclient.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    boolean_t im_bInitialized;
    u8 im_u8Reserved[7];

    u32 im_u32Reserved[4];

    jf_mutex_t im_jmLock;

} internal_messaging_t;

static internal_messaging_t ls_imMessaging;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initDispatcherMessaging(internal_messaging_t * pim, jf_messaging_init_param_t * pjmip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pjmip != NULL);
    assert(pjmip->jmip_fnProcessMsg != NULL);
    assert((pjmip->jmip_sMaxMsg > 0) && (pjmip->jmip_sMaxMsg < JF_MESSAGING_MAX_MSG_SIZE));

    u32Ret = jf_mutex_init(&pim->im_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createUdsDir();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_messaging_server_param_t cdmsp;
        ol_bzero(&cdmsp, sizeof(cdmsp));
        cdmsp.cdmsp_u32MaxConnInServer = DISPATCHER_MAX_CONN_IN_SERVICE_SERVER;
        cdmsp.cdmsp_pstrSocketDir = DISPATCHER_UDS_DIR;
        cdmsp.cdmsp_fnProcessMsg = pjmip->jmip_fnProcessMsg;
        cdmsp.cdmsp_pstrMessagingIn = pjmip->jmip_pstrMessagingIn;
        cdmsp.cdmsp_pstrName = pjmip->jmip_pstrName;
        cdmsp.cdmsp_sMaxMsg = pjmip->jmip_sMaxMsg;
        cdmsp.cdmsp_u32MaxNumMsg = pjmip->jmip_u32MaxNumMsg;

        u32Ret = createDispatcherMessagingServer(&cdmsp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_messaging_client_param_t cdmcp;
        ol_bzero(&cdmcp, sizeof(cdmcp));
        cdmcp.cdmcp_pstrSocketDir = DISPATCHER_UDS_DIR;
        cdmcp.cdmcp_pstrMessagingOut = pjmip->jmip_pstrMessagingOut;
        cdmcp.cdmcp_pstrName = pjmip->jmip_pstrName;
        cdmcp.cdmcp_sMaxMsg = pjmip->jmip_sMaxMsg;
        cdmcp.cdmcp_u32MaxNumMsg = pjmip->jmip_u32MaxNumMsg;

        u32Ret = createDispatcherMessagingClient(&cdmcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pim->im_bInitialized = TRUE;
    else
        jf_messaging_fini();

    return u32Ret;
}

static u32 _finiDispatcherMessaging(internal_messaging_t * pim)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("fini messaging");

    destroyDispatcherMessagingClient();

    destroyDispatcherMessagingServer();

    jf_mutex_fini(&pim->im_jmLock);
    
    pim->im_bInitialized = FALSE;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_messaging_init(jf_messaging_init_param_t * pjmip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_messaging_t * pim = &ls_imMessaging;

    assert(pjmip != NULL);
    assert(pjmip->jmip_fnProcessMsg != NULL);
    assert(pjmip->jmip_pstrMessagingIn != NULL);
    assert(pjmip->jmip_pstrMessagingOut != NULL);

    JF_LOGGER_INFO(
        "name: %s, MessagingIn: %s, MessagingOut: %s, sMaxMsg: %d, MaxNumMsg: %u",
        pjmip->jmip_pstrName, pjmip->jmip_pstrMessagingIn, pjmip->jmip_pstrMessagingOut,
        pjmip->jmip_sMaxMsg, pjmip->jmip_u32MaxNumMsg);

    u32Ret = _initDispatcherMessaging(pim, pjmip);

    return u32Ret;
}

u32 jf_messaging_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_messaging_t * pim = &ls_imMessaging;

    JF_LOGGER_INFO("fini");

    _finiDispatcherMessaging(pim);

    return u32Ret;
}

u32 jf_messaging_start(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;

    JF_LOGGER_INFO("start");

    u32Ret = startDispatcherMessagingClient();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherMessagingServer();

    return u32Ret;
}

u32 jf_messaging_stop(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;

    JF_LOGGER_INFO("stop");

    stopDispatcherMessagingClient();

    stopDispatcherMessagingServer();

    return u32Ret;
}

u32 jf_messaging_sendMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;
    dispatcher_msg_t * pdm = NULL;
    u16 u16MsgId = getMessagingMsgId(pu8Msg, sMsg);

    JF_LOGGER_DEBUG("msg id: %u", u16MsgId);

    u32Ret = createDispatcherMsg(&pdm, pu8Msg, sMsg);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = sendDispatcherMessagingMsg(pdm);
    }

    /*Destroy the message if the message queue is full.*/
    if (u32Ret == JF_ERR_MSG_QUEUE_FULL)
    {
        JF_LOGGER_DEBUG("msg queue full");
        freeDispatcherMsg(&pdm);
    }

    return u32Ret;
}

u32 jf_messaging_sendInternalMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;
    u16 u16MsgId = getMessagingMsgId(pu8Msg, sMsg);

    JF_LOGGER_DEBUG("msg id: %u", u16MsgId);

    u32Ret = createDispatcherMsg(&pdm, pu8Msg, sMsg);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = addDispatcherMessagingMsg(pdm);
    }

    /*Destroy the message if the message queue is full.*/
    if (u32Ret == JF_ERR_MSG_QUEUE_FULL)
    {
        JF_LOGGER_DEBUG("msg queue full");
        freeDispatcherMsg(&pdm);
    }

    return u32Ret;
}

u32 jf_messaging_initMsgHeader(u8 * pu8Msg, u16 u16MsgId, u8 u8MsgPrio, u32 u32PayloadSize)
{
    return initMessagingMsgHeader(pu8Msg, u16MsgId, u8MsgPrio, u32PayloadSize);
}

u16 jf_messaging_getMsgId(u8 * pu8Msg, olsize_t sMsg)
{
    return getMessagingMsgId(pu8Msg, sMsg);
}

u32 jf_messaging_setMsgDestinationId(u8 * pu8Msg, u32 destinationId)
{
    return setMessagingMsgDestinationId(pu8Msg, destinationId);
}

u32 jf_messaging_setMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize)
{
    return setMessagingMsgPayloadSize(pu8Msg, u32PayloadSize);
}

u32 jf_messaging_setMsgTransactionId(u8 * pu8Msg, u32 transactionId)
{
    return setMessagingMsgTransactionId(pu8Msg, transactionId);
}

u32 jf_messaging_getMsgTransactionId(u8 * pu8Msg, olsize_t sMsg)
{
    return getMessagingMsgTransactionId(pu8Msg, sMsg);
}

u32 jf_messaging_isFullMsg(u8 * pu8Msg, olsize_t sMsg)
{
    return isMessagingFullMsg(pu8Msg, sMsg);
}

olsize_t jf_messaging_getMsgSize(u8 * pu8Msg)
{
    return getMessagingMsgSize(pu8Msg);
}

/*------------------------------------------------------------------------------------------------*/
