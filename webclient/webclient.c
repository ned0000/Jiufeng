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

typedef void  web_dataobject_t;

typedef struct internal_web_request
{
#define IWR_MAX_NUM_OF_BUF  8
    olint_t iwr_u32NumOfBuffers;
    u8 * iwr_pu8Buffer[IWR_MAX_NUM_OF_BUF];
    olsize_t iwr_sBuffer[IWR_MAX_NUM_OF_BUF];
    /**If UserFree is TRUE, webclient will not free the buffer
       If UserFree is FALSE, webclient will free the buffer*/
    boolean_t iwr_bUserFree[IWR_MAX_NUM_OF_BUF];
    as_mem_owner_t iwr_amoMemOwner[IWR_MAX_NUM_OF_BUF];

    u16 iwr_u16Reserved[4];

    void * iwr_pUser;
    fnWebclientOnResponse_t iwr_fnOnResponse;
} internal_web_request_t;

typedef struct internal_webclient
{
    basic_chain_object_header_t iw_bcohHeader;

    asocket_t ** iw_ppAsockets;
    u32 iw_u32NumOfAsockets;

    hashtree_t iw_hData;
    /**The web data object is put to idle hash tree when no more
       request in the queue of the web data object*/
    hashtree_t iw_hIdle;
    /**The web data object is put the backlog queue when the object doesn't
       connected to remote server*/
    basic_queue_t iw_bqBacklog;

    utimer_t * iw_uUtimer;
    u32 iw_u32IdleCount;

    olsize_t iw_sBuffer;

    basic_chain_t *iw_bcChain;
    sync_mutex_t iw_smLock;
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

    ip_addr_t iwd_iaRemote;
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
    packet_header_t * iwd_pphHeader;

    basic_queue_t iwd_baRequest;
    asocket_t * iwd_pasSock;

    ip_addr_t iwd_iaLocal;

} internal_web_dataobject_t;

/* --- private routine section---------------------------------------------- */

static olint_t _getStringHashKey(olchar_t * key, ip_addr_t * addr, u16 port)
{
    olint_t keyLength;
    olchar_t strIpAddr[50];

    getStringIpAddr(strIpAddr, addr);
    keyLength = ol_sprintf(key, "%s:%d", strIpAddr, port);

    return keyLength;
}

static u32 _destroyWebRequest(internal_web_request_t ** ppRequest)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_request_t * piwr;
    olint_t i;

    logInfoMsg("destroy web req");

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
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_request_t * piwr;

    logInfoMsg("new web req");

    u32Ret = xmalloc((void **)&piwr, sizeof(internal_web_request_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(piwr, 0, sizeof(internal_web_request_t));

        piwr->iwr_u32NumOfBuffers = u32Num;


    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        _destroyWebRequest(&piwr);

    return u32Ret;
}

/** Free resources associated with a web data object
 *
 *  @param ppDataobject [in/out] The web data object to free
 */
static u32 _destroyWebDataobject(web_dataobject_t ** ppDataobject)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) *ppDataobject;
    internal_web_request_t * wr;
    boolean_t zero = FALSE;
    olchar_t addr[64];
//    olint_t addrlen;

    _getStringHashKey(addr, &piwd->iwd_iaRemote, piwd->iwd_u16Port);
    logInfoMsg("destroy web data obj %s", addr);

    if ((piwd->iwd_pasSock != NULL) && (! isAsocketFree(piwd->iwd_pasSock)))
    {
        /*This connection needs to be disconnected first*/
        asDisconnect(piwd->iwd_pasSock);
    }

    if (piwd->iwd_pphHeader != NULL)
    {
        /*The header needs to be freed*/
        destroyPacketHeader(&(piwd->iwd_pphHeader));
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
    wr = dequeue(&piwd->iwd_baRequest);
    while (wr != NULL)
    {
        /*If this is a client request, then we need to signal
          that this request is being aborted*/
        wr->iwr_fnOnResponse(
            NULL, WIF_WEB_REQUEST_DELETED, NULL, wr->iwr_pUser, &zero);
        _destroyWebRequest(&wr);

        wr = dequeue(&piwd->iwd_baRequest);
    }
    finiQueue(&piwd->iwd_baRequest);

    xfree((void **)ppDataobject);

    return u32Ret;
}

static u32 _createWebDataobject(
    internal_web_dataobject_t ** ppiwd, asocket_t * pAsocket,
    internal_webclient_t * piw, ip_addr_t * piaRemote, u16 u16Port)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd = NULL;
    olchar_t addr[64];
    olint_t addrlen;

    logInfoMsg("create web data obj");

    u32Ret = xcalloc((void **)&piwd, sizeof(internal_web_dataobject_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        initQueue(&piwd->iwd_baRequest);
        piwd->iwd_pasSock = pAsocket;
        piwd->iwd_piwParent = piw;
        memcpy(&piwd->iwd_iaRemote, piaRemote, sizeof(ip_addr_t));
        piwd->iwd_u16Port = u16Port;

        addrlen = _getStringHashKey(addr, piaRemote, u16Port);
        u32Ret = addHashtreeEntry(&piw->iw_hData, addr, addrlen, piwd);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppiwd = piwd;
    else if (piwd != NULL)
        _destroyWebDataobject((web_dataobject_t **)&piwd);

    return u32Ret;
}

static u32 _processWebRequest(
    internal_webclient_t * piw, ip_addr_t * piaRemote, u16 u16Port,
    internal_web_request_t * request)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bForceUnBlock = FALSE;
    olchar_t addr[64];
    olint_t addrlen;
    internal_web_dataobject_t *piwd;
    olint_t i;
    boolean_t bHashEntry = FALSE, bQueueEmpty = FALSE;

    addrlen = _getStringHashKey(addr, piaRemote, u16Port);

    logInfoMsg("process web req, %s", addr);

    bHashEntry = hasHashtreeEntry(&piw->iw_hData, addr, addrlen);
    if (bHashEntry)
    {
        logInfoMsg("process web req, has entry");
        /*It does*/
        getHashtreeEntry(
            &piw->iw_hData, addr, addrlen, (void **)&piwd);
    }
    else
    {
        /*There is no previous connection, so we need to set it up*/
        u32Ret = _createWebDataobject(&piwd, NULL, piw, piaRemote, u16Port);
        if (u32Ret == OLERR_NO_ERROR)
        {
            /*Queue it up in our Backlog, because we don't want to burden
              ourselves, so we need to see if we have the resources for it.
              The Pool will grab one when it can. The chain doesn't know
              about us, so we need to force it to unblock, to process this*/
            bForceUnBlock = TRUE;
            u32Ret = enqueue(&piw->iw_bqBacklog, piwd);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg(
            "process web req, WaitForClose %d", piwd->iwd_bWaitForClose);

        bQueueEmpty = isQueueEmpty(&piwd->iwd_baRequest);

        u32Ret = enqueue(&piwd->iwd_baRequest, request);
    }

    if ((u32Ret == OLERR_NO_ERROR) && bHashEntry && bQueueEmpty)
    {
        /*the web dataobject is already there*/
        /*There are no pending requests however, so we can try to
          send this right away*/
        /*Take out of Idle State*/
        piw->iw_u32IdleCount =
            piw->iw_u32IdleCount == 0 ? 0 : piw->iw_u32IdleCount - 1;
        deleteHashtreeEntry(&piw->iw_hIdle, addr, addrlen);
        removeUtimerItem(piw->iw_uUtimer, piwd);
        if ((piwd->iwd_pasSock == NULL) ||
            isAsocketFree(piwd->iwd_pasSock))
        {
            /*If this was in our iw_hIdle, then most likely the select
              doesn't know about it, so we need to force it to unblock*/
            logInfoMsg("process web req, asocket is not valid");
            bForceUnBlock = TRUE;
            u32Ret = enqueue(&piw->iw_bqBacklog, piwd);
        }
        else if (piwd->iwd_pasSock != NULL)
        {
            /*Socket is still there*/
            if (! piwd->iwd_bWaitForClose)
            {
                logInfoMsg("process web req, send data");
                for (i = 0; i < request->iwr_u32NumOfBuffers; ++i)
                {
                    asSendData(
                        piwd->iwd_pasSock, request->iwr_pu8Buffer[i],
                        request->iwr_sBuffer[i], request->iwr_amoMemOwner[i]);
                }
            }
        }
    }

    if (bForceUnBlock)
    {
        wakeupBasicChain(piw->iw_bcChain);
    }

    return u32Ret;
}

/** Pre select handler for basic chain
 *
 *  @param pWebclient [in] the web client object
 *  @param readset: 
 *  @param writeset: 
 *  @param errorset: 
 *  @param blocktime: 
 */
static u32 _preWebclientProcess(
    void * pWebclient, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_web_dataobject_t * piwd;
    olint_t i;
    boolean_t bOK = FALSE;

    /*Try and satisfy as many things as we can. If we have resources
      grab a socket and go*/
//    acquireSyncMutex(&(piw->iw_smLock));
    while ((! bOK) && (! isQueueEmpty(&piw->iw_bqBacklog)))
    {
        bOK = TRUE;
        for (i = 0; i < piw->iw_u32NumOfAsockets; ++i)
        {
            if (isAsocketFree(piw->iw_ppAsockets[i]))
            {
                bOK = FALSE;
                piwd = dequeue(&piw->iw_bqBacklog);
                piwd->iwd_nClosing = 0;
                u32Ret = asConnectTo(
                    piw->iw_ppAsockets[i], &piwd->iwd_iaRemote,
                    piwd->iwd_u16Port, piwd);
            }
            if (isQueueEmpty(&piw->iw_bqBacklog))
            {
                break;
            }
        }
    }
//    releaseSyncMutex(&(piw->iw_smLock));

    return u32Ret;
}

static u32 _webclientTimerInterruptSink(void ** object)
{
    u32 u32Ret = OLERR_NO_ERROR;

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
    u32 u32Ret = OLERR_NO_ERROR;
    hashtree_enumerator_t enumerator;
    olchar_t addr[200];
    olint_t addrlen;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)object;
    internal_web_dataobject_t * piwd2;
    olchar_t * pstrKey;
    olsize_t u32KeyLen;
    void * data;
    void * DisconnectSocket = NULL;

    logInfoMsg("web client timer sink");

    if (isQueueEmpty(&piwd->iwd_baRequest))
    {
        logInfoMsg("web client timer sink, queue is empty");
        /*This connection is idle, because there are no pending requests */
        if ((piwd->iwd_pasSock != NULL) && (! isAsocketFree(piwd->iwd_pasSock)))
        {
            /*We need to close this socket*/
            logInfoMsg("web client timer sink, close the connection");
            DisconnectSocket = piwd->iwd_pasSock;
            piwd->iwd_pasSock = NULL;
        }

        if (piwd->iwd_piwParent->iw_u32IdleCount > MAX_IDLE_SESSIONS)
        {
            /*Remove an entry from iw_hIdle, if there are too many entries in it*/
            -- piwd->iwd_piwParent->iw_u32IdleCount;

            initHashtreeEnumerator(&piwd->iwd_piwParent->iw_hIdle, &enumerator);
            moveHashtreeNext(&enumerator);
            getHashtreeValue(&enumerator, &pstrKey, &u32KeyLen, &data);
            finiHashtreeEnumerator(&enumerator);
            getHashtreeEntry(
                &piwd->iwd_piwParent->iw_hData, pstrKey, u32KeyLen, (void **)&piwd2);

            logInfoMsg("web client timer sink, delete entry with key %s", pstrKey);
            deleteHashtreeEntry(&piwd->iwd_piwParent->iw_hData, pstrKey, u32KeyLen);
            deleteHashtreeEntry(&piwd->iwd_piwParent->iw_hIdle, pstrKey, u32KeyLen);

            _destroyWebDataobject((web_dataobject_t **)&piwd2);
        }

        /*Add this DataObject into the iw_hIdle for use later*/
        addrlen = _getStringHashKey(addr, &piwd->iwd_iaRemote, piwd->iwd_u16Port);
        logInfoMsg(
            "web client timer sink, add data object %s to idle hashtree", addr);
        addHashtreeEntry(
            &piwd->iwd_piwParent->iw_hIdle, addr, addrlen, piwd);
        ++ piwd->iwd_piwParent->iw_u32IdleCount;
    }

    /*Let the user know, the socket has been disconnected*/
    if (DisconnectSocket != NULL)
    {
        asDisconnect(DisconnectSocket);
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
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_request_t * wr;
    olint_t i;

    logInfoMsg(
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
    if (piwd->iwd_pphHeader != NULL)
        destroyPacketHeader(&piwd->iwd_pphHeader);

    wr = dequeue(&piwd->iwd_baRequest);
    _destroyWebRequest(&wr);
    wr = peekQueue(&piwd->iwd_baRequest);
    if (wr == NULL)
    {
        /*Since the request queue is empty, that means this connection is now
          idle. Set a timed callback, so we can free this resource if
          neccessary*/
        addUtimerItem(
            piwd->iwd_piwParent->iw_uUtimer, piwd, WC_HTTP_SESSION_IDLE_TIMEOUT,
            _webclientTimerSink, _webclientTimerInterruptSink);
    }
    else
    {
        logInfoMsg("wc finish response, queue is not empty");
        /*There are still pending requests in the queue, so try to send them*/
        if (piwd->iwd_u32PipelineFlags == PIPELINE_NO)
        {
            /*Pipelining is not supported, so we should just close the socket,
              instead of waiting for the other guy to close it, because if they
              forget to, it will screw us over if there are pending requests */

            /*It should also be noted, that when this closes, the module will
              realize there are pending requests, in which case it will open a
              new connection for the requests.*/
            logInfoMsg(
                "wc finish response, pipeline is no, disconnect and add web"
                " data object to backlog");
            asDisconnect(piwd->iwd_pasSock);
            piwd->iwd_pasSock = NULL;
            u32Ret = enqueue(&piwd->iwd_piwParent->iw_bqBacklog, piwd);
        }
        else
        {
            /*If the connection is still open, and we didn't flag this as not
              supporting persistent connections, than obviously it is
              supported*/
            logInfoMsg("wc finish response, pipeline is yes");
            piwd->iwd_u32PipelineFlags = PIPELINE_YES;

            for (i = 0; i < wr->iwr_u32NumOfBuffers; ++i)
            {
                /*Try to send the request*/
                asSendData(
                    piwd->iwd_pasSock, wr->iwr_pu8Buffer[i],
                    wr->iwr_sBuffer[i], wr->iwr_amoMemOwner[i]);
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
    asocket_t * pAsocket, internal_web_dataobject_t * piwd, u8 * buffer,
    olsize_t * psBeginPointer, olsize_t endPointer)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t *hex;
    olint_t i;
    olsize_t sBeginPointer = *psBeginPointer;
    internal_web_request_t * wr;

    logInfoMsg("process chunk %d:%d", *psBeginPointer, endPointer);

    wr = peekQueue(&piwd->iwd_baRequest);

    if (piwd->iwd_piwcChunk == NULL)
    {
        // Create a state object for the Chunk Processor
        u32Ret = xcalloc(
            (void **)&piwd->iwd_piwcChunk, sizeof(internal_web_chunkdata_t));
        if (u32Ret != OLERR_NO_ERROR)
            return u32Ret;

        u32Ret = xmalloc(
            (void **)&piwd->iwd_piwcChunk->iwc_pu8Buffer,
            piwd->iwd_piwParent->iw_sBuffer);
        if (u32Ret != OLERR_NO_ERROR)
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
            logInfoMsg("process chunk, STARTCHUNK");
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
                    logInfoMsg(
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
            logInfoMsg("process chunk, ENDCHUNK");
            if (endPointer >= 2)
            {
                /*There is more chunks to come*/
                *psBeginPointer = 2;
                piwd->iwd_piwcChunk->iwc_u32Flags = STARTCHUNK;
            }
            break;
        case DATACHUNK:
            logInfoMsg("process chunk, DATACHUNK");
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
                logInfoMsg("process chunk, realloc memory");
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
            logInfoMsg("process chunk, FOOTERCHUNK");
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
                            piwd->iwd_pphHeader->ph_pu8Body =
                                piwd->iwd_piwcChunk->iwc_pu8Buffer;
                            piwd->iwd_pphHeader->ph_sBody =
                                piwd->iwd_piwcChunk->iwc_u32Offset;

                            wr->iwr_fnOnResponse(
                                piwd->iwd_pasSock, 0, piwd->iwd_pphHeader,
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
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t key[64];
    olint_t keyLength;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *)object;
//    internal_webclient_t *wcm = piwd->iwd_piwParent;

    piwd->iwd_u32ExponentialBackoff = (piwd->iwd_u32ExponentialBackoff == 0) ?
        1 : piwd->iwd_u32ExponentialBackoff * 2;

    keyLength = _getStringHashKey(key, &piwd->iwd_iaRemote, piwd->iwd_u16Port);
    logInfoMsg("retry sink, %s, eb %u", key, piwd->iwd_u32ExponentialBackoff);

    if (piwd->iwd_u32ExponentialBackoff >=
        (olint_t) pow((oldouble_t) 2, (oldouble_t) HTTP_CONNECT_RETRY_COUNT))
    {
        /*Retried enough times, give up*/
        logInfoMsg("retry sink, give up");

        deleteHashtreeEntry(&piwd->iwd_piwParent->iw_hData, key, keyLength);
        deleteHashtreeEntry(&piwd->iwd_piwParent->iw_hIdle, key, keyLength);

        _destroyWebDataobject((web_dataobject_t **)&piwd);
    }
    else
    {
        /*Lets retry again*/
        logInfoMsg("retry sink, retry later");
        u32Ret = enqueue(&piwd->iwd_piwParent->iw_bqBacklog, piwd);
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
    asocket_t * pAsocket, boolean_t Connected, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pUser;
    internal_web_request_t *r;
    olint_t i;

    piwd->iwd_pasSock = pAsocket;
    piwd->iwd_bInitialRequestAnswered = FALSE;

    if (Connected)
    {
        logInfoMsg("web client connected");
        //Success: Send First Request
        getLocalInterfaceOfAsocket(pAsocket, &piwd->iwd_iaLocal);
        r = peekQueue(&piwd->iwd_baRequest);
        for (i = 0; i < r->iwr_u32NumOfBuffers; ++i)
        {
            asSendData(
                pAsocket, r->iwr_pu8Buffer[i],
                r->iwr_sBuffer[i], r->iwr_amoMemOwner[i]);
        }
    }
    else
    {
        logInfoMsg("web client connect failed, retry later");
        /*The connection failed, so lets set a timed callback, and try again*/
        piwd->iwd_u32PipelineFlags = PIPELINE_UNKNOWN;
        addUtimerItem(
            piwd->iwd_piwParent->iw_uUtimer, piwd,
            piwd->iwd_u32ExponentialBackoff, _retrySink, NULL);
    }

    return u32Ret;
}               

/** Internal method dispatched by the disconnect event of the underlying asocket
 *
 *  @param pAsocket [in] the underlying asocket
 *  @param pUser [in] the associated web data object
 */
static u32 _webclientOnDisconnect(
    asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd = (internal_web_dataobject_t *)pUser;
    internal_web_request_t * iwr;
//    u8 * buffer;
//    olsize_t BeginPointer, EndPointer;
//    packet_header_t * h;

    logInfoMsg(
        "web client disconnect, WaitForClose %d, PipelineFlags %d",
        piwd->iwd_bWaitForClose, piwd->iwd_u32PipelineFlags);

    if (peekQueue(&piwd->iwd_baRequest) != NULL)
    {
        /*If there are still pending requests, than obviously this server
          doesn't do persistent connections*/
        logInfoMsg("web client disconnect, pipeline is no");
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

        iwr = dequeue(&piwd->iwd_baRequest);
        if (iwr != NULL)
        {
            iwr->iwr_fnOnResponse(
                pAsocket, 0, piwd->iwd_pphHeader, iwr->iwr_pUser,
                &piwd->iwd_bPause);
            //_webclientFinishedResponse(piwd);        
            _destroyWebRequest(&iwr);
        }

        if (piwd->iwd_pphHeader != NULL)
            destroyPacketHeader(&piwd->iwd_pphHeader);
    }

    piwd->iwd_pasSock = NULL;

    iwr = peekQueue(&piwd->iwd_baRequest);
    if (iwr != NULL)
    {
        /*Still Requests to be made
          Make Another Connection and Continue*/
        logInfoMsg("web client disconnect, retry later");
        addUtimerItem(
            piwd->iwd_piwParent->iw_uUtimer, piwd,
            piwd->iwd_u32ExponentialBackoff, _retrySink, NULL);
    }

    return u32Ret;
}


/** Internal method dispatched by the send ok event of the underlying asocket
 *
 *  @param pAsocket [in] The underlying asocket
 *  @param user [in] the associated web data object
 */
static u32 _webclientOnSendOK(asocket_t * pAsocket, void * user)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("web client send ok");

    return u32Ret;
}

static u32 _parseHttpHeader(
     asocket_t * pAsocket, internal_web_dataobject_t * piwd,
     internal_web_request_t * wr, u8 * pu8Buffer,
     olsize_t * psBeginPointer, olsize_t sEndPointer, olsize_t sHeader)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_t * tph;
    packet_header_field_t * phfn;
    olsize_t zero = 0;

    logInfoMsg("parse http header");
    /*Headers are delineated with a CRLF, and terminated with an empty line*/
    piwd->iwd_sHeaderLen = sHeader + 3;
    piwd->iwd_bWaitForClose = TRUE;
    piwd->iwd_nBytesLeft = -1;
    piwd->iwd_bFinHeader = TRUE;
    u32Ret = parsePacketHeader(
        &piwd->iwd_pphHeader, (olchar_t *)pu8Buffer,
        *psBeginPointer, sEndPointer - (*psBeginPointer));
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*Introspect Request, to see what to do next*/
        phfn = piwd->iwd_pphHeader->ph_pphfFirst;
        while (phfn != NULL)
        {
            if (phfn->phf_sName == 17 &&
                ol_strncasecmp(phfn->phf_pstrName, "transfer-encoding", 17) == 0)
            {
                if (phfn->phf_sData == 7 &&
                    ol_strncasecmp(phfn->phf_pstrData, "chunked", 7) == 0)
                {
                    /*This packet was chunk encoded*/
                    piwd->iwd_bWaitForClose = FALSE;
                    piwd->iwd_bChunked = TRUE;
                    logInfoMsg(
                        "parse http header, chunk");
                }
            }
            if (phfn->phf_sName == 14 &&
                ol_strncasecmp(phfn->phf_pstrName, "content-length", 14) == 0)
            {
                /*This packet has a Content-Length*/
                piwd->iwd_bWaitForClose = FALSE;
                phfn->phf_pstrData[phfn->phf_sData] = '\0';
                piwd->iwd_nBytesLeft = atoi(phfn->phf_pstrData);
                logInfoMsg(
                    "parse http header, content-length %d",
                    piwd->iwd_nBytesLeft);
            }
            phfn = phfn->phf_pphfNext;
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
                pAsocket, 0, piwd->iwd_pphHeader,
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
                    logInfoMsg("parse http header, got entire packet");
                    piwd->iwd_pphHeader->ph_pu8Body = pu8Buffer + sHeader + 4;
                    piwd->iwd_pphHeader->ph_sBody = piwd->iwd_nBytesLeft;
                    /*We have the entire body, so we have the entire packet*/
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pphHeader,
                        wr->iwr_pUser, &(piwd->iwd_bPause));
                    *psBeginPointer =
                        *psBeginPointer + sHeader + 4 + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
                else
                {
                    /*We read some of the body, but not all of it yet*/
                    logInfoMsg("parse http header, got partial packet");
                    piwd->iwd_sHeaderLen = 0;
                    *psBeginPointer = sHeader + 4;
                    u32Ret = clonePacketHeader(&tph, piwd->iwd_pphHeader);
                    if (u32Ret == OLERR_NO_ERROR)
                    {
                        destroyPacketHeader(&piwd->iwd_pphHeader);
                        piwd->iwd_pphHeader = tph;
                    }

                    if ((u32Ret == OLERR_NO_ERROR) &&
                        (piwd->iwd_nBytesLeft >
                         piwd->iwd_piwParent->iw_sBuffer))
                    {
                        /*asocket buffer is not enough to hold the HTTP body*/
                        logInfoMsg("parse http header, alloc memory for body");
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
                    u32Ret = clonePacketHeader(&tph, piwd->iwd_pphHeader);
                    if (u32Ret == OLERR_NO_ERROR)
                    {
                        destroyPacketHeader(&piwd->iwd_pphHeader);
                        piwd->iwd_pphHeader = tph;
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
 *  @param buffer [in] the receive buffer
 *  @param psBeginPointer [in] start pointer in the buffer
 *  @param sEndPointer [in] the length of the buffer
 *  @param pUser [in] User data that can be set/received
 *  @param pbPause [in] flag to tell the underlying socket to pause reading data
 */
static u32 _webclientOnData(
    asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser, boolean_t * pbPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pUser;
    internal_web_request_t *wr;
    olint_t i = 0;
    olint_t Fini;

    logInfoMsg(
        "web client data, %d:%d, WaitForClose %d",
        *psBeginPointer, sEndPointer, piwd->iwd_bWaitForClose);
/*
    logDataMsgWithAscii(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer,
        "web client data");
*/
    wr = (internal_web_request_t *)peekQueue(&piwd->iwd_baRequest);
    if (wr == NULL)
    {
        logInfoMsg("web client data, ignore");
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
                logInfoMsg(
                    "web client data, body in asocket buffer, Fini %d", Fini);
                if (Fini >= 0)
                {
                    setBody(
                        piwd->iwd_pphHeader, pu8Buffer + *psBeginPointer,
                        piwd->iwd_nBytesLeft, FALSE);
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pphHeader,
                        wr->iwr_pUser, &piwd->iwd_bPause);
                    *psBeginPointer = *psBeginPointer + piwd->iwd_nBytesLeft;
                    _webclientFinishedResponse(piwd);
                }
            }
            else
            {
                Fini = piwd->iwd_u32BodyOffset +
                    sEndPointer - *psBeginPointer - piwd->iwd_nBytesLeft;
                logInfoMsg(
                    "web client data, body in data object buffer, Fini %d, offset %u",
                    Fini, piwd->iwd_u32BodyOffset);
                if (Fini >= 0)
                {
                    Fini = piwd->iwd_nBytesLeft - piwd->iwd_u32BodyOffset;
                    memcpy(piwd->iwd_pu8BodyBuf + piwd->iwd_u32BodyOffset,
                           pu8Buffer + *psBeginPointer, Fini);
                    setBody(
                        piwd->iwd_pphHeader, piwd->iwd_pu8BodyBuf,
                        piwd->iwd_nBytesLeft, FALSE);
                    wr->iwr_fnOnResponse(
                        pAsocket, 0, piwd->iwd_pphHeader,
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
    if (! isAsocketFree(pAsocket))
    {
        /*If the user said to pause this connection, do so*/
        *pbPause = piwd->iwd_bPause;
    }

    return u32Ret;
}

/*Allocate memory for the buffers in web request which are volatile*/
static u32 _webRequestStaticMemory(internal_web_request_t * request)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    u8 * pu8Buffer;

    for (i = 0; i < request->iwr_u32NumOfBuffers; ++ i)
    {
        if (request->iwr_amoMemOwner[i] == AS_MEM_OWNER_USER)
        {
            u32Ret = dupMemory(
                (void **)&pu8Buffer, request->iwr_pu8Buffer[i],
                request->iwr_sBuffer[i]);
            if (u32Ret == OLERR_NO_ERROR)
            {
                request->iwr_pu8Buffer[i] = pu8Buffer;
                request->iwr_bUserFree[1] = FALSE;
                request->iwr_amoMemOwner[i] = AS_MEM_OWNER_STATIC;
            }
        }
    }

    return u32Ret;
}

static u32 _internalWebclientOnResponse(
    asocket_t * pAsocket, olint_t InterruptFlag,
    packet_header_t * header, void * user, boolean_t * pbPause)
{
    u32 u32Ret = OLERR_NO_ERROR;


    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 destroyWebclient(void ** ppWebclient)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) *ppWebclient;
    hashtree_enumerator_t en;
    web_dataobject_t * piwd;
    olchar_t * pstrKey;
    olsize_t u32KeyLen;
    u32 u32Index;

    /*Iterate through all the web data objects*/
    initHashtreeEnumerator(&piw->iw_hData, &en);
    while (! isHashtreeEnumeratorEmptyNode(&en))
    {
        /*Free the web data object*/
        getHashtreeValue(&en, &pstrKey, &u32KeyLen, &piwd);
        _destroyWebDataobject(&piwd);
        moveHashtreeNext(&en);
    }
    finiHashtreeEnumerator(&en);

    if (piw->iw_ppAsockets != NULL)
    {
        for (u32Index = 0;
             (u32Index < piw->iw_u32NumOfAsockets) && (u32Ret == OLERR_NO_ERROR);
             u32Index ++)
        {
            if (piw->iw_ppAsockets[u32Index] != NULL)
                destroyAsocket(&(piw->iw_ppAsockets[u32Index]));
        }
    }

    /*Free all the other associated resources*/
    finiQueue(&piw->iw_bqBacklog);
    finiHashtree(&piw->iw_hIdle);
    finiHashtree(&piw->iw_hData);
    finiSyncMutex(&piw->iw_smLock);

    if (piw->iw_ppAsockets != NULL)
        xfree((void **)&(piw->iw_ppAsockets));

    return u32Ret;
}


u32 createWebclient(
    basic_chain_t * pbc, webclient_t ** ppWebclient, webclient_param_t * pwp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Index;
    asocket_param_t ap;
    internal_webclient_t *piw;

    assert((pbc != NULL) && (ppWebclient != NULL));
    assert((pwp != NULL) && (pwp->wp_nPoolSize > 0) &&
           (pwp->wp_nPoolSize < 100));

    u32Ret = xcalloc((void **)&piw, sizeof(internal_webclient_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        piw->iw_bcohHeader.bcoh_fnPreSelect = _preWebclientProcess;
        //piw->PostSelect = &_preWebclientProcess;
        piw->iw_bcChain = pbc;
        piw->iw_sBuffer =
            pwp->wp_sBuffer ? pwp->wp_sBuffer : INITIAL_BUFFER_SIZE;

        initHashtree(&piw->iw_hIdle);
        initQueue(&piw->iw_bqBacklog);
        initHashtree(&piw->iw_hData);

        piw->iw_u32NumOfAsockets = pwp->wp_nPoolSize;
        u32Ret = xmalloc(
            (void **)&(piw->iw_ppAsockets),
            pwp->wp_nPoolSize * sizeof(void *));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = initSyncMutex(&(piw->iw_smLock));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createUtimer(pbc, &(piw->iw_uUtimer));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = appendToBasicChain(pbc, piw);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&ap, 0, sizeof(asocket_param_t));
        ap.ap_sInitialBuf = piw->iw_sBuffer;
        ap.ap_fnOnData = _webclientOnData;
        ap.ap_fnOnConnect = _webclientOnConnect;
        ap.ap_fnOnDisconnect = _webclientOnDisconnect;
        ap.ap_fnOnSendOK = _webclientOnSendOK;

        // Create our pool of sockets
        for (u32Index = 0;
             (u32Index < pwp->wp_nPoolSize) && (u32Ret == OLERR_NO_ERROR);
             u32Index ++)
        {
            u32Ret = createAsocket(
                pbc, &(piw->iw_ppAsockets[u32Index]), &ap);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppWebclient = piw;
    else if (piw != NULL)
        destroyWebclient((void **)&piw);

    return u32Ret;
}

u32 pipelineWebRequest(
    void * WebClient, ip_addr_t * piaRemote, u16 u16Port,
    packet_header_t * packet, fnWebclientOnResponse_t fnOnResponse, void * user)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) WebClient;
    internal_web_request_t * request = NULL;

    logInfoMsg("pipeline web req");

    u32Ret = _newWebRequest(&request, 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getRawPacket(
            packet, (olchar_t **)&request->iwr_pu8Buffer[0],
            &request->iwr_sBuffer[0]);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        request->iwr_bUserFree[0] = FALSE;
        request->iwr_amoMemOwner[0] = AS_MEM_OWNER_STATIC;
        request->iwr_fnOnResponse =
            (fnOnResponse == NULL) ? _internalWebclientOnResponse : fnOnResponse;
        request->iwr_pUser = user;

        u32Ret = _processWebRequest(piw, piaRemote, u16Port, request);
    }

    return u32Ret;
}

u32 pipelineWebRequestEx(
    void * pWebclient, ip_addr_t * piaRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, boolean_t bStaticHeader,
    olchar_t * pstrBody, olsize_t sBody, boolean_t bStaticBody,
    fnWebclientOnResponse_t fnOnResponse, void * user)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_web_request_t * request;
    u32 u32NumOfBuffers;

    logInfoMsg("pipeline web req ex");
    logDataMsgWithAscii((u8 *)pstrHeader, sHeader, "HTTP request header:");
    if (pstrBody != NULL)
        logDataMsgWithAscii((u8 *)pstrBody, sBody, "HTTP request body:");

    u32NumOfBuffers = pstrBody != NULL ? 2 : 1;
    u32Ret = _newWebRequest(&request, u32NumOfBuffers);
    if (u32Ret == OLERR_NO_ERROR)
    {
        request->iwr_pu8Buffer[0] = (u8 *)pstrHeader;
        request->iwr_sBuffer[0] = sHeader;
        request->iwr_bUserFree[0] = TRUE;
        if (bStaticHeader)
            request->iwr_amoMemOwner[0] = AS_MEM_OWNER_STATIC;
        else
            request->iwr_amoMemOwner[0] = AS_MEM_OWNER_USER;

        if (pstrBody != NULL)
        {
            request->iwr_pu8Buffer[1] = (u8 *)pstrBody;
            request->iwr_sBuffer[1] = sBody;
            request->iwr_bUserFree[1] = TRUE;
            if (bStaticBody)
                request->iwr_amoMemOwner[1] = AS_MEM_OWNER_STATIC;
            else
                request->iwr_amoMemOwner[1] = AS_MEM_OWNER_USER;
        }

        request->iwr_fnOnResponse =
            (fnOnResponse == NULL) ? _internalWebclientOnResponse : fnOnResponse;
        request->iwr_pUser = user;

        u32Ret = _webRequestStaticMemory(request);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _processWebRequest(piw, piaRemote, u16Port, request);

    return u32Ret;
}


/** Returns the headers from a given web data object
 *
 *  @param pDataObject [in] the web data object to query

 *  @return the packet header
 */
packet_header_t * getPacketHeaderFromWebDataobject(
    web_dataobject_t * pDataObject)
{
    return (((internal_web_dataobject_t *) pDataObject)->iwd_pphHeader);
}

u32 deleteWebRequests(
    webclient_t * pWebclient, ip_addr_t * piaRemote, u16 u16Port)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_webclient_t *piw = (internal_webclient_t *) pWebclient;
    olchar_t addr[25];
    internal_web_dataobject_t *piwd;
    olint_t addrlen;
    internal_web_request_t *piwr;
    basic_queue_t bqRemove;
    boolean_t zero = FALSE;

    initQueue(&bqRemove);

    addrlen = _getStringHashKey(addr, piaRemote, u16Port);

    /* Are there any pending requests to this IP/Port combo */
//    acquireSyncMutex(&(piw->iw_smLock));
    if (hasHashtreeEntry(&piw->iw_hData, addr, addrlen))
    {
        /* Yes, iterate through them */
        getHashtreeEntry(
            &piw->iw_hData, addr, addrlen, (void **)&piwd);
        while (! isQueueEmpty(&piwd->iwd_baRequest))
        {
            /*Put all the pending requests into this queue, so we can
              trigger them outside of this lock*/
            piwr = (internal_web_request_t *)dequeue(&piwd->iwd_baRequest);
            u32Ret = enqueue(&bqRemove, piwr);
        }
    }
//    releaseSyncMutex(&(piw->iw_smLock));

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Lets iterate through all the requests that we need to get rid of */
        while (! isQueueEmpty(&bqRemove))
        {
            /* Tell the user, we are aborting these requests */
            piwr = (internal_web_request_t *) dequeue(&bqRemove);
            piwr->iwr_fnOnResponse(
                pWebclient, WIF_WEB_REQUEST_DELETED, NULL, piwr->iwr_pUser,
                &zero);

            _destroyWebRequest(&piwr);
        }
    }

    finiQueue(&bqRemove);

    return u32Ret;
}

/** Resumes a paused connection. If the client has set the pause flag, the
 *  underlying socket will no longer read data from the NIC. This method resumes
 *  the socket.
 *
 *  @param piwd [in] the associated web data object
 */
u32 resumeWebDataobject(web_dataobject_t * pDataobject)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_web_dataobject_t * piwd =
        (internal_web_dataobject_t *) pDataobject;

    piwd->iwd_bPause = FALSE;
    u32Ret = resumeAsocket(piwd->iwd_pasSock);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


