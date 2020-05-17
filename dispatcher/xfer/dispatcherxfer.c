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
#include "prioqueue.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the internal dispatcher xfer data type.
 */
typedef struct internal_dispatcher_xfer
{
    /**The network chain.*/
    jf_network_chain_t * idx_pjncChain;

    u8 idx_u8Reserved[32];

    /**The xfer object pool.*/
    dispatcher_xfer_object_pool_t * idx_pdxopPool;

} internal_dispatcher_xfer_t;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 dispatcher_xfer_destroy(dispatcher_xfer_t ** ppXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) *ppXfer;

    JF_LOGGER_DEBUG("destroy xfer");

    /*Destroy the xfer object pool.*/
    if (pidx->idx_pdxopPool != NULL)
        destroyDispatcherXferObjectPool(&pidx->idx_pdxopPool);

    jf_jiukun_freeMemory(ppXfer);

    return u32Ret;
}

u32 dispatcher_xfer_create(
    jf_network_chain_t * pjnc, dispatcher_xfer_t ** ppXfer, dispatcher_xfer_create_param_t * pdxcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_xfer_pool_create_param_t dxpcp;
    internal_dispatcher_xfer_t * pidx = NULL;

    assert((pjnc != NULL) && (ppXfer != NULL));
    assert((pdxcp != NULL) && (pdxcp->dxcp_pstrName != NULL));
    assert((pdxcp->dxcp_sMaxMsg > 0) && (pdxcp->dxcp_u32MaxNumMsg > 0));
    assert((pdxcp->dxcp_u32MaxAddress > 0) &&
           (pdxcp->dxcp_u32MaxAddress <= DISPATCHER_XFER_MAX_NUM_OF_ADDRESS));

    JF_LOGGER_INFO(
        "name: %s, sMaxMsg: %d, MaxNumMsg: %u, MaxAddress: %u",
        pdxcp->dxcp_pstrName, pdxcp->dxcp_sMaxMsg, pdxcp->dxcp_u32MaxNumMsg, pdxcp->dxcp_u32MaxAddress);

    u32Ret = jf_jiukun_allocMemory((void **)&pidx, sizeof(internal_dispatcher_xfer_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the dispatcher xfer.*/
        ol_bzero(pidx, sizeof(internal_dispatcher_xfer_t));
        pidx->idx_pjncChain = pjnc;
    }

    /*Create xfer object pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32 u32Index = 0;

        ol_bzero(&dxpcp, sizeof(dxpcp));
        dxpcp.dxpcp_sMaxMsg = pdxcp->dxcp_sMaxMsg;
        dxpcp.dxpcp_u32MaxNumMsg = pdxcp->dxcp_u32MaxNumMsg;
        dxpcp.dxpcp_u32MaxAddress = pdxcp->dxcp_u32MaxAddress;
        for (u32Index = 0; u32Index < dxpcp.dxpcp_u32MaxAddress; u32Index ++)
        {
            dxpcp.dxpcp_pjiRemote[u32Index] = pdxcp->dxcp_pjiRemote[u32Index];
            dxpcp.dxpcp_u16RemotePort[u32Index] = pdxcp->dxcp_u16RemotePort[u32Index];
        }
        dxpcp.dxpcp_pstrName = pdxcp->dxcp_pstrName;

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

    u32Ret = pauseDispatcherXferObjectPool(pidx->idx_pdxopPool);

    return u32Ret;
}

u32 dispatcher_xfer_resume(dispatcher_xfer_t * pXfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;

    u32Ret = resumeDispatcherXferObjectPool(pidx->idx_pdxopPool);

    return u32Ret;
}

u32 dispatcher_xfer_sendMsg(dispatcher_xfer_t * pXfer, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_xfer_t * pidx = (internal_dispatcher_xfer_t *) pXfer;

    u32Ret = sendMsgByDispatcherXferObjectPool(pidx->idx_pdxopPool, pdm);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
