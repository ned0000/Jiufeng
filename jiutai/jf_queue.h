/**
 *  @file jf_queue.h
 *
 *  @brief The base queue data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_queue object.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# The object is not thread safe.
 *  
 */

#ifndef JIUTAI_QUEUE_H
#define JIUTAI_QUEUE_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define queue node data type.
 */
typedef struct jf_queue_node
{
    /**The data.*/
    void * jqn_pData;
    /**The next queue node.*/
    struct jf_queue_node * jqn_pjqnNext;
} jf_queue_node_t;

/** Define queue data type.
 */
typedef struct jf_queue
{
    /**The head of the queue node list.*/
    jf_queue_node_t * jq_pjqnHead;
    /**The tail of the queue node list.*/
    jf_queue_node_t * jq_pjqnTail;
} jf_queue_t;

/** The callback function for freeing the queue data.
 */
typedef u32 (* jf_queue_fnFreeData_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize an empty Queue.
 *
 *  @param pQueue [in] The basic queue to be initialized.
 *
 *  @return Void.
 */
void jf_queue_init(jf_queue_t * pQueue);

/** Finalize the queue.
 *
 *  @param pQueue [in] The queue to finalize.
 *  
 *  @return Void.
 */
void jf_queue_fini(jf_queue_t * pQueue);

/** Finalize the queue and data.
 *
 *  @param pQueue [in] The linklist to finalize.
 *  @param fnFreeData [in] The call back function to free data.
 *
 *  @return Void.
 */
void jf_queue_finiQueueAndData(jf_queue_t * pQueue, jf_queue_fnFreeData_t fnFreeData);

/** Check to see if a queue is empty.
 *
 *  @param pQueue [in] The queue to check.
 *
 *  @return The queue empty state.
 *  @retval TRUE the queue is empty.
 *  @retval FALSE the queue is not empty.
 */
boolean_t jf_queue_isEmpty(jf_queue_t * pQueue);

/** Add an item to the queue.
 *
 *  @param pQueue [in] The queue to add.
 *  @param data [in] The data to add to the queue.
 *
 *  @return The error code.
 */
u32 jf_queue_enqueue(jf_queue_t * pQueue, void * data);

/** Remove an item from the queue.
 *
 *  @param pQueue [in] The queue to remove an item from.
 *
 *  @return The queue entry.
 */
void * jf_queue_dequeue(jf_queue_t * pQueue);

/** Peek an item from the queue.
 *
 *  @param pQueue [in] The queue to peek an item from.
 *
 *  @return The queue entry.
 */
void * jf_queue_peek(jf_queue_t * pQueue);

#endif /*JIUTAI_QUEUE_H*/

/*------------------------------------------------------------------------------------------------*/

