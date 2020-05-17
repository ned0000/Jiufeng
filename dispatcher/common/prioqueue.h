/**
 *  @file dispatcher/common/prioqueue.h
 *
 *  @brief Header file which defines the priority queue data type.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The object is based on the queue data type.
 *  -# The object is thread safe.
 */

#ifndef DISPATCHER_COMMON_PRIO_QUEUE_H
#define DISPATCHER_COMMON_PRIO_QUEUE_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_queue.h"

#include "dispatchercommon.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define priority queue data type.
 */
typedef void  dispatcher_prio_queue_t;

typedef struct
{
    u32 cdpqp_u32MaxNumMsg;
    u32 cdpqp_u32Reserved[15];
} create_dispatcher_prio_queue_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a priority Queue.
 *
 *  @param ppQueue [out] The priority queue to be created.
 *  @param param [in] The parameter for creating the queue.
 *
 *  @return Void.
 */
u32 createDispatcherPrioQueue(
    dispatcher_prio_queue_t ** ppQueue, create_dispatcher_prio_queue_param_t * param);

/** Destroy the priority queue.
 *
 *  @param ppQueue [in] The queue to finalize.
 *  
 *  @return Void.
 */
u32 destroyDispatcherPrioQueue(dispatcher_prio_queue_t ** ppQueue);

/** Check to see if the priority queue is empty.
 *
 *  @param pQueue [in] The queue to check.
 *
 *  @return The queue empty state.
 *  @retval TRUE the queue is empty.
 *  @retval FALSE the queue is not empty.
 */
boolean_t isEmptyDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue);

/** Add an item to the priority queue.
 *
 *  @note
 *  -# The item is added to the end of the queue.
 *
 *  @param pQueue [in] The queue to add.
 *  @param pMsg [in] The data to add to the queue.
 *
 *  @return The error code.
 */
u32 enqueueDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue, dispatcher_msg_t * pMsg);

/** Remove an item from the priority queue.
 *
 *  @note
 *  -# The item is removed from the head of the queue.
 *
 *  @param pQueue [in] The queue to remove an item from.
 *
 *  @return The queue entry.
 */
dispatcher_msg_t * dequeueDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue);

/** Peek an item from the priority queue.
 *
 *  @note
 *  -# After peek, the item is still in the queue.
 *
 *  @param pQueue [in] The queue to peek an item from.
 *
 *  @return The queue entry.
 */
dispatcher_msg_t * peekDispatcherPrioQueue(dispatcher_prio_queue_t * pQueue);

#endif /*DISPATCHER_COMMON_PRIO_QUEUE_H*/

/*------------------------------------------------------------------------------------------------*/
