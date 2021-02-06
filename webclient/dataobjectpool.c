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

#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_webclient.h"
#include "jf_jiukun.h"
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

/** The name for async client socket and utimer.
 */
#define WEBCLIENT_DATAOBJECT_NETWORK_OBJECT_NAME    "webclient-dataobject"

/** Define the pipeline type.
 */
enum pipeline_type
{
    /**Doesn't know yet if the server supports persistent connections.*/
    PIPELINE_UNKNOWN = 0,
    /**The server does indeed support persistent connections.*/
    PIPELINE_YES,
    /**The server does not support persistent connections.*/
    PIPELINE_NO,
};

/** Define the state ID for data object state machine.
 */
enum webclient_dataobject_state_id
{
    WDS_INITIAL = 0,
    WDS_CONNECTING,
    WDS_OPERATIVE,
    WDS_IDLE,
};

/** Define the event ID for data object state machine.
 */
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

/** Pre-declare the internal webclient data object pool.
 */
struct internal_webclient_dataobject_pool;

/** Define the internal webclient data object.
 */
typedef struct internal_webclient_dataobject
{
    /*Ip address of remote server.*/
    jf_ipaddr_t iwd_jiRemote;
    /*Port of remote server.*/
    u16 iwd_u16RemotePort;
    u16 iwd_u16Reserved[3];

    /*Webclient data object pool.*/
    struct internal_webclient_dataobject_pool * iwd_piwdpPool;

    /*State machine of data object.*/
    jf_hsm_t * iwd_pjhDataobject;

    /*Pipeline flags.*/
    u8 iwd_u8PipelineFlags;
    u8 iwd_u8Reserved[7];

    /*Exponential backoff for retry of making connection.*/
    u32 iwd_u32ExponentialBackoff;

    /*Http parser data object.*/
    jf_httpparser_dataobject_t * iwd_pjhdDataobject;

    /*Webclient request queue.*/
    jf_queue_t iwd_jqRequest;

    /*Established connection.*/
    jf_network_asocket_t * iwd_pjnaConn;

    /*Local ip address of the connection.*/
    jf_ipaddr_t iwd_jiLocal;

} internal_webclient_dataobject_t;

/** Define the internal webclient data object pool.
 */
typedef struct internal_webclient_dataobject_pool
{
    /*The async client socket.*/
    jf_network_acsocket_t * iwdp_pjnaAcsocket;
    /*Number of connection in async client socket pool.*/
    u32 iwdp_u32PoolSize;
    u32 iwdp_u32Reserved;

    /**The web data object is put to data hash tree when the connection is established and there
       are web request.*/
    jf_hashtree_t iwdp_jhDataobject;

    /*Network utimer.*/
    jf_network_utimer_t * iwdp_pjnuUtimer;

    /*Size of the buffer.*/
    olsize_t iwdp_sBuffer;
    olsize_t iwdp_sReserved;

    /*Network chain.*/
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
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _destroyWebclientDataobject(webclient_dataobject_t ** ppDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *) *ppDataobject;
    internal_webclient_request_t * piwr = NULL;
    olchar_t key[64];

    getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    JF_LOGGER_DEBUG("key: %s", key);

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
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _webclientDataobjectRetryConnect(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength = 0;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    JF_LOGGER_DEBUG("key: %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) WEBCLIENT_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        JF_LOGGER_DEBUG("give up");

        /*Delete data object from hash tree.*/
        jf_hashtree_deleteEntry(&piwd->iwd_piwdpPool->iwdp_jhDataobject, key, keyLength);
        /*Destroy the data object.*/
        _destroyWebclientDataobject((webclient_dataobject_t **)&piwd);
    }
    else
    {
        /*Lets retry again*/
        jf_hsm_event_t event;
        JF_LOGGER_DEBUG("try to connect");

        /*Trigger a send data event.*/
        jf_hsm_initEvent(&event, WDE_SEND_DATA, piwd, 0);
        u32Ret = jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);
    }

    return u32Ret;
}

static u32 _webclientDataOjectFreeTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;
    olchar_t key[64];
    olint_t keyLength = 0;

    keyLength = getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);

    JF_LOGGER_DEBUG("key: %s", key);

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        /*This connection is idle, because there are no pending requests */
        JF_LOGGER_DEBUG("queue is empty");

        /*Delete data object from hash tree.*/
        jf_hashtree_deleteEntry(&piwd->iwd_piwdpPool->iwdp_jhDataobject, key, keyLength);

        /*Destroy data object.*/
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

    /*Peek a webclient request.*/
    piwr = (internal_webclient_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);

    /*Send the data in request.*/
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
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _webclientDataobjectIdleTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)object;

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        JF_LOGGER_DEBUG("queue is empty, close the connection");
        /*This connection is idle, because there are no pending requests. Disconnect this socket.*/
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
        JF_LOGGER_DEBUG("pipeline is no, disconnect the data object");
        jf_network_disconnectAcsocket(
            piwd->iwd_piwdpPool->iwdp_pjnaAcsocket, piwd->iwd_pjnaConn);
    }
    else
    {
        /*If the connection is still open, and we didn't flag this as not supporting persistent
          connections, than obviously it is supported.*/
        JF_LOGGER_DEBUG("pipeline is yes");
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

    piwd->iwd_pjnaConn = NULL;

    /*Peek a webclient request.*/
    piwr = jf_queue_peek(&piwd->iwd_jqRequest);

    if (piwr != NULL)
    {
        /*If there are still pending requests, then obviously this server doesn't do persistent
          connections.*/
        JF_LOGGER_DEBUG("pipeline is no, retry later");
        piwd->iwd_u8PipelineFlags = PIPELINE_NO;

        /*There are still requests to be made, make another connection and continue.*/
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

    JF_LOGGER_DEBUG("idle state");

    u32Ret = jf_network_addUtimerItem(
        piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd, WEBCLIENT_DATAOBJECT_IDLE_TIMEOUT,
        _webclientDataobjectIdleTimerHandler, NULL);

    return u32Ret;
}

static u32 _onExitWdsIdleState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("exit state");

    u32Ret = jf_network_removeUtimerItem(piwd->iwd_piwdpPool->iwdp_pjnuUtimer, piwd);

    return u32Ret;
}

static u32 _onEntryWdsInitialState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("initial state");

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

    JF_LOGGER_DEBUG("initial state");

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

    /*Allocate memory for internal webclient data object.*/
    u32Ret = jf_jiukun_allocMemory((void **)&piwd, sizeof(*piwd));

    /*Initialize the internal webclient data object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwd, sizeof(*piwd));

        jf_queue_init(&piwd->iwd_jqRequest);
        ol_memcpy(&piwd->iwd_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        piwd->iwd_u16RemotePort = u16Port;
        piwd->iwd_piwdpPool = pPool;

        /*Create http parser data object.*/
        u32Ret = jf_httpparser_createtDataobject(&piwd->iwd_pjhdDataobject, pPool->iwdp_sBuffer);
    }

    /*Create the state machine of data object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_create(&piwd->iwd_pjhDataobject, transionTable, WDS_INITIAL);
    }

    /*Add state callback for idle state.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateCallback(
            piwd->iwd_pjhDataobject, WDS_IDLE, _onEntryWdsIdleState, _onExitWdsIdleState);
    }

    /*Add state callback for initial state.*/
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
    olint_t keyLength = 0;
    internal_webclient_dataobject_t * piwd = NULL;

    keyLength = getStringHashKey(key, &piwr->iwr_jiRemote, piwr->iwr_u16RemotePort);

    JF_LOGGER_DEBUG("key: %s", key);

    /*Get data object.*/
    u32Ret = jf_hashtree_getEntry(&pPool->iwdp_jhDataobject, key, keyLength, (void **)&piwd);

    if (u32Ret == JF_ERR_HASHTREE_ENTRY_NOT_FOUND)
    {
        /*There is no previous connection, so we need to set it up.*/
        JF_LOGGER_DEBUG("create data object");

        /*Create data object.*/
        u32Ret = _createWebclientDataobject(
            &piwd, pPool, &piwr->iwr_jiRemote, piwr->iwr_u16RemotePort);

        /*Add the data object to hash tree.*/
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
        jf_hsm_initEvent(&event, WDE_SEND_DATA, piwd, 0);

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
    olint_t keyLength = 0;

    keyLength = getStringHashKey(key, &pReq->iwr_jiRemote, pReq->iwr_u16RemotePort);

    /*Are there any pending requests to this IP/Port combo.*/
    if (jf_hashtree_hasEntry(&pPool->iwdp_jhDataobject, key, keyLength))
    {
        /*Get data object.*/
        jf_hashtree_getEntry(&pPool->iwdp_jhDataobject, key, keyLength, (void **)&piwd);

        /*Iterate through the request queue.*/
        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
        while (piwr != NULL)
        {
            /*Notify the upper layer that request is deleted.*/
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
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _finishWebclientDataobjectResponse(internal_webclient_dataobject_t * piwd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;
    jf_hsm_event_t event;

    /*Re-initialize the data object.*/
    jf_httpparser_reinitDataobject(piwd->iwd_pjhdDataobject);

    /*Dequeue the request and destroy it.*/
    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    destroyWebclientRequest(&piwr);

    /*Trigger a data sent event.*/
    jf_hsm_initEvent(&event, WDE_DATA_SENT, piwd, 0);
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
 *  @retval JF_ERR_NO_ERROR Success.
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

    JF_LOGGER_DEBUG("data: %d:%d", *psBeginPointer, sEndPointer);
/*
    JF_LOGGER_DATAA(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer, "data");
*/
    piwr = (internal_webclient_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr == NULL)
    {
        JF_LOGGER_DEBUG("no request, ignore");
        /*There are no pending requests, so we have no idea what we are supposed to do with this
          data, other than just recycling the receive buffer.
          If this code executes, this usually signifies a processing error of some sort. Most of
          the time, it means the remote server is sending invalid packets.*/
        *psBeginPointer = sEndPointer;
        return u32Ret;
    }

    /*Process the receiving data.*/
    u32Ret = jf_httpparser_processDataobject(
        piwd->iwd_pjhdDataobject, pu8Buffer, psBeginPointer, sEndPointer);

    /*Test if full packet is received.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bFullPacket = jf_httpparser_getDataobjectFullPacket(piwd->iwd_pjhdDataobject, &pjhphHeader);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && bFullPacket)
    {
        /*Notify upper layer the incoming data.*/
        u32Ret = piwr->iwr_fnOnEvent(
            piwd->iwd_pjnaConn, JF_WEBCLIENT_EVENT_INCOMING_DATA, pjhphHeader, piwr->iwr_pUser);

        /*Do clean stuff.*/
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
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
        /*Connected.*/
        JF_LOGGER_DEBUG("connected");
        /*Do not clear backoff here. Test shows it will loop without any wait in some cases.
          No error status doesn't mean the connection is right, it will fail when sending data.*/
//        piwd->iwd_u32ExponentialBackoff = 0;
        piwd->iwd_pjnaConn = pAsocket;
        jf_network_getLocalInterfaceOfAcsocket(pAcsocket, pAsocket, &piwd->iwd_jiLocal);

        /*Trigger a connected event.*/
        jf_hsm_initEvent(&event, WDE_CONNECTED, piwd, 0);
        jf_hsm_processEvent(piwd->iwd_pjhDataobject, &event);

        stateId = jf_hsm_getCurrentStateId(piwd->iwd_pjhDataobject);
        JF_LOGGER_DEBUG("state: %s", _getStringWebclientDataobjectState(stateId));
    }
    else
    {
        /*Connection failed.*/
        JF_LOGGER_DEBUG("failed, retry later");

        /*Set a timed callback, and try again.*/
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _webclientDataobjectOnDisconnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_dataobject_t * piwd = (internal_webclient_dataobject_t *)pUser;
    jf_hsm_event_t event;

    JF_LOGGER_DEBUG("PipelineFlags: %u", piwd->iwd_u8PipelineFlags);

    /*Trigger a disconnect event.*/
    jf_hsm_initEvent(&event, WDE_DISCONNECTED, piwd, 0);
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _webclientDataobjectOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("status: 0x%X", u32Status);

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

    JF_LOGGER_DEBUG("key: %s", key);

    if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_SEND_DATA)
    {
        /*Send data request.*/
        u32Ret = _processWebclientRequestSendData(piwdp, piwr);
    }
    else if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST)
    {
        /*Delete request.*/
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
    internal_webclient_dataobject_pool_t * piwdp = *ppPool;

    JF_LOGGER_INFO("destroy");

    /*Destroy async client socket.*/
    if (piwdp->iwdp_pjnaAcsocket != NULL)
        jf_network_destroyAcsocket(&piwdp->iwdp_pjnaAcsocket);

    /*Finalize all data objects and the hash tree.*/
    jf_hashtree_finiHashtreeAndData(&piwdp->iwdp_jhDataobject, _destroyWebclientDataobject);

    /*Destroy the utimer.*/
    if (piwdp->iwdp_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&piwdp->iwdp_pjnuUtimer);

    /*Free memory for the data object pool.*/
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

    JF_LOGGER_INFO("pool size: %u, sBuffer: %d", pwdpcp->wdpcp_u32PoolSize, pwdpcp->wdpcp_sBuffer);

    /*Allocate memory for data object pool.*/
    u32Ret = jf_jiukun_allocMemory((void **)&piwdp, sizeof(*piwdp));

    /*Initialize the data object pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwdp, sizeof(*piwdp));

        piwdp->iwdp_pjncChain = pjnc;
        piwdp->iwdp_sBuffer = pwdpcp->wdpcp_sBuffer;
        piwdp->iwdp_u32PoolSize = pwdpcp->wdpcp_u32PoolSize;
        jf_hashtree_init(&piwdp->iwdp_jhDataobject);

        /*Create utimer.*/
        u32Ret = jf_network_createUtimer(
            pjnc, &piwdp->iwdp_pjnuUtimer, WEBCLIENT_DATAOBJECT_NETWORK_OBJECT_NAME);
    }

    /*Create async client socket.*/
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
