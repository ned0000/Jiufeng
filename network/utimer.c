/**
 *  @file utimer.c
 *
 *  @brief timer implementation
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
#include "xtime.h"
#include "network.h"
#include "syncmutex.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
typedef struct utimer_item
{
    u32 ui_u32Expire;
    void * ui_pData;
    fnCallbackOfUtimerItem_t ui_fnCallback;
    fnDestroyUtimerItem_t ui_fnDestroy;
    struct utimer_item * ui_puiPrev;
    struct utimer_item * ui_puiNext;
} utimer_item_t;

typedef struct utimer
{
    basic_chain_object_header_t iu_bcohHeader;
    basic_chain_t * iu_pbcChain;
    utimer_item_t * iu_puiItems;
    sync_mutex_t iu_smLock;
} internal_utimer_t;

/* --- private routine section---------------------------------------------- */

/** Checks the utimer item.
 *
 *  @param pObject [in] the basic chain object
 *  @param readset [in] no use, but necessay 
 *  @param writeset [in] no use, but necessay  
 *  @param errorset [in] no use, but necessay  
 *  @param pu32Blocktime [out] max block time specified in the chain 
 *
 *  @return the error code
 */
static u32 _checkUtimer(
    basic_chain_object_t * pObject, fd_set * readset,
	fd_set * writeset, fd_set * errorset, u32 * pu32Blocktime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    struct timeval tv;
    utimer_item_t * temp = NULL, * evt = NULL, * last = NULL;
    u32 nexttick, current;
    internal_utimer_t * piu = (internal_utimer_t *)pObject;

    assert(piu != NULL);

    acquireSyncMutex(&(piu->iu_smLock));

    if (piu->iu_puiItems != NULL)
    {
        /*Get the current tick count for reference*/
        getTimeOfDay(&tv);
        /*Current tick in Millisecond*/
        current = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
        temp = piu->iu_puiItems;
        /*Keep looping until we find a node that doesn't need to be triggered*/
        while (temp != NULL && temp->ui_u32Expire <= current)
        {
            /*Since these are in sorted order, evt will always poolint_t to 
              the first node that needs to be triggered. last will poolint_t 
              to the last node that needs to be triggered. temp will poolint_t 
              to the first node that doesn't need to be triggered.*/
            evt = piu->iu_puiItems;
            last = temp;
            temp = temp->ui_puiNext;
        }

        if (evt != NULL)
        {
            if (temp != NULL)
            {
                /*There are still nodes that will need to be triggered 
                  later, so reset it.
                  temp poolint_t to the node that need to be triggered*/
                piu->iu_puiItems = temp;
                piu->iu_puiItems->ui_puiPrev = NULL;
                last->ui_puiNext = NULL;
            }
            else
            {
                /*There are no more nodes that will need to be triggered later.*/
                piu->iu_puiItems = NULL;
            }
        }

        releaseSyncMutex(&(piu->iu_smLock));

        /*Iterate through all the triggers that we need to fire*/
        while (evt != NULL)
        {
            temp = evt->ui_puiNext;
            /*ToDo: We may want to check the table below first, 
              before we start triggering*/
            evt->ui_fnCallback(evt->ui_pData);

            if (evt->ui_fnDestroy != NULL)
            {
                evt->ui_fnDestroy(&(evt->ui_pData));
            }
            xfree((void **)&evt);

            evt = temp;
        }

        acquireSyncMutex(&(piu->iu_smLock));

        /*If there are more triggers that need to be fired later, we need to 
          recalculate what the max block time for our select should be*/
        if (piu->iu_puiItems != NULL)
        {
            nexttick = piu->iu_puiItems->ui_u32Expire - current;
            if (nexttick < * pu32Blocktime)
            {
                *pu32Blocktime = nexttick;
            }
        }
    }

    releaseSyncMutex(&(piu->iu_smLock));

    return u32Ret;
}

/** Flushes all timed callbacks from the utimer
 *
 *  @note Before destroying the utimer item structure, fnDestroyUtimerItem_t is
 *   called
 *
 *  @param piu [in] the utimer flush items from 
 *
 *  @return the error code
 */
static u32 _flushUtimer(internal_utimer_t * piu)
{
    u32 u32Ret = OLERR_NO_ERROR;
    utimer_item_t *temp, *temp2;

    acquireSyncMutex(&(piu->iu_smLock));

    temp = piu->iu_puiItems;
    piu->iu_puiItems = NULL;

    releaseSyncMutex(&(piu->iu_smLock));

    while (temp != NULL)
    {
        temp2 = temp->ui_puiNext;
        if (temp->ui_fnDestroy != NULL)
        {
            temp->ui_fnDestroy(&(temp->ui_pData));
        }
        xfree((void **)&temp);
        temp = temp2;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 addUtimerItem(
    utimer_t * pUtimer, void * pData, u32 u32Seconds,
    fnCallbackOfUtimerItem_t fnCallback, fnDestroyUtimerItem_t fnDestroy)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bUnblock = FALSE;
    struct timeval tv;
    utimer_item_t * pui, * temp;
    internal_utimer_t * piu = (internal_utimer_t *) pUtimer;

#if defined(DEBUG_UTIMER)
    logInfoMsg("add utimer item");
#endif
    u32Ret = xmalloc((void **)&pui, sizeof(utimer_item_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(pui, 0, sizeof(utimer_item_t));

        /*Get the current time for reference*/
        getTimeOfDay(&tv);

        /*Set the trigger time*/
        pui->ui_pData = pData;
        pui->ui_u32Expire = (tv.tv_sec * 1000) + (tv.tv_usec / 1000) + 
            (u32Seconds * 1000);

        /*Set the callback handlers*/
        pui->ui_fnCallback = fnCallback;
        pui->ui_fnDestroy = fnDestroy;

        acquireSyncMutex(&(piu->iu_smLock));
        if (piu->iu_puiItems == NULL)
        {
            /*There are no current triggers, so this is the first, which also
              means, the Select timeout may not be short enough, so we need to
              force an unblock, which will then reset the timeout
              appropriately*/
            piu->iu_puiItems = pui;
            bUnblock = TRUE;
        }
        else
        {
            /*There are already triggers, so we just insert this one in sorted
              order*/
            temp = piu->iu_puiItems;
            while (temp != NULL)
            {
                if (pui->ui_u32Expire <= temp->ui_u32Expire)
                {
                    pui->ui_puiNext = temp;
                    if (temp->ui_puiPrev == NULL)
                    {
                        /*This is the shortest trigger, so again, the select
                          timeout may not be short enough, so we need to force
                          unblock, to recalculate the timeout for the select*/
                        piu->iu_puiItems = pui;
                        temp->ui_puiPrev = pui;
                        bUnblock = TRUE;
                    }
                    else
                    {
                        /*This isn't the shortest trigger, so we are gauranteed
                          that the thread will unblock in time to reset the
                          timeouts*/
                        pui->ui_puiPrev = temp->ui_puiPrev;
                        temp->ui_puiPrev->ui_puiNext = pui;
                        temp->ui_puiPrev = pui;
                    }
                    break;
                }
                else if (temp->ui_puiNext == NULL)
                {
                    /*If there aren't any more triggers left, this means we have
                      the largest trigger, so just tack it on the end*/
                    pui->ui_puiNext = NULL;
                    pui->ui_puiPrev = temp;
                    temp->ui_puiNext = pui;
                    break;
                }
                temp = temp->ui_puiNext;
            }
        }
        releaseSyncMutex(&(piu->iu_smLock));

        if (bUnblock)
        {
            wakeupBasicChain(piu->iu_pbcChain);
        }
    }

    return u32Ret;
}

u32 removeUtimerItem(utimer_t * pUtimer, void * pData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_utimer_t * piu = (internal_utimer_t *) pUtimer;
    utimer_item_t *first, *last, *evt;

    evt = last = NULL;

    acquireSyncMutex(&(piu->iu_smLock));

    first = piu->iu_puiItems;
    while (first != NULL)
    {
        if (first->ui_pData == pData)
        {
            /*Found a match, now remove it from the list*/
            if (first->ui_puiPrev == NULL)
            {
                piu->iu_puiItems = first->ui_puiNext;
                if (piu->iu_puiItems != NULL)
                {
                    piu->iu_puiItems->ui_puiPrev = NULL;
                }
            }
            else
            {
                first->ui_puiPrev->ui_puiNext = first->ui_puiNext;
                if (first->ui_puiNext != NULL)
                {
                    first->ui_puiNext->ui_puiPrev = first->ui_puiPrev;
                }
            }

            if (evt == NULL)
            {
                /*If this is the first match, create a new list*/
                evt = last = first;
                evt->ui_puiPrev = evt->ui_puiNext = NULL;
            }
            else
            {
                /*Attach this match to the end of the list of matches*/
                last->ui_puiNext = first;
                first->ui_puiPrev = last;
                first->ui_puiNext = NULL;
                last = first;
            }
        }
        first = first->ui_puiNext;
    }

    releaseSyncMutex(&(piu->iu_smLock));

    /*Iterate through each node that is to be removed*/
    if (evt == NULL)
        u32Ret = OLERR_UTIMER_ITEM_NOT_FOUND;
    else
        while (evt != NULL)
        {
            first = evt->ui_puiNext;
            if (evt->ui_fnDestroy != NULL)
            {
                evt->ui_fnDestroy(&(evt->ui_pData));
            }
            xfree((void **)&evt);
            evt = first;
        }

    return u32Ret;
}

u32 destroyUtimer(utimer_t ** ppUtimer)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_utimer_t * piu;

    assert((ppUtimer != NULL) && (*ppUtimer != NULL));

    piu = (internal_utimer_t *) *ppUtimer;

    _flushUtimer(piu);

    finiSyncMutex(&(piu->iu_smLock));

    xfree(ppUtimer);
    *ppUtimer = NULL;

    return u32Ret;
}

u32 createUtimer(basic_chain_t * pChain, utimer_t ** ppUtimer)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_utimer_t * piu;

    u32Ret = xmalloc((void **)&piu, sizeof(internal_utimer_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(piu, 0, sizeof(internal_utimer_t));

        piu->iu_bcohHeader.bcoh_fnPreSelect = _checkUtimer;
        piu->iu_pbcChain = pChain;

        u32Ret = initSyncMutex(&(piu->iu_smLock));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = appendToBasicChain(pChain, (basic_chain_object_t *)piu);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppUtimer = piu;
    }
    else if (piu != NULL)
    {
        destroyUtimer((void **)&piu);
    }

    return u32Ret;
}

/*-----------------------------------------------------------------*/
