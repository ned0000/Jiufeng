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

/** Maximum connection in the async server socket for a messaging server, one for the receiving
 *  message, another is for backup.
 */
#define MAX_CONN_IN_MESSAGING_SERVER           (2)

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

    jf_logger_logInfoMsg("init dispatcher messaging");

    u32Ret = jf_mutex_init(&pim->im_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createUdsDir();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_messaging_server_param_t cdmsp;
        ol_bzero(&cdmsp, sizeof(cdmsp));
        cdmsp.cdmsp_u32MaxConnInServer = MAX_CONN_IN_MESSAGING_SERVER;
        cdmsp.cdmsp_pstrSocketDir = DISPATCHER_UDS_DIR;
        cdmsp.cdmsp_fnProcessMsg = pjmip->jmip_fnProcessMsg;
        cdmsp.cdmsp_pstrMessagingIn = pjmip->jmip_pstrMessagingIn;
        cdmsp.cdmsp_pstrName = pjmip->jmip_pstrName;
        cdmsp.cdmsp_sMaxMsg = pjmip->jmip_sMaxMsg;

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

    jf_logger_logInfoMsg("fini dispatcher messaging");

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

    jf_logger_logInfoMsg("init messaging");

    u32Ret = _initDispatcherMessaging(pim, pjmip);

    return u32Ret;
}

u32 jf_messaging_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_messaging_t * pim = &ls_imMessaging;

    jf_logger_logInfoMsg("fini messaging");

    _finiDispatcherMessaging(pim);

    return u32Ret;
}

u32 jf_messaging_start(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;

    jf_logger_logInfoMsg("start messaging");

    u32Ret = startDispatcherMessagingClient();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherMessagingServer();

    return u32Ret;
}

u32 jf_messaging_stop(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;

    jf_logger_logInfoMsg("stop messaging");

    stopDispatcherMessagingClient();

    stopDispatcherMessagingServer();

    return u32Ret;
}

u32 jf_messaging_sendMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;
    dispatcher_msg_t * pdm = NULL;
    u32 u32MsgId = getMessagingMsgId(pu8Msg, sMsg);

    jf_logger_logDebugMsg("messaging send msg id: %u", u32MsgId);

    u32Ret = createDispatcherMsg(&pdm, pu8Msg, sMsg);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = sendDispatcherMessagingMsg(pdm);
    }

    return u32Ret;
}

u32 jf_messaging_initMsgHeader(u8 * pu8Msg, u32 u32MsgId, u8 u8MsgPrio, u32 u32PayloadSize)
{
    return initMessagingMsgHeader(pu8Msg, u32MsgId, u8MsgPrio, u32PayloadSize);
}

u32 jf_messaging_getMsgId(u8 * pu8Msg, olsize_t sMsg)
{
    return getMessagingMsgId(pu8Msg, sMsg);
}

u32 jf_messaging_setMsgDestinationId(u8 * pu8Msg, pid_t destinationId)
{
    return setMessagingMsgDestinationId(pu8Msg, destinationId);
}

u32 jf_messaging_setMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize)
{
    return setMessagingMsgPayloadSize(pu8Msg, u32PayloadSize);
}

/*------------------------------------------------------------------------------------------------*/


