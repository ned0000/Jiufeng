/**
 *  @file dispatcher/common/prioqueue.c
 *
 *  @brief The implementation file for priority queue data type.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <unistd.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_mutex.h"

#include "prioqueue.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    /**Mutex lock for the message queue.*/
    jf_mutex_t idpq_jmMsg;
    /**Message queue.*/
    jf_queue_t idpq_jqMsg;

    /**Number of high priority message.*/
    u32 idpq_u32NumOfHighPrioMsg;
    /**Number of mid priority message.*/
    u32 idpq_u32NumOfMidPrioMsg;
    /**Number of low priority message.*/
    u32 idpq_u32NumOfLowPrioMsg;
    /**Maximum number of message allowed in the queue.*/
    u32 idpq_u32MaxNumMsg;

} internal_dispatcher_prio_queue_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _getNumOfMsgInDispatcherPrioQueue(internal_dispatcher_prio_queue_t * prioqueue)
{
    u32 u32Num = 0;

    u32Num = prioqueue->idpq_u32NumOfHighPrioMsg + prioqueue->idpq_u32NumOfMidPrioMsg +
        prioqueue->idpq_u32NumOfLowPrioMsg;

    return u32Num;
}

/** Increase number of priority message in queue.
 */
static void _incNumOfMsgInDispatcherPrioQueue(internal_dispatcher_prio_queue_t * prioqueue, u8 u8MsgPrio)
{
    if (u8MsgPrio == JF_MESSAGING_PRIO_HIGH)
        prioqueue->idpq_u32NumOfHighPrioMsg ++;
    else if (u8MsgPrio == JF_MESSAGING_PRIO_MID)
        prioqueue->idpq_u32NumOfMidPrioMsg ++;
    else
        prioqueue->idpq_u32NumOfLowPrioMsg ++;
}

/** Decrease number of priority message in queue.
 */
static void _decNumOfMsgInDispatcherPrioQueue(internal_dispatcher_prio_queue_t * prioqueue, u8 u8MsgPrio)
{
    if (u8MsgPrio == JF_MESSAGING_PRIO_HIGH)
        prioqueue->idpq_u32NumOfHighPrioMsg --;
    else if (u8MsgPrio == JF_MESSAGING_PRIO_MID)
        prioqueue->idpq_u32NumOfMidPrioMsg --;
    else
        prioqueue->idpq_u32NumOfLowPrioMsg --;
}

static boolean_t _canDispatcherMsgBeAdded(internal_dispatcher_prio_queue_t * prioqueue, u8 u8MsgPrio)
{
    boolean_t bRet = FALSE, bDequeue = FALSE;
    u32 u32Num = _getNumOfMsgInDispatcherPrioQueue(prioqueue);
    dispatcher_msg_t * msg = NULL;

    /*Maximum number of message is not reached, return NO ERROR.*/
    if (u32Num < prioqueue->idpq_u32MaxNumMsg)
        return TRUE;

    /*Check if the new message can be added to queue.*/
    if (u8MsgPrio == JF_MESSAGING_PRIO_HIGH)
    {
        /*For high priority message, remove the oldest message anyway.*/
        bDequeue = TRUE;
    }
    else if (u8MsgPrio == JF_MESSAGING_PRIO_MID)
    {
        /*For middle priority message, remove the oldest message if no high priority message in queue.*/
        if (prioqueue->idpq_u32NumOfHighPrioMsg == 0)
            bDequeue = TRUE;
    }
    else
    {
        if ((prioqueue->idpq_u32NumOfHighPrioMsg == 0) && (prioqueue->idpq_u32NumOfMidPrioMsg == 0))
            bDequeue = TRUE;
    }

    if (bDequeue)
    {
        JF_LOGGER_DEBUG("dequeue msg");
        msg = jf_queue_dequeue(&prioqueue->idpq_jqMsg);
        freeDispatcherMsg(&msg);

        _decNumOfMsgInDispatcherPrioQueue(prioqueue, u8MsgPrio);
        bRet = TRUE;
    }

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherPrioQueue(
    dispatcher_prio_queue_t ** ppQueue, create_dispatcher_prio_queue_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_prio_queue_t * prioqueue = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&prioqueue, sizeof(*prioqueue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(prioqueue, sizeof(*prioqueue));

        prioqueue->idpq_u32MaxNumMsg = param->cdpqp_u32MaxNumMsg;
        jf_queue_init(&prioqueue->idpq_jqMsg);

        u32Ret = jf_mutex_init(&prioqueue->idpq_jmMsg);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppQueue = prioqueue;
    else if (prioqueue != NULL)
        destroyDispatcherPrioQueue((dispatcher_prio_queue_t **)&prioqueue);

    return u32Ret;
}

u32 destroyDispatcherPrioQueue(dispatcher_prio_queue_t ** ppQueue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_prio_queue_t * prioqueue = *ppQueue;

	assert(ppQueue != NULL);

    jf_queue_fini(&prioqueue->idpq_jqMsg);

    jf_mutex_fini(&prioqueue->idpq_jmMsg);

    return u32Ret;
}

u32 destroyDispatcherPrioQueueAndData(
    dispatcher_prio_queue_t ** ppQueue, jf_queue_fnFreeData_t fnFreeData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_prio_queue_t * prioqueue = *ppQueue;

	assert(ppQueue != NULL);

    jf_queue_finiQueueAndData(&prioqueue->idpq_jqMsg, fnFreeData);

    jf_mutex_fini(&prioqueue->idpq_jmMsg);

    return u32Ret;
}

boolean_t isDispatcherPrioQueueEmpty(dispatcher_prio_queue_t * pQueue)
{
    internal_dispatcher_prio_queue_t * prioqueue = pQueue;
    return jf_queue_isEmpty(&prioqueue->idpq_jqMsg);
}

u32 enqueueDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue, dispatcher_msg_t * pMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_prio_queue_t * prioqueue = pQueue;
    u8 u8MsgPrio = getDispatcherMsgPrio(pMsg);

    jf_mutex_acquire(&prioqueue->idpq_jmMsg);

    if (_canDispatcherMsgBeAdded(prioqueue, u8MsgPrio))
    {
        u32Ret = jf_queue_enqueue(&prioqueue->idpq_jqMsg, pMsg);
        if (u32Ret == JF_ERR_NO_ERROR)
            _incNumOfMsgInDispatcherPrioQueue(prioqueue, u8MsgPrio);
    }
    else
    {
        JF_LOGGER_DEBUG("msg queue full");
        u32Ret = JF_ERR_MSG_QUEUE_FULL;
    }

    jf_mutex_release(&prioqueue->idpq_jmMsg);

    return u32Ret;
}

dispatcher_msg_t * dequeueDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue)
{
    dispatcher_msg_t * retval = NULL;
    internal_dispatcher_prio_queue_t * prioqueue = pQueue;

    jf_mutex_acquire(&prioqueue->idpq_jmMsg);

    retval = jf_queue_dequeue(&prioqueue->idpq_jqMsg);
    _decNumOfMsgInDispatcherPrioQueue(prioqueue, getDispatcherMsgPrio(retval));
    freeDispatcherMsg(&retval);

    jf_mutex_release(&prioqueue->idpq_jmMsg);

    return retval;
}

dispatcher_msg_t * peekDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue)
{
    dispatcher_msg_t * retval = NULL;
    internal_dispatcher_prio_queue_t * prioqueue = pQueue;

    jf_mutex_acquire(&prioqueue->idpq_jmMsg);

    retval = jf_queue_peek(&prioqueue->idpq_jqMsg);

    jf_mutex_release(&prioqueue->idpq_jmMsg);

    return retval;
}

/*------------------------------------------------------------------------------------------------*/
