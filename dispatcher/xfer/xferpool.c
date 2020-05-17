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
#include "prioqueue.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** This is the number of times, a connection will be attempted, before it fails. This module
 *  utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will
 *  retry after 1 second, then 2, then 4, etc.
 */
#define DISPATCHER_XFER_CONNECT_RETRY_COUNT             (5)

/** The name for acsocket and utimer
 */
#define DISPATCHER_XFER_OBJECT_NAME                     "xfer-object"

/** State of xfer object.
 */
enum dispatcher_xfer_object_pool_state_id
{
    DXOPS_INITIAL = 0,
    DXOPS_PAUSED,
    DXOPS_OPERATIVE,
};

/** Event ID of xfer object.
 */
enum dispatcher_xfer_object_pool_event_id
{
    /**Start the pool.*/
    DXOPE_START = 0,
    /**Stop the pool.*/
    DXOPE_STOP,
    /**Pause the pool.*/
    DXOPE_PAUSE,
    /**Resume the pool.*/
    DXOPE_RESUME,
    /**Send message.*/
    DXOPE_SEND_MSG,

};

struct internal_dispatcher_xfer_object_pool;

/** Define the internal dispatcher xfer object data type.
 */
typedef struct internal_dispatcher_xfer_object
{
    /**The network chain object header. MUST BE the first field.*/
    jf_network_chain_object_header_t idxo_jncohHeader;

    /**Name of this object.*/
    olchar_t idxo_strName[32];

    /**The pool who own this xfer object.*/
    struct internal_dispatcher_xfer_object_pool * idxo_pidxopPool;

    /**Index in the object array of object pool.*/
    u8 idxo_u8Index;
    /**Connected to server if TRUE.*/
    boolean_t idxo_bFinConnect;
    /**Object is in use if TRUE.*/
    boolean_t idxo_bInUse;
    u8 idxo_u8Reserved[5];

    /**The address of remote server.*/
    jf_ipaddr_t idxo_jiRemote;
    /**The port of remote server.*/
    u16 idxo_u16RemotePort;
    u16 idxo_u16Reserved[3];

    /**Number of seconds for retry.*/
    u32 idxo_u32ExponentialBackoff;

    /**The connection.*/
    jf_network_socket_t * idxo_pjnsSocket;

    /**Local address of the connection.*/
    jf_ipaddr_t idxo_jiLocal;
    /**The port of local.*/
    u16 idxo_u16LocalPort;
    u16 idxo_u16Reserved2[3];

} internal_dispatcher_xfer_object_t;

/** Define the internal dispatcher xfer object pool data type.
 */
typedef struct internal_dispatcher_xfer_object_pool
{
    /**The network chain object header. MUST BE the first field.*/
    jf_network_chain_object_header_t idxop_jncohHeader;

    /**The async client socket.*/
    jf_network_acsocket_t * idxop_pjnaAcsocket;

    /**Name of the object pool.*/
    olchar_t idxop_strName[32];

    /**Maximum message size.*/
    olsize_t idxop_sMaxMsg;
    /**Number of object in pool.*/
    u32 idxop_u32NumOfObject;
    /**The xfer object array, each entry represent a connection to server.*/
    internal_dispatcher_xfer_object_t * idxop_pidxoObjects[DISPATCHER_XFER_MAX_NUM_OF_ADDRESS];

    /**The utimer object.*/
    jf_network_utimer_t * idxop_pjnuUtimer;

    /**The basic chain.*/
    jf_network_chain_t * idxop_pjncChain;

    /**Dispatcher priority queue.*/
    dispatcher_prio_queue_t * idxop_pdpqMsg;

    /**Message is being sent.*/
    dispatcher_msg_t * idxop_pdmMsg;
    /**Bytes have been sent already.*/
    olsize_t idxop_sBytesSent;

    /**The state machine of the xfer pool.*/
    jf_hsm_t * idxop_pjhPool;

    boolean_t idxop_bPause;
    u8 idxop_u8Reserved4[7];

} internal_dispatcher_xfer_object_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

static olchar_t * _getStringDispatcherXferObjectPoolState(jf_hsm_state_id_t stateId)
{
    olchar_t * str = "DXOS Unknown";
    switch (stateId)
    {
    case DXOPS_INITIAL:
        str = "DXOPS Initial";
        break;
    case DXOPS_PAUSED:
        str = "DXOPS Paused";
        break;
    case DXOPS_OPERATIVE:
        str = "DXOPS Operative";
        break;
    default:
        break;
    }

    return str;
};

static u32 _enqueueDispatcherXferMsgToQueue(
    internal_dispatcher_xfer_object_pool_t * pidxop, dispatcher_msg_t * pdm, boolean_t * pbWakeup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*If the request queue is empty, chain should be waken up.*/
    *pbWakeup = isEmptyDispatcherPrioQueue(pidxop->idxop_pdpqMsg);
    u32Ret = enqueueDispatcherPrioQueue(pidxop->idxop_pdpqMsg, pdm);

    return u32Ret;
}

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
//    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;

    JF_LOGGER_DEBUG("name: %s", pidxo->idxo_strName);

    /*Destroy utimer item.*/
    jf_network_removeUtimerItem(pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo);

    /*Destroy the socket to close the connection.*/
    if (pidxo->idxo_pjnsSocket != NULL)
        jf_network_destroySocket(&pidxo->idxo_pjnsSocket);

    jf_jiukun_freeMemory((void **)ppObject);

    return u32Ret;
}

static u32 _destroyDispatcherXferObjects(internal_dispatcher_xfer_object_pool_t * pidxop)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;

    for (u32Index = 0;
         (u32Index < pidxop->idxop_u32NumOfObject) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index ++)
    {
        u32Ret = _destroyDispatcherXferObject(&pidxop->idxop_pidxoObjects[u32Index]);
    }

    return u32Ret;
}

static u32 _startConnInDispatcherXferObject(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("index: %u", pidxo->idxo_u8Index);

    /*If there isn't a socket already allocated, we need to allocate one.*/
    if (pidxo->idxo_pjnsSocket == NULL)
    {
        u32Ret = jf_network_createTypeStreamSocket(
            pidxo->idxo_jiRemote.ji_u8AddrType, &pidxo->idxo_pjnsSocket);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_network_setSocketNonblock(pidxo->idxo_pjnsSocket);
        }
    }

    /*Connect to the server, no need to wakeup the chain*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_connect(
            pidxo->idxo_pjnsSocket, &pidxo->idxo_jiRemote, pidxo->idxo_u16RemotePort);
    }

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
static u32 _fnUtimerRetryConnInDispatcherXferObject(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = object;
    olchar_t str[128];

    /*Calculate the backoff time for the next retry.*/
    pidxo->idxo_u32ExponentialBackoff = (pidxo->idxo_u32ExponentialBackoff == 0) ?
        1 : pidxo->idxo_u32ExponentialBackoff * 2;

    jf_ipaddr_getStringIpAddrPort(str, &pidxo->idxo_jiRemote, pidxo->idxo_u16RemotePort);
    JF_LOGGER_DEBUG("server addr: %s, eb: %u", str, pidxo->idxo_u32ExponentialBackoff);

    if (pidxo->idxo_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) DISPATCHER_XFER_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, rollback to 0 and retry.*/
        JF_LOGGER_DEBUG("rollback to 0");
        pidxo->idxo_u32ExponentialBackoff = 0;
    }

    _startConnInDispatcherXferObject(pidxo);

    return u32Ret;
}

static u32 _fnDxopEventActionStartPool(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;

    JF_LOGGER_DEBUG("name: %s", pidxop->idxop_strName);


    return u32Ret;
}

static u32 _fnDxopEventActionStopPool(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;

    JF_LOGGER_DEBUG("name: %s", pidxop->idxop_strName);


    return u32Ret;
}

static u32 _clearDispatcherXferObject(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("name: %s", pidxo->idxo_strName);

    ol_memset(&pidxo->idxo_jiLocal, 0, sizeof(jf_ipaddr_t));
    pidxo->idxo_u16LocalPort = 0;
    pidxo->idxo_bFinConnect = FALSE;

    return u32Ret;
}

static u32 _disconnectDispatcherXferObject(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("name: %s", pidxo->idxo_strName);

    if (pidxo->idxo_pjnsSocket != NULL)
        jf_network_destroySocket(&pidxo->idxo_pjnsSocket);

    _clearDispatcherXferObject(pidxo);

    return u32Ret;
}

/** After pause, all connections are closed, messages remain.
 */
static u32 _fnDxopEventActionPausePool(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;
    internal_dispatcher_xfer_object_t * pidxo = NULL;
    u32 u32Index = 0;

    JF_LOGGER_DEBUG("name: %s", pidxop->idxop_strName);

    /*Close all connections.*/
    for (u32Index = 0; u32Index < pidxop->idxop_u32NumOfObject; u32Index ++)
    {
        pidxo = pidxop->idxop_pidxoObjects[u32Index];
        _disconnectDispatcherXferObject(pidxo);
        pidxo->idxo_bInUse = FALSE;
    }

    /*Reset the bytes sent, the pending message will send later with full message.*/
    pidxop->idxop_sBytesSent = 0;

    return u32Ret;
}

static u32 _fnDxopEventActionResumePool(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;

    JF_LOGGER_DEBUG("name: %s", pidxop->idxop_strName);

    /*Wakeup the network chain.*/
    u32Ret = jf_network_wakeupChain(pidxop->idxop_pjncChain);

    return u32Ret;
}

/** Check if there are pending message in the xfer.
 */
static boolean_t _isPendingDispatcherXferMsg(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = TRUE;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;

    /*Check if there is pending message in pool.*/
    if (pidxop->idxop_pdmMsg != NULL)
        return bRet;

    /*Check if there is message in queue.*/
    if (! isEmptyDispatcherPrioQueue(pidxop->idxop_pdpqMsg))
        return bRet;

    return FALSE;
}

/** Check if we have in use xfer object which is making a connetion or has established a connection.
 */
static boolean_t _hasInUseDispatcherXferObject(
    internal_dispatcher_xfer_object_pool_t * pidxop)
{
    boolean_t bRet = FALSE;
    u8 u8Index = 0;

    for (u8Index = 0; u8Index < pidxop->idxop_u32NumOfObject; u8Index ++)
    {
        if (pidxop->idxop_pidxoObjects[u8Index]->idxo_bInUse)
        {
            bRet = TRUE;
            break;
        }
    }

    return bRet;
}

static internal_dispatcher_xfer_object_t * _getFreeDispatcherXferObject(
    internal_dispatcher_xfer_object_pool_t * pidxop)
{
    internal_dispatcher_xfer_object_t * pidxo = NULL;
    u8 u8Index = 0;

    /*Use the first object. TODO: improve this to use best object.*/
    for (u8Index = 0; u8Index < pidxop->idxop_u32NumOfObject; u8Index ++)
    {
        if (! pidxop->idxop_pidxoObjects[u8Index]->idxo_bInUse)
        {
            pidxo = pidxop->idxop_pidxoObjects[u8Index];
            pidxo->idxo_bInUse = TRUE;
            break;
        }
    }

    return pidxo;
}

static u32 _retryConnInDispatcherXferObject(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("index: %u", pidxo->idxo_u8Index);

    jf_network_addUtimerItem(
        pidxo->idxo_pidxopPool->idxop_pjnuUtimer, pidxo, pidxo->idxo_u32ExponentialBackoff,
        _fnUtimerRetryConnInDispatcherXferObject, NULL);

    return u32Ret;
}

static u32 _fnDxopEventActionStartConn(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pEvent->jhe_pData;
    internal_dispatcher_xfer_object_t * pidxo = NULL;

    /*Check if we have xfer object in use.*/
    if (! _hasInUseDispatcherXferObject(pidxop))
    {
        /*No object in use, get one free object and start connection.*/
        pidxo = _getFreeDispatcherXferObject(pidxop);

        /*Lets try to start the connection.*/
        _startConnInDispatcherXferObject(pidxo);
    }

    return u32Ret;
}

static u32 _preSelectDispatcherXferObject(
    jf_network_chain_object_t * pXferObject, fd_set * readset, fd_set * writeset, fd_set * errorset,
    u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = pXferObject;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;

    if (pidxo->idxo_pjnsSocket != NULL)
    {
        if (! pidxo->idxo_bFinConnect)
        {
            JF_LOGGER_DEBUG("not connected, add to write set");
            /*Not Connected Yet.*/
            jf_network_setSocketToFdSet(pidxo->idxo_pjnsSocket, writeset);
            jf_network_setSocketToFdSet(pidxo->idxo_pjnsSocket, errorset);
        }
        else
        {
            /*Already connected, check writable.*/
            jf_network_setSocketToFdSet(pidxo->idxo_pjnsSocket, errorset);
            /*Add to readset for disconnection event.*/
            jf_network_setSocketToFdSet(pidxo->idxo_pjnsSocket, readset);

            if (! isEmptyDispatcherPrioQueue(pidxop->idxop_pdpqMsg))
            {
                /*If there is pending data to be sent, then we need to check when the socket is
                  writable.*/
                jf_network_setSocketToFdSet(pidxo->idxo_pjnsSocket, writeset);
            }
        }
    }

    return u32Ret;
}

static u32 _recvDataByDispatcherXferObject(internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Buffer[32];
    olsize_t sRecv = sizeof(u8Buffer);

    JF_LOGGER_DEBUG("name: %s", pidxo->idxo_strName);

    u32Ret = jf_network_recv(pidxo->idxo_pjnsSocket, u8Buffer, &sRecv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sRecv == 0)
            u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        JF_LOGGER_ERR(u32Ret, "name: %s", pidxo->idxo_strName);
        _disconnectDispatcherXferObject(pidxo);
        _retryConnInDispatcherXferObject(pidxo);
    }

    return u32Ret;
}

static u32 _sendPendingMsgInDispatcherXferObjectPool(
    internal_dispatcher_xfer_object_pool_t * pidxop, internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent = 0;

    assert(pidxop->idxop_pdmMsg != NULL);

    JF_LOGGER_DEBUG("size: %d", pidxop->idxop_pdmMsg->dm_sMsg);

    bytesSent = pidxop->idxop_pdmMsg->dm_sMsg - pidxop->idxop_sBytesSent;

    u32Ret = jf_network_send(
        pidxo->idxo_pjnsSocket, pidxop->idxop_pdmMsg->dm_u8Msg + pidxop->idxop_sBytesSent, &bytesSent);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pidxop->idxop_sBytesSent += bytesSent;
        if (pidxop->idxop_sBytesSent == pidxop->idxop_pdmMsg->dm_sMsg)
        {
            freeDispatcherMsg(&pidxop->idxop_pdmMsg);
            pidxop->idxop_sBytesSent = 0;
        }
        else
        {
            u32Ret = JF_ERR_MSG_PARTIAL_SENT;
        }
    }

    return u32Ret;
}

static u32 _sendMsgInDispatcherPrioQueue(
    internal_dispatcher_xfer_object_pool_t * pidxop, internal_dispatcher_xfer_object_t * pidxo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_prio_queue_t * prioqueue = pidxop->idxop_pdpqMsg;

    /*First send the pending message in pool.*/
    if (pidxop->idxop_pdmMsg != NULL)
        u32Ret = _sendPendingMsgInDispatcherXferObjectPool(pidxop, pidxo);

    /*The pending message has been sent.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pidxop->idxop_pdmMsg = dequeueDispatcherPrioQueue(prioqueue);

        /*Send the message until we cannot send any more.*/
        while ((pidxop->idxop_pdmMsg != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {
            u32Ret = _sendPendingMsgInDispatcherXferObjectPool(pidxop, pidxo);

            if (u32Ret == JF_ERR_NO_ERROR)
                pidxop->idxop_pdmMsg = dequeueDispatcherPrioQueue(prioqueue);
        }
    }

    return u32Ret;
}

static u32 _postSelectDispatcherXferObject(
    jf_network_chain_object_t * pXferObject, olint_t slct, fd_set * readset, fd_set * writeset,
    fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_t * pidxo = pXferObject;
    internal_dispatcher_xfer_object_pool_t * pidxop = pidxo->idxo_pidxopPool;
    u8 u8Addr[256];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nLen = sizeof(u8Addr);

    /*Write handling.*/
    if ((pidxo->idxo_pjnsSocket != NULL) && pidxo->idxo_bFinConnect &&
        jf_network_isSocketSetInFdSet(pidxo->idxo_pjnsSocket, writeset) != 0)
    {
        /*The socket is writable, and message needs to be sent*/
        u32Ret = _sendMsgInDispatcherPrioQueue(pidxop, pidxo);
    }

    /*Connection handling / read handling.*/
    if (pidxo->idxo_pjnsSocket != NULL)
    {
        /*Close the connection if socket is in the errorset, maybe peer is closed*/
        if (jf_network_isSocketSetInFdSet(pidxo->idxo_pjnsSocket, errorset) != 0)
        {
            JF_LOGGER_DEBUG("name: %s, in errorset", pidxo->idxo_strName);

            /*Connection failed.*/
            _disconnectDispatcherXferObject(pidxo);
            _retryConnInDispatcherXferObject(pidxo);
        }
        else if ((! pidxo->idxo_bFinConnect) &&
                 (jf_network_isSocketSetInFdSet(pidxo->idxo_pjnsSocket, writeset) != 0))
        {
            /* Connected */
            JF_LOGGER_DEBUG("name: %s, connected", pidxo->idxo_strName);

            jf_network_getSocketName(pidxo->idxo_pjnsSocket, psa, &nLen);
            jf_ipaddr_convertSockAddrToIpAddr(
                psa, nLen, &pidxo->idxo_jiLocal, &pidxo->idxo_u16LocalPort);

            pidxo->idxo_bFinConnect = TRUE;
        }
        else if (jf_network_isSocketSetInFdSet(pidxo->idxo_pjnsSocket, readset) != 0)
        {
            /* Data Available */
            u32Ret = _recvDataByDispatcherXferObject(pidxo);
        }
    }

    return u32Ret;
}

static u32 _createDispatcherXferObject(
    internal_dispatcher_xfer_object_t ** ppObject, internal_dispatcher_xfer_object_pool_t * pidxop,
    olchar_t * pstrName, jf_ipaddr_t * pjiRemote, u16 u16Port, u8 u8Index)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[128];
    internal_dispatcher_xfer_object_t * pidxo = NULL;

    jf_ipaddr_getStringIpAddrPort(str, pjiRemote, u16Port);
    JF_LOGGER_DEBUG("name: %s, server addr: %s", pstrName, str);

    u32Ret = jf_jiukun_allocMemory((void **)&pidxo, sizeof(*pidxo));

    /*Create the state machine.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pidxo, sizeof(*pidxo));
        ol_strncpy(pidxo->idxo_strName, pstrName, sizeof(pidxo->idxo_strName) - 1);
        pidxo->idxo_pidxopPool = pidxop;
        ol_memcpy(&pidxo->idxo_jiRemote, pjiRemote, sizeof(pidxo->idxo_jiRemote));
        pidxo->idxo_u16RemotePort = u16Port;
        pidxo->idxo_jncohHeader.jncoh_fnPreSelect = _preSelectDispatcherXferObject;
        pidxo->idxo_jncohHeader.jncoh_fnPostSelect = _postSelectDispatcherXferObject;
        pidxo->idxo_u8Index = u8Index;

        u32Ret = jf_network_appendToChain(pidxop->idxop_pjncChain, pidxo);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppObject = pidxo;
    else if (pidxo != NULL)
        _destroyDispatcherXferObject(&pidxo);

    return u32Ret;
}

static u32 _createDispatcherXferObjects(
    internal_dispatcher_xfer_object_pool_t * pidxop, dispatcher_xfer_pool_create_param_t * pdxpcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;
    olchar_t strName[32];

    /*One object for one address.*/
    pidxop->idxop_u32NumOfObject = pdxpcp->dxpcp_u32MaxAddress;
    for (u32Index = 0;
         (u32Index < pidxop->idxop_u32NumOfObject) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index ++)
    {
        ol_sprintf(strName, "%s-%s-%u", pdxpcp->dxpcp_pstrName, DISPATCHER_XFER_OBJECT_NAME, u32Index);

        u32Ret = _createDispatcherXferObject(
            &pidxop->idxop_pidxoObjects[u32Index], pidxop, strName, pdxpcp->dxpcp_pjiRemote[u32Index],
            pdxpcp->dxpcp_u16RemotePort[u32Index], (u8)u32Index);
    }

    return u32Ret;
}

static u32 _preSelectDispatcherXferObjectPool(
    void * pXferObjectPool, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pXferObjectPool;
    jf_hsm_event_t event;

    jf_hsm_initEvent(&event, DXOPE_SEND_MSG, pidxop, NULL); 
    u32Ret = jf_hsm_processEvent(pidxop->idxop_pjhPool, &event);

    return u32Ret;
}

static u32 _createDispatcherXferObjectPoolHsm(internal_dispatcher_xfer_object_pool_t * pidxop)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hsm_transition_t transionTable[] = {
        {DXOPS_INITIAL, DXOPE_START, NULL, _fnDxopEventActionStartPool, DXOPS_OPERATIVE},
        {DXOPS_INITIAL, DXOPE_PAUSE, NULL, _fnDxopEventActionPausePool, DXOPS_PAUSED},
        {DXOPS_OPERATIVE, DXOPE_PAUSE, NULL, _fnDxopEventActionPausePool, DXOPS_PAUSED},
        {DXOPS_OPERATIVE, DXOPE_SEND_MSG, _isPendingDispatcherXferMsg, _fnDxopEventActionStartConn, DXOPS_OPERATIVE},
        {DXOPS_OPERATIVE, DXOPE_STOP, NULL, _fnDxopEventActionStopPool, DXOPS_INITIAL},
        {DXOPS_PAUSED, DXOPE_RESUME, NULL, _fnDxopEventActionResumePool, DXOPS_OPERATIVE},
        {DXOPS_PAUSED, DXOPE_STOP, NULL, _fnDxopEventActionStopPool, DXOPS_INITIAL},
        {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},
    };

    /*Create the state machine.*/
    u32Ret = jf_hsm_create(&pidxop->idxop_pjhPool, transionTable, DXOPS_INITIAL);

    /*Transite the state to OPERATIVE.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_hsm_event_t event;

        jf_hsm_initEvent(&event, DXOPE_START, pidxop, NULL);
        u32Ret = jf_hsm_processEvent(pidxop->idxop_pjhPool, &event);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG(
            "pool state: %s", _getStringDispatcherXferObjectPoolState(
                jf_hsm_getCurrentStateId(pidxop->idxop_pjhPool)));
    }
    else if (pidxop->idxop_pjhPool != NULL)
    {
        jf_hsm_destroy(&pidxop->idxop_pjhPool);
    }

    return u32Ret;
}

static u32 _fnUtimerPauseDispatcherXferPool(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = object;
    jf_hsm_event_t event;

    jf_hsm_initEvent(&event, DXOPE_PAUSE, pidxop, NULL); 

    u32Ret = jf_hsm_processEvent(pidxop->idxop_pjhPool, &event);

    JF_LOGGER_DEBUG(
        "pool state: %s", _getStringDispatcherXferObjectPoolState(
            jf_hsm_getCurrentStateId(pidxop->idxop_pjhPool)));

    return u32Ret;
}

static u32 _fnUtimerResumeDispatcherXferPool(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = object;
    jf_hsm_event_t event;

    jf_hsm_initEvent(&event, DXOPE_RESUME, pidxop, NULL); 

    u32Ret = jf_hsm_processEvent(pidxop->idxop_pjhPool, &event);

    JF_LOGGER_DEBUG(
        "pool state: %s", _getStringDispatcherXferObjectPoolState(
            jf_hsm_getCurrentStateId(pidxop->idxop_pjhPool)));

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyDispatcherXferObjectPool(dispatcher_xfer_object_pool_t ** ppPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = *ppPool;

    JF_LOGGER_DEBUG("name: %s", pidxop->idxop_strName);

    /*Destroy the xfer object.*/
    _destroyDispatcherXferObjects(pidxop);

    /*Destroy the hsm object.*/
    if (pidxop->idxop_pjhPool != NULL)
        jf_hsm_destroy(&pidxop->idxop_pjhPool);

    /*Destory the pending message in pool.*/
    if (pidxop->idxop_pdmMsg != NULL)
        freeDispatcherMsg(&pidxop->idxop_pdmMsg);

    /*Destroy the priority message queue.*/
    if (pidxop->idxop_pdpqMsg != NULL)
        destroyDispatcherPrioQueue(&pidxop->idxop_pdpqMsg);

    /*Destroy the utimer.*/
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
    internal_dispatcher_xfer_object_pool_t * pidxop = NULL;

    JF_LOGGER_DEBUG(
        "name: %s, MaxAddress: %u", pdxpcp->dxpcp_pstrName, pdxpcp->dxpcp_u32MaxAddress);

    u32Ret = jf_jiukun_allocMemory((void **)&pidxop, sizeof(*pidxop));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pidxop, sizeof(*pidxop));

        /*The callback function for network chain object header.*/
        pidxop->idxop_jncohHeader.jncoh_fnPreSelect = _preSelectDispatcherXferObjectPool;
        pidxop->idxop_pjncChain = pjnc;
        pidxop->idxop_sMaxMsg = pdxpcp->dxpcp_sMaxMsg;
        ol_snprintf(
            pidxop->idxop_strName, sizeof(pidxop->idxop_strName) - 1, "%s-xfer-pool", pdxpcp->dxpcp_pstrName);

        u32Ret = jf_network_createUtimer(pjnc, &pidxop->idxop_pjnuUtimer, pidxop->idxop_strName);
    }

    /*Add the object header to network chain.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pjnc, pidxop);

    /*Create dispatcher priority queue.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_prio_queue_param_t cdpqp;

        ol_bzero(&cdpqp, sizeof(cdpqp));
        cdpqp.cdpqp_u32MaxNumMsg = pdxpcp->dxpcp_u32MaxNumMsg;

        u32Ret = createDispatcherPrioQueue(&pidxop->idxop_pdpqMsg, &cdpqp);
    }

    /*Create state machine for object pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherXferObjectPoolHsm(pidxop);

    /*Create the objects in pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherXferObjects(pidxop, pdxpcp);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPool = pidxop;
    else if (pidxop != NULL)
        destroyDispatcherXferObjectPool((dispatcher_xfer_object_pool_t **)&pidxop);

    return u32Ret;
}

u32 sendMsgByDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pPool;
    boolean_t bWakeup = FALSE;

    /*Add the message to queue.*/
    u32Ret = _enqueueDispatcherXferMsgToQueue(pidxop, pdm, &bWakeup);

    if ((u32Ret == JF_ERR_NO_ERROR) && bWakeup)
    {
        u32Ret = jf_network_wakeupChain(pidxop->idxop_pjncChain);
    }

    return u32Ret;
}

u32 pauseDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pPool;

    /*Start a timer to pause pool to avoid locking.*/
    u32Ret = jf_network_addUtimerItem(
        pidxop->idxop_pjnuUtimer, pidxop, 0, _fnUtimerPauseDispatcherXferPool, NULL);

    return u32Ret;
}

u32 resumeDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_object_pool_t * pidxop = pPool;

    /*Start a timer to resume pool to avoid locking.*/
    u32Ret = jf_network_addUtimerItem(
        pidxop->idxop_pjnuUtimer, pidxop, 0, _fnUtimerResumeDispatcherXferPool, NULL);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
