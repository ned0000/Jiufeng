/**
 *  @file dataobjectpool.c
 *
 *  @brief Webclient data object pool implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_mutex.h"
#include "jf_network.h"
#include "jf_httpparser.h"
#include "jf_webclient.h"
#include "jf_jiukun.h"
#include "jf_string.h"
#include "jf_hex.h"
#include "jf_hashtree.h"
#include "jf_queue.h"
#include "jf_datavec.h"
#include "jf_hsm.h"

#include "common.h"
#include "dataobjectpool.h"
#include "webclientrequest.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** This is the number of seconds for IDLE state of data object. When no request in data object,
 *  the timer is started. When the timer is triggered, the connection for the data object is closed
 *  and the data object state is set to FREE.
 */
#define WEBCLIENT_DATAOBJECT_IDLE_TIMEOUT     (20)

/** This is the number of seconds for INITIAL state of data object. When the data object enter
 *  INITIAL state, the timer is started. When the timer is triggered, the data object is destroyed
 */
#define WEBCLIENT_DATAOBJECT_FREE_TIMEOUT     (30)

/** This is the number of times, an HTTP connection will be attempted, before it fails. This module
 *  utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will
 *  retry after 1 second, then 2, then 4, etc.
 */
#define WEBCLIENT_CONNECT_RETRY_COUNT         (3)

/** The name for acsocket and utimer
 */
#define WEBCLIENT_DATAOBJECT_NETWORK_OBJECT_NAME    "webclient-dataobject"

enum pipeline_type
{
    /**Doesn't know yet if the server supports persistent connections.*/
    PIPELINE_UNKNOWN = 0,
    /**The server does indeed support persistent connections.*/
    PIPELINE_YES,
    /**The server does not support persistent connections.*/
    PIPELINE_NO,
};

enum webclient_dataobject_state_id
{
    WDS_INITIAL = 0,
    WDS_CONNECTING,
    WDS_OPERATIVE,
    WDS_IDLE,
};

enum webclient_dataobject_event_id
{
    /**Connection to web server is established.*/
    WDE_CONNECTED,
    /**Send data.*/
    WDE_SEND_DATA,
    /**Data is sent.*/
    WDE_DATA_SENT,
    /**Connection is closed.*/
    WDE_DISCONNECTED,
};

struct internal_webclient_dataobject_pool;

typedef struct internal_webclient_dataobject
{
    jf_ipaddr_t iwd_jiRemote;
    u16 iwd_u16RemotePort;
    u16 iwd_u16Reserved[3];

    struct internal_webclient_dataobject_pool * iwd_piwdpPool;

    jf_hsm_t * iwd_pjhDataobject;

    u8 iwd_u8PipelineFlags;
    u8 iwd_u8Reserved[7];

    u32 iwd_u32ExponentialBackoff;

    jf_httpparser_dataobject_t * iwd_pjhdDataobject;

    jf_queue_t iwd_jqRequest;

    jf_network_asocket_t * iwd_pjnaConn;

    jf_ipaddr_t iwd_jiLocal;

} internal_webclient_dataobject_t;

typedef struct internal_webclient_dataobject_pool
{
    jf_network_acsocket_t * iwdp_pjnaAcsocket;
    u32 iwdp_u32PoolSize;
    u32 iwdp_u32Reserved;

    /**The web data object is put to data hash tree when the connection is established and there
       are web request.*/
    jf_hashtree_t iwdp_jhDataobject;

    jf_network_utimer_t * iwdp_pjnuUtimer;

    olsize_t iwdp_sBuffer;
    olsize_t iwdp_sReserved;

    jf_network_chain_t * iwdp_pjncChain;

} internal_webclient_dataobject_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

static olchar_t * _getStringWebclientDataobjectState(jf_hsm_state_id_t stateId)
{
    olchar_t * str = "WDS Unknown";
    switch (stateId)
    {
    case WDS_INITIAL:
        str = "WDS Initial";
        break;
    case WDS_CONNECTING:
        str = "WDS Connecting";
        break;
    case WDS_OPERATIVE:
        str = "WDS Operative";
        break;
    case WDS_IDLE:
        str = "WDS Idle";
        break;
    default:
        break;
    }

    return str;
};

/** Free resources associated with a webclient data object.
 *
 *  @note
 *  -# The connection should be disconnected before. This function is not responsible for closing
 *   the connection.
 *
 *  @param ppDataobject [in/out] The webclient data object to free.
 */
static u32 _destroyWebclientDataobject(webclient_dataobject_t ** ppDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *) *ppDataobject;
    internal_webclient_request_t * piwr = NULL;
    olchar_t key[64];

    getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg("destroy webclient data obj %s", key);

    assert(piwd->iwd_pjnaConn == NULL);

    /*Destroy utimer item.*/
    jf_network_removeUtimerItem(piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd);

    /*Destroy the hsm object.*/
    if (piwd->iwd_pjhDataobject != NULL)
        jf_hsm_destroy(&piwd->iwd_pjhDataobject);

    /*Destroy the httpparse data object.*/
    if (piwd->iwd_pjhdDataobject != NULL)
        jf_httpparser_destroyDataobject(&piwd->iwd_pjhdDataobject);

    /*Iterate through all the pending requests.*/
    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    while (piwr != NULL)
    {
        /*If this is a client request, then we need to signal that this request is being aborted.*/
        piwr->iwr_fnOnEvent(NULL, JF_WEBCLIENT_EVENT_HTTP_REQ_DELETED, NULL, piwr->iwr_pUser);
        destroyWebclientRequest(&piwr);

        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    }
    jf_queue_fini(&piwd->iwd_jqRequest);

    jf_jiukun_freeMemory((void **)ppDataobject);

    return u32Ret;
}

/** Internal method dispatched by the utimer, to retry refused connections.
 *
 *  @note
 *  -# The module does an exponential backoff, when retrying connections. The number of retries is
 *   determined by the value of WEBCLIENT_CONNECT_RETRY_COUNT.
 *
 *  @param object [in] The associated web data object.
 */
static u32 _webclientDataobjectRetryConnect(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg(
        "webclient retry connect, %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) WEBCLIENT_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        jf_logger_logInfoMsg("webclient retry connect, give up");

        jf_hashtree_deleteEntry(&piwd->iwd_piwdpPool->iwdp_jhDataobject, key, keyLength);
        _destroyWebclientDataobject((webclient_dataobject_t **)&piwd);
    }
    else
    {
        /*Lets retry again*/
        jf_hsm_event_t event;
        jf_logger_logInfoMsg("webclient retry connect, try to connect");

        jf_hsm_initEvent(&event, WDE_SEND_DATA, piwd, NULL);

        u32Ret = jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);
    }

    return u32Ret;
}

static u32 _webclientDataOjectFreeTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;
    olchar_t key[64];
    olint_t keyLength;

    keyLength = getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);

    jf_logger_logInfoMsg("webclient free timer handler");

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        /*This connection is idle, because there are no pending requests */
        jf_logger_logInfoMsg("webclient free timer handler, queue is empty");

        jf_hashtree_deleteEntry(&piwd->iwd_piwdpPool->iwdp_jhDataobject, key, keyLength);
        _destroyWebclientDataobject((webclient_dataobject_t **)&piwd);
    }

    return u32Ret;
}

static u32 _sendWebclientRequestData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    internal_webclient_request_t * piwr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    for (u16Index = 0;
         (u16Index < piwr->iwr_jdDataVec.jd_u16NumOfEntry) && (u32Ret == JF_ERR_NO_ERROR);
         ++ u16Index)
    {
        entry = &piwr->iwr_jdDataVec.jd_jdeEntry[u16Index];

        u32Ret = jf_network_sendAcsocketData(
            pAcsocket, pAsocket, entry->jde_pu8Data, entry->jde_sOffset);
    }

    return u32Ret;
}

static u32 _fnEventActionStartConn(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    /*If the data object is created, connect to remote server. The async socket will wake
      up the chain so webclient doesn't have to care about it.*/
    u32Ret = jf_network_connectAcsocketTo(
        piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort, piwd);

    return u32Ret;
}

static boolean_t _isDataobjectRequestInQueue(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    if (! jf_queue_isEmpty(&piwd->iwd_jqRequest))
        bRet = TRUE;

    return bRet;
}

static boolean_t _isEmptyDataobjectRequestQueue(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
        bRet = TRUE;

    return bRet;
}

static u32 _fnEventActionSendData(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;
    internal_webclient_request_t * piwr = NULL;

    piwr = (internal_webclient_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr != NULL)
    {
        u32Ret = _sendWebclientRequestData(
            piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, piwd->iwd_pjnaConn, piwr);
    }

    return u32Ret;
}

/** The timed callback is used to close idle sockets.
 *
 *  @note
 *  -# A socket is considered idle if after a request is answered, another request isn't received
 *   within the time specified by WEBCLIENT_DATAOBJECT_IDLE_TIMEOUT.
 *
 *  @param object [in] The web data object.
 */
static u32 _webclientDataobjectIdleTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;

    jf_logger_logInfoMsg("webclient idle timer handler");

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        jf_logger_logInfoMsg("webclient idle timer handler, queue is empty");
        /*This connection is idle, because there are no pending requests. We need to close this
          socket.*/
        jf_logger_logInfoMsg("webclient idle timer handler, close the connection");
        jf_network_disconnectAcsocket(
            piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, piwd->iwd_pjnaConn);
    }

    return u32Ret;
}

static u32 _fnEventActionDataSent(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;
    internal_webclient_request_t * piwr = NULL;

    piwr = (internal_webclient_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);

    if (piwd->iwd_u8PipelineFlags == PIPELINE_NO)
    {
        /*Pipelining is not supported, so we should just close the socket instead of waiting for
          the other guy to close it, because if they forget to, it will screw us over if there
          are pending requests.*/

        /*It should also be noted, that when this closes, the module will realize there are
          pending requests, in which case it will open a new connection for the requests.*/
        jf_logger_logInfoMsg(
            "webclient finish response, pipeline is no, disconnect the data object");
        jf_network_disconnectAcsocket(
            piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, piwd->iwd_pjnaConn);
    }
    else
    {
        /*If the connection is still open, and we didn't flag this as not supporting persistent
          connections, than obviously it is supported.*/
        jf_logger_logInfoMsg("webclient finish response, pipeline is yes");
        piwd->iwd_u8PipelineFlags = PIPELINE_YES;

        u32Ret = _sendWebclientRequestData(
            piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, piwd->iwd_pjnaConn, piwr);
    }

    return u32Ret;
}

static u32 _fnEventActionDisconnected(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;
    internal_webclient_request_t * piwr = NULL;

    jf_logger_logInfoMsg(
        "disconnected event action, PipelineFlags %d", piwd->iwd_u8PipelineFlags);

    piwd->iwd_pjnaConn = NULL;

    piwr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr != NULL)
    {
        /*If there are still pending requests, then obviously this server doesn't do persistent
          connections.*/
        jf_logger_logInfoMsg("web client disconnect, pipeline is no");
        piwd->iwd_u8PipelineFlags = PIPELINE_NO;

        /*There are still requests to be made, make another connection and continue.*/
        jf_logger_logInfoMsg("web client disconnect, retry later");
        jf_network_addUtimerItem(
            piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd, piwd->iwd_u32ExponentialBackoff,
            _webclientDataobjectRetryConnect, NULL);
    }

    return u32Ret;
}

/** Set a timer when entering idle state, close the connection in the timer handler. This is to
 *  restrict stay time in idle state.
 */
static u32 _onEntryWdsIdleState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    jf_logger_logDebugMsg("onentry wds_idle state");

    u32Ret = jf_network_addUtimerItem(
        piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd, WEBCLIENT_DATAOBJECT_IDLE_TIMEOUT,
        _webclientDataobjectIdleTimerHandler, NULL);

    return u32Ret;
}

static u32 _onExitWdsIdleState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    jf_logger_logDebugMsg("onexit wds_idle state");

    u32Ret = jf_network_removeUtimerItem(piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd);

    return u32Ret;
}

static u32 _onEntryWdsInitialState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    jf_logger_logDebugMsg("onentry wds_initial state");

    /*Set a timer to destroy webclient data object if object is in initial for certain time.*/
    jf_network_addUtimerItem(
        piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd, WEBCLIENT_DATAOBJECT_FREE_TIMEOUT,
        _webclientDataOjectFreeTimerHandler, NULL);

    return u32Ret;
}

static u32 _onExitWdsInitialState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    jf_logger_logDebugMsg("onexit wds_initial state");

    u32Ret = jf_network_removeUtimerItem(piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd);

    return u32Ret;
}

static u32 _createWebclientDataobject(
    internal_webclient_dataobject_t ** ppDataobject, internal_webclient_dataobject_pool_t * pPool,
    jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = NULL;
    jf_hsm_transition_t transionTable[] = {
        {WDS_INITIAL, WDE_SEND_DATA, NULL, _fnEventActionStartConn, WDS_CONNECTING},
        {WDS_CONNECTING, WDE_CONNECTED, NULL, _fnEventActionSendData, WDS_OPERATIVE},
        {WDS_OPERATIVE, WDE_DATA_SENT, _isEmptyDataobjectRequestQueue, NULL, WDS_IDLE},
        {WDS_OPERATIVE, WDE_DATA_SENT, _isDataobjectRequestInQueue, _fnEventActionDataSent, WDS_OPERATIVE},
        {WDS_OPERATIVE, WDE_DISCONNECTED, NULL, _fnEventActionDisconnected, WDS_INITIAL},
        {WDS_IDLE, WDE_DISCONNECTED, NULL, _fnEventActionDisconnected, WDS_INITIAL},
        {WDS_IDLE, WDE_SEND_DATA, _isDataobjectRequestInQueue, _fnEventActionSendData, WDS_OPERATIVE},
        {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},
    };

    jf_logger_logInfoMsg("create webclient data obj");

    u32Ret = jf_jiukun_allocMemory((void **)&piwd, sizeof(*piwd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwd, sizeof(*piwd));

        jf_queue_init(&piwd->iwd_jqRequest);
        ol_memcpy(&piwd->iwd_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        piwd->iwd_u16RemotePort = u16Port;
        piwd->iwd_piwdpPool = pPool;

        u32Ret = jf_httpparser_createtDataobject(&piwd->iwd_pjhdDataobject, pPool->iwdp_sBuffer);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_create(
            &piwd->iwd_pjhDataobject, transionTable, WDS_INITIAL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateCallback(
            piwd->iwd_pjhDataobject, WDS_IDLE, _onEntryWdsIdleState, _onExitWdsIdleState);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateCallback(
            piwd->iwd_pjhDataobject, WDS_INITIAL, _onEntryWdsInitialState, _onExitWdsInitialState);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDataobject = piwd;
    else if (piwd != NULL)
        _destroyWebclientDataobject((webclient_dataobject_t **)&piwd);

    return u32Ret;
}

static u32 _processWebclientRequestSendData(
    internal_webclient_dataobject_pool_t * pPool, internal_webclient_request_t * piwr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_webclient_dataobject_t * piwd = NULL;

    keyLength = getStringHashKey(key, &piwr->iwr_jiRemote, piwr->iwr_u16RemotePort);

    jf_logger_logInfoMsg("process webclient req send data, %s", key);

    u32Ret = jf_hashtree_getEntry(&pPool->iwdp_jhDataobject, key, keyLength, (void **)&piwd);
    if (u32Ret == JF_ERR_HASHTREE_ENTRY_NOT_FOUND)
    {
        /*There is no previous connection, so we need to set it up.*/
        jf_logger_logInfoMsg("process webclient req send data, create data object");

        u32Ret = _createWebclientDataobject(
            &piwd, pPool, &piwr->iwr_jiRemote, piwr->iwr_u16RemotePort);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_hashtree_addEntry(&pPool->iwdp_jhDataobject, key, keyLength, piwd);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Enqueue the request.*/
        u32Ret = jf_queue_enqueue(&piwd->iwd_jqRequest, piwr);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_hsm_event_t event;
        jf_hsm_initEvent(&event, WDE_SEND_DATA, piwd, piwr); 

        u32Ret = jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);
    }

    return u32Ret;
}


static u32 _processWebclientRequestDeleteRequest(
    internal_webclient_dataobject_pool_t * pPool, internal_webclient_request_t * pReq)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = NULL;
    internal_webclient_request_t * piwr = NULL;
    olchar_t key[64];
    olint_t keyLength;

    keyLength = getStringHashKey(key, &pReq->iwr_jiRemote, pReq->iwr_u16RemotePort);

    /*Are there any pending requests to this IP/Port combo.*/
    if (jf_hashtree_hasEntry(&pPool->iwdp_jhDataobject, key, keyLength))
    {
        /*Yes, iterate through them.*/
        jf_hashtree_getEntry(&pPool->iwdp_jhDataobject, key, keyLength, (void **)&piwd);
        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
        while (piwr != NULL)
        {
            piwr->iwr_fnOnEvent(
                NULL, JF_WEBCLIENT_EVENT_HTTP_REQ_DELETED, NULL, piwr->iwr_pUser);
            destroyWebclientRequest(&piwr);

            piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
        }
    }

    return u32Ret;
}

/** Internal method called when web client has finished processing a request or response.
 *
 *  @param piwd [in] The associated internal web data object.
 */
static u32 _finishWebclientDataobjectResponse(internal_webclient_dataobject_t * piwd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;
    jf_hsm_event_t event;

    jf_logger_logInfoMsg(
        "webclient finishd response, pipeline flag %d", piwd->iwd_u8PipelineFlags);

    jf_httpparser_reinitDataobject(piwd->iwd_pjhdDataobject);

    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    destroyWebclientRequest(&piwr);

    jf_hsm_initEvent(&event, WDE_DATA_SENT, piwd, NULL);
    u32Ret = jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying asocket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param pu8Buffer [in] The receive buffer.
 *  @param psBeginPointer [in] Start pointer in the buffer.
 *  @param sEndPointer [in] The end pointer of the buffer.
 *  @param pUser [in] The associated webclient data object.
 *
 *  @return The error code.
 */
static u32 _webclientDataobjectOnData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *) pUser;
    internal_webclient_request_t * piwr = NULL;
    jf_httpparser_packet_header_t * pjhphHeader;
    boolean_t bFullPacket = FALSE;

    jf_logger_logInfoMsg(
        "webclient dataobject data, %d:%d", *psBeginPointer, sEndPointer);
/*
    jf_logger_logDataMsgWithAscii(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer,
        "web client data");
*/
    piwr = (internal_webclient_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr == NULL)
    {
        jf_logger_logInfoMsg("web client data, no request, ignore");
        /*There are no pending requests, so we have no idea what we are supposed to do with this
          data, other than just recycling the receive buffer.
          If this code executes, this usually signifies a processing error of some sort. Most of
          the time, it means the remote server is sending invalid packets.*/
        *psBeginPointer = sEndPointer;
        return u32Ret;
    }

    u32Ret = jf_httpparser_processDataobject(
        piwd->iwd_pjhdDataobject, pu8Buffer, psBeginPointer, sEndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bFullPacket = jf_httpparser_getDataobjectFullPacket(piwd->iwd_pjhdDataobject, &pjhphHeader);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && bFullPacket)
    {
        u32Ret = piwr->iwr_fnOnEvent(
            piwd->iwd_pjnaConn, JF_WEBCLIENT_EVENT_INCOMING_DATA, pjhphHeader, piwr->iwr_pUser);

        _finishWebclientDataobjectResponse(piwd);
    }

    return u32Ret;
}

/** Internal method dispatched by the connect event of the underlying asocket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The connection status.
 *  @param pUser [in] The associated web data object.
 */
static u32 _webclientDataobjectOnConnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *) pUser;
    jf_hsm_event_t event;
    jf_hsm_state_id_t stateId = JF_HSM_LAST_STATE_ID;

    if (u32Status == JF_ERR_NO_ERROR)
    {
        jf_logger_logDebugMsg("webclient dataobject onconnect, connected");
//        piwd->iwd_u32ExponentialBackoff = 0;
        piwd->iwd_pjnaConn = pAsocket;
        jf_network_getLocalInterfaceOfAcsocket(pAcsocket, pAsocket, &piwd->iwd_jiLocal);

        jf_hsm_initEvent(&event, WDE_CONNECTED, piwd, NULL);
        jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);

        stateId = jf_hsm_getCurrentStateId(piwd->iwd_pjhDataobject);
        jf_logger_logDebugMsg(
            "webclient dataobject onconnect, state: %s",
            _getStringWebclientDataobjectState(stateId));
    }
    else
    {
        jf_logger_logInfoMsg("webclient dataobject onconnect, failed, retry later");
        /*The connection failed, so lets set a timed callback, and try again*/
        piwd->iwd_u8PipelineFlags = PIPELINE_UNKNOWN;
        jf_network_addUtimerItem(
            piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd, piwd->iwd_u32ExponentialBackoff,
            _webclientDataobjectRetryConnect, NULL);
    }

    return u32Ret;
}               

/** Internal method dispatched by the disconnect event of the underlying acsocket.
 *
 *  @param pAcsocket [in] The underlying async client socket.
 *  @param pAsocket [in] The underlying async socket.
 *  @param u32Status [in] The status of the disconnection.
 *  @param pUser [in] The associated web data object.
 *
 *  @return The error code.
 */
static u32 _webclientDataobjectOnDisconnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pUser;
    jf_hsm_event_t event;

    jf_logger_logInfoMsg(
        "webclient disconnect, PipelineFlags %d", piwd->iwd_u8PipelineFlags);

    jf_hsm_initEvent(&event, WDE_DISCONNECTED, piwd, NULL);
    u32Ret = jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);

    return u32Ret;
}

/** Internal method dispatched by the send ok event of the underlying asocket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status of data transmission.
 *  @param pu8Buffer [in] The receive buffer.
 *  @param sBuf [in] The size of the buffer.
 *  @param pUser [in] The associated webclient data object.
 *
 *  @return The error code.
 */
static u32 _webclientDataobjectOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("webclient dataobject send data, status: 0x%X", u32Status);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 processWebclientRequest(
    webclient_dataobject_pool_t * pPool, internal_webclient_request_t * piwr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_pool_t * piwdp = (internal_webclient_dataobject_pool_t *)pPool; 
    olchar_t key[64];

    getStringHashKey(key, &piwr->iwr_jiRemote, piwr->iwr_u16RemotePort);

    jf_logger_logInfoMsg("process webclient req, %s", key);

    if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_SEND_DATA)
    {
        u32Ret = _processWebclientRequestSendData(piwdp, piwr);
    }
    else if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST)
    {
        u32Ret = _processWebclientRequestDeleteRequest(piwdp, piwr);

        destroyWebclientRequest(&piwr);
    }
    else
    {
        /*Unkwnown operation code, destroy the webclient request.*/
        destroyWebclientRequest(&piwr);
    }

    return u32Ret;
}

u32 destroyWebclientDataobjectPool(webclient_dataobject_pool_t ** ppPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_pool_t * piwdp = NULL;

    jf_logger_logDebugMsg("destroy webclient dataobject pool");
    piwdp = (internal_webclient_dataobject_pool_t *) *ppPool;

    if (piwdp->iwdp_pjnaAcsocket != NULL)
        jf_network_destroyAcsocket(&piwdp->iwdp_pjnaAcsocket);

    /*Iterate through all the web data objects.*/
    jf_hashtree_finiHashtreeAndData(&piwdp->iwdp_jhDataobject, _destroyWebclientDataobject);

    if (piwdp->iwdp_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&piwdp->iwdp_pjnuUtimer);

    jf_jiukun_freeMemory(ppPool);

    return u32Ret;
}

u32 createWebclientDataobjectPool(
    jf_network_chain_t * pjnc, webclient_dataobject_pool_t ** ppPool,
    webclient_dataobject_pool_create_param_t * pwdpcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;
    internal_webclient_dataobject_pool_t * piwdp = NULL;

    jf_logger_logDebugMsg("create webclient dataobject pool");

    u32Ret = jf_jiukun_allocMemory((void **)&piwdp, sizeof(*piwdp));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwdp, sizeof(*piwdp));

        piwdp->iwdp_pjncChain = pjnc;
        piwdp->iwdp_sBuffer = pwdpcp->wdpcp_sBuffer;
        piwdp->iwdp_u32PoolSize = pwdpcp->wdpcp_u32PoolSize;
        jf_hashtree_init(&piwdp->iwdp_jhDataobject);

        u32Ret = jf_network_createUtimer(
            pjnc, &piwdp->iwdp_pjnuUtimer, WEBCLIENT_DATAOBJECT_NETWORK_OBJECT_NAME);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jnacp, sizeof(jnacp));
        jnacp.jnacp_sInitialBuf = piwdp->iwdp_sBuffer;
        jnacp.jnacp_u32MaxConn = piwdp->iwdp_u32PoolSize;
        jnacp.jnacp_fnOnData = _webclientDataobjectOnData;
        jnacp.jnacp_fnOnConnect = _webclientDataobjectOnConnect;
        jnacp.jnacp_fnOnDisconnect = _webclientDataobjectOnDisconnect;
        jnacp.jnacp_fnOnSendData = _webclientDataobjectOnSendData;
        jnacp.jnacp_pstrName = WEBCLIENT_DATAOBJECT_NETWORK_OBJECT_NAME;

        u32Ret = jf_network_createAcsocket(pjnc, &piwdp->iwdp_pjnaAcsocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPool = piwdp;
    else if (piwdp != NULL)
        destroyWebclientDataobjectPool((webclient_dataobject_pool_t **)&piwdp);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

