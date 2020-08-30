/**
 *  @file jf_attask.c
 *
 *  @brief Implemention file for at task object which is task container and scheduler.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_attask.h"
#include "jf_time.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Default block time in second.
 */
#define JF_ATTASK_DEF_BLOCK_TIME         (10 * JF_TIME_SECOND_TO_MILLISECOND);

/** Define the attask item data type.
 */
typedef struct attask_item
{
    /**Expire time in milli-second.*/
    u64 ai_u64Expire;
    /**The user's data.*/
    void * ai_pData;
    /**The callback function when the time is expired.*/
    jf_attask_fnCallbackOfItem_t ai_fnCallback;
    /**The callback function to destroy user's data.*/
    jf_attask_fnDestroyItem_t ai_fnDestroy;
    /**Previous item in the list.*/
    struct attask_item * ai_paiPrev;
    /**Next item in the list.*/
    struct attask_item * ai_paiNext;
} attask_item_t;

/** Define the internal attask data type.
 */
typedef struct attask
{
    /**The head of item list.*/
    attask_item_t * ia_paiItems;
} internal_attask_t;

/* --- private routine section ------------------------------------------------------------------ */

/** Free attask item list.
 *
 *  @param item [in] The attask item list.
 *  @param bCallback [in] Call the callback function if it's TRUE.
 *
 *  @return Void.
 */
static void _freeAttaskItemList(attask_item_t * item, boolean_t bCallback)
{
    attask_item_t * pai = item, * temp = NULL;

    while (pai != NULL)
    {
        temp = pai->ai_paiNext;

        if (bCallback)
            pai->ai_fnCallback(pai->ai_pData);

        if (pai->ai_fnDestroy != NULL)
            pai->ai_fnDestroy(&pai->ai_pData);

        jf_jiukun_freeMemory((void **)&pai);

        pai = temp;
    }

}

/** Flushes all task from the attask.
 *
 *  @note
 *  -# Before destroying the attask item structure, the item data is destroyed by callback function
 *   if it's available.
 *
 *  @param piu [in] The internal attask object.
 *
 *  @return The error code.
 */
static u32 _flushAttask(internal_attask_t * piu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    attask_item_t * temp = NULL;

    temp = piu->ia_paiItems;
    piu->ia_paiItems = NULL;

    _freeAttaskItemList(temp, FALSE);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_attask_check(jf_attask_t * pAttask, u32 * pu32Blocktime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_time_spec_t jts;
    attask_item_t * temp = NULL, * evt = NULL, * last = NULL;
    u64 current = 0;
    internal_attask_t * pia = (internal_attask_t *)pAttask;

    assert(pia != NULL);

    *pu32Blocktime = JF_ATTASK_DEF_BLOCK_TIME;

    /*Return if the item list is empty.*/
    if (pia->ia_paiItems == NULL)
        return u32Ret;

    /*Get the current tick count for reference.*/
    u32Ret = jf_time_getClockTime(JF_TIME_CLOCK_MONOTONIC_RAW, &jts);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Current tick in Millisecond.*/
        current = (jts.jts_u64Second * JF_TIME_SECOND_TO_MILLISECOND) + 
            (jts.jts_u64NanoSecond / JF_TIME_MILLISECOND_TO_NANOSECOND);
        temp = pia->ia_paiItems;
        /*Keep looping until we find a node that doesn't need to be triggered.*/
        while ((temp != NULL) && (temp->ai_u64Expire <= current))
        {
            /*Since these are in sorted order, evt will always point to the first node that needs to
              be triggered. last will point to the last node that needs to be triggered. temp will
              point to the first node that doesn't need to be triggered.*/
            evt = pia->ia_paiItems;
            last = temp;
            temp = temp->ai_paiNext;
        }

        if (evt != NULL)
        {
            if (temp != NULL)
            {
                /*There are still nodes that will need to be triggered later, so reset it.
                  temp point to the node that need to be triggered.*/
                pia->ia_paiItems = temp;
                pia->ia_paiItems->ai_paiPrev = NULL;
                last->ai_paiNext = NULL;
            }
            else
            {
                /*There are no more nodes that will need to be triggered later.*/
                pia->ia_paiItems = NULL;
            }
        }

        /*Iterate through all the triggers that we need to fire*/
        _freeAttaskItemList(evt, TRUE);

        /*If there are more triggers that need to be fired later, we need to recalculate what the
          max block time for our select should be.*/
        if (pia->ia_paiItems != NULL)
            *pu32Blocktime = (u32)(pia->ia_paiItems->ai_u64Expire - current);
    }

    return u32Ret;
}

u32 jf_attask_addItem(
    jf_attask_t * pAttask, void * pData, u32 u32Milliseconds,
    jf_attask_fnCallbackOfItem_t fnCallback, jf_attask_fnDestroyItem_t fnDestroy)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_time_spec_t jts;
    attask_item_t * pai = NULL, * temp = NULL;
    internal_attask_t * pia = (internal_attask_t *) pAttask;

    u32Ret = jf_jiukun_allocMemory((void **)&pai, sizeof(attask_item_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Get the current time for reference.*/
        u32Ret = jf_time_getClockTime(JF_TIME_CLOCK_MONOTONIC_RAW, &jts);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pai, sizeof(attask_item_t));
        /*Set the trigger time.*/
        pai->ai_u64Expire = (jts.jts_u64Second * JF_TIME_SECOND_TO_MILLISECOND) +
            (jts.jts_u64NanoSecond / JF_TIME_MILLISECOND_TO_NANOSECOND) + u32Milliseconds;
        pai->ai_pData = pData;

        /*Set the callback handlers.*/
        pai->ai_fnCallback = fnCallback;
        pai->ai_fnDestroy = fnDestroy;

        if (pia->ia_paiItems == NULL)
        {
            /*There are no current triggers, so this is the first, which also means, the select
              timeout may not be short enough.*/
            pia->ia_paiItems = pai;
        }
        else
        {
            /*There are already triggers, so we just insert this one in sorted order.*/
            temp = pia->ia_paiItems;
            while (temp != NULL)
            {
                if (pai->ai_u64Expire <= temp->ai_u64Expire)
                {
                    pai->ai_paiNext = temp;
                    if (temp->ai_paiPrev == NULL)
                    {
                        /*This is the shortest trigger, so again, the select timeout may not be
                          short enough, so we need to force unblock, to recalculate the timeout for
                          the select.*/
                        pia->ia_paiItems = pai;
                        temp->ai_paiPrev = pai;
                    }
                    else
                    {
                        /*This isn't the shortest trigger, so we are gauranteed that the thread will 
                          unblock in time to reset the timeouts.*/
                        pai->ai_paiPrev = temp->ai_paiPrev;
                        temp->ai_paiPrev->ai_paiNext = pai;
                        temp->ai_paiPrev = pai;
                    }
                    break;
                }
                else if (temp->ai_paiNext == NULL)
                {
                    /*If there aren't any more triggers left, this means we have the largest
                      trigger, so just tack it on the end.*/
                    pai->ai_paiNext = NULL;
                    pai->ai_paiPrev = temp;
                    temp->ai_paiNext = pai;
                    break;
                }
                temp = temp->ai_paiNext;
            }
        }
    }

    return u32Ret;
}

u32 jf_attask_removeItem(jf_attask_t * pAttask, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia = (internal_attask_t *) pAttask;
    attask_item_t * first = NULL, * last = NULL, * evt = NULL;

    evt = last = NULL;

    first = pia->ia_paiItems;
    while (first != NULL)
    {
        if (first->ai_pData == pData)
        {
            /*Found a match, now remove it from the list.*/
            if (first->ai_paiPrev == NULL)
            {
                pia->ia_paiItems = first->ai_paiNext;
                if (pia->ia_paiItems != NULL)
                {
                    pia->ia_paiItems->ai_paiPrev = NULL;
                }
            }
            else
            {
                first->ai_paiPrev->ai_paiNext = first->ai_paiNext;
                if (first->ai_paiNext != NULL)
                {
                    first->ai_paiNext->ai_paiPrev = first->ai_paiPrev;
                }
            }

            /*Add the removed item to temporary item list.*/
            if (evt == NULL)
            {
                /*If this is the first match, create a new list.*/
                evt = last = first;
                evt->ai_paiPrev = evt->ai_paiNext = NULL;
            }
            else
            {
                /*Attach this match to the end of the list of matches.*/
                last->ai_paiNext = first;
                first->ai_paiPrev = last;
                first->ai_paiNext = NULL;
                last = first;
            }
        }
        first = first->ai_paiNext;
    }

    /*Iterate through each node that is to be removed.*/
    if (evt == NULL)
        u32Ret = JF_ERR_ATTASK_ITEM_NOT_FOUND;
    else
        _freeAttaskItemList(evt, FALSE);

    return u32Ret;
}

u32 jf_attask_destroy(jf_attask_t ** ppAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia = NULL;

    assert((ppAttask != NULL) && (*ppAttask != NULL));

    pia = (internal_attask_t *) *ppAttask;

    _flushAttask(pia);

    jf_jiukun_freeMemory(ppAttask);

    return u32Ret;
}

u32 jf_attask_create(jf_attask_t ** ppAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia;

    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_attask_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia, sizeof(internal_attask_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAttask = pia;
    else if (pia != NULL)
        jf_attask_destroy((void **)&pia);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

