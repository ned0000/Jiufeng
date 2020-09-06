/**
 *  @file jf_queue.c
 *
 *  @brief The implementation file for queue data type.
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
#include "jf_queue.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

void jf_queue_init(jf_queue_t * pQueue)
{
    pQueue->jq_pjqnHead = pQueue->jq_pjqnTail = NULL;
}

void jf_queue_fini(jf_queue_t * pQueue)
{
    jf_queue_node_t * pjqn, * temp;

	assert(pQueue != NULL);

	temp = pQueue->jq_pjqnHead;
    while (temp != NULL)
    {
        pjqn = temp->jqn_pjqnNext;
        jf_jiukun_freeMemory((void **)&temp);
        temp = pjqn;
    }
}

void jf_queue_finiQueueAndData(jf_queue_t * pQueue, jf_queue_fnFreeData_t fnFreeData)
{
    jf_queue_node_t * pjqn, * temp;

	assert(pQueue != NULL);

    /*Free the item in the queue one by one.*/
	temp = pQueue->jq_pjqnHead;
    while (temp != NULL)
    {
        pjqn = temp->jqn_pjqnNext;

        fnFreeData(&(temp->jqn_pData));

        jf_jiukun_freeMemory((void **)&temp);
        temp = pjqn;
    }

    /*Initialize the queue.*/
    jf_queue_init(pQueue);
}

boolean_t jf_queue_isEmpty(jf_queue_t * pQueue)
{
    return (pQueue->jq_pjqnHead == NULL ? TRUE : FALSE);
}

u32 jf_queue_enqueue(jf_queue_t * pQueue, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_queue_node_t * pjqn;

    u32Ret = jf_jiukun_allocMemory((void **)&pjqn, sizeof(*pjqn));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pjqn, sizeof(*pjqn));
        pjqn->jqn_pData = data;

        if (pQueue->jq_pjqnHead == NULL)
        {
            /*If there is no head, this new entry is the head.*/
            pQueue->jq_pjqnHead = pjqn;
            pQueue->jq_pjqnTail = pjqn;
        }
        else
        {
            /*Since there is already a head, just attach this entry to the tail, and call this
              the new tail.*/
            pQueue->jq_pjqnTail->jqn_pjqnNext = pjqn;
            pQueue->jq_pjqnTail = pjqn;
        }
    }

    return u32Ret;
}

void * jf_queue_dequeue(jf_queue_t * pQueue)
{
    jf_queue_node_t * temp = NULL;
    void * retval = NULL;

    assert(pQueue != NULL);

    if (pQueue->jq_pjqnHead == NULL)
        return NULL;

    temp = pQueue->jq_pjqnHead;
    retval = temp->jqn_pData;
    pQueue->jq_pjqnHead = pQueue->jq_pjqnHead->jqn_pjqnNext;
    if (pQueue->jq_pjqnHead == NULL)
    {
        pQueue->jq_pjqnTail = NULL;
    }
    jf_jiukun_freeMemory((void **)&temp);

    return retval;
}

void * jf_queue_peek(jf_queue_t * pQueue)
{
    if (pQueue->jq_pjqnHead == NULL)
        return NULL;
    else
        return pQueue->jq_pjqnHead->jqn_pData;
}

/*------------------------------------------------------------------------------------------------*/
