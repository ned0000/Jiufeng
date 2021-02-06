/**
 *  @file webclient.c
 *
 *  @brief The implementation file of webclient library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_mutex.h"
#include "jf_network.h"
#include "jf_webclient.h"
#include "jf_jiukun.h"
#include "jf_queue.h"

#include "dataobjectpool.h"
#include "webclientrequest.h"
#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Initial size of the receive buffer.
 */
#define WEBCLIENT_INITIAL_BUFFER_SIZE         (2048)

/** Maximum number of data object for the webclient.
 */
#define MAX_WEBCLIENT_DATA_OBJECT             (100)

/** Define the internal webclient data type.
 */
typedef struct internal_webclient
{
    /**Network chain object header.*/
    jf_network_chain_object_header_t iw_jncohHeader;

    u32 iw_u32Reserved[4];

    /**Network chain.*/
    jf_network_chain_t *iw_pjncChain;

    /**Mutext lock for request queue.*/
    jf_mutex_t iw_jmReqeustQueueLock;
    /**Request queue.*/
    jf_queue_t iw_jqReqeustQueue;

    /**Webclient data object pool.*/
    webclient_dataobject_pool_t * iw_pwdpPool;
    
} internal_webclient_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _enqueueWebclientRequestToQueue(
    internal_webclient_t * piw, internal_webclient_request_t * piwr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mutex_acquire(&piw->iw_jmReqeustQueueLock);
    jf_queue_enqueue(&piw->iw_jqReqeustQueue, piwr);
    jf_mutex_release(&piw->iw_jmReqeustQueueLock);

    return u32Ret;
}

static internal_webclient_request_t * _dequeueWebclientRequestFromQueue(internal_webclient_t * piw)
{
    internal_webclient_request_t * piwr = NULL;

    jf_mutex_acquire(&piw->iw_jmReqeustQueueLock);
    piwr = jf_queue_dequeue(&piw->iw_jqReqeustQueue);
    jf_mutex_release(&piw->iw_jmReqeustQueueLock);

    return piwr;
}

static boolean_t _isEmptyWebclientRequestQueue(internal_webclient_t * piw)
{
    boolean_t bRet = FALSE;

    jf_mutex_acquire(&piw->iw_jmReqeustQueueLock);
    bRet = jf_queue_isEmpty(&piw->iw_jqReqeustQueue);
    jf_mutex_release(&piw->iw_jmReqeustQueueLock);

    return bRet;
}

/** Pre select handler for chain.
 *
 *  @param pWebclient [in] The web client object.
 *  @param readset [out] The read fd set.
 *  @param writeset [out] The write fd set.
 *  @param errorset [out] The error fd set.
 *  @param pu32BlockTime [out] The block time in millisecond.
 *
 *  @return The error code.
 */
static u32 _preWebclientProcess(
    void * pWebclient, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_webclient_request_t * piwr = NULL;

    /*Dequeue webclient request.*/
    piwr = _dequeueWebclientRequestFromQueue(piw);
    while ((piwr != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /*Process webclient request.*/
        u32Ret = processWebclientRequest(piw->iw_pwdpPool, piwr);

        /*Dequeue webclient request.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            piwr = _dequeueWebclientRequestFromQueue(piw);
    }

    return u32Ret;
}

static boolean_t _isWakeupChainRequired(internal_webclient_t * piw)
{
    boolean_t bWakeup = FALSE;

    /*if the request queue is empty, chain should be waken up*/
    if (_isEmptyWebclientRequestQueue(piw))
        bWakeup = TRUE;

    return bWakeup;
}

static u32 _fnCallbackDestroyWebclientRequest(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = destroyWebclientRequest((internal_webclient_request_t **)ppData);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_webclient_destroy(jf_webclient_t ** ppWebclient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) *ppWebclient;

    /*Destroy webclient data object pool.*/
    if (piw->iw_pwdpPool != NULL)
        destroyWebclientDataobjectPool(&piw->iw_pwdpPool);

    /*Finalize the request queue and data.*/
    jf_queue_finiQueueAndData(&piw->iw_jqReqeustQueue, _fnCallbackDestroyWebclientRequest);

    /*Finalize the mutex.*/
    jf_mutex_fini(&piw->iw_jmReqeustQueueLock);

    /*Free memory for internal webclient object.*/
    jf_jiukun_freeMemory((void **)ppWebclient);

    return u32Ret;
}

u32 jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient, jf_webclient_create_param_t * pjwcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    webclient_dataobject_pool_create_param_t wdpcp;
    internal_webclient_t * piw = NULL;

    assert((pjnc != NULL) && (ppWebclient != NULL));
    assert((pjwcp != NULL) &&
           (pjwcp->jwcp_u32PoolSize > 0) && (pjwcp->jwcp_u32PoolSize < MAX_WEBCLIENT_DATA_OBJECT));

    ol_bzero(&wdpcp, sizeof(wdpcp));

    /*Allocate memory for internal webclient object.*/
    u32Ret = jf_jiukun_allocMemory((void **)&piw, sizeof(internal_webclient_t));

    /*Initialize the internal webclient object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piw, sizeof(internal_webclient_t));
        piw->iw_jncohHeader.jncoh_fnPreSelect = _preWebclientProcess;
        piw->iw_pjncChain = pjnc;

        jf_mutex_init(&piw->iw_jmReqeustQueueLock);
        jf_queue_init(&piw->iw_jqReqeustQueue);

        /*Add to network chain.*/
        u32Ret = jf_network_appendToChain(pjnc, piw);
    }

    /*Create webclient data object pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        wdpcp.wdpcp_u32PoolSize = pjwcp->jwcp_u32PoolSize;
        wdpcp.wdpcp_sBuffer =
            pjwcp->jwcp_sBuffer ? pjwcp->jwcp_sBuffer : WEBCLIENT_INITIAL_BUFFER_SIZE;

        u32Ret = createWebclientDataobjectPool(pjnc, &piw->iw_pwdpPool, &wdpcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppWebclient = piw;
    else if (piw != NULL)
        jf_webclient_destroy((void **)&piw);

    return u32Ret;
}

u32 jf_webclient_sendHttpPacket(
    jf_webclient_t * pWebClient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_httpparser_packet_header_t * packet, jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebClient;
    internal_webclient_request_t * piwr = NULL;
    u8 * pu8Data[1] = {NULL};
    olsize_t sData[1];
    boolean_t bWakeup = FALSE;
    olchar_t strServer[64];

    getStringHashKey(strServer, pjiRemote, u16Port);
    JF_LOGGER_DEBUG("%s", strServer);

    /*Conver the HTTP packet to raw buffer.*/
    u32Ret = jf_httpparser_getRawPacket(packet, (olchar_t **)&pu8Data[0], &sData[0]);

    /*Create webclient request.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = createWebclientRequestSendData(
            &piwr, pu8Data, sData, 1, pjiRemote, u16Port, fnOnEvent, user);
    }

    /*Enqueue the webclient request.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bWakeup = _isWakeupChainRequired(piw);

        u32Ret = _enqueueWebclientRequestToQueue(piw, piwr);
    }

    /*Wakeup chain if necessary.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && bWakeup)
    {
        u32Ret = jf_network_wakeupChain(piw->iw_pjncChain);
    }

    /*Free the raw buffer.*/
    if (pu8Data[0] != NULL)
        jf_jiukun_freeMemory((void **)&pu8Data[0]);

    return u32Ret;
}

u32 jf_webclient_sendHttpHeaderAndBody(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, olchar_t * pstrBody, olsize_t sBody,
    jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_webclient_request_t * piwr = NULL;
    u16 u16NumOfData;
    u8 * pu8Data[2];
    olsize_t sData[2];
    boolean_t bWakeup = FALSE;

    JF_LOGGER_DATAA((u8 *)pstrHeader, sHeader, "HTTP request header:");
    if (pstrBody != NULL)
        JF_LOGGER_DATAA((u8 *)pstrBody, sBody, "HTTP request body:");

    u16NumOfData = (pstrBody != NULL) ? 2 : 1;

    pu8Data[0] = (u8 *)pstrHeader;
    sData[0] = sHeader;

    if (pstrBody != NULL)
    {
        pu8Data[1] = (u8 *)pstrBody;
        sData[1] = sBody;
    }

    /*Create webclient request.*/
    u32Ret = createWebclientRequestSendData(
        &piwr, pu8Data, sData, u16NumOfData, pjiRemote, u16Port, fnOnEvent, user);

    /*Enqueue the webclient request.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bWakeup = _isWakeupChainRequired(piw);

        u32Ret = _enqueueWebclientRequestToQueue(piw, piwr);
    }

    /*Wakeup chain if necessary.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && bWakeup)
    {
        u32Ret = jf_network_wakeupChain(piw->iw_pjncChain);
    }

    return u32Ret;
}

u32 jf_webclient_deleteRequest(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_t * piw = (internal_webclient_t *) pWebclient;
    internal_webclient_request_t * piwr = NULL;
    boolean_t bWakeup = FALSE;
    olchar_t strServer[64];

    getStringHashKey(strServer, pjiRemote, u16Port);
    JF_LOGGER_DEBUG("%s", strServer);

    /*Create webclient request.*/
    u32Ret = createWebclientRequestDeleteRequest(&piwr, pjiRemote, u16Port);

    /*Enqueue the webclient request.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        bWakeup = _isWakeupChainRequired(piw);

        u32Ret = _enqueueWebclientRequestToQueue(piw, piwr);
    }

    /*Wakeup chain if necessary.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && bWakeup)
    {
        u32Ret = jf_network_wakeupChain(piw->iw_pjncChain);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
