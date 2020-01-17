/**
 *  @file xferpool.c
 *
 *  @brief Implementation file for dispatcher xfer object pool.
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
#include "jf_jiukun.h"
#include "jf_hashtree.h"
#include "jf_queue.h"
#include "jf_hsm.h"

#include "xferpool.h"
#include "dispatcherxfer.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Number of dispatcher xfer object in pool.
 */
#define DISPATCHER_XFER_OBJECT_IN_POOL          (2)

/** This is the number of seconds for IDLE state of data object. When no request in data object,
 *  the timer is started. When the timer is triggered, the connection for the data object is closed
 *  and the data object state is set to FREE.
 */
#define DISPATCHER_XFER_OBJECT_IDLE_TIMEOUT     (300)

/** This is the number of seconds for INITIAL state of data object. When the data object enter
 *  INITIAL state, the timer is started. When the timer is triggered, the data object is destroyed
 */
#define DISPATCHER_XFER_OBJECT_FREE_TIMEOUT     (300)

/** This is the number of times, a connection will be attempted, before it fails. This module
 *  utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will
 *  retry after 1 second, then 2, then 4, etc.
 */
#define DISPATCHER_XFER_CONNECT_RETRY_COUNT     (5)

/** The name for acsocket and utimer
 */
#define DISPATCHER_XFER_OBJECT_NAME             "xfer-object"

/** State of xfer object.
 */
enum dispatcher_xfer_object_state_id
{
    DXOS_INITIAL = 0,
    DXOS_CONNECTING,
    DXOS_OPERATIVE,
    DXOS_IDLE,
};

/** Event ID of xfer object.
 */
enum dispatcher_xfer_object_event_id
{
    /**Connection to server is established.*/
    DXOE_CONNECTED,
    /**Send data.*/
    DXOE_SEND_DATA,
    /**Data is sent.*/
    DXOE_DATA_SENT,
    /**Connection is closed.*/
    DXOE_DISCONNECTED,
};

struct internal_dispatcher_xfer_object_pool;

/** Define the internal dispatcher xfer object data type.
 */
typedef struct internal_dispatcher_xfer_object
{
    struct internal_dispatcher_xfer_object_pool * idxo_pidxopPool;

    jf_hsm_t * idxo_pjhObject;

    u8 idxo_u8Reserved[8];

    u32 idxo_u32ExponentialBackoff;

    dispatcher_msg_t * idxo_pdmMsg;

    jf_network_asocket_t * idxo_pjnaConn;

    jf_ipaddr_t idxo_jiLocal;

} internal_dispatcher_xfer_object_t;

/** Define the internal dispatcher xfer object pool data type.
 */
typedef struct internal_dispatcher_xfer_object_pool
{
    jf_network_acsocket_t * idxop_pjnaAcsocket;
    u32 idxop_u32PoolSize;
    u32 idxop_u32Reserved;

    /**The dispatcher xfer object is put to object hash tree when the connection is established and
       there are xfer request.*/
    jf_hashtree_t idxop_jhObject;

    internal_dispatcher_xfer_object_t * idxop_pidxoMsg;

    jf_network_utimer_t * idxop_pjnuUtimer;

    olsize_t idxop_sBuffer;
    olsize_t idxop_sReserved;

    jf_network_chain_t * idxop_pjncChain;

    /**The address of remote server.*/
    jf_ipaddr_t idxop_jiRemote;
    /**The port of remote server.*/
    u16 idxop_u16RemotePort;
    u16 idx_u16Reserved[3];

    fnOnDispatcherXferObjectEvent_t idxop_fnOnEvent;
    void * idxop_pUser;

} internal_dispatcher_xfer_object_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

static olchar_t * _getStringDispatcherXferObjectState(jf_hsm_state_id_t stateId)
{
    olchar_t * str = "DXOS Unknown";
    switch (stateId)
    {
    case DXOS_INITIAL:
        str = "DXOS Initial";
        break;
    case DXOS_CONNECTING:
        str = "DXOS Connecting";
        break;
    case DXOS_OPERATIVE:
        str = "DXOS Operative";
        break;
    case DXOS_IDLE:
        str = "DXOS Idle";
        break;
    default:
        break;
    }

    return str;
};

/** Free resources associated with a dispatcher xfer object.
 *
 *  @note
 *  -# The connection should be disconnected before. This function is not responsible for closing
 *   the connection.
 *
 *  @param ppObject [in/out] The dispatcher xfer object to free.
 */
static u32 _destroyDispatcherXferObject(internal_dispatcher_xfer_object_t ** ppObject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = *ppObject;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;
    olchar_t str[128];

    jf_ipaddr_getStringIpAddrPort(str, &pidxop->idxop_jiRemote, pidxop->idxop_u16RemotePort);
    JF_LOGGER_DEBUG("destroy xfer obj");

    assert(pidxo->idxo_pjnaConn == NULL);

    /*Destroy utimer item.*/
    jf_network_removeUtimerItem(pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo);

    /*Destroy the hsm object.*/
    if (pidxo->idxo_pjhObject != NULL)
        jf_hsm_destroy(&pidxo->idxo_pjhObject);

    /*Set the message to NULL.*/
    pidxo->idxo_pdmMsg = NULL;

    jf_jiukun_freeMemory((void **)ppObject);

    return u32Ret;
}

/** Internal method dispatched by the utimer, to retry failed connections.
 *
 *  @note
 *  -# The module does an exponential backoff, when retrying connections. The number of retries
 *   is determined by the value of DISPATCHER_XFER_CONNECT_RETRY_COUNT.
 *
 *  @param object [in] The associated dispatcher xfer object.
 */
static u32 _dispatcherXferObjectRetryConnect(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)object;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;
    jf_hsm_event_t event;
    olchar_t str[128];

    /*Calculate the backoff time for the next retry.*/
    pidxo->idxo_u32ExponentialBackoff = (pidxo->idxo_u32ExponentialBackoff == 0) ?
        1 : pidxo->idxo_u32ExponentialBackoff * 2;

    jf_ipaddr_getStringIpAddrPort(str, &pidxop->idxop_jiRemote, pidxop->idxop_u16RemotePort);
    JF_LOGGER_INFO("server addr: %s, eb %u", str, pidxo->idxo_u32ExponentialBackoff);

    if (pidxo->idxo_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) DISPATCHER_XFER_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, rollback to 0 and retry.*/
        JF_LOGGER_INFO("rollback to 0");
        pidxo->idxo_u32ExponentialBackoff = 0;
    }

    /*Lets retry again.*/
    JF_LOGGER_INFO("try to connect");

    jf_hsm_initEvent(&event, DXOE_SEND_DATA, pidxo, NULL); 
    u32Ret = jf_hsm_processEvent(pidxo->idxo_pjhObject, &event);

    return u32Ret;
}

static u32 _dispatcherXferObjectFreeTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)object;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;

    JF_LOGGER_INFO("timer handler");

    if (pidxo->idxo_pdmMsg == NULL)
    {
        /*This connection is idle, because there are no pending message.*/
        JF_LOGGER_INFO("no pending message");

        _destroyDispatcherXferObject(&pidxop->idxop_pidxoMsg);
    }

    return u32Ret;
}

static u32 _sendDispatcherXferObjectMsg(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("send msg");

    u32Ret = jf_network_sendAcsocketStaticData(pAcsocket, pAsocket, pu8Msg, sMsg);

    return u32Ret;
}

static u32 _fnDxoEventActionStartConn(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo =
        (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;

    JF_LOGGER_DEBUG("action start conn");

    /*If the data object is created, connect to remote server. The async socket will wake
      up the chain so dispatcher xfer doesn't have to care about it.*/
    u32Ret = jf_network_connectAcsocketTo(
        pidxop->idxop_pjnaAcsocket, &pidxop->idxop_jiRemote, pidxop->idxop_u16RemotePort, pidxo);

    return u32Ret;
}

/** Check if there are pending message in the xfer.
 */
static boolean_t _isPendingDispatcherXferMsg(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;
    internal_dispatcher_xfer_object_t * pidxo =
        (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    if (pidxo->idxo_pdmMsg != NULL)
        bRet = TRUE;

    return bRet;
}

/** Check if there are no pending message in the xfer.
 */
static boolean_t _isNoPendingDispatcherXferMsg(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = TRUE;

    if (_isPendingDispatcherXferMsg(pEvent))
        bRet = FALSE;

    return bRet;
}

static u32 _fnDxoEventActionSendMsg(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = pEvent->jhe_pData;

    JF_LOGGER_DEBUG("action send msg");

    if (pidxo->idxo_pdmMsg != NULL)
    {
        u32Ret = _sendDispatcherXferObjectMsg(
            pidxo->idxo_pidxopPool->idxop_pjnaAcsocket, pidxo->idxo_pjnaConn,
            pidxo->idxo_pdmMsg->dm_u8Msg, pidxo->idxo_pdmMsg->dm_sMsg);
    }

    return u32Ret;
}

/** The timed callback is used to close idle sockets. A socket is considered idle if after a request
 *  is answered, another request isn't received within the time specified by
 *  DISPATCHER_XFER_OBJECT_IDLE_TIMEOUT.
 *
 *  @param object [in] The dispatcher xfer object.
 */
static u32 _dispatcherXferObjectIdleTimerHandler(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)object;

    JF_LOGGER_INFO("idle timer handler");

    if (pidxo->idxo_pdmMsg == NULL)
    {
        JF_LOGGER_INFO("queue is empty, close the connection");
        /*This connection is idle, because there are no pending requests */
        /*We need to close this socket.*/
        jf_network_disconnectAcsocket(
            pidxo->idxo_pidxopPool->idxop_pjnaAcsocket, pidxo->idxo_pjnaConn);
    }

    return u32Ret;
}

static u32 _fnDxoEventActionDataSent(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo =
        (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_INFO("action data sent");

    /*Send the next message.*/
    if (pidxo->idxo_pdmMsg != NULL)
    {
        u32Ret = _sendDispatcherXferObjectMsg(
            pidxo->idxo_pidxopPool->idxop_pjnaAcsocket, pidxo->idxo_pjnaConn,
            pidxo->idxo_pdmMsg->dm_u8Msg, pidxo->idxo_pdmMsg->dm_sMsg);
    }

    return u32Ret;
}

static u32 _fnDxoEventActionDisconnected(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo =
        (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("action disconnected");

    pidxo->idxo_pjnaConn = NULL;

    if (pidxo->idxo_pdmMsg != NULL)
    {
        /*There are still message to be sent, make another connection and continue.*/
        JF_LOGGER_DEBUG("retry later");
        jf_network_addUtimerItem(
            pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo, pidxo->idxo_u32ExponentialBackoff,
            _dispatcherXferObjectRetryConnect, NULL);
    }

    return u32Ret;
}

/** Set a timer when entering idle state, close the connection in the timer handler. This is to
 *  restrict stay time in idle state.
 */
static u32 _onEntryDxoIdleState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("on entry idle state");

    u32Ret = jf_network_addUtimerItem(
        pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo, DISPATCHER_XFER_OBJECT_IDLE_TIMEOUT,
        _dispatcherXferObjectIdleTimerHandler, NULL);

    return u32Ret;
}

static u32 _onExitDxoIdleState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("on exit idle state");

    u32Ret = jf_network_removeUtimerItem(pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo);

    return u32Ret;
}

static u32 _onEntryDxoInitialState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("on entry initial state");

    /*Set a timer to destroy dispatcher xfer data object if object is in initial for certain time.*/
    jf_network_addUtimerItem(
        pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo, DISPATCHER_XFER_OBJECT_FREE_TIMEOUT,
        _dispatcherXferObjectFreeTimerHandler, NULL);

    return u32Ret;
}

static u32 _onExitDxoInitialState(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)pEvent->jhe_pData;

    JF_LOGGER_DEBUG("on exit initial state");

    u32Ret = jf_network_removeUtimerItem(pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo);

    return u32Ret;
}

static u32 _createDispatcherXferObject(
    internal_dispatcher_xfer_object_t ** ppObject, internal_dispatcher_xfer_object_pool_t * pPool,
    jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = NULL;
    jf_hsm_transition_t transionTable[] = {
        {DXOS_INITIAL, DXOE_SEND_DATA, NULL, _fnDxoEventActionStartConn, DXOS_CONNECTING},
        {DXOS_CONNECTING, DXOE_CONNECTED, NULL, _fnDxoEventActionSendMsg, DXOS_OPERATIVE},
        {DXOS_OPERATIVE, DXOE_DATA_SENT, _isNoPendingDispatcherXferMsg, NULL, DXOS_IDLE},
        {DXOS_OPERATIVE, DXOE_DATA_SENT, _isPendingDispatcherXferMsg, _fnDxoEventActionDataSent, DXOS_OPERATIVE},
        {DXOS_OPERATIVE, DXOE_DISCONNECTED, NULL, _fnDxoEventActionDisconnected, DXOS_INITIAL},
        {DXOS_IDLE, DXOE_DISCONNECTED, NULL, _fnDxoEventActionDisconnected, DXOS_INITIAL},
        {DXOS_IDLE, DXOE_SEND_DATA, _isPendingDispatcherXferMsg, _fnDxoEventActionSendMsg, DXOS_OPERATIVE},
        {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},
    };

    JF_LOGGER_INFO("create xfer obj");

    u32Ret = jf_jiukun_allocMemory((void **)&pidxo, sizeof(*pidxo));

    /*Create the state machine.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pidxo, sizeof(*pidxo));
        pidxo->idxo_pidxopPool = pPool;

        u32Ret = jf_hsm_create(&pidxo->idxo_pjhObject, transionTable, DXOS_INITIAL);
    }

    /*Add callback function for entering and exiting IDLE state.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateCallback(
            pidxo->idxo_pjhObject, DXOS_IDLE, _onEntryDxoIdleState, _onExitDxoIdleState);
    }

    /*Add callback function for entering and exiting INITIAL state.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateCallback(
            pidxo->idxo_pjhObject, DXOS_INITIAL, _onEntryDxoInitialState, _onExitDxoInitialState);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppObject = pidxo;
    else if (pidxo != NULL)
        _destroyDispatcherXferObject(&pidxo);

    return u32Ret;
}

static u32 _processDispatcherXferSendMsg(
    internal_dispatcher_xfer_object_pool_t * pidxop, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[128];

    jf_ipaddr_getStringIpAddrPort(str, &pidxop->idxop_jiRemote, pidxop->idxop_u16RemotePort);
    JF_LOGGER_INFO("server addr: %s", str);

    if (pidxop->idxop_pidxoMsg == NULL)
    {
        /*There is no previous connection, so we need to set it up*/
        JF_LOGGER_INFO("create object");

        u32Ret = _createDispatcherXferObject(
            &pidxop->idxop_pidxoMsg, pidxop, &pidxop->idxop_jiRemote, pidxop->idxop_u16RemotePort);
    }
    else if (pidxop->idxop_pidxoMsg->idxo_pdmMsg != NULL)
    {
        /*Previous message is not sent, return an error.*/
        u32Ret = JF_ERR_PREVIOUS_DISPATCHER_MSG_NOT_SENT;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pidxop->idxop_pidxoMsg->idxo_pdmMsg = pdm;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_hsm_event_t event;
        jf_hsm_initEvent(&event, DXOE_SEND_DATA, pidxop->idxop_pidxoMsg, NULL); 

        u32Ret = jf_hsm_processEvent(pidxop->idxop_pidxoMsg->idxo_pjhObject, &event);
    }

    return u32Ret;
}

/** Internal method called when xfer object has finished sending a message.
 *
 *  @param pidxo [in] The associated internal dispatcher xfer object.
 */
static u32 _finishSendingDispatcherXferObjectMsg(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hsm_event_t event;

    JF_LOGGER_INFO("finish msg");

    /*Post a data sent event to the state machine.*/
    jf_hsm_initEvent(&event, DXOE_DATA_SENT, pidxo, NULL);
    u32Ret = jf_hsm_processEvent(pidxo->idxo_pjhObject, &event);

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying asocket.
 *
 *  @param pAcsocket [in] the Async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param pu8Buffer [in] The received buffer.
 *  @param psBeginPointer [in] Start pointer in the buffer.
 *  @param sEndPointer [in] The end pointer of the buffer.
 *  @param pUser [in] The associated dispatcher xfer data object.
 *
 *  @return The error code.
 */
static u32 _dispatcherXferObjectOnData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("unexpected data");

    *psBeginPointer = sEndPointer;

    return u32Ret;
}

/** Internal method dispatched by the connect event of the underlying asocket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The connection status.
 *  @param pUser [in] The associated dispatcher xfer object.
 */
static u32 _dispatcherXferObjectOnConnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *) pUser;
    jf_hsm_event_t event;
    jf_hsm_state_id_t stateId = JF_HSM_LAST_STATE_ID;

    if (u32Status == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("connected");
        /*Do not set the backoff time to 0. The test shows when we connect to the no listening uds,
          the connection is make successfully, and then fails to send data and we know the
          connection is broken. If the backoff time is clear to 0, it will loop without any pause.
        */
//        pidxo->idxo_u32ExponentialBackoff = 0;
        pidxo->idxo_pjnaConn = pAsocket;
        jf_network_getLocalInterfaceOfAcsocket(pAcsocket, pAsocket, &pidxo->idxo_jiLocal);

        jf_hsm_initEvent(&event, DXOE_CONNECTED, pidxo, NULL);
        jf_hsm_processEvent(pidxo->idxo_pjhObject, &event);

        stateId = jf_hsm_getCurrentStateId(pidxo->idxo_pjhObject);
        JF_LOGGER_DEBUG("hsm state: %s", _getStringDispatcherXferObjectState(stateId));
    }
    else
    {
        JF_LOGGER_INFO("failed, retry later");
        /*The connection failed, so let's set a timed callback, and try again.*/
        jf_network_addUtimerItem(
            pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo, pidxo->idxo_u32ExponentialBackoff,
            _dispatcherXferObjectRetryConnect, NULL);
    }

    return u32Ret;
}               

/** Internal method dispatched by the disconnect event of the underlying acsocket.
 *
 *  @param pAcsocket [in] The underlying async client socket.
 *  @param pAsocket [in] The underlying async socket.
 *  @param u32Status [in] The status of the disconnection.
 *  @param pUser [in] The associated xfer data object.
 *
 *  @return The error code.
 */
static u32 _dispatcherXferObjectOnDisconnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *)pUser;
    jf_hsm_event_t event;

    JF_LOGGER_INFO("disconnect");

    jf_hsm_initEvent(&event, DXOE_DISCONNECTED, pidxo, NULL);
    u32Ret = jf_hsm_processEvent(pidxo->idxo_pjhObject, &event);

    return u32Ret;
}

/** Internal method dispatched by the send ok event of the underlying asocket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status of data transmission.
 *  @param pu8Buffer [in] The receive buffer.
 *  @param sBuf [in] The size of the buffer.
 *  @param pUser [in] The associated dispatcher xfer data object.
 *
 *  @return The error code.
 */
static u32 _dispatcherXferObjectOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = (internal_dispatcher_xfer_object_t *) pUser;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;
    dispatcher_msg_t * pdm = NULL;

    JF_LOGGER_INFO("status: 0x%X", u32Status);

    if (u32Status == JF_ERR_NO_ERROR)
    {
        /*Data is sent successfully, notify the application.*/
        if (pidxo->idxo_pdmMsg != NULL)
        {
            /*Remove the message first, as new message may be coming in the next callback function.
             */
            pdm = pidxo->idxo_pdmMsg;
            pidxo->idxo_pdmMsg = NULL;

            pidxop->idxop_fnOnEvent(
                DISPATCHER_XFER_OBJECT_EVENT_MSG_SENT, (u8 *)pdm, NULL, 0, pidxop->idxop_pUser);

            /*No response is expected, finish the message sending.*/
            _finishSendingDispatcherXferObjectMsg(pidxo);
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyDispatcherXferObjectPool(dispatcher_xfer_object_pool_t ** ppPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = NULL;

    JF_LOGGER_DEBUG("destroy xfer object pool");
    pidxop = (internal_dispatcher_xfer_object_pool_t *) *ppPool;

    /*Destroy the xfer object.*/
    if (pidxop->idxop_pidxoMsg != NULL)
    {
        /*Set the connection to NULL.*/
        pidxop->idxop_pidxoMsg->idxo_pjnaConn = NULL;

        _destroyDispatcherXferObject(&pidxop->idxop_pidxoMsg);
    }

    /*Destroy the async client socket.*/
    if (pidxop->idxop_pjnaAcsocket != NULL)
        jf_network_destroyAcsocket(&pidxop->idxop_pjnaAcsocket);

    /*Destroy the timer.*/
    if (pidxop->idxop_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&pidxop->idxop_pjnuUtimer);

    jf_jiukun_freeMemory(ppPool);

    return u32Ret;
}

u32 createDispatcherXferObjectPool(
    jf_network_chain_t * pjnc, dispatcher_xfer_object_pool_t ** ppPool,
    dispatcher_xfer_pool_create_param_t * pdxpcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;
    internal_dispatcher_xfer_object_pool_t * pidxop = NULL;
    olchar_t strName[128];

    JF_LOGGER_DEBUG("create xfer object pool");

    u32Ret = jf_jiukun_allocMemory((void **)&pidxop, sizeof(*pidxop));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pidxop, sizeof(*pidxop));

        pidxop->idxop_pjncChain = pjnc;
        pidxop->idxop_sBuffer = pdxpcp->dxpcp_sBuffer;
        pidxop->idxop_u32PoolSize = DISPATCHER_XFER_OBJECT_IN_POOL;
        ol_memcpy(&pidxop->idxop_jiRemote, pdxpcp->dxpcp_pjiRemote, sizeof(pidxop->idxop_jiRemote));
        pidxop->idxop_u16RemotePort = pdxpcp->dxpcp_u16RemotePort;
        pidxop->idxop_fnOnEvent = pdxpcp->dxpcp_fnOnEvent;
        pidxop->idxop_pUser = pdxpcp->dxpcp_pUser;
        sprintf(strName, "%s-xfer-pool", pdxpcp->dxpcp_pstrName);

        u32Ret = jf_network_createUtimer(pjnc, &pidxop->idxop_pjnuUtimer, strName);
    }

    /*Create the async client asocket*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jnacp, sizeof(jnacp));
        jnacp.jnacp_sInitialBuf = pidxop->idxop_sBuffer;
        jnacp.jnacp_u32MaxConn = pidxop->idxop_u32PoolSize;
        jnacp.jnacp_fnOnData = _dispatcherXferObjectOnData;
        jnacp.jnacp_fnOnConnect = _dispatcherXferObjectOnConnect;
        jnacp.jnacp_fnOnDisconnect = _dispatcherXferObjectOnDisconnect;
        jnacp.jnacp_fnOnSendData = _dispatcherXferObjectOnSendData;
        sprintf(strName, "%s-%s", pdxpcp->dxpcp_pstrName, DISPATCHER_XFER_OBJECT_NAME);
        jnacp.jnacp_pstrName = strName;

        u32Ret = jf_network_createAcsocket(pjnc, &pidxop->idxop_pjnaAcsocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPool = pidxop;
    else if (pidxop != NULL)
        destroyDispatcherXferObjectPool((dispatcher_xfer_object_pool_t **)&pidxop);

    return u32Ret;
}

u32 sendDispatcherXferPoolMsg(dispatcher_xfer_object_pool_t * pPool, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop =
        (internal_dispatcher_xfer_object_pool_t *)pPool;

    JF_LOGGER_DEBUG("send msg");

    u32Ret = _processDispatcherXferSendMsg(pidxop, pdm);

    return u32Ret;
}

/** TODO: improve this later.
 */
u32 deleteDispatcherXferPoolMsg(dispatcher_xfer_object_pool_t * pPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop =
        (internal_dispatcher_xfer_object_pool_t *)pPool;

    JF_LOGGER_INFO("close the connection");

    pidxop->idxop_pidxoMsg->idxo_pdmMsg = NULL;

    u32Ret = jf_network_addUtimerItem(
        pidxop->idxop_pjnuUtimer, pidxop->idxop_pidxoMsg, 0,
        _dispatcherXferObjectIdleTimerHandler, NULL);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

