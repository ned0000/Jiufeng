/**
 *  @file attask.c
 *
 *  @brief task container and scheduler
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "attask.h"
#include "xtime.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
typedef struct attask_item
{
    u32 ai_u32Expire;
    void * ai_pData;
    fnCallbackOfAttaskItem_t ai_fnCallback;
    fnDestroyAttaskItem_t ai_fnDestroy;
    struct attask_item * ai_paiPrev;
    struct attask_item * ai_paiNext;
} attask_item_t;

typedef struct attask
{
    attask_item_t * ia_paiItems;
} internal_attask_t;

/* --- private routine section---------------------------------------------- */

/** Flushes all task from the attask
 *
 *  @note Before destroying the attask item structure, (* destroy)( ) is called
 *
 *  @param piu [in] the attask
 *
 *  @return the error code
 */
static u32 _flushAttask(internal_attask_t * piu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    attask_item_t *temp, *temp2;

    temp = piu->ia_paiItems;
    piu->ia_paiItems = NULL;

    while (temp != NULL)
    {
        temp2 = temp->ai_paiNext;
        if (temp->ai_fnDestroy != NULL)
        {
            temp->ai_fnDestroy(&(temp->ai_pData));
        }
        xfree((void **)&temp);
        temp = temp2;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 checkAttask(attask_t * pAttask, u32 * pu32Blocktime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timeval tv;
    attask_item_t * temp = NULL, * evt = NULL, * last = NULL;
    u32 current;
    internal_attask_t * pia = (internal_attask_t *)pAttask;

    assert(pia != NULL);

    if (pia->ia_paiItems != NULL)
    {
        /*Get the current tick count for reference*/
        getTimeOfDay(&tv);
        /*Current tick in Millisecond*/
        current = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        temp = pia->ia_paiItems;
        /*Keep looping until we find a node that doesn't need to be triggered*/
        while (temp != NULL && temp->ai_u32Expire <= current)
        {
            /*Since these are in sorted order, evt will always point to 
              the first node that needs to be triggered. last will point 
              to the last node that needs to be triggered. temp will point 
              to the first node that doesn't need to be triggered.*/
            evt = pia->ia_paiItems;
            last = temp;
            temp = temp->ai_paiNext;
        }

        if (evt != NULL)
        {
            if (temp != NULL)
            {
                /*There are still nodes that will need to be triggered 
                  later, so reset it.
                  temp point to the node that need to be triggered*/
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
        while (evt != NULL)
        {
            temp = evt->ai_paiNext;
            /*ToDo: We may want to check the table below first, 
              before we start triggering*/
            evt->ai_fnCallback(evt->ai_pData);

            if (evt->ai_fnDestroy != NULL)
            {
                evt->ai_fnDestroy(&(evt->ai_pData));
            }
            xfree((void **)&evt);

            evt = temp;
        }

        /*If there are more triggers that need to be fired later, we need to 
          recalculate what the max block time for our select should be*/
        if (pia->ia_paiItems != NULL)
            *pu32Blocktime = pia->ia_paiItems->ai_u32Expire - current;
        else
            *pu32Blocktime = INFINITE;
    }

    return u32Ret;
}

u32 addAttaskItem(
    attask_t * pAttask, void * pData, u32 u32Milliseconds,
    fnCallbackOfAttaskItem_t fnCallback, fnDestroyAttaskItem_t fnDestroy)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timeval tv;
    attask_item_t * pai, * temp;
    internal_attask_t * pia = (internal_attask_t *) pAttask;

    u32Ret = xcalloc((void **)&pai, sizeof(attask_item_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Get the current time for reference*/
        getTimeOfDay(&tv);

        /*Set the trigger time*/
        pai->ai_pData = pData;
        pai->ai_u32Expire = (tv.tv_sec * 1000) + (tv.tv_usec / 1000) + 
            u32Milliseconds;

        /*Set the callback handlers*/
        pai->ai_fnCallback = fnCallback;
        pai->ai_fnDestroy = fnDestroy;

        if (pia->ia_paiItems == NULL)
        {
            /*There are no current triggers, so this is the first, which also
              means, the select timeout may not be short enough*/
            pia->ia_paiItems = pai;
        }
        else
        {
            /*There are already triggers, so we just insert this one in sorted
              order*/
            temp = pia->ia_paiItems;
            while (temp != NULL)
            {
                if (pai->ai_u32Expire <= temp->ai_u32Expire)
                {
                    pai->ai_paiNext = temp;
                    if (temp->ai_paiPrev == NULL)
                    {
                        /*This is the shortest trigger, so again, the select
                          timeout may not be short enough, so we need to force
                          unblock, to recalculate the timeout for the select*/
                        pia->ia_paiItems = pai;
                        temp->ai_paiPrev = pai;
                    }
                    else
                    {
                        /*This isn't the shortest trigger, so we are 
                          gauranteed that the thread will 
                          unblock in time to reset the timeouts*/
                        pai->ai_paiPrev = temp->ai_paiPrev;
                        temp->ai_paiPrev->ai_paiNext = pai;
                        temp->ai_paiPrev = pai;
                    }
                    break;
                }
                else if (temp->ai_paiNext == NULL)
                {
                    /*If there aren't any more triggers left, this means 
                      we have the largest trigger, so just tack it on the end*/
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

u32 removeAttaskItem(attask_t * pAttask, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia = (internal_attask_t *) pAttask;
    attask_item_t *first, *last, *evt;

    evt = last = NULL;

    first = pia->ia_paiItems;
    while (first != NULL)
    {
        if (first->ai_pData == pData)
        {
            /*Found a match, now remove it from the list*/
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

            if (evt == NULL)
            {
                /*If this is the first match, create a new list*/
                evt = last = first;
                evt->ai_paiPrev = evt->ai_paiNext = NULL;
            }
            else
            {
                /*Attach this match to the end of the list of matches*/
                last->ai_paiNext = first;
                first->ai_paiPrev = last;
                first->ai_paiNext = NULL;
                last = first;
            }
        }
        first = first->ai_paiNext;
    }

    /*Iterate through each node that is to be removed*/
    if (evt == NULL)
        u32Ret = JF_ERR_ATTASK_ITEM_NOT_FOUND;
    else
        while (evt != NULL)
        {
            first = evt->ai_paiNext;
            if (evt->ai_fnDestroy != NULL)
            {
                evt->ai_fnDestroy(&(evt->ai_pData));
            }
            xfree((void **)&evt);
            evt = first;
        }

    return u32Ret;
}

u32 destroyAttask(attask_t ** ppAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia;

    assert((ppAttask != NULL) && (*ppAttask != NULL));

    pia = (internal_attask_t *) *ppAttask;

    _flushAttask(pia);

    xfree(ppAttask);
    *ppAttask = NULL;

    return u32Ret;
}

u32 createAttask(attask_t ** ppAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_attask_t * pia;

    u32Ret = xcalloc((void **)&pia, sizeof(internal_attask_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAttask = pia;
    else if (pia != NULL)
        destroyAttask((void **)&pia);

    return u32Ret;
}

/*-----------------------------------------------------------------*/

