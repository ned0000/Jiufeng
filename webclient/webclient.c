/**
 *  @file webclient.c
 *
 *  @brief The webclient library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

/* --- private data/data structure section ------------------------------------------------------ */

/** This is the number of seconds for IDLE state of data object. When no request in data object,
 *  the timer is started. When the timer is triggered, the connection for the data object is closed
 *  and the data object state is set to FREE.
 */
#define WEBCLIENT_DATA_OBJECT_IDLE_TIMEOUT    (30)

/** This is the number of seconds for FREE state of data object. When the data object enter FREE
 *  state, the timer is started. When the timer is triggered, the data object is destroyed
 */
#define WEBCLIENT_DATA_OBJECT_FREE_TIMEOUT    (30)

/** This is the number of times, an HTTP connection will be attempted, before it fails. This module
 *  utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will
 *  retry after 1 second, then 2, then 4, etc.
 */
#define WEBCLIENT_CONNECT_RETRY_COUNT         (3)

/** This initial size of the receive buffer
 */
#define WEBCLIENT_INITIAL_BUFFER_SIZE         (2048)

/** Maximum number of data object for the webclient
 */
#define MAX_WEBCLIENT_DATA_OBJECT             (100)

enum pipeline_type
{
    PIPELINE_UNKNOWN = 0, /**< doesn't know yet if the server supports persistent connections */
    PIPELINE_YES, /**< The server does indeed support persistent connections */
    PIPELINE_NO, /**< The server does not support persistent connections*/
};

typedef void  webclient_dataobject_t;

typedef struct internal_web_request
{
    jf_datavec_t iwr_jdDataVec;

    u16 iwr_u16Reserved[4];

    void * iwr_pUser;
    jf_webclient_fnOnEvent_t iwr_fnOnEvent;
} internal_web_request_t;

typedef struct internal_webclient
{
    jf_network_chain_object_header_t iw_jncohHeader;

    jf_network_acsocket_t * iw_pjnaAcsocket;
    u32 iw_u32PoolSize;
    u32 iw_u32Reserved;

    /**The web data object is put to data hash tree when the connection is established and there
       are web request*/
    jf_hashtree_t iw_jhData;

    jf_network_utimer_t * iw_pjnuUtimer;

    olsize_t iw_sBuffer;
    olsize_t iw_sReserved;

    jf_network_chain_t *iw_pjncChain;
    jf_mutex_t iw_jmLock;
} internal_webclient_t;

typedef enum
{
    WEBCLIENT_DATAOBJECT_STATE_INITIAL = 0,
    WEBCLIENT_DATAOBJECT_STATE_OPERATIVE,
    WEBCLIENT_DATAOBJECT_STATE_IDLE,
    WEBCLIENT_DATAOBJECT_STATE_FREE,
} webclient_dataobject_state_t;

typedef struct internal_web_dataobject
{
    jf_ipaddr_t iwd_jiRemote;
    u16 iwd_u16RemotePort;
    u16 iwd_u16Reserved[3];

    internal_webclient_t * iwd_piwParent;

    /**we've got the HTTP header*/
    boolean_t iwd_bFinHeader;
    /**the data in HTTP body is chunked*/
    boolean_t iwd_bChunked;
    u8 iwd_u8State;
    u8 iwd_u8PipelineFlags;
    u8 iwd_u8Reserved[4];

    /**if the asocket buffer is not big enough to hold HTTP body, use this buffer for the data*/
    u8 * iwd_pu8BodyBuf;
    u32 iwd_u32BodyOffset;

    u32 iwd_u32ExponentialBackoff;

    olint_t iwd_nBytesLeft;
    olint_t iwd_nReserved;

    jf_httpparser_chunk_processor_t * iwd_pjhcpProcessor;

    jf_httpparser_packet_header_t * iwd_pjhphHeader;

    jf_queue_t iwd_jqRequest;
    jf_network_asocket_t * iwd_pjnaConn;

    jf_ipaddr_t iwd_jiLocal;

} internal_web_dataobject_t;

/* --- private routine section ------------------------------------------------------------------ */

static olint_t _getStringHashKey(olchar_t * key, jf_ipaddr_t * addr, u16 port)
{
    olint_t keyLength;
    olchar_t strIpAddr[50];

    jf_ipaddr_getStringIpAddr(strIpAddr, addr);
    keyLength = ol_sprintf(key, "%s:%d", strIpAddr, port);

    return keyLength;
}

static u32 _destroyWebRequest(internal_web_request_t ** ppRequest)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * piwr = NULL;

    jf_logger_logInfoMsg("destroy web req");

    piwr = (internal_web_request_t *) *ppRequest;

    jf_datavec_freeData(&piwr->iwr_jdDataVec);
    
    jf_jiukun_freeMemory((void **)ppRequest);

    return u32Ret;
}

static u32 _newWebRequest(
    internal_web_request_t ** ppRequest, u8 ** ppu8Data, olsize_t * psData, u16 u16Num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * piwr = NULL;

    jf_logger_logInfoMsg("new web req");

    u32Ret = jf_jiukun_allocMemory((void **)&piwr, sizeof(internal_web_request_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwr, sizeof(internal_web_request_t));

        u32Ret = jf_datavec_cloneData(&piwr->iwr_jdDataVec, ppu8Data, psData, u16Num);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        _destroyWebRequest(&piwr);

    return u32Ret;
}

/** Free resources associated with a web data object
 *
 *  @note The connection should be disconnected before. This function is not responsible for closing
 *   the connection
 *
 *  @param ppDataobject [in/out] The web data object to free
 */
static u32 _destroyWebDataobject(webclient_dataobject_t ** ppDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *) *ppDataobject;
    internal_web_request_t * piwr = NULL;
//    internal_webclient_t * piw = piwd->iwd_piwParent;
    olchar_t key[64];

    _getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg("destroy web data obj %s", key);

    assert(piwd->iwd_pjnaConn == NULL);

    /*The header needs to be freed*/
    if (piwd->iwd_pjhphHeader != NULL)
        jf_httpparser_destroyPacketHeader(&(piwd->iwd_pjhphHeader));

    /*Destroy the chunk processor*/
    if (piwd->iwd_pjhcpProcessor != NULL)
        jf_httpparser_destroyChunkProcessor(&piwd->iwd_pjhcpProcessor);

    if (piwd->iwd_pu8BodyBuf != NULL)
        jf_jiukun_freeMemory((void **)&piwd->iwd_pu8BodyBuf);

    /*Iterate through all the pending requests*/
    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    while (piwr != NULL)
    {
        /*If this is a client request, then we need to signal that this request is being aborted*/
        piwr->iwr_fnOnEvent(NULL, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL, piwr->iwr_pUser);
        _destroyWebRequest(&piwr);

        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    }
    jf_queue_fini(&piwd->iwd_jqRequest);

    jf_jiukun_freeMemory((void **)ppDataobject);

    return u32Ret;
}

static u32 _createWebDataobject(
    internal_web_dataobject_t ** ppiwd, jf_network_asocket_t * pAsocket,
    internal_webclient_t * piw, jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = NULL;
    olchar_t addr[64];
    olint_t addrlen;

    jf_logger_logInfoMsg("create web data obj");

    u32Ret = jf_jiukun_allocMemory((void **)&piwd, sizeof(internal_web_dataobject_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwd, sizeof(internal_web_dataobject_t));

        jf_queue_init(&piwd->iwd_jqRequest);
        piwd->iwd_pjnaConn = pAsocket;
        piwd->iwd_piwParent = piw;
        ol_memcpy(&piwd->iwd_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        piwd->iwd_u16RemotePort = u16Port;
        piwd->iwd_u8State = WEBCLIENT_DATAOBJECT_STATE_INITIAL;

        addrlen = _getStringHashKey(addr, pjiRemote, u16Port);
        u32Ret = jf_hashtree_addEntry(&piw->iw_jhData, addr, addrlen, piwd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppiwd = piwd;
    else if (piwd != NULL)
        _destroyWebDataobject((webclient_dataobject_t **)&piwd);

    return u32Ret;
}

static u32 _sendWebclientRequestData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    internal_web_request_t * piwr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    for (u16Index = 0;
         (u16Index < piwr->iwr_jdDataVec.jd_u16CurEntry) && (u32Ret == JF_ERR_NO_ERROR);
         ++ u16Index)
    {
        entry = &piwr->iwr_jdDataVec.jd_jdeEntry[u16Index];

        u32Ret = jf_network_sendAcsocketData(
            pAcsocket, pAsocket, entry->jde_pu8Buf, entry->jde_sOffset);
    }

    return u32Ret;
}

static u32 _processWebRequest(
    internal_webclient_t * piw, jf_ipaddr_t * pjiRemote, u16 u16Port,
    internal_web_request_t * request)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t addr[64];
    olint_t addrlen;
    internal_web_dataobject_t * piwd = NULL;
    boolean_t bQueueEmpty = FALSE;

    addrlen = _getStringHashKey(addr, pjiRemote, u16Port);

    jf_logger_logInfoMsg("process web req, %s", addr);

    u32Ret = jf_hashtree_getEntry(&piw->iw_jhData, addr, addrlen, (void **)&piwd);
    if (u32Ret == JF_ERR_HASHTREE_ENTRY_NOT_FOUND)
    {
        /*There is no previous connection, so we need to set it up*/
        jf_logger_logInfoMsg("process web req, create data object");

        u32Ret = _createWebDataobject(&piwd, NULL, piw, pjiRemote, u16Port);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*enqueue the request*/
        bQueueEmpty = jf_queue_isEmpty(&piwd->iwd_jqRequest);
        u32Ret = jf_queue_enqueue(&piwd->iwd_jqRequest, request);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (piwd->iwd_u8State == WEBCLIENT_DATAOBJECT_STATE_OPERATIVE)
        {
            if (bQueueEmpty)
            {
                /*The queue is empty, send the data now*/
                u32Ret = _sendWebclientRequestData(
                    piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn, request);
            }
        }
        else
        {
            /*Connect to remote server, the async socket will wake up the chain so webclient doesn't
              have to care about it*/
            u32Ret = jf_network_connectAcsocketTo(
                piw->iw_pjnaAcsocket, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort, piwd);
        }
    }

    return u32Ret;
}

/** Pre select handler for chain
 *
 *  @param pWebclient [in] the web client object
 *  @param readset [out] the read fd set
 *  @param writeset [out] the write fd set
 *  @param errorset [out] the error fd set
 *  @param pu32BlockTime [out] the block time in millisecond
 *
 *  @return the error code
 */
static u32 _preWebclientProcess(
    void * pWebclient, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;


    return u32Ret;
}

/** The timed callback is used to close idle sockets. A socket is considered idle if after a request
 *  is answered, another request isn't received within the time specified by
 *  WEBCLIENT_DATA_OBJECT_IDLE_TIMEOUT
 *
 *  @param object [in] the web data object
 */
static u32 _webclientTimerSink(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;

    jf_logger_logInfoMsg("webclient timer sink");

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        jf_logger_logInfoMsg("webclient timer sink, queue is empty");
        /*This connection is idle, because there are no pending requests */
        if (piwd->iwd_pjnaConn != NULL)
        {
            /*We need to close this socket*/
            jf_logger_logInfoMsg("webclient timer sink, close the connection");
            jf_network_disconnectAcsocket(piwd->iwd_piwParent->iw_pjnaAcsocket, piwd->iwd_pjnaConn);
        }
    }

    return u32Ret;
}

static u32 _webclientTimerDestroyDataOject(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;
    internal_webclient_t * piw = piwd->iwd_piwParent;
    olchar_t key[64];
    olint_t keyLength;

    keyLength = _getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);

    jf_logger_logInfoMsg("webclient timer destroy data object");

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        /*This connection is idle, because there are no pending requests */
        jf_logger_logInfoMsg("webclient timer destroy data object, queue is empty");

        jf_hashtree_deleteEntry(&piw->iw_jhData, key, keyLength);
        _destroyWebDataobject((webclient_dataobject_t **)&piwd);
    }

    return u32Ret;
}

/** Internal method called when web client has finished processing a request or response
 *
 *  @param piwd [in] the associated internal web data object
 */
static u32 _webclientFinishedResponse(internal_web_dataobject_t * piwd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * piwr = NULL;
    internal_webclient_t * piw = piwd->iwd_piwParent;

    jf_logger_logInfoMsg(
        "webclient finishd response, pipeline flag %d", piwd->iwd_u8PipelineFlags);

    /*Reset the flags*/
    piwd->iwd_bFinHeader = FALSE;
    piwd->iwd_bChunked = FALSE;

    piwd->iwd_u32BodyOffset = 0;
    if (piwd->iwd_pu8BodyBuf != NULL)
        jf_jiukun_freeMemory((void **)&piwd->iwd_pu8BodyBuf);

    if (piwd->iwd_pjhcpProcessor != NULL)
        jf_httpparser_destroyChunkProcessor(&piwd->iwd_pjhcpProcessor);

    if (piwd->iwd_pjhphHeader != NULL)
        jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);

    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    _destroyWebRequest(&piwr);

    piwr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr == NULL)
    {
        /*Since the request queue is empty, that means this connection is now idle. Set a timed
          callback, so we can free this resource if neccessary*/
        piwd->iwd_u8State = WEBCLIENT_DATAOBJECT_STATE_IDLE;

        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, WEBCLIENT_DATA_OBJECT_IDLE_TIMEOUT,
            _webclientTimerSink, NULL);
    }
    else
    {
        /*There are still pending requests in the queue, so try to send them*/
        jf_logger_logInfoMsg("webclient finish response, queue is not empty");

        if (piwd->iwd_u8PipelineFlags == PIPELINE_NO)
        {
            /*Pipelining is not supported, so we should just close the socket instead of waiting for
              the other guy to close it, because if they forget to, it will screw us over if there
              are pending requests */

            /*It should also be noted, that when this closes, the module will realize there are
              pending requests, in which case it will open a new connection for the requests.*/
            jf_logger_logInfoMsg(
                "webclient finish response, pipeline is no, disconnect the data object");
            jf_network_disconnectAcsocket(piwd->iwd_piwParent->iw_pjnaAcsocket, piwd->iwd_pjnaConn);
        }
        else
        {
            /*If the connection is still open, and we didn't flag this as not supporting persistent
              connections, than obviously it is supported*/
            jf_logger_logInfoMsg("webclient finish response, pipeline is yes");
            piwd->iwd_u8PipelineFlags = PIPELINE_YES;

            u32Ret = _sendWebclientRequestData(piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn, piwr);
        }
    }

    return u32Ret;
}

/** Internal method dispatched by the utimer, to retry refused connections
 *
 *  @note The module does an exponential backoff, when retrying connections. The
 *  number of retries is determined by the value of WEBCLIENT_CONNECT_RETRY_COUNT
 *
 *  @param object [in] the associated web data object
 */
static u32 _webclientRetrySink(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;
    internal_webclient_t * piw = piwd->iwd_piwParent;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = _getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg("webclient retry sink, %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) WEBCLIENT_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        jf_logger_logInfoMsg("webclient retry sink, give up");

        jf_hashtree_deleteEntry(&piw->iw_jhData, key, keyLength);
        _destroyWebDataobject((webclient_dataobject_t **)&piwd);
    }
    else
    {
        /*Lets retry again*/
        jf_logger_logInfoMsg("webclient retry sink, try to connect");
        u32Ret = jf_network_connectAcsocketTo(
            piw->iw_pjnaAcsocket, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort, piwd);
    }

    return u32Ret;
}

/** Internal method dispatched by the connect event of the underlying asocket
 *
 *  @param pAsocket [in] the underlying asocket
 *  @param u32Status [in] connection status
 *  @param pUser [in] the associated web data object
 */
static u32 _webclientOnConnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *) pUser;
    internal_web_request_t * piwr = NULL;

    piwd->iwd_pjnaConn = pAsocket;

    if (u32Status == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("web client connected");
        piwd->iwd_u8State = WEBCLIENT_DATAOBJECT_STATE_OPERATIVE;
        piwd->iwd_u32ExponentialBackoff = 0;
        jf_network_getLocalInterfaceOfAcsocket(pAcsocket, pAsocket, &piwd->iwd_jiLocal);
        piwr = jf_queue_peek(&piwd->iwd_jqRequest);
        //Success: Send First Request
        u32Ret = _sendWebclientRequestData(pAcsocket, pAsocket, piwr);
    }
    else
    {
        jf_logger_logInfoMsg("web client connect failed, retry later");
        /*The connection failed, so lets set a timed callback, and try again*/
        piwd->iwd_u8PipelineFlags = PIPELINE_UNKNOWN;
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, piwd->iwd_u32ExponentialBackoff,
            _webclientRetrySink, NULL);
    }

    return u32Ret;
}               

/** Internal method dispatched by the disconnect event of the underlying acsocket
 *
 *  @param pAcsocket [in] the underlying async client socket
 *  @param pAsocket [in] the underlying async socket
 *  @param u32Status [in] the status of the disconnection
 *  @param pUser [in] the associated web data object
 *
 *  @return the error code
 */
static u32 _webclientOnDisconnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)pUser;
    internal_web_request_t * piwr = NULL;

    jf_logger_logInfoMsg(
        "webclient disconnect, PipelineFlags %d", piwd->iwd_u8PipelineFlags);

    piwd->iwd_pjnaConn = NULL;
    piwd->iwd_u8State = WEBCLIENT_DATAOBJECT_STATE_FREE;

    piwr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr != NULL)
    {
        /*If there are still pending requests, then obviously this server doesn't do persistent
          connections*/
        jf_logger_logInfoMsg("web client disconnect, pipeline is no");
        piwd->iwd_u8PipelineFlags = PIPELINE_NO;

        /*There are still requests to be made, make another connection and continue*/
        jf_logger_logInfoMsg("web client disconnect, retry later");
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, piwd->iwd_u32ExponentialBackoff,
            _webclientRetrySink, NULL);
    }
    else
    {
        /*no pending request, set a timer to destroy webclient data object*/
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, WEBCLIENT_DATA_OBJECT_FREE_TIMEOUT,
            _webclientTimerDestroyDataOject, NULL);
    }

    return u32Ret;
}


/** Internal method dispatched by the send ok event of the underlying asocket
 *
 *  @param pAsocket [in] The underlying asocket
 *  @param user [in] the associated web data object
 *
 *  @return the erro code
 */
static u32 _webclientOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("webclient send data, status: 0x%X", u32Status);

    return u32Ret;
}

static u32 _parseHttpHeaderContent(internal_web_dataobject_t * piwd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Encoding = JF_HTTPPARSER_TRANSFER_ENCODING_UNKNOWN;

    jf_logger_logInfoMsg("parse http header content");

    /*Introspect Request, to see what to do next*/
    u32Ret = jf_httpparser_parseHeaderTransferEncoding(piwd->iwd_pjhphHeader, &u8Encoding);
    if ((u32Ret == JF_ERR_NO_ERROR) && (u8Encoding == JF_HTTPPARSER_TRANSFER_ENCODING_CHUNKED))
    {
        /*This packet was chunk encoded*/
        piwd->iwd_bChunked = TRUE;
        jf_logger_logDebugMsg("parse http header content, chunk");

        u32Ret = jf_httpparser_createChunkProcessor(
            &piwd->iwd_pjhcpProcessor, piwd->iwd_piwParent->iw_sBuffer);
    }
    else
    {
        u32Ret = jf_httpparser_parseHeaderContentLength(
            piwd->iwd_pjhphHeader, &piwd->iwd_nBytesLeft);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_logger_logDebugMsg("parse http header, content-length %d", piwd->iwd_nBytesLeft);

        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((piwd->iwd_nBytesLeft == -1) && (! piwd->iwd_bChunked))
        {
            /*This request has no body*/
            piwd->iwd_nBytesLeft = 0;
        }
    }

    return u32Ret;
}

static u32 _parseHttpHeader(
    jf_network_asocket_t * pAsocket, internal_web_dataobject_t * piwd,
    internal_web_request_t * piwr, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, olsize_t sHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph = NULL;
    olsize_t zero = 0;

    jf_logger_logInfoMsg("parse http header");
    /*Headers are delineated with a CRLF, and terminated with an empty line*/
    piwd->iwd_nBytesLeft = -1;
    piwd->iwd_bFinHeader = TRUE;
    u32Ret = jf_httpparser_parsePacketHeader(
        &piwd->iwd_pjhphHeader, (olchar_t *)pu8Buffer, *psBeginPointer,
        sEndPointer - (*psBeginPointer));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Introspect Request, to see what to do next*/
        u32Ret = _parseHttpHeaderContent(piwd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (piwd->iwd_nBytesLeft == 0)
        {
            /*We already have the complete Response Packet*/
            piwr->iwr_fnOnEvent(
                pAsocket, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader, piwr->iwr_pUser);
            *psBeginPointer = *psBeginPointer + sHeader + 4;
            _webclientFinishedResponse(piwd);
        }
        else
        {
            /*There is still data we need to read. Lets see if any of the body arrived yet*/
            if (! piwd->iwd_bChunked)
            {
                /*This isn't chunked, so we can process normally*/
                if (piwd->iwd_nBytesLeft != -1 &&
                    (sEndPointer - (*psBeginPointer)) - (sHeader + 4) >= piwd->iwd_nBytesLeft)
                {
                    /*Get the whole packet*/
                    jf_logger_logInfoMsg("parse http header, got entire packet");
                    piwd->iwd_pjhphHeader->jhph_pu8Body = pu8Buffer + sHeader + 4;
                    piwd->iwd_pjhphHeader->jhph_sBody = piwd->iwd_nBytesLeft;
                    /*We have the entire body, so we have the entire packet*/
                    piwr->iwr_fnOnEvent(
                        pAsocket, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader,
                        piwr->iwr_pUser);
                    *psBeginPointer = *psBeginPointer + sHeader + 4 + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
                else
                {
                    /*We read some of the body, but not all of it yet*/
                    jf_logger_logInfoMsg("parse http header, got partial packet");
                    *psBeginPointer = sHeader + 4;
                    u32Ret = jf_httpparser_clonePacketHeader(&pjhph, piwd->iwd_pjhphHeader);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);
                        piwd->iwd_pjhphHeader = pjhph;
                    }

                    if ((u32Ret == JF_ERR_NO_ERROR) &&
                        (piwd->iwd_nBytesLeft > piwd->iwd_piwParent->iw_sBuffer))
                    {
                        /*asocket buffer is not enough to hold the HTTP body*/
                        jf_logger_logInfoMsg("parse http header, alloc memory for body");
                        u32Ret = jf_jiukun_allocMemory(
                            (void **)&piwd->iwd_pu8BodyBuf, piwd->iwd_nBytesLeft);
                    }
                }
            }
            else
            {
                /*This packet is chunk encoded, so we need to run it through our Chunk Processor*/
                u32Ret = jf_httpparser_processChunk(
                    piwd->iwd_pjhcpProcessor, piwd->iwd_pjhphHeader, pu8Buffer + sHeader + 4,
                    &zero, (sEndPointer - (*psBeginPointer) - (sHeader + 4)));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    *psBeginPointer = sHeader + 4 + zero;

                    if (piwd->iwd_pjhphHeader->jhph_pu8Body != NULL)
                    {
                        piwr->iwr_fnOnEvent(
                            piwd->iwd_pjnaConn, JF_WEBCLIENT_EVENT_INCOMING_DATA,
                            piwd->iwd_pjhphHeader, piwr->iwr_pUser);
                        _webclientFinishedResponse(piwd);
                    }
                    else
                    {
                        /*Header doesn't have body, it means the chunk is not completed processed*/
                        u32Ret = jf_httpparser_clonePacketHeader(&pjhph, piwd->iwd_pjhphHeader);
                        if (u32Ret == JF_ERR_NO_ERROR)
                        {
                            jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);
                            piwd->iwd_pjhphHeader = pjhph;
                        }
                    }
                }
            }
        }
    }

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying asocket
 *
 *  @param pAsocket [in] the underlying asocket
 *  @param pu8Buffer [in] the receive buffer
 *  @param psBeginPointer [in] start pointer in the buffer
 *  @param sEndPointer [in] the length of the buffer
 *  @param pUser [in] User data that can be set/received
 *
 *  @return the error code
 */
static u32 _webclientOnData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *) pUser;
    internal_web_request_t * piwr = NULL;
    olsize_t sHeader = 0;
    olint_t Fini;

    jf_logger_logInfoMsg(
        "web client data, %d:%d", *psBeginPointer, sEndPointer);
/*
    jf_logger_jf_logger_logDataMsgWithAscii(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer,
        "web client data");
*/
    piwr = (internal_web_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
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

    if (! piwd->iwd_bFinHeader)
    {
        /*Still Reading Headers*/
        u32Ret = jf_httpparser_findHeader(pu8Buffer, *psBeginPointer, sEndPointer, &sHeader);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseHttpHeader(
                pAsocket, piwd, piwr, pu8Buffer, psBeginPointer, sEndPointer, sHeader);
        }
    }
    else
    {
        /*We already processed the headers, so we are only expecting the body now*/
        if (! piwd->iwd_bChunked)
        {
            /*This isn't chunk encoded*/
            if (piwd->iwd_pu8BodyBuf == NULL)
            {
                /*the asocket buffer can hold the whole body*/
                Fini = sEndPointer - *psBeginPointer - piwd->iwd_nBytesLeft;
                jf_logger_logInfoMsg("web client data, body in asocket buffer, Fini %d", Fini);
                if (Fini >= 0)
                {
                    jf_httpparser_setBody(
                        piwd->iwd_pjhphHeader, pu8Buffer + *psBeginPointer, piwd->iwd_nBytesLeft,
                        FALSE);
                    piwr->iwr_fnOnEvent(
                        pAsocket, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader,
                        piwr->iwr_pUser);
                    *psBeginPointer = *psBeginPointer + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
            }
            else
            {
                /*the asocket buffer cannot hold the whole body, copy the asocket buffer to another
                  big buffer*/
                Fini = piwd->iwd_u32BodyOffset +
                    sEndPointer - *psBeginPointer - piwd->iwd_nBytesLeft;
                jf_logger_logInfoMsg(
                    "web client data, body in data object buffer, Fini %d, offset %u",
                    Fini, piwd->iwd_u32BodyOffset);
                if (Fini >= 0)
                {
                    Fini = piwd->iwd_nBytesLeft - piwd->iwd_u32BodyOffset;
                    ol_memcpy(piwd->iwd_pu8BodyBuf + piwd->iwd_u32BodyOffset,
                              pu8Buffer + *psBeginPointer, Fini);
                    jf_httpparser_setBody(
                        piwd->iwd_pjhphHeader, piwd->iwd_pu8BodyBuf, piwd->iwd_nBytesLeft, FALSE);
                    piwr->iwr_fnOnEvent(
                        pAsocket, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader,
                        piwr->iwr_pUser);
                    *psBeginPointer = *psBeginPointer + Fini;
                    _webclientFinishedResponse(piwd);

                }
                else
                {
                    Fini = sEndPointer - *psBeginPointer;
                    ol_memcpy(piwd->iwd_pu8BodyBuf + piwd->iwd_u32BodyOffset,
                              pu8Buffer + *psBeginPointer, Fini);
                    piwd->iwd_u32BodyOffset += Fini;
                    *psBeginPointer = sEndPointer;
                }
            }
        }
        else
        {
            /*This is chunk encoded, so run it through our Chunk Processor*/
            u32Ret = jf_httpparser_processChunk(
                piwd->iwd_pjhcpProcessor, piwd->iwd_pjhphHeader, pu8Buffer, psBeginPointer,
                sEndPointer);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (piwd->iwd_pjhphHeader->jhph_pu8Body != NULL)
                {
                    piwr->iwr_fnOnEvent(
                        piwd->iwd_pjnaConn, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader,
                        piwr->iwr_pUser);
                    _webclientFinishedResponse(piwd);
                }
            }
        }
    }

    return u32Ret;
}

static u32 _internalWebclientOnEvent(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t event,
    jf_httpparser_packet_header_t * header, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_webclient_destroy(jf_webclient_t ** ppWebclient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) *ppWebclient;

    /*Iterate through all the web data objects*/
    jf_hashtree_finiHashtreeAndData(&piw->iw_jhData, _destroyWebDataobject);

    if (piw->iw_pjnaAcsocket != NULL)
    {
        jf_network_destroyAcsocket(&piw->iw_pjnaAcsocket);
    }

    /*Free all the other associated resources*/
    jf_hashtree_fini(&piw->iw_jhData);
    jf_mutex_fini(&piw->iw_jmLock);

    jf_jiukun_freeMemory((void **)ppWebclient);

    return u32Ret;
}

u32 jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient, jf_webclient_create_param_t * pjwcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;
    internal_webclient_t * piw = NULL;

    assert((pjnc != NULL) && (ppWebclient != NULL));
    assert((pjwcp != NULL) &&
           (pjwcp->jwcp_u32PoolSize > 0) && (pjwcp->jwcp_u32PoolSize < MAX_WEBCLIENT_DATA_OBJECT));

    u32Ret = jf_jiukun_allocMemory((void **)&piw, sizeof(internal_webclient_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piw, sizeof(internal_webclient_t));
        piw->iw_jncohHeader.jncoh_fnPreSelect = _preWebclientProcess;
        piw->iw_pjncChain = pjnc;
        piw->iw_sBuffer = pjwcp->jwcp_sBuffer ? pjwcp->jwcp_sBuffer : WEBCLIENT_INITIAL_BUFFER_SIZE;
        piw->iw_u32PoolSize = pjwcp->jwcp_u32PoolSize;

        jf_hashtree_init(&piw->iw_jhData);

        u32Ret = jf_mutex_init(&piw->iw_jmLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createUtimer(pjnc, &(piw->iw_pjnuUtimer));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_appendToChain(pjnc, piw);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jnacp, sizeof(jnacp));
        jnacp.jnacp_sInitialBuf = piw->iw_sBuffer;
        jnacp.jnacp_u32MaxConn = piw->iw_u32PoolSize;
        jnacp.jnacp_fnOnData = _webclientOnData;
        jnacp.jnacp_fnOnConnect = _webclientOnConnect;
        jnacp.jnacp_fnOnDisconnect = _webclientOnDisconnect;
        jnacp.jnacp_fnOnSendData = _webclientOnSendData;

        u32Ret = jf_network_createAcsocket(pjnc, &piw->iw_pjnaAcsocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppWebclient = piw;
    else if (piw != NULL)
        jf_webclient_destroy((void **)&piw);

    return u32Ret;
}

u32 jf_webclient_pipelineWebRequest(
    jf_webclient_t * pWebClient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_httpparser_packet_header_t * packet, jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebClient;
    internal_web_request_t * piwr = NULL;
    u8 * pu8Data[1];
    olsize_t sData[1];

    jf_logger_logDebugMsg("pipeline web req");

    u32Ret = jf_httpparser_getRawPacket(packet, (olchar_t **)&pu8Data[0], &sData[0]);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _newWebRequest(&piwr, pu8Data, sData, 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piwr->iwr_fnOnEvent = (fnOnEvent == NULL) ? _internalWebclientOnEvent : fnOnEvent;
        piwr->iwr_pUser = user;

        u32Ret = _processWebRequest(piw, pjiRemote, u16Port, piwr);
    }

    return u32Ret;
}

u32 jf_webclient_pipelineWebRequestEx(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, olchar_t * pstrBody, olsize_t sBody,
    jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_web_request_t * piwr = NULL;
    u16 u16NumOfData;
    u8 * pu8Data[2];
    olsize_t sData[2];

    jf_logger_logInfoMsg("pipeline web req ex");
    jf_logger_logDataMsgWithAscii((u8 *)pstrHeader, sHeader, "HTTP request header:");
    if (pstrBody != NULL)
        jf_logger_logDataMsgWithAscii((u8 *)pstrBody, sBody, "HTTP request body:");

    u16NumOfData = (pstrBody != NULL) ? 2 : 1;

    pu8Data[0] = (u8 *)pstrHeader;
    sData[0] = sHeader;

    if (pstrBody != NULL)
    {
        pu8Data[1] = (u8 *)pstrBody;
        sData[1] = sBody;
    }

    u32Ret = _newWebRequest(&piwr, pu8Data, sData, u16NumOfData);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piwr->iwr_fnOnEvent = (fnOnEvent == NULL) ? _internalWebclientOnEvent : fnOnEvent;
        piwr->iwr_pUser = user;

        u32Ret = _processWebRequest(piw, pjiRemote, u16Port, piwr);
    }

    return u32Ret;
}

u32 jf_webclient_deleteWebRequests(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    olchar_t addr[25] = {0};
    internal_web_dataobject_t * piwd = NULL;
    olint_t addrlen = 0;
    internal_web_request_t * piwr = NULL;
    jf_queue_t jqRemove;

    jf_queue_init(&jqRemove);

    addrlen = _getStringHashKey(addr, pjiRemote, u16Port);

    /* Are there any pending requests to this IP/Port combo */
//    jf_mutex_acquire(&piw->iw_jmLock);
    if (jf_hashtree_hasEntry(&piw->iw_jhData, addr, addrlen))
    {
        /* Yes, iterate through them */
        jf_hashtree_getEntry(&piw->iw_jhData, addr, addrlen, (void **)&piwd);
        while ((! jf_queue_isEmpty(&piwd->iwd_jqRequest)) && (u32Ret == JF_ERR_NO_ERROR)) 
        {
            /*Put all the pending requests into this queue, so we can trigger them outside of
              this lock*/
            piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
            u32Ret = jf_queue_enqueue(&jqRemove, piwr);
        }
    }
//    jf_mutex_release(&piw->iw_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* Lets iterate through all the requests that we need to get rid of */
        while (! jf_queue_isEmpty(&jqRemove))
        {
            /* Tell the user, we are aborting these requests */
            piwr = jf_queue_dequeue(&jqRemove);
            piwr->iwr_fnOnEvent(
                pWebclient, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL, piwr->iwr_pUser);

            _destroyWebRequest(&piwr);
        }
    }

    jf_queue_fini(&jqRemove);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

