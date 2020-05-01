/**
 *  @file dispatcherxfer.c
 *
 *  @brief The implentation file for dispatcher xfer library.
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
#include "jf_jiukun.h"
#include "jf_hashtree.h"
#include "jf_queue.h"

#include "dispatchercommon.h"
#include "xferpool.h"
#include "dispatcherxfer.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** This initial size of the receive buffer.
 */
#define DISPATCHER_XFER_INITIAL_BUFFER_SIZE            (2048)

/** Define the internal dispatcher xfer data type.
 */
typedef struct internal_dispatcher_xfer
{
    /**The network chain object header.*/
    jf_network_chain_object_header_t idx_jncohHeader;

    u32 idx_u32Reserved[4];

    /*The network chain.*/
    jf_network_chain_t * idx_pjncChain;

    /**Mutex lock for the message queue.*/
    jf_mutex_t idx_jmMsg;
    /**Message queue.*/
    jf_queue_t idx_jqMsg;
    /**xfer is paused if it's TRUE.*/
    boolean_t idx_bPause;
    u8 idx_u8Reserved[7];
    /**Number of high priority message.*/
    u16 idx_u16NumOfHighPrioMsg;
    /**Number of mid priority message.*/
    u16 idx_u16NumOfMidPrioMsg;
    /**Number of low priority message.*/
    u16 idx_u16NumOfLowPrioMsg;

    u16 idx_u16Reserved;
    /**Maximum number of message allowed in the queue.*/
    u32 idx_u32MaxNumMsg;
    /**The xfer object pool*/
    dispatcher_xfer_object_pool_t * idx_pdxopPool;

} internal_dispatcher_xfer_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _enqueueDispatcherXferMsgToQueue(
    internal_dispatcher_xfer_t * pidx, dispatcher_msg_t * pdm, boolean_t * pbWakeup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mutex_acquire(&pidx->idx_jmMsg);
    /*If the request queue is empty, chain should be waken up.*/
    *pbWakeup = jf_queue_isEmpty(&pidx->idx_jqMsg);
    jf_queue_enqueue(&pidx->idx_jqMsg, pdm);
    jf_mutex_release(&pidx->idx_jmMsg);

    return u32Ret;
}

static dispatcher_msg_t * _dequeueDispatcherXferMsgFromQueue(internal_dispatcher_xfer_t * pidx)
{
    dispatcher_msg_t * pdm = NULL;

    jf_mutex_acquire(&pidx->idx_jmMsg);
    pdm = jf_queue_dequeue(&pidx->idx_jqMsg);
    jf_mutex_release(&pidx->idx_jmMsg);

    return pdm;
}

static dispatcher_msg_t * _peekDispatcherXferMsgFromQueue(internal_dispatcher_xfer_t * pidx)
{
    dispatcher_msg_t * pdm = NULL;

    jf_mutex_acquire(&pidx->idx_jmMsg);
    pdm = jf_queue_peek(&pidx->idx_jqMsg);
    jf_mutex_release(&pidx->idx_jmMsg);

    return pdm;
}

static u32 _sendDispatcherXferMsg(internal_dispatcher_xfer_t * pidx)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;
    boolean_t bPause = FALSE;

    /*If the pause flag is set, do not send any message.*/
    jf_mutex_acquire(&pidx->idx_jmMsg);
    bPause = pidx->idx_bPause;
    jf_mutex_release(&pidx->idx_jmMsg);

    if (bPause)
        return u32Ret;

    /*Peek the message from queue. The message is destroyed when it's sent.*/
    pdm = _peekDispatcherXferMsgFromQueue(pidx);
    if (pdm != NULL)
        u32Ret = sendDispatcherXferPoolMsg(pidx->idx_pdxopPool, pdm);

    return u32Ret;
}

/** Pre select handler for chain.
 *
 *  @param pXfer [in] The web client object.
 *  @param readset [out] The read fd set.
 *  @param writeset [out] The write fd set.
 *  @param errorset [out] The error fd set.
 *  @param pu32BlockTime [out] The block time in millisecond.
 *
 *  @return The error code.
 */
static u32 _preDispatcherXferProcess(
    void * pXfer, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;

    JF_LOGGER_DEBUG("pre dispatcher xfer");

    u32Ret = _sendDispatcherXferMsg(pidx);

    return u32Ret;
}

static u32 _fnOnDispatcherXferObjectEvent(
    dispatcher_xfer_object_event_t event, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *)pUser;
    dispatcher_msg_t * pdm = NULL;

    if (event == DISPATCHER_XFER_OBJECT_EVENT_MSG_SENT)
    {
        /*Message is sent so we can remove it from queue and free it.*/
        JF_LOGGER_DEBUG("msg sent");
        pdm = _dequeueDispatcherXferMsgFromQueue(pidx);
        assert(pdm != NULL);
        freeDispatcherMsg(&pdm);

        u32Ret = _sendDispatcherXferMsg(pidx);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 dispatcher_xfer_destroy(dispatcher_xfer_t ** ppXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) *ppXfer;

    JF_LOGGER_DEBUG("destroy xfer");

    /*Destroy the xfer object pool.*/
    if (pidx->idx_pdxopPool != NULL)
        destroyDispatcherXferObjectPool(&pidx->idx_pdxopPool);

    /*Finalize the message queue and free all the message.*/
    jf_queue_finiQueueAndData(&pidx->idx_jqMsg, fnFreeDispatcherMsg);
    jf_mutex_fini(&pidx->idx_jmMsg);

    jf_jiukun_freeMemory((void **)ppXfer);

    return u32Ret;
}

u32 dispatcher_xfer_create(
    jf_network_chain_t * pjnc, dispatcher_xfer_t ** ppXfer, dispatcher_xfer_create_param_t * pdxcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_xfer_pool_create_param_t dxpcp;
    internal_dispatcher_xfer_t * pidx = NULL;

    assert((pjnc != NULL) && (ppXfer != NULL));
    assert((pdxcp != NULL) && (pdxcp->dxcp_sMaxMsg != 0));

    JF_LOGGER_INFO("name: %s", pdxcp->dxcp_pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&pidx, sizeof(internal_dispatcher_xfer_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the dispatcher xfer.*/
        ol_bzero(pidx, sizeof(internal_dispatcher_xfer_t));
        /*The callback function for network chain object header.*/
        pidx->idx_jncohHeader.jncoh_fnPreSelect = _preDispatcherXferProcess;
        pidx->idx_pjncChain = pjnc;
        pidx->idx_u32MaxNumMsg = pdxcp->dxcp_u32MaxNumMsg;

        jf_mutex_init(&pidx->idx_jmMsg);
        jf_queue_init(&pidx->idx_jqMsg);

        /*Add the oject header to network chain.*/
        u32Ret = jf_network_appendToChain(pjnc, pidx);
    }

    /*Create xfer object pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&dxpcp, sizeof(dxpcp));
        dxpcp.dxpcp_sBuffer =
            pdxcp->dxcp_sMaxMsg ? pdxcp->dxcp_sMaxMsg : DISPATCHER_XFER_INITIAL_BUFFER_SIZE;
        dxpcp.dxpcp_pjiRemote = pdxcp->dxcp_pjiRemote;
        dxpcp.dxpcp_u16RemotePort = pdxcp->dxcp_u16RemotePort;
        dxpcp.dxpcp_pstrName = pdxcp->dxcp_pstrName;
        dxpcp.dxpcp_fnOnEvent = _fnOnDispatcherXferObjectEvent;
        dxpcp.dxpcp_pUser = pidx;

        u32Ret = createDispatcherXferObjectPool(pjnc, &pidx->idx_pdxopPool, &dxpcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppXfer = pidx;
    else if (pidx != NULL)
        dispatcher_xfer_destroy((void **)&pidx);

    return u32Ret;
}

u32 dispatcher_xfer_pause(dispatcher_xfer_t * pXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;

    /*Set the flag.*/
    jf_mutex_acquire(&pidx->idx_jmMsg);
    pidx->idx_bPause = TRUE;
    jf_mutex_release(&pidx->idx_jmMsg);

    return u32Ret;
}

u32 dispatcher_xfer_resume(dispatcher_xfer_t * pXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;

    /*Clear the flag.*/
    jf_mutex_acquire(&pidx->idx_jmMsg);
    pidx->idx_bPause = FALSE;
    jf_mutex_release(&pidx->idx_jmMsg);

    /*Wakeup the network chain.*/
    u32Ret = jf_network_wakeupChain(pidx->idx_pjncChain);

    return u32Ret;
}

u32 dispatcher_xfer_sendMsg(dispatcher_xfer_t * pXfer, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;
    boolean_t bWakeup = FALSE;

    JF_LOGGER_INFO("send msg");

    /*Add the message to queue.*/
    u32Ret = _enqueueDispatcherXferMsgToQueue(pidx, pdm, &bWakeup);

    if ((u32Ret == JF_ERR_NO_ERROR) && bWakeup)
    {
        u32Ret = jf_network_wakeupChain(pidx->idx_pjncChain);
    }

    return u32Ret;
}

u32 dispatcher_xfer_clearMsgQueue(dispatcher_xfer_t * pXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;
    dispatcher_msg_t * pdm = NULL;

    /*TODO: delete the message in the xfer object as the message is going to be destroyed.*/

    jf_mutex_acquire(&pidx->idx_jmMsg);
    pdm = jf_queue_dequeue(&pidx->idx_jqMsg);
    while (pdm != NULL)
    {
        freeDispatcherMsg(&pdm);

        pdm = jf_queue_dequeue(&pidx->idx_jqMsg);
    }
    jf_mutex_release(&pidx->idx_jmMsg);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

