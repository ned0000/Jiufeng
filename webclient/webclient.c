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

/* --- private data/data structure section ------------------------------------------------------ */
/** Keep a table of all the connections. This is the maximum number allowed to be idle. Since we
 *  have in the constructor a pool size, this feature may be depracted.
 *  ToDo: Look into depracating this
 */
#define MAX_WEBCLIENT_IDLE_SESSIONS           (20)

/** This is the number of seconds that a connection must be idle for, before it will be
 *  automatically closed. Idle means there are no pending requests
 */
#define WEBCLIENT_SESSION_IDLE_TIMEOUT        (20)

/** This is the number of times, an HTTP connection will be attempted, before it fails. This module
 *  utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will
 *  retry after 1 second, then 2, then 4, etc.
 */
#define WEBCLIENT_CONNECT_RETRY_COUNT         (4)

/** This initial size of the receive buffer
 */
#define WEBCLIENT_INITIAL_BUFFER_SIZE         (2048)

/** Maximum connection for the webclient
 */
#define MAX_WEBCLIENT_CONNECTION              (100)

enum pipeline_type
{
    PIPELINE_UNKNOWN = 0, /**< doesn't know yet if the server supports persistent connections */
    PIPELINE_YES, /**< The server does indeed support persistent connections */
    PIPELINE_NO, /**< The server does not support persistent connections*/
};

/** Chunk processing flags
 */
#define STARTCHUNK      (0)
#define ENDCHUNK        (1)
#define DATACHUNK       (2)
#define FOOTERCHUNK     (3)

typedef void  webclient_dataobject_t;

typedef struct internal_web_request
{
#define IWR_MAX_NUM_OF_BUF  8
    u32 iwr_u32NumOfBuffers;
    u8 * iwr_pu8Buffer[IWR_MAX_NUM_OF_BUF];
    olsize_t iwr_sBuffer[IWR_MAX_NUM_OF_BUF];

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
    /**The web data object is put to idle hash tree when no more request in the queue of the
       web data object. NOTE: the data object IS NOT REMOVED from data hash tree*/
    jf_hashtree_t iw_jhIdle;
    /**The web data object is put the backlog queue when the object doesn't connected to
       remote server*/
    jf_queue_t iw_jqBacklog;

    jf_network_utimer_t * iw_pjnuUtimer;
    u32 iw_u32IdleCount;
    olsize_t iw_sBuffer;

    jf_network_chain_t *iw_pjncChain;
    jf_mutex_t iw_jmLock;
} internal_webclient_t;

typedef struct internal_web_chunkdata
{
    u8 * iwc_pu8Buffer;
    u32 iwc_u32Flags;
    u32 iwc_u32Offset;
    u32 iwc_u32MallocSize;
    olint_t iwc_nBytesLeft;
} internal_web_chunkdata_t;

typedef struct internal_web_dataobject
{
    u32 iwd_u32PipelineFlags;
    u32 iwd_u32ActivityCounter;

    jf_ipaddr_t iwd_jiRemote;
    u16 iwd_u16RemotePort;
    u16 iwd_u16Reserved[3];

    internal_webclient_t * iwd_piwParent;
    /**we've got the HTTP header*/
    boolean_t iwd_bFinHeader;
    /**the data in HTTP body is chunked*/
    boolean_t iwd_bChunked;
    /**set to true when we get response*/
    boolean_t iwd_bWaitForClose;
    boolean_t iwd_bInitialRequestAnswered;
    u8 iwd_u8Reserved[4];

    /**if the asocket buffer is not big enough to hold HTTP body, use this buffer for the data*/
    u8 * iwd_pu8BodyBuf;
    u32 iwd_u32BodyOffset;

    /**0, start connecting,
      < 0, the connection is being closed
      > 0, connected*/
    olint_t iwd_nClosing;
    olint_t iwd_nBytesLeft;

    u32 iwd_sHeaderLen;

    u32 iwd_u32ExponentialBackoff;

    internal_web_chunkdata_t * iwd_piwcChunk;
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
    u32 u32Index = 0;

    jf_logger_logInfoMsg("destroy web req");

    piwr = (internal_web_request_t *) *ppRequest;

    for (u32Index = 0; u32Index < piwr->iwr_u32NumOfBuffers; ++ u32Index)
    {
        /* free the memory */
        if (piwr->iwr_pu8Buffer[u32Index] != NULL)
            jf_jiukun_freeMemory((void **)&piwr->iwr_pu8Buffer[u32Index]);
    }

    jf_jiukun_freeMemory((void **)ppRequest);

    return u32Ret;
}

static u32 _newWebRequest(
    internal_web_request_t ** ppRequest, u8 ** ppu8Buffer, olsize_t * psBuf, u32 u32Num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * piwr = NULL;
    u32 u32Index = 0;

    jf_logger_logInfoMsg("new web req");

    u32Ret = jf_jiukun_allocMemory((void **)&piwr, sizeof(internal_web_request_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwr, sizeof(internal_web_request_t));

        piwr->iwr_u32NumOfBuffers = u32Num;

        for (u32Index = 0; (u32Index < u32Num) && (u32Ret == JF_ERR_NO_ERROR); ++ u32Index)
        {
            piwr->iwr_sBuffer[u32Index] = psBuf[u32Index];
            u32Ret = jf_jiukun_cloneMemory(
                (void **)&piwr->iwr_pu8Buffer[u32Index], ppu8Buffer[u32Index], psBuf[u32Index]);
        }
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
    internal_webclient_t * piw = piwd->iwd_piwParent;
    olchar_t addr[64];
    olint_t addrlen;

    addrlen = _getStringHashKey(addr, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg("destroy web data obj %s", addr);

    assert(piwd->iwd_pjnaConn == NULL);

#if 0
    if ((piwd->iwd_pjnaConn != NULL) &&
        (! jf_network_isAcsocketFree(piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn)))
    {
        /*This connection needs to be disconnected first*/
        jf_network_disconnectAcsocket(piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn);
    }
#endif

    if (piwd->iwd_pjhphHeader != NULL)
    {
        /*The header needs to be freed*/
        jf_httpparser_destroyPacketHeader(&(piwd->iwd_pjhphHeader));
    }

    if (piwd->iwd_piwcChunk != NULL)
    {
        /*The resources associated with the Chunk Processing needs to be freed*/
        if (piwd->iwd_piwcChunk->iwc_pu8Buffer != NULL)
            jf_jiukun_freeMemory((void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer);
        jf_jiukun_freeMemory((void **)&piwd->iwd_piwcChunk);
    }

    if (piwd->iwd_pu8BodyBuf != NULL)
        jf_jiukun_freeMemory((void **)&piwd->iwd_pu8BodyBuf);

    /*Iterate through all the pending requests*/
    piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    while (piwr != NULL)
    {
        /*If this is a client request, then we need to signal that this request is being aborted*/
        piwr->iwr_fnOnEvent(
            NULL, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL, piwr->iwr_pUser);
        _destroyWebRequest(&piwr);

        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    }
    jf_queue_fini(&piwd->iwd_jqRequest);

    jf_hashtree_deleteEntry(&piw->iw_jhData, addr, addrlen);

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

        addrlen = _getStringHashKey(addr, pjiRemote, u16Port);
        u32Ret = jf_hashtree_addEntry(&piw->iw_jhData, addr, addrlen, piwd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppiwd = piwd;
    else if (piwd != NULL)
        _destroyWebDataobject((webclient_dataobject_t **)&piwd);

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
    olint_t i;
    boolean_t bHashEntry = FALSE, bQueueEmpty = FALSE;

    addrlen = _getStringHashKey(addr, pjiRemote, u16Port);

    jf_logger_logInfoMsg("process web req, %s", addr);

    u32Ret = jf_hashtree_getEntry(&piw->iw_jhData, addr, addrlen, (void **)&piwd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("process web req, has entry");

        /*It does*/
        bQueueEmpty = jf_queue_isEmpty(&piwd->iwd_jqRequest);
        u32Ret = jf_queue_enqueue(&piwd->iwd_jqRequest, request);


    }
    else
    {
        /*There is no previous connection, so we need to set it up*/
        u32Ret = _createWebDataobject(&piwd, NULL, piw, pjiRemote, u16Port);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_queue_enqueue(&piwd->iwd_jqRequest, request);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Connect to remote server, the async socket will wake up the chain so webclient doesn't
              have to care about it*/
            u32Ret = jf_network_connectAcsocketTo(
                piw->iw_pjnaAcsocket, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort, piwd);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg(
            "process web req, WaitForClose %d", piwd->iwd_bWaitForClose);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && bHashEntry && bQueueEmpty)
    {
        /*the web dataobject is already there*/
        /*There are no pending requests however, so we can try to send this right away*/
        piw->iw_u32IdleCount = piw->iw_u32IdleCount == 0 ? 0 : piw->iw_u32IdleCount - 1;
        jf_hashtree_deleteEntry(&piw->iw_jhIdle, addr, addrlen);
        jf_network_removeUtimerItem(piw->iw_pjnuUtimer, piwd);
        if ((piwd->iwd_pjnaConn == NULL) ||
            jf_network_isAcsocketFree(piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn))
        {
            /*If this was in our iw_jhIdle, then most likely the select
              doesn't know about it, so we need to force it to unblock*/
            jf_logger_logInfoMsg("process web req, asocket is not valid");
            u32Ret = jf_queue_enqueue(&piw->iw_jqBacklog, piwd);
        }
        else if (piwd->iwd_pjnaConn != NULL)
        {
            /*Socket is still there*/
            if (! piwd->iwd_bWaitForClose)
            {
                jf_logger_logInfoMsg("process web req, send data");
                for (i = 0; i < request->iwr_u32NumOfBuffers; ++i)
                {
                    jf_network_sendAcsocketData(
                        piw->iw_pjnaAcsocket, piwd->iwd_pjnaConn, request->iwr_pu8Buffer[i],
                        request->iwr_sBuffer[i]);
                }
            }
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
    void * pWebclient, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_web_dataobject_t * piwd;

    /*Try and satisfy as many things as we can. If we have resources grab a socket and go*/
//    jf_mutex_acquire(&(piw->iw_jmLock));
    while (! jf_queue_isEmpty(&piw->iw_jqBacklog))
    {
        piwd = jf_queue_dequeue(&piw->iw_jqBacklog);
        piwd->iwd_nClosing = 0;
        u32Ret = jf_network_connectAcsocketTo(
            piw->iw_pjnaAcsocket, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort, piwd);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            break;
        }
    }
//    jf_mutex_release(&(piw->iw_jmLock));

    return u32Ret;
}

static u32 _webclientTimerInterruptSink(void ** object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}


/** The timed callback is used to close idle sockets. A socket is considered
 *  idle if after a request is answered, another request isn't received 
 *  within the time specified by WEBCLIENT_SESSION_IDLE_TIMEOUT
 *
 *  @param object [in] the web data object
 */
static u32 _webclientTimerSink(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtree_enumerator_t enumerator;
    olchar_t addr[200];
    olint_t addrlen;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;
    internal_web_dataobject_t * piwd2;
    olchar_t * pstrKey;
    olsize_t u32KeyLen;
    void * data;
    void * DisconnectSocket = NULL;

    jf_logger_logInfoMsg("web client timer sink");

    if (jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        jf_logger_logInfoMsg("web client timer sink, queue is empty");
        /*This connection is idle, because there are no pending requests */
        if ((piwd->iwd_pjnaConn != NULL) &&
            (! jf_network_isAcsocketFree(piwd->iwd_piwParent->iw_pjnaAcsocket, piwd->iwd_pjnaConn)))
        {
            /*We need to close this socket*/
            jf_logger_logInfoMsg("web client timer sink, close the connection");
            DisconnectSocket = piwd->iwd_pjnaConn;
            piwd->iwd_pjnaConn = NULL;
        }

        if (piwd->iwd_piwParent->iw_u32IdleCount > MAX_WEBCLIENT_IDLE_SESSIONS)
        {
            /*Remove an entry from iw_jhIdle, if there are too many entries in it*/
            -- piwd->iwd_piwParent->iw_u32IdleCount;

            jf_hashtree_initEnumerator(&piwd->iwd_piwParent->iw_jhIdle, &enumerator);
            jf_hashtree_moveEnumeratorNext(&enumerator);
            jf_hashtree_getEnumeratorValue(&enumerator, &pstrKey, &u32KeyLen, &data);
            jf_hashtree_finiEnumerator(&enumerator);
            jf_hashtree_getEntry(
                &piwd->iwd_piwParent->iw_jhData, pstrKey, u32KeyLen, (void **)&piwd2);

            jf_logger_logInfoMsg("web client timer sink, delete entry with key %s", pstrKey);
            jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhData, pstrKey, u32KeyLen);
            jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhIdle, pstrKey, u32KeyLen);

            _destroyWebDataobject((webclient_dataobject_t **)&piwd2);
        }

        /*Add this DataObject into the iw_jhIdle for use later*/
        addrlen = _getStringHashKey(addr, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
        jf_logger_logInfoMsg(
            "web client timer sink, add data object %s to idle hashtree", addr);
        jf_hashtree_addEntry(
            &piwd->iwd_piwParent->iw_jhIdle, addr, addrlen, piwd);
        ++ piwd->iwd_piwParent->iw_u32IdleCount;
    }

    /*Let the user know, the socket has been disconnected*/
    if (DisconnectSocket != NULL)
    {
        jf_network_disconnectAcsocket(piwd->iwd_piwParent->iw_pjnaAcsocket, DisconnectSocket);
    }

    return u32Ret;
}

/** Internal method called when web client has finished processing a request
 *  or response
 *
 *  @param piwd [in] the associated internal web data object
 */
static u32 _webclientFinishedResponse(internal_web_dataobject_t * piwd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * wr;
    olint_t i;

    jf_logger_logInfoMsg(
        "wc finishd response, pipeline flag %d", piwd->iwd_u32PipelineFlags);

    /*Reset the flags*/
    piwd->iwd_bFinHeader = FALSE;
    piwd->iwd_bChunked = FALSE;
    piwd->iwd_bWaitForClose = FALSE;
    piwd->iwd_bInitialRequestAnswered = TRUE;
    if (piwd->iwd_pu8BodyBuf != NULL)
        jf_jiukun_freeMemory((void **)&piwd->iwd_pu8BodyBuf);
    piwd->iwd_u32BodyOffset = 0;
    if (piwd->iwd_piwcChunk != NULL)
    {
        if (piwd->iwd_piwcChunk->iwc_pu8Buffer != NULL)
            jf_jiukun_freeMemory((void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer);
        jf_jiukun_freeMemory((void **)&piwd->iwd_piwcChunk);
    }
    if (piwd->iwd_pjhphHeader != NULL)
        jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);

    wr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    _destroyWebRequest(&wr);
    wr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (wr == NULL)
    {
        /*Since the request queue is empty, that means this connection is now
          idle. Set a timed callback, so we can free this resource if
          neccessary*/
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, WEBCLIENT_SESSION_IDLE_TIMEOUT,
            _webclientTimerSink, _webclientTimerInterruptSink);
    }
    else
    {
        jf_logger_logInfoMsg("wc finish response, queue is not empty");
        /*There are still pending requests in the queue, so try to send them*/
        if (piwd->iwd_u32PipelineFlags == PIPELINE_NO)
        {
            /*Pipelining is not supported, so we should just close the socket,
              instead of waiting for the other guy to close it, because if they
              forget to, it will screw us over if there are pending requests */

            /*It should also be noted, that when this closes, the module will
              realize there are pending requests, in which case it will open a
              new connection for the requests.*/
            jf_logger_logInfoMsg(
                "wc finish response, pipeline is no, disconnect and add data object to backlog");
            jf_network_disconnectAcsocket(piwd->iwd_piwParent->iw_pjnaAcsocket, piwd->iwd_pjnaConn);
            piwd->iwd_pjnaConn = NULL;
            u32Ret = jf_queue_enqueue(&piwd->iwd_piwParent->iw_jqBacklog, piwd);
        }
        else
        {
            /*If the connection is still open, and we didn't flag this as not
              supporting persistent connections, than obviously it is
              supported*/
            jf_logger_logInfoMsg("wc finish response, pipeline is yes");
            piwd->iwd_u32PipelineFlags = PIPELINE_YES;

            for (i = 0; i < wr->iwr_u32NumOfBuffers; ++i)
            {
                /*Try to send the request*/
                jf_network_sendAcsocketData(
                    piwd->iwd_piwParent->iw_pjnaAcsocket, piwd->iwd_pjnaConn, wr->iwr_pu8Buffer[i],
                    wr->iwr_sBuffer[i]);
            }
        }
    }

    return u32Ret;
}

/** Internal method called to decode chunked transfers
 *
 *  @param pAsocket [in] the async socket
 *  @param piwd [in] The associated web data object
 *  @param buffer [in] the receive buffer
 *  @param psBeginPointer [out] the buffer start pointer
 *  @param endPointer [in] the length of the buffer
 */
static u32 _processChunk(
    jf_network_asocket_t * pAsocket, internal_web_dataobject_t * piwd, u8 * buffer,
    olsize_t * psBeginPointer, olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t *hex;
    olint_t i;
    olsize_t sBeginPointer = *psBeginPointer;
    internal_web_request_t * piwr;

    jf_logger_logInfoMsg("process chunk %d:%d", *psBeginPointer, endPointer);

    piwr = jf_queue_peek(&piwd->iwd_jqRequest);

    if (piwd->iwd_piwcChunk == NULL)
    {
        // Create a state object for the Chunk Processor
        u32Ret = jf_jiukun_allocMemory(
            (void **)&piwd->iwd_piwcChunk, sizeof(internal_web_chunkdata_t));
        if (u32Ret != JF_ERR_NO_ERROR)
            return u32Ret;

        ol_bzero(piwd->iwd_piwcChunk, sizeof(internal_web_chunkdata_t));

        u32Ret = jf_jiukun_allocMemory(
            (void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer, piwd->iwd_piwParent->iw_sBuffer);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            jf_jiukun_freeMemory((void **)&piwd->iwd_piwcChunk);
            return u32Ret;
        }
        piwd->iwd_piwcChunk->iwc_u32MallocSize = WEBCLIENT_INITIAL_BUFFER_SIZE;
    }

    while (*psBeginPointer < endPointer)
    {
        switch (piwd->iwd_piwcChunk->iwc_u32Flags)
        {
            /*Based on the Chunk Flag, we can figure out how to parse this thing*/
        case STARTCHUNK:
            jf_logger_logInfoMsg("process chunk, STARTCHUNK");
            /*Reading Chunk Header*/
            if (endPointer < 3)
            {
                return u32Ret;
            }
            for (i = 2; i < endPointer; ++i)
            {
                if (buffer[i - 2] == '\r' && buffer[i - 1] == '\n')
                {
                    /*The chunk header is terminated with a CRLF. The part before
                      the CRLF is the hex number representing the length of the
                      chunk*/
                    buffer[i - 2] = '\0';
                    piwd->iwd_piwcChunk->iwc_nBytesLeft =
                        (olint_t) strtol((olchar_t *)buffer, &hex, 16);
                    jf_logger_logInfoMsg(
                        "process chunk, chunk size %d", piwd->iwd_piwcChunk->iwc_nBytesLeft);
                    *psBeginPointer = i;
                    piwd->iwd_piwcChunk->iwc_u32Flags =
                        piwd->iwd_piwcChunk->iwc_nBytesLeft == 0 ? FOOTERCHUNK : DATACHUNK;
                    break;
                }
            }
            break;
        case ENDCHUNK:
            jf_logger_logInfoMsg("process chunk, ENDCHUNK");
            if (endPointer >= 2)
            {
                /*There is more chunks to come*/
                *psBeginPointer = 2;
                piwd->iwd_piwcChunk->iwc_u32Flags = STARTCHUNK;
            }
            break;
        case DATACHUNK:
            jf_logger_logInfoMsg("process chunk, DATACHUNK");
            if (endPointer >= piwd->iwd_piwcChunk->iwc_nBytesLeft)
            {
                /*Only consume what we need*/
                piwd->iwd_piwcChunk->iwc_u32Flags = ENDCHUNK;
                i = piwd->iwd_piwcChunk->iwc_nBytesLeft;
            }
            else
            {
                /*Consume all of the data*/
                i = endPointer;
            }

            if (piwd->iwd_piwcChunk->iwc_u32Offset + endPointer >
                piwd->iwd_piwcChunk->iwc_u32MallocSize)
            {
                /*The buffer is too small, we need to make it bigger
                  ToDo: Add code to enforce a max buffer size if specified
                  ToDo: Does the above layer need to know when buffers were realloced? */
                jf_logger_logInfoMsg("process chunk, realloc memory");
                piwd->iwd_piwcChunk->iwc_pu8Buffer = (u8 *) realloc(
                    piwd->iwd_piwcChunk->iwc_pu8Buffer,
                    piwd->iwd_piwcChunk->iwc_u32MallocSize + piwd->iwd_piwParent->iw_sBuffer);
                piwd->iwd_piwcChunk->iwc_u32MallocSize += piwd->iwd_piwParent->iw_sBuffer;
            }

            /*Write the decoded chunk blob into the buffer*/
            memcpy(piwd->iwd_piwcChunk->iwc_pu8Buffer +
                   piwd->iwd_piwcChunk->iwc_u32Offset, buffer, i);
            assert(piwd->iwd_piwcChunk->iwc_u32Offset + i <=
                   piwd->iwd_piwcChunk->iwc_u32MallocSize);

            /*Adjust our counters*/
            piwd->iwd_piwcChunk->iwc_nBytesLeft -= i;
            piwd->iwd_piwcChunk->iwc_u32Offset += i;

            *psBeginPointer = i;
            break;
        case FOOTERCHUNK:
            jf_logger_logInfoMsg("process chunk, FOOTERCHUNK");
            if (endPointer >= 2)
            {
                for (i = 2; i <= endPointer; ++i)
                {
                    if (buffer[i - 2] == '\r' && buffer[i - 1] == '\n')
                    {
                        /*An empty line means the chunk is finished*/
                        if (i == 2)
                        {
                            /*FINISHED*/
                            piwd->iwd_pjhphHeader->jhph_pu8Body =
                                piwd->iwd_piwcChunk->iwc_pu8Buffer;
                            piwd->iwd_pjhphHeader->jhph_sBody = piwd->iwd_piwcChunk->iwc_u32Offset;

                            piwr->iwr_fnOnEvent(
                                piwd->iwd_pjnaConn, 0, piwd->iwd_pjhphHeader, piwr->iwr_pUser);
                            _webclientFinishedResponse(piwd);
                            *psBeginPointer = 2;

                            break;
                        }
                        else
                        {
                            /*ToDo: This is where to add code to add support trailers*/
                        }
                    }
                }
            }
            break;
        }

        endPointer -= *psBeginPointer;
        buffer += *psBeginPointer;
        sBeginPointer += *psBeginPointer;
        *psBeginPointer = 0;
    }

    *psBeginPointer = sBeginPointer;

    return u32Ret;
}

/** Internal method dispatched by the utimer, to retry refused connections
 *
 *  @note The module does an exponential backoff, when retrying connections. The
 *  number of retries is determined by the value of WEBCLIENT_CONNECT_RETRY_COUNT
 *
 *  @param object [in] the associated web data object
 */
static u32 _retrySink(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;
//    internal_webclient_t *wcm = piwd->iwd_piwParent;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = _getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16RemotePort);
    jf_logger_logInfoMsg("retry sink, %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) WEBCLIENT_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        jf_logger_logInfoMsg("retry sink, give up");

        jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhData, key, keyLength);
        jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhIdle, key, keyLength);

        _destroyWebDataobject((webclient_dataobject_t **)&piwd);
    }
    else
    {
        /*Lets retry again*/
        jf_logger_logInfoMsg("retry sink, retry later");
        u32Ret = jf_queue_enqueue(&piwd->iwd_piwParent->iw_jqBacklog, piwd);
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
    olint_t i;

    piwd->iwd_pjnaConn = pAsocket;
    piwd->iwd_bInitialRequestAnswered = FALSE;

    if (u32Status == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("web client connected");
        //Success: Send First Request
        jf_network_getLocalInterfaceOfAcsocket(
            piwd->iwd_piwParent->iw_pjnaAcsocket, pAsocket, &piwd->iwd_jiLocal);
        piwr = jf_queue_peek(&piwd->iwd_jqRequest);
        for (i = 0; i < piwr->iwr_u32NumOfBuffers; ++i)
        {
            jf_network_sendAcsocketData(
                piwd->iwd_piwParent->iw_pjnaAcsocket, pAsocket, piwr->iwr_pu8Buffer[i],
                piwr->iwr_sBuffer[i]);
        }
    }
    else
    {
        jf_logger_logInfoMsg("web client connect failed, retry later");
        /*The connection failed, so lets set a timed callback, and try again*/
        piwd->iwd_u32PipelineFlags = PIPELINE_UNKNOWN;
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd,
            piwd->iwd_u32ExponentialBackoff, _retrySink, NULL);
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
//    u8 * buffer;
//    olsize_t BeginPointer, EndPointer;
//    jf_httpparser_packet_header_t * h;

    jf_logger_logInfoMsg(
        "web client disconnect, WaitForClose %d, PipelineFlags %d",
        piwd->iwd_bWaitForClose, piwd->iwd_u32PipelineFlags);

    if (! jf_queue_isEmpty(&piwd->iwd_jqRequest))
    {
        /*If there are still pending requests, then obviously this server doesn't do persistent
          connections*/
        jf_logger_logInfoMsg("web client disconnect, pipeline is no");
        piwd->iwd_u32PipelineFlags = PIPELINE_NO;
    }

    if (piwd->iwd_bWaitForClose)
    {
        /*Since we had to read until the socket closes, we finally have all the data we need*/
        piwd->iwd_bInitialRequestAnswered = TRUE;
        piwd->iwd_u32PipelineFlags = PIPELINE_NO;
        piwd->iwd_bFinHeader = FALSE;

        piwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
        if (piwr != NULL)
        {
            piwr->iwr_fnOnEvent(pAsocket, 0, piwd->iwd_pjhphHeader, piwr->iwr_pUser);
            //_webclientFinishedResponse(piwd);        
            _destroyWebRequest(&piwr);
        }

        if (piwd->iwd_pjhphHeader != NULL)
            jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);
    }

    piwd->iwd_pjnaConn = NULL;

    piwr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr != NULL)
    {
        /*There are still requests to be made, make another connection and continue*/
        jf_logger_logInfoMsg("web client disconnect, retry later");
        jf_network_addUtimerItem(
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd,
            piwd->iwd_u32ExponentialBackoff, _retrySink, NULL);
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
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("web client send ok");

    return u32Ret;
}

static u32 _parseHttpHeader(
     jf_network_asocket_t * pAsocket, internal_web_dataobject_t * piwd,
     internal_web_request_t * wr, u8 * pu8Buffer,
     olsize_t * psBeginPointer, olsize_t sEndPointer, olsize_t sHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph;
    jf_httpparser_packet_header_field_t * pjhphf;
    olsize_t zero = 0;

    jf_logger_logInfoMsg("parse http header");
    /*Headers are delineated with a CRLF, and terminated with an empty line*/
    piwd->iwd_sHeaderLen = sHeader + 3;
    piwd->iwd_bWaitForClose = TRUE;
    piwd->iwd_nBytesLeft = -1;
    piwd->iwd_bFinHeader = TRUE;
    u32Ret = jf_httpparser_parsePacketHeader(
        &piwd->iwd_pjhphHeader, (olchar_t *)pu8Buffer, *psBeginPointer,
        sEndPointer - (*psBeginPointer));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Introspect Request, to see what to do next*/
        pjhphf = piwd->iwd_pjhphHeader->jhph_pjhphfFirst;
        while (pjhphf != NULL)
        {
            if (pjhphf->jhphf_sName == 17 &&
                ol_strncasecmp(pjhphf->jhphf_pstrName, "transfer-encoding", 17) == 0)
            {
                if (pjhphf->jhphf_sData == 7 &&
                    ol_strncasecmp(pjhphf->jhphf_pstrData, "chunked", 7) == 0)
                {
                    /*This packet was chunk encoded*/
                    piwd->iwd_bWaitForClose = FALSE;
                    piwd->iwd_bChunked = TRUE;
                    jf_logger_logInfoMsg("parse http header, chunk");
                }
            }
            else if (pjhphf->jhphf_sName == 14 &&
                     ol_strncasecmp(pjhphf->jhphf_pstrName, "content-length", 14) == 0)
            {
                /*This packet has a Content-Length*/
                piwd->iwd_bWaitForClose = FALSE;
                pjhphf->jhphf_pstrData[pjhphf->jhphf_sData] = '\0';
                piwd->iwd_nBytesLeft = atoi(pjhphf->jhphf_pstrData);
                jf_logger_logInfoMsg(
                    "parse http header, content-length %d", piwd->iwd_nBytesLeft);
            }
            pjhphf = pjhphf->jhphf_pjhphfNext;
        }
        if (piwd->iwd_nBytesLeft == -1 && (! piwd->iwd_bChunked))
        {
            /*This request has no body*/
            piwd->iwd_nBytesLeft = 0;
        }
        if (piwd->iwd_nBytesLeft == 0)
        {
            /*We already have the complete Response Packet*/
            wr->iwr_fnOnEvent(
                pAsocket, JF_WEBCLIENT_EVENT_INCOMING_DATA, piwd->iwd_pjhphHeader, wr->iwr_pUser);
            *psBeginPointer = *psBeginPointer + sHeader + 4;
            _webclientFinishedResponse(piwd);
        }
        else
        {
            /*There is still data we need to read.
              Lets see if any of the body arrived yet*/
            if (! piwd->iwd_bChunked)
            {
                /*This isn't chunked, so we can process normally*/
                if (piwd->iwd_nBytesLeft != -1 &&
                    (sEndPointer - (*psBeginPointer)) - (sHeader + 4) >=
                    piwd->iwd_nBytesLeft)
                {
                    jf_logger_logInfoMsg("parse http header, got entire packet");
                    piwd->iwd_pjhphHeader->jhph_pu8Body = pu8Buffer + sHeader + 4;
                    piwd->iwd_pjhphHeader->jhph_sBody = piwd->iwd_nBytesLeft;
                    /*We have the entire body, so we have the entire packet*/
                    wr->iwr_fnOnEvent(
                        pAsocket, 0, piwd->iwd_pjhphHeader, wr->iwr_pUser);
                    *psBeginPointer =
                        *psBeginPointer + sHeader + 4 + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
                else
                {
                    /*We read some of the body, but not all of it yet*/
                    jf_logger_logInfoMsg("parse http header, got partial packet");
                    piwd->iwd_sHeaderLen = 0;
                    *psBeginPointer = sHeader + 4;
                    u32Ret = jf_httpparser_clonePacketHeader(&pjhph, piwd->iwd_pjhphHeader);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);
                        piwd->iwd_pjhphHeader = pjhph;
                    }

                    if ((u32Ret == JF_ERR_NO_ERROR) &&
                        (piwd->iwd_nBytesLeft >
                         piwd->iwd_piwParent->iw_sBuffer))
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
                /*This packet is chunk encoded, so we need to run it through our
                  Chunk Processor*/
                _processChunk(
                    pAsocket, piwd, pu8Buffer + sHeader + 4, &zero,
                    (sEndPointer - (*psBeginPointer) - (sHeader + 4)));
                *psBeginPointer = sHeader + 4 + zero;
                /*the chunk is not freed, it means the chunk is not completed
                  processed*/
                if (piwd->iwd_piwcChunk != NULL)
                {
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
    else
    {
        /*ToDo: There was an error parsing the headers.
          Right now, we don't care*/
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
    olint_t i = 0;
    olint_t Fini;

    jf_logger_logInfoMsg(
        "web client data, %d:%d, WaitForClose %d",
        *psBeginPointer, sEndPointer, piwd->iwd_bWaitForClose);
/*
    jf_logger_jf_logger_logDataMsgWithAscii(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer,
        "web client data");
*/
    piwr = (internal_web_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
    if (piwr == NULL)
    {
        jf_logger_logInfoMsg("web client data, ignore");
        /*There are no pending requests, so we have no idea what we are
          supposed to do with this data, other than just recycling the receive
          buffer, so we don't leak memory. If this code executes, this usually
          signifies a processing error of some sort. Most of the time, it means
          the remote endpoolint_t is sending invalid packets.*/
        *psBeginPointer = sEndPointer;
        return u32Ret;
    }
    if (! piwd->iwd_bFinHeader)
    {
        /*Still Reading Headers*/
        if (sEndPointer - (*psBeginPointer) >= 4)
        {
            while (i <= (sEndPointer - (*psBeginPointer)) - 4)
            {
                if (pu8Buffer[*psBeginPointer + i] == '\r' &&
                    pu8Buffer[*psBeginPointer + i + 1] == '\n' &&
                    pu8Buffer[*psBeginPointer + i + 2] == '\r' &&
                    pu8Buffer[*psBeginPointer + i + 3] == '\n')
                {
                    u32Ret = _parseHttpHeader(
                        pAsocket, piwd, piwr, pu8Buffer, psBeginPointer, sEndPointer, i);
                    break;
                }

                ++i;
            }
        }
    }
    else
    {
        /*We already processed the headers, so we are only expecting the
          body now*/
        if (! piwd->iwd_bChunked)
        {
            /*This isn't chunk encoded*/
            if (piwd->iwd_pu8BodyBuf == NULL)
            {
                /*the asocket buffer can hold the whole body*/
                Fini = sEndPointer - *psBeginPointer - piwd->iwd_nBytesLeft;
                jf_logger_logInfoMsg(
                    "web client data, body in asocket buffer, Fini %d", Fini);
                if (Fini >= 0)
                {
                    jf_httpparser_setBody(
                        piwd->iwd_pjhphHeader, pu8Buffer + *psBeginPointer,
                        piwd->iwd_nBytesLeft, FALSE);
                    piwr->iwr_fnOnEvent(
                        pAsocket, 0, piwd->iwd_pjhphHeader, piwr->iwr_pUser);
                    *psBeginPointer = *psBeginPointer + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
            }
            else
            {
                Fini = piwd->iwd_u32BodyOffset +
                    sEndPointer - *psBeginPointer - piwd->iwd_nBytesLeft;
                jf_logger_logInfoMsg(
                    "web client data, body in data object buffer, Fini %d, offset %u",
                    Fini, piwd->iwd_u32BodyOffset);
                if (Fini >= 0)
                {
                    Fini = piwd->iwd_nBytesLeft - piwd->iwd_u32BodyOffset;
                    memcpy(piwd->iwd_pu8BodyBuf + piwd->iwd_u32BodyOffset,
                           pu8Buffer + *psBeginPointer, Fini);
                    jf_httpparser_setBody(
                        piwd->iwd_pjhphHeader, piwd->iwd_pu8BodyBuf,
                        piwd->iwd_nBytesLeft, FALSE);
                    piwr->iwr_fnOnEvent(
                        pAsocket, 0, piwd->iwd_pjhphHeader, piwr->iwr_pUser);
                    *psBeginPointer = *psBeginPointer + Fini;
                    _webclientFinishedResponse(piwd);

                }
                else
                {
                    Fini = sEndPointer - *psBeginPointer;
                    memcpy(piwd->iwd_pu8BodyBuf + piwd->iwd_u32BodyOffset,
                           pu8Buffer + *psBeginPointer, Fini);
                    piwd->iwd_u32BodyOffset += Fini;
                    *psBeginPointer = sEndPointer;
                }
            }
        }
        else
        {
            /*This is chunk encoded, so run it through our Chunk Processor*/
            _processChunk(pAsocket, piwd, pu8Buffer, psBeginPointer, sEndPointer);
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
    jf_hashtree_enumerator_t jhe;
    webclient_dataobject_t * piwd = NULL;
    olchar_t * pstrKey;
    olsize_t u32KeyLen;

    /*Iterate through all the web data objects*/
    jf_hashtree_initEnumerator(&piw->iw_jhData, &jhe);
    while (! jf_hashtree_isEnumeratorEmptyNode(&jhe))
    {
        /*Free the web data object*/
        jf_hashtree_getEnumeratorValue(&jhe, &pstrKey, &u32KeyLen, &piwd);
        _destroyWebDataobject(&piwd);
        jf_hashtree_moveEnumeratorNext(&jhe);
    }
    jf_hashtree_finiEnumerator(&jhe);

    if (piw->iw_pjnaAcsocket != NULL)
    {
        jf_network_destroyAcsocket(&piw->iw_pjnaAcsocket);
    }

    /*Free all the other associated resources*/
    jf_queue_fini(&piw->iw_jqBacklog);
    jf_hashtree_fini(&piw->iw_jhIdle);
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
           (pjwcp->jwcp_u32PoolSize > 0) && (pjwcp->jwcp_u32PoolSize < MAX_WEBCLIENT_CONNECTION));

    u32Ret = jf_jiukun_allocMemory((void **)&piw, sizeof(internal_webclient_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piw, sizeof(internal_webclient_t));
        piw->iw_jncohHeader.jncoh_fnPreSelect = _preWebclientProcess;
        piw->iw_pjncChain = pjnc;
        piw->iw_sBuffer = pjwcp->jwcp_sBuffer ? pjwcp->jwcp_sBuffer : WEBCLIENT_INITIAL_BUFFER_SIZE;
        piw->iw_u32PoolSize = pjwcp->jwcp_u32PoolSize;

        jf_hashtree_init(&piw->iw_jhIdle);
        jf_hashtree_init(&piw->iw_jhData);
        jf_queue_init(&piw->iw_jqBacklog);

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
    internal_web_request_t * request = NULL;

    jf_logger_logDebugMsg("pipeline web req");

    u32Ret = _newWebRequest(&request, NULL, NULL, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_httpparser_getRawPacket(
            packet, (olchar_t **)&request->iwr_pu8Buffer[0], &request->iwr_sBuffer[0]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        request->iwr_u32NumOfBuffers = 1;
        request->iwr_fnOnEvent = (fnOnEvent == NULL) ? _internalWebclientOnEvent : fnOnEvent;
        request->iwr_pUser = user;

        u32Ret = _processWebRequest(piw, pjiRemote, u16Port, request);
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
    u32 u32NumOfBuffers;
    u8 * pu8Buffer[2];
    olsize_t sBuffer[2];

    jf_logger_logInfoMsg("pipeline web req ex");
    jf_logger_logDataMsgWithAscii((u8 *)pstrHeader, sHeader, "HTTP request header:");
    if (pstrBody != NULL)
        jf_logger_logDataMsgWithAscii((u8 *)pstrBody, sBody, "HTTP request body:");

    u32NumOfBuffers = (pstrBody != NULL) ? 2 : 1;

    pu8Buffer[0] = (u8 *)pstrHeader;
    sBuffer[0] = sHeader;

    if (pstrBody != NULL)
    {
        pu8Buffer[1] = (u8 *)pstrBody;
        sBuffer[1] = sBody;
    }

    u32Ret = _newWebRequest(&piwr, pu8Buffer, sBuffer, u32NumOfBuffers);
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
//    jf_mutex_acquire(&(piw->iw_jmLock));
    if (jf_hashtree_hasEntry(&piw->iw_jhData, addr, addrlen))
    {
        /* Yes, iterate through them */
        jf_hashtree_getEntry(&piw->iw_jhData, addr, addrlen, (void **)&piwd);
        while (! jf_queue_isEmpty(&piwd->iwd_jqRequest))
        {
            /*Put all the pending requests into this queue, so we can
              trigger them outside of this lock*/
            piwr = (internal_web_request_t *)jf_queue_dequeue(&piwd->iwd_jqRequest);
            u32Ret = jf_queue_enqueue(&jqRemove, piwr);
        }
    }
//    jf_mutex_release(&(piw->iw_jmLock));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* Lets iterate through all the requests that we need to get rid of */
        while (! jf_queue_isEmpty(&jqRemove))
        {
            /* Tell the user, we are aborting these requests */
            piwr = (internal_web_request_t *) jf_queue_dequeue(&jqRemove);
            piwr->iwr_fnOnEvent(
                pWebclient, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL, piwr->iwr_pUser);

            _destroyWebRequest(&piwr);
        }
    }

    jf_queue_fini(&jqRemove);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

