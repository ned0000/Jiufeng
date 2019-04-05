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

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "syncmutex.h"
#include "network.h"
#include "httpparser.h"
#include "webclient.h"
#include "bases.h"
#include "xmalloc.h"
#include "stringparse.h"
#include "hexstr.h"

/* --- private data/data structure section --------------------------------- */
/** Keep a table of all the connections. This is the maximum number allowed
 *  to be idle. Since we have in the constructor a pool size, this feature may
 *  be depracted. ToDo: Look into depracating this
 */
#define MAX_IDLE_SESSIONS             (20)

/** This is the number of seconds that a connection must be idle for, before
 *  it will be automatically closed. Idle means there are no pending requests
 */
#define WC_HTTP_SESSION_IDLE_TIMEOUT  (20)

/** This is the number of times, an HTTP connection will be attempted,
 *  before it fails. This module utilizes an exponential backoff algorithm.
 *  That is, it will retry immediately, then it will retry after 1 second,
 *  then 2, then 4, etc.
 */
#define HTTP_CONNECT_RETRY_COUNT      (4)

/** This initial size of the receive buffer
 */
#define INITIAL_BUFFER_SIZE           (2048)

enum pipeline_type
{
    PIPELINE_UNKNOWN = 0, /**< doesn't know yet if the server supports
                             persistent connections */
    PIPELINE_YES, /**< The server does indeed support persistent connections */
    PIPELINE_NO, /**< The server does not support persistent connections*/
};

/** Chunk processing flags
 */
#define STARTCHUNK      (0)
#define ENDCHUNK        (1)
#define DATACHUNK       (2)
#define FOOTERCHUNK     (3)

typedef void  jf_webclient_dataobject_t;

typedef struct internal_web_request
{
#define IWR_MAX_NUM_OF_BUF  8
    olint_t iwr_u32NumOfBuffers;
    u8 * iwr_pu8Buffer[IWR_MAX_NUM_OF_BUF];
    olsize_t iwr_sBuffer[IWR_MAX_NUM_OF_BUF];
    /**If UserFree is TRUE, webclient will not free the buffer
       If UserFree is FALSE, webclient will free the buffer*/
    boolean_t iwr_bUserFree[IWR_MAX_NUM_OF_BUF];
    jf_network_mem_owner_t iwr_jnmoMemOwner[IWR_MAX_NUM_OF_BUF];

    u16 iwr_u16Reserved[4];

    void * iwr_pUser;
    jf_webclient_fnOnResponse_t iwr_fnOnResponse;
} internal_web_request_t;

typedef struct internal_webclient
{
    jf_network_chain_object_header_t iw_jncohHeader;

    jf_network_asocket_t ** iw_ppjnaAsockets;
    u32 iw_u32NumOfAsockets;

    jf_hashtree_t iw_jhData;
    /**The web data object is put to idle hash tree when no more
       request in the queue of the web data object*/
    jf_hashtree_t iw_jhIdle;
    /**The web data object is put the backlog queue when the object doesn't
       connected to remote server*/
    jf_queue_t iw_jqBacklog;

    jf_network_utimer_t * iw_pjnuUtimer;
    u32 iw_u32IdleCount;

    olsize_t iw_sBuffer;

    jf_network_chain_t *iw_pjncChain;
    jf_mutex_t iw_jmLock;
} internal_webclient_t;

typedef struct internal_web_chunkdata
{
    u32 iwc_u32Flags;
    u8 * iwc_pu8Buffer;
    u32 iwc_u32Offset;
    u32 iwc_u32MallocSize;

    olint_t iwc_nBytesLeft;
} internal_web_chunkdata_t;

typedef struct internal_web_dataobject
{
    u32 iwd_u32PipelineFlags;
    u32 iwd_u32ActivityCounter;

    jf_ipaddr_t iwd_jiRemote;
    u16 iwd_u16Port;
    u16 iwd_u16Reserved[3];

    internal_webclient_t * iwd_piwParent;
    /*we've got the HTTP header*/
    boolean_t iwd_bFinHeader;
    /*the data in HTTP body is chunked*/
    boolean_t iwd_bChunked;
    /*set to true when we get response*/
    boolean_t iwd_bWaitForClose;
    boolean_t iwd_bInitialRequestAnswered;
    boolean_t iwd_bPause;
    u8 iwd_u8Reserved[3];

    /*if the asocket buffer is not big enough to hold HTTP body, use this buffer
     for the data*/
    u8 * iwd_pu8BodyBuf;
    u32 iwd_u32BodyOffset;

    /*0, start connecting,
      < 0, the connection is being closed
      > 0, connected*/
    olint_t iwd_nClosing;
    olint_t iwd_nBytesLeft;

    u32 iwd_sHeaderLen;

    u32 iwd_u32ExponentialBackoff;

    internal_web_chunkdata_t * iwd_piwcChunk;
    jf_httpparser_packet_header_t * iwd_pjhphHeader;

    jf_queue_t iwd_jqRequest;
    jf_network_asocket_t * iwd_pjnaSock;

    jf_ipaddr_t iwd_jiLocal;

} internal_web_dataobject_t;

/* --- private routine section---------------------------------------------- */

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
    internal_web_request_t * piwr;
    olint_t i;

    jf_logger_logInfoMsg("destroy web req");

    piwr = (internal_web_request_t *) *ppRequest;

    for (i = 0; i < piwr->iwr_u32NumOfBuffers; ++i)
    {
        /*If we own the memory, we need to free it*/
        if (! piwr->iwr_bUserFree[i])
            xfree((void **)&piwr->iwr_pu8Buffer[i]);
    }

    xfree((void **)ppRequest);

    return u32Ret;
}

static u32 _newWebRequest(internal_web_request_t ** ppRequest, u32 u32Num)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_request_t * piwr;

    jf_logger_logInfoMsg("new web req");

    u32Ret = xmalloc((void **)&piwr, sizeof(internal_web_request_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(piwr, 0, sizeof(internal_web_request_t));

        piwr->iwr_u32NumOfBuffers = u32Num;


    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        _destroyWebRequest(&piwr);

    return u32Ret;
}

/** Free resources associated with a web data object
 *
 *  @param ppDataobject [in/out] The web data object to free
 */
static u32 _destroyWebDataobject(jf_webclient_dataobject_t ** ppDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) *ppDataobject;
    internal_web_request_t * wr;
    boolean_t zero = FALSE;
    olchar_t addr[64];
//    olint_t addrlen;

    _getStringHashKey(addr, &piwd->iwd_jiRemote, piwd->iwd_u16Port);
    jf_logger_logInfoMsg("destroy web data obj %s", addr);

    if ((piwd->iwd_pjnaSock != NULL) &&
        (! jf_network_isAsocketFree(piwd->iwd_pjnaSock)))
    {
        /*This connection needs to be disconnected first*/
        jf_network_disconnectAsocket(piwd->iwd_pjnaSock);
    }

    if (piwd->iwd_pjhphHeader != NULL)
    {
        /*The header needs to be freed*/
        jf_httpparser_destroyPacketHeader(&(piwd->iwd_pjhphHeader));
    }

    if (piwd->iwd_piwcChunk != NULL)
    {
        /*The resources associated with the Chunk Processing needs to be freed*/
        if (piwd->iwd_piwcChunk->iwc_pu8Buffer != NULL)
            xfree((void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer);
        xfree((void **)&piwd->iwd_piwcChunk);
    }

    if (piwd->iwd_pu8BodyBuf != NULL)
        xfree((void **)&piwd->iwd_pu8BodyBuf);

    /*Iterate through all the pending requests*/
    wr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    while (wr != NULL)
    {
        /*If this is a client request, then we need to signal
          that this request is being aborted*/
        wr->iwr_fnOnResponse(
            NULL, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL,
            wr->iwr_pUser, &zero);
        _destroyWebRequest(&wr);

        wr = jf_queue_dequeue(&piwd->iwd_jqRequest);
    }
    jf_queue_fini(&piwd->iwd_jqRequest);

    xfree((void **)ppDataobject);

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

    u32Ret = xcalloc((void **)&piwd, sizeof(internal_web_dataobject_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_queue_init(&piwd->iwd_jqRequest);
        piwd->iwd_pjnaSock = pAsocket;
        piwd->iwd_piwParent = piw;
        memcpy(&piwd->iwd_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        piwd->iwd_u16Port = u16Port;

        addrlen = _getStringHashKey(addr, pjiRemote, u16Port);
        u32Ret = jf_hashtree_addEntry(&piw->iw_jhData, addr, addrlen, piwd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppiwd = piwd;
    else if (piwd != NULL)
        _destroyWebDataobject((jf_webclient_dataobject_t **)&piwd);

    return u32Ret;
}

static u32 _processWebRequest(
    internal_webclient_t * piw, jf_ipaddr_t * pjiRemote, u16 u16Port,
    internal_web_request_t * request)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bForceUnBlock = FALSE;
    olchar_t addr[64];
    olint_t addrlen;
    internal_web_dataobject_t *piwd;
    olint_t i;
    boolean_t bHashEntry = FALSE, bQueueEmpty = FALSE;

    addrlen = _getStringHashKey(addr, pjiRemote, u16Port);

    jf_logger_logInfoMsg("process web req, %s", addr);

    bHashEntry = jf_hashtree_hasEntry(&piw->iw_jhData, addr, addrlen);
    if (bHashEntry)
    {
        jf_logger_logInfoMsg("process web req, has entry");
        /*It does*/
        jf_hashtree_getEntry(
            &piw->iw_jhData, addr, addrlen, (void **)&piwd);
    }
    else
    {
        /*There is no previous connection, so we need to set it up*/
        u32Ret = _createWebDataobject(&piwd, NULL, piw, pjiRemote, u16Port);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Queue it up in our Backlog, because we don't want to burden
              ourselves, so we need to see if we have the resources for it.
              The Pool will grab one when it can. The chain doesn't know
              about us, so we need to force it to unblock, to process this*/
            bForceUnBlock = TRUE;
            u32Ret = jf_queue_enqueue(&piw->iw_jqBacklog, piwd);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg(
            "process web req, WaitForClose %d", piwd->iwd_bWaitForClose);

        bQueueEmpty = jf_queue_isEmpty(&piwd->iwd_jqRequest);

        u32Ret = jf_queue_enqueue(&piwd->iwd_jqRequest, request);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && bHashEntry && bQueueEmpty)
    {
        /*the web dataobject is already there*/
        /*There are no pending requests however, so we can try to
          send this right away*/
        /*Take out of Idle State*/
        piw->iw_u32IdleCount =
            piw->iw_u32IdleCount == 0 ? 0 : piw->iw_u32IdleCount - 1;
        jf_hashtree_deleteEntry(&piw->iw_jhIdle, addr, addrlen);
        jf_network_removeUtimerItem(piw->iw_pjnuUtimer, piwd);
        if ((piwd->iwd_pjnaSock == NULL) ||
            jf_network_isAsocketFree(piwd->iwd_pjnaSock))
        {
            /*If this was in our iw_jhIdle, then most likely the select
              doesn't know about it, so we need to force it to unblock*/
            jf_logger_logInfoMsg("process web req, asocket is not valid");
            bForceUnBlock = TRUE;
            u32Ret = jf_queue_enqueue(&piw->iw_jqBacklog, piwd);
        }
        else if (piwd->iwd_pjnaSock != NULL)
        {
            /*Socket is still there*/
            if (! piwd->iwd_bWaitForClose)
            {
                jf_logger_logInfoMsg("process web req, send data");
                for (i = 0; i < request->iwr_u32NumOfBuffers; ++i)
                {
                    jf_network_sendAsocketData(
                        piwd->iwd_pjnaSock, request->iwr_pu8Buffer[i],
                        request->iwr_sBuffer[i], request->iwr_jnmoMemOwner[i]);
                }
            }
        }
    }

    if (bForceUnBlock)
    {
        jf_network_wakeupChain(piw->iw_pjncChain);
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
    olint_t i;
    boolean_t bOK = FALSE;

    /*Try and satisfy as many things as we can. If we have resources
      grab a socket and go*/
//    jf_mutex_acquire(&(piw->iw_jmLock));
    while ((! bOK) && (! jf_queue_isEmpty(&piw->iw_jqBacklog)))
    {
        bOK = TRUE;
        for (i = 0; i < piw->iw_u32NumOfAsockets; ++i)
        {
            if (jf_network_isAsocketFree(piw->iw_ppjnaAsockets[i]))
            {
                bOK = FALSE;
                piwd = jf_queue_dequeue(&piw->iw_jqBacklog);
                piwd->iwd_nClosing = 0;
                u32Ret = jf_network_connectAsocketTo(
                    piw->iw_ppjnaAsockets[i], &piwd->iwd_jiRemote,
                    piwd->iwd_u16Port, piwd);
            }
            if (jf_queue_isEmpty(&piw->iw_jqBacklog))
            {
                break;
            }
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
 *  within the time specified by WC_HTTP_SESSION_IDLE_TIMEOUT
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
        if ((piwd->iwd_pjnaSock != NULL) &&
            (! jf_network_isAsocketFree(piwd->iwd_pjnaSock)))
        {
            /*We need to close this socket*/
            jf_logger_logInfoMsg("web client timer sink, close the connection");
            DisconnectSocket = piwd->iwd_pjnaSock;
            piwd->iwd_pjnaSock = NULL;
        }

        if (piwd->iwd_piwParent->iw_u32IdleCount > MAX_IDLE_SESSIONS)
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

            _destroyWebDataobject((jf_webclient_dataobject_t **)&piwd2);
        }

        /*Add this DataObject into the iw_jhIdle for use later*/
        addrlen = _getStringHashKey(addr, &piwd->iwd_jiRemote, piwd->iwd_u16Port);
        jf_logger_logInfoMsg(
            "web client timer sink, add data object %s to idle hashtree", addr);
        jf_hashtree_addEntry(
            &piwd->iwd_piwParent->iw_jhIdle, addr, addrlen, piwd);
        ++ piwd->iwd_piwParent->iw_u32IdleCount;
    }

    /*Let the user know, the socket has been disconnected*/
    if (DisconnectSocket != NULL)
    {
        jf_network_disconnectAsocket(DisconnectSocket);
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
        xfree((void **)&piwd->iwd_pu8BodyBuf);
    piwd->iwd_u32BodyOffset = 0;
    if (piwd->iwd_piwcChunk != NULL)
    {
        if (piwd->iwd_piwcChunk->iwc_pu8Buffer != NULL)
            xfree((void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer);
        xfree((void **)&piwd->iwd_piwcChunk);
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
            piwd->iwd_piwParent->iw_pjnuUtimer, piwd, WC_HTTP_SESSION_IDLE_TIMEOUT,
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
                "wc finish response, pipeline is no, disconnect and add web"
                " data object to backlog");
            jf_network_disconnectAsocket(piwd->iwd_pjnaSock);
            piwd->iwd_pjnaSock = NULL;
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
                jf_network_sendAsocketData(
                    piwd->iwd_pjnaSock, wr->iwr_pu8Buffer[i],
                    wr->iwr_sBuffer[i], wr->iwr_jnmoMemOwner[i]);
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
    internal_web_request_t * wr;

    jf_logger_logInfoMsg("process chunk %d:%d", *psBeginPointer, endPointer);

    wr = jf_queue_peek(&piwd->iwd_jqRequest);

    if (piwd->iwd_piwcChunk == NULL)
    {
        // Create a state object for the Chunk Processor
        u32Ret = xcalloc(
            (void **)&piwd->iwd_piwcChunk, sizeof(internal_web_chunkdata_t));
        if (u32Ret != JF_ERR_NO_ERROR)
            return u32Ret;

        u32Ret = xmalloc(
            (void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer,
            piwd->iwd_piwParent->iw_sBuffer);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            xfree((void **)&piwd->iwd_piwcChunk);
            return u32Ret;
        }
        piwd->iwd_piwcChunk->iwc_u32MallocSize = INITIAL_BUFFER_SIZE;
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
                        "process chunk, chunk size %d",
                        piwd->iwd_piwcChunk->iwc_nBytesLeft);
                    *psBeginPointer = i;
                    piwd->iwd_piwcChunk->iwc_u32Flags =
                        piwd->iwd_piwcChunk->iwc_nBytesLeft == 0 ?
                        FOOTERCHUNK : DATACHUNK;
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
                    piwd->iwd_piwcChunk->iwc_u32MallocSize +
                    piwd->iwd_piwParent->iw_sBuffer);
                piwd->iwd_piwcChunk->iwc_u32MallocSize +=
                    piwd->iwd_piwParent->iw_sBuffer;
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
                            piwd->iwd_pjhphHeader->jhph_sBody =
                                piwd->iwd_piwcChunk->iwc_u32Offset;

                            wr->iwr_fnOnResponse(
                                piwd->iwd_pjnaSock, 0, piwd->iwd_pjhphHeader,
                                wr->iwr_pUser, &piwd->iwd_bPause);
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
 *  number of retries is determined by the value of HTTP_CONNECT_RETRY_COUNT
 *
 *  @param object [in] the associated web data object
 */
static u32 _retrySink(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *)object;
//    internal_webclient_t *wcm = piwd->iwd_piwParent;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = _getStringHashKey(key, &piwd->iwd_jiRemote, piwd->iwd_u16Port);
    jf_logger_logInfoMsg("retry sink, %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) HTTP_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        jf_logger_logInfoMsg("retry sink, give up");

        jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhData, key, keyLength);
        jf_hashtree_deleteEntry(&piwd->iwd_piwParent->iw_jhIdle, key, keyLength);

        _destroyWebDataobject((jf_webclient_dataobject_t **)&piwd);
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
 *  @param Connected [in] flag indicating connect status, TRUE indicates success
 *  @param pUser [in] the associated web data object
 */
static u32 _webclientOnConnect(
    jf_network_asocket_t * pAsocket, boolean_t Connected, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pUser;
    internal_web_request_t *r;
    olint_t i;

    piwd->iwd_pjnaSock = pAsocket;
    piwd->iwd_bInitialRequestAnswered = FALSE;

    if (Connected)
    {
        jf_logger_logInfoMsg("web client connected");
        //Success: Send First Request
        jf_network_getLocalInterfaceOfAsocket(pAsocket, &piwd->iwd_jiLocal);
        r = jf_queue_peek(&piwd->iwd_jqRequest);
        for (i = 0; i < r->iwr_u32NumOfBuffers; ++i)
        {
            jf_network_sendAsocketData(
                pAsocket, r->iwr_pu8Buffer[i],
                r->iwr_sBuffer[i], r->iwr_jnmoMemOwner[i]);
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

/** Internal method dispatched by the disconnect event of the underlying asocket
 *
 *  @param pAsocket [in] the underlying async socket
 *  @param u32Status [in] the status of the disconnection
 *  @param pUser [in] the associated web data object
 *
 *  @return the error code
 */
static u32 _webclientOnDisconnect(
    jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)pUser;
    internal_web_request_t * iwr;
//    u8 * buffer;
//    olsize_t BeginPointer, EndPointer;
//    jf_httpparser_packet_header_t * h;

    jf_logger_logInfoMsg(
        "web client disconnect, WaitForClose %d, PipelineFlags %d",
        piwd->iwd_bWaitForClose, piwd->iwd_u32PipelineFlags);

    if (jf_queue_peek(&piwd->iwd_jqRequest) != NULL)
    {
        /*If there are still pending requests, than obviously this server
          doesn't do persistent connections*/
        jf_logger_logInfoMsg("web client disconnect, pipeline is no");
        piwd->iwd_u32PipelineFlags = PIPELINE_NO;
    }

    if (piwd->iwd_bWaitForClose)
    {
        /*Since we had to read until the socket closes, we finally have
          all the data we need*/
//        getBufferOfAsocket(pAsocket, &buffer, &BeginPointer, &EndPointer);

        piwd->iwd_bInitialRequestAnswered = TRUE;
        piwd->iwd_u32PipelineFlags = PIPELINE_NO;
        piwd->iwd_bFinHeader = FALSE;

        iwr = jf_queue_dequeue(&piwd->iwd_jqRequest);
        if (iwr != NULL)
        {
            iwr->iwr_fnOnResponse(
                pAsocket, 0, piwd->iwd_pjhphHeader, iwr->iwr_pUser,
                &piwd->iwd_bPause);
            //_webclientFinishedResponse(piwd);        
            _destroyWebRequest(&iwr);
        }

        if (piwd->iwd_pjhphHeader != NULL)
            jf_httpparser_destroyPacketHeader(&piwd->iwd_pjhphHeader);
    }

    piwd->iwd_pjnaSock = NULL;

    iwr = jf_queue_peek(&piwd->iwd_jqRequest);
    if (iwr != NULL)
    {
        /*Still Requests to be made
          Make Another Connection and Continue*/
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
static u32 _webclientOnSendOK(jf_network_asocket_t * pAsocket, void * user)
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
        &piwd->iwd_pjhphHeader, (olchar_t *)pu8Buffer,
        *psBeginPointer, sEndPointer - (*psBeginPointer));
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
                    jf_logger_logInfoMsg(
                        "parse http header, chunk");
                }
            }
            if (pjhphf->jhphf_sName == 14 &&
                ol_strncasecmp(pjhphf->jhphf_pstrName, "content-length", 14) == 0)
            {
                /*This packet has a Content-Length*/
                piwd->iwd_bWaitForClose = FALSE;
                pjhphf->jhphf_pstrData[pjhphf->jhphf_sData] = '\0';
                piwd->iwd_nBytesLeft = atoi(pjhphf->jhphf_pstrData);
                jf_logger_logInfoMsg(
                    "parse http header, content-length %d",
                    piwd->iwd_nBytesLeft);
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
            wr->iwr_fnOnResponse(
                pAsocket, 0, piwd->iwd_pjhphHeader,
                wr->iwr_pUser, &piwd->iwd_bPause);
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
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pjhphHeader,
                        wr->iwr_pUser, &(piwd->iwd_bPause));
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
                        u32Ret = xmalloc(
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
 *  @param pbPause [in] flag to tell the underlying socket to pause reading data
 *
 *  @return the error code
 */
static u32 _webclientOnData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser, boolean_t * pbPause)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pUser;
    internal_web_request_t *wr;
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
    wr = (internal_web_request_t *)jf_queue_peek(&piwd->iwd_jqRequest);
    if (wr == NULL)
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
                        pAsocket, piwd, wr, pu8Buffer,
                        psBeginPointer, sEndPointer, i);
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
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pjhphHeader,
                        wr->iwr_pUser, &piwd->iwd_bPause);
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
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pjhphHeader,
                        wr->iwr_pUser, &piwd->iwd_bPause);
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
    if (! jf_network_isAsocketFree(pAsocket))
    {
        /*If the user said to pause this connection, do so*/
        *pbPause = piwd->iwd_bPause;
    }

    return u32Ret;
}

/*Allocate memory for the buffers in web request which are volatile*/
static u32 _webRequestStaticMemory(internal_web_request_t * request)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    u8 * pu8Buffer;

    for (i = 0; i < request->iwr_u32NumOfBuffers; ++ i)
    {
        if (request->iwr_jnmoMemOwner[i] == JF_NETWORK_MEM_OWNER_USER)
        {
            u32Ret = dupMemory(
                (void **)&pu8Buffer, request->iwr_pu8Buffer[i],
                request->iwr_sBuffer[i]);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                request->iwr_pu8Buffer[i] = pu8Buffer;
                request->iwr_bUserFree[1] = FALSE;
                request->iwr_jnmoMemOwner[i] = JF_NETWORK_MEM_OWNER_STATIC;
            }
        }
    }

    return u32Ret;
}

static u32 _internalWebclientOnResponse(
    jf_network_asocket_t * pAsocket, olint_t nEvent,
    jf_httpparser_packet_header_t * header, void * user, boolean_t * pbPause)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_webclient_destroy(jf_webclient_t ** ppWebclient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) *ppWebclient;
    jf_hashtree_enumerator_t en;
    jf_webclient_dataobject_t * piwd;
    olchar_t * pstrKey;
    olsize_t u32KeyLen;
    u32 u32Index;

    /*Iterate through all the web data objects*/
    jf_hashtree_initEnumerator(&piw->iw_jhData, &en);
    while (! jf_hashtree_isEnumeratorEmptyNode(&en))
    {
        /*Free the web data object*/
        jf_hashtree_getEnumeratorValue(&en, &pstrKey, &u32KeyLen, &piwd);
        _destroyWebDataobject(&piwd);
        jf_hashtree_moveEnumeratorNext(&en);
    }
    jf_hashtree_finiEnumerator(&en);

    if (piw->iw_ppjnaAsockets != NULL)
    {
        for (u32Index = 0;
             (u32Index < piw->iw_u32NumOfAsockets) && (u32Ret == JF_ERR_NO_ERROR);
             u32Index ++)
        {
            if (piw->iw_ppjnaAsockets[u32Index] != NULL)
                jf_network_destroyAsocket(&(piw->iw_ppjnaAsockets[u32Index]));
        }
    }

    /*Free all the other associated resources*/
    jf_queue_fini(&piw->iw_jqBacklog);
    jf_hashtree_fini(&piw->iw_jhIdle);
    jf_hashtree_fini(&piw->iw_jhData);
    jf_mutex_fini(&piw->iw_jmLock);

    if (piw->iw_ppjnaAsockets != NULL)
        xfree((void **)&(piw->iw_ppjnaAsockets));

    return u32Ret;
}


u32 jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient,
    jf_webclient_create_param_t * pjwcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;
    jf_network_asocket_create_param_t jnacp;
    internal_webclient_t *piw;

    assert((pjnc != NULL) && (ppWebclient != NULL));
    assert((pjwcp != NULL) && (pjwcp->jwcp_nPoolSize > 0) &&
           (pjwcp->jwcp_nPoolSize < 100));

    u32Ret = xcalloc((void **)&piw, sizeof(internal_webclient_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piw->iw_jncohHeader.jncoh_fnPreSelect = _preWebclientProcess;
        //piw->PostSelect = &_preWebclientProcess;
        piw->iw_pjncChain = pjnc;
        piw->iw_sBuffer =
            pjwcp->jwcp_sBuffer ? pjwcp->jwcp_sBuffer : INITIAL_BUFFER_SIZE;

        jf_hashtree_init(&piw->iw_jhIdle);
        jf_queue_init(&piw->iw_jqBacklog);
        jf_hashtree_init(&piw->iw_jhData);

        piw->iw_u32NumOfAsockets = pjwcp->jwcp_nPoolSize;
        u32Ret = xmalloc(
            (void **)&(piw->iw_ppjnaAsockets),
            pjwcp->jwcp_nPoolSize * sizeof(void *));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mutex_init(&(piw->iw_jmLock));
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
        memset(&jnacp, 0, sizeof(jnacp));
        jnacp.jnacp_sInitialBuf = piw->iw_sBuffer;
        jnacp.jnacp_fnOnData = _webclientOnData;
        jnacp.jnacp_fnOnConnect = _webclientOnConnect;
        jnacp.jnacp_fnOnDisconnect = _webclientOnDisconnect;
        jnacp.jnacp_fnOnSendOK = _webclientOnSendOK;

        // Create our pool of sockets
        for (u32Index = 0;
             (u32Index < pjwcp->jwcp_nPoolSize) && (u32Ret == JF_ERR_NO_ERROR);
             u32Index ++)
        {
            u32Ret = jf_network_createAsocket(
                pjnc, &(piw->iw_ppjnaAsockets[u32Index]), &jnacp);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppWebclient = piw;
    else if (piw != NULL)
        jf_webclient_destroy((void **)&piw);

    return u32Ret;
}

u32 jf_webclient_pipelineWebRequest(
    jf_webclient_t * pWebClient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_httpparser_packet_header_t * packet, jf_webclient_fnOnResponse_t fnOnResponse,
    void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebClient;
    internal_web_request_t * request = NULL;

    jf_logger_logInfoMsg("pipeline web req");

    u32Ret = _newWebRequest(&request, 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_httpparser_getRawPacket(
            packet, (olchar_t **)&request->iwr_pu8Buffer[0],
            &request->iwr_sBuffer[0]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        request->iwr_bUserFree[0] = FALSE;
        request->iwr_jnmoMemOwner[0] = JF_NETWORK_MEM_OWNER_STATIC;
        request->iwr_fnOnResponse =
            (fnOnResponse == NULL) ? _internalWebclientOnResponse : fnOnResponse;
        request->iwr_pUser = user;

        u32Ret = _processWebRequest(piw, pjiRemote, u16Port, request);
    }

    return u32Ret;
}

u32 jf_webclient_pipelineWebRequestEx(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, boolean_t bStaticHeader,
    olchar_t * pstrBody, olsize_t sBody, boolean_t bStaticBody,
    jf_webclient_fnOnResponse_t fnOnResponse, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_web_request_t * request;
    u32 u32NumOfBuffers;

    jf_logger_logInfoMsg("pipeline web req ex");
    jf_logger_logDataMsgWithAscii((u8 *)pstrHeader, sHeader, "HTTP request header:");
    if (pstrBody != NULL)
        jf_logger_logDataMsgWithAscii((u8 *)pstrBody, sBody, "HTTP request body:");

    u32NumOfBuffers = pstrBody != NULL ? 2 : 1;
    u32Ret = _newWebRequest(&request, u32NumOfBuffers);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        request->iwr_pu8Buffer[0] = (u8 *)pstrHeader;
        request->iwr_sBuffer[0] = sHeader;
        request->iwr_bUserFree[0] = TRUE;
        if (bStaticHeader)
            request->iwr_jnmoMemOwner[0] = JF_NETWORK_MEM_OWNER_STATIC;
        else
            request->iwr_jnmoMemOwner[0] = JF_NETWORK_MEM_OWNER_USER;

        if (pstrBody != NULL)
        {
            request->iwr_pu8Buffer[1] = (u8 *)pstrBody;
            request->iwr_sBuffer[1] = sBody;
            request->iwr_bUserFree[1] = TRUE;
            if (bStaticBody)
                request->iwr_jnmoMemOwner[1] = JF_NETWORK_MEM_OWNER_STATIC;
            else
                request->iwr_jnmoMemOwner[1] = JF_NETWORK_MEM_OWNER_USER;
        }

        request->iwr_fnOnResponse =
            (fnOnResponse == NULL) ? _internalWebclientOnResponse : fnOnResponse;
        request->iwr_pUser = user;

        u32Ret = _webRequestStaticMemory(request);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _processWebRequest(piw, pjiRemote, u16Port, request);

    return u32Ret;
}

/** Returns the headers from a given web data object
 *
 *  @param pDataObject [in] the web data object to query

 *  @return the packet header
 */
jf_httpparser_packet_header_t * jf_webclient_getPacketHeaderFromDataobject(
    jf_webclient_dataobject_t * pDataObject)
{
    return (((internal_web_dataobject_t *) pDataObject)->iwd_pjhphHeader);
}

u32 jf_webclient_deleteWebRequests(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t *piw = (internal_webclient_t *) pWebclient;
    olchar_t addr[25];
    internal_web_dataobject_t *piwd;
    olint_t addrlen;
    internal_web_request_t *piwr;
    jf_queue_t jqRemove;
    boolean_t zero = FALSE;

    jf_queue_init(&jqRemove);

    addrlen = _getStringHashKey(addr, pjiRemote, u16Port);

    /* Are there any pending requests to this IP/Port combo */
//    jf_mutex_acquire(&(piw->iw_jmLock));
    if (jf_hashtree_hasEntry(&piw->iw_jhData, addr, addrlen))
    {
        /* Yes, iterate through them */
        jf_hashtree_getEntry(
            &piw->iw_jhData, addr, addrlen, (void **)&piwd);
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
            piwr->iwr_fnOnResponse(
                pWebclient, JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED, NULL,
                piwr->iwr_pUser, &zero);

            _destroyWebRequest(&piwr);
        }
    }

    jf_queue_fini(&jqRemove);

    return u32Ret;
}

/** Resumes a paused connection. If the client has set the pause flag, the
 *  underlying socket will no longer read data from the NIC. This method resumes
 *  the socket.
 *
 *  @param pDataobject [in] the associated web data object
 *
 *  @return the error code
 */
u32 jf_webclient_resumeDataobject(jf_webclient_dataobject_t * pDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pDataobject;

    piwd->iwd_bPause = FALSE;
    u32Ret = jf_network_resumeAsocket(piwd->iwd_pjnaSock);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


