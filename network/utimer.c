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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_time.h"
#include "jf_network.h"
#include "jf_mutex.h"
#include "jf_jiukun.h"
#include "jf_listhead.h"

/* --- private data/data structure section ------------------------------------------------------ */
typedef struct utimer_item
{
    u32 ui_u32Expire;
    void * ui_pData;
    jf_network_fnCallbackOfUtimerItem_t ui_fnCallback;
    jf_network_fnDestroyUtimerItemData_t ui_fnDestroy;

    jf_listhead_t ui_jlList;
} utimer_item_t;

typedef struct utimer
{
    jf_network_chain_object_header_t iu_jncohHeader;
    jf_network_chain_t * iu_pbcChain;
    utimer_item_t * iu_puiItem;

    olchar_t iu_strName[JF_NETWORK_MAX_NAME_LEN];

    /*start of lock protected section*/
    /**mutex lock*/
    jf_mutex_t iu_jmLock;
    jf_listhead_t iu_jlItem;
    /*end of lock protected section*/

} internal_utimer_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _freeUtimerItem(utimer_item_t ** ppItem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    utimer_item_t * item = *ppItem;

    if (item->ui_fnDestroy != NULL)
        item->ui_fnDestroy(&item->ui_pData);

    jf_jiukun_freeMemory((void **)ppItem);

    return u32Ret;
}

static u32 _destroyUtimerItems(internal_utimer_t * piu, jf_listhead_t * list, boolean_t bCallback)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    utimer_item_t * temp = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;

    jf_listhead_forEachSafe(list, pos, temppos)
    {
        temp = jf_listhead_getEntry(pos, utimer_item_t, ui_jlList);

        if (bCallback)
            temp->ui_fnCallback(temp->ui_pData);

#if defined(DEBUG_UTIMER)
        JF_LOGGER_DEBUG("utimer: %s, expire: %d", piu->iu_strName, temp->ui_u32Expire);
#endif

        _freeUtimerItem(&temp);
    }

    return u32Ret;
}

/** Checks the utimer item.
 *
 *  @param pObject [in] the chain object
 *  @param readset [in] no use, but necessay 
 *  @param writeset [in] no use, but necessay  
 *  @param errorset [in] no use, but necessay  
 *  @param pu32Blocktime [out] max block time specified in the chain 
 *
 *  @return the error code
 */
static u32 _checkUtimer(
    jf_network_chain_object_t * pObject, fd_set * readset, fd_set * writeset, fd_set * errorset,
    u32 * pu32Blocktime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timespec tp;
    utimer_item_t * temp = NULL;
    u32 nexttick, current;
    internal_utimer_t * piu = (internal_utimer_t *)pObject;
    jf_listhead_t * pos = NULL, * temppos = NULL;
    JF_LISTHEAD(jlTriggerItem);

    /*Get the current time for reference*/
    u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        current = (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000);
#if defined(DEBUG_UTIMER)
        JF_LOGGER_DEBUG("utimer: %s, current: %d", piu->iu_strName, current);
#endif
        jf_mutex_acquire(&piu->iu_jmLock);

        jf_listhead_forEachSafe(&piu->iu_jlItem, pos, temppos)
        {
            temp = jf_listhead_getEntry(pos, utimer_item_t, ui_jlList);

            if (temp->ui_u32Expire <= current)
            {
                /*temp should be triggered*/
                jf_listhead_moveTail(&jlTriggerItem, pos);
            }
            else
            {
                /*Since the items are in sorted order, break the loop*/
                break;
            }
        }

        if (! jf_listhead_isEmpty(&piu->iu_jlItem))
        {
            temp = jf_listhead_getEntry(piu->iu_jlItem.jl_pjlNext, utimer_item_t, ui_jlList);

            nexttick = temp->ui_u32Expire - current;
            if (nexttick < *pu32Blocktime)
            {
                *pu32Blocktime = nexttick;
#if defined(DEBUG_UTIMER)
                JF_LOGGER_DEBUG("utimer: %s, blocktime: %d", piu->iu_strName, nexttick);
#endif
            }
        }

        jf_mutex_release(&piu->iu_jmLock);        

        _destroyUtimerItems(piu, &jlTriggerItem, TRUE);
    }

    return u32Ret;
}

/** Flushes all timed callbacks from the utimer
 *
 *  @note Before destroying the utimer item structure, fnDestroy is called
 *
 *  @param piu [in] the utimer flush item from 
 *
 *  @return the error code
 */
static u32 _flushUtimer(internal_utimer_t * piu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    JF_LISTHEAD(jlOldItem);

    jf_mutex_acquire(&piu->iu_jmLock);
    if (! jf_listhead_isEmpty(&piu->iu_jlItem))
        jf_listhead_spliceTail(&jlOldItem, &piu->iu_jlItem);
    jf_mutex_release(&piu->iu_jmLock);

    u32Ret = _destroyUtimerItems(piu, &jlOldItem, FALSE);
    
    return u32Ret;
}

static u32 _removeUtimerItem(internal_utimer_t * piu, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    JF_LISTHEAD(jlRemoveItem);
    utimer_item_t * temp = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;

    jf_mutex_acquire(&piu->iu_jmLock);

    jf_listhead_forEachSafe(&piu->iu_jlItem, pos, temppos)
    {
        temp = jf_listhead_getEntry(pos, utimer_item_t, ui_jlList);

        if (temp->ui_pData == pData)
            jf_listhead_moveTail(&jlRemoveItem, &temp->ui_jlList);
        
    }

    jf_mutex_release(&piu->iu_jmLock);

    _destroyUtimerItems(piu, &jlRemoveItem, FALSE);
    
    return u32Ret;
}

static u32 _insertUtimerItem(internal_utimer_t * piu, utimer_item_t * pui)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos = NULL;
    utimer_item_t * temp = NULL;

    jf_mutex_acquire(&piu->iu_jmLock);

    /*There are no triggers, add the new item to list*/
    if (jf_listhead_isEmpty(&piu->iu_jlItem))
    {
        jf_listhead_add(&piu->iu_jlItem, &pui->ui_jlList);
    }
    else
    {
        jf_listhead_forEach(&piu->iu_jlItem, pos)
        {
            temp = jf_listhead_getEntry(pos, utimer_item_t, ui_jlList);

            /*There are already triggers, so we just insert this one in sorted order*/
            if (pui->ui_u32Expire < temp->ui_u32Expire)
            {
                jf_listhead_addTail(pos, &pui->ui_jlList);
                break;
            }
        }

        /*reach the end of list, add to the tail*/
        if (&piu->iu_jlItem == pos)
            jf_listhead_addTail(&piu->iu_jlItem, &pui->ui_jlList);;
    }

    jf_mutex_release(&piu->iu_jmLock);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_addUtimerItem(
    jf_network_utimer_t * pUtimer, void * pData, u32 u32Seconds,
    jf_network_fnCallbackOfUtimerItem_t fnCallback, jf_network_fnDestroyUtimerItemData_t fnDestroy)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timespec tp;
    utimer_item_t * pui;
    internal_utimer_t * piu = (internal_utimer_t *) pUtimer;

    assert((pData != NULL) && (fnCallback != NULL));
    
#if defined(DEBUG_UTIMER)
    JF_LOGGER_DEBUG("utimer: %s", piu->iu_strName);
#endif
    u32Ret = jf_jiukun_allocMemory((void **)&pui, sizeof(utimer_item_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Get the current time for reference*/
        u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pui, 0, sizeof(utimer_item_t));
        /*Set the trigger time*/
        pui->ui_u32Expire = (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000) + (u32Seconds * 1000);
#if defined(DEBUG_UTIMER)
        JF_LOGGER_DEBUG("utimer: %s, expire at: %d", piu->iu_strName, pui->ui_u32Expire);
#endif
        pui->ui_pData = pData;
        /*Set the callback handlers*/
        pui->ui_fnCallback = fnCallback;
        pui->ui_fnDestroy = fnDestroy;
        jf_listhead_init(&pui->ui_jlList);

        u32Ret = _insertUtimerItem(piu, pui);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_wakeupChain(piu->iu_pbcChain);

    if ((u32Ret != JF_ERR_NO_ERROR) && (pui != NULL))
        _freeUtimerItem(&pui);

    return u32Ret;
}

u32 jf_network_removeUtimerItem(jf_network_utimer_t * pUtimer, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_utimer_t * piu = (internal_utimer_t *) pUtimer;

#if defined(DEBUG_UTIMER)
    JF_LOGGER_DEBUG("utimer: %s", piu->iu_strName);
#endif

    u32Ret = _removeUtimerItem(piu, pData);

    return u32Ret;
}

u32 jf_network_destroyUtimer(jf_network_utimer_t ** ppUtimer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_utimer_t * piu;

    assert((ppUtimer != NULL) && (*ppUtimer != NULL));

    piu = (internal_utimer_t *) *ppUtimer;

    _flushUtimer(piu);

    jf_mutex_fini(&piu->iu_jmLock);

    jf_jiukun_freeMemory(ppUtimer);

    return u32Ret;
}

u32 jf_network_createUtimer(
    jf_network_chain_t * pChain, jf_network_utimer_t ** ppUtimer, const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_utimer_t * piu = NULL;

    JF_LOGGER_DEBUG("name: %s", pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&piu, sizeof(internal_utimer_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(piu, 0, sizeof(internal_utimer_t));

        piu->iu_jncohHeader.jncoh_fnPreSelect = _checkUtimer;
        piu->iu_pbcChain = pChain;
        jf_listhead_init(&piu->iu_jlItem);
        ol_strncpy(piu->iu_strName, pstrName, JF_NETWORK_MAX_NAME_LEN - 1);

        u32Ret = jf_mutex_init(&piu->iu_jmLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_appendToChain(pChain, (jf_network_chain_object_t *)piu);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppUtimer = piu;
    else if (piu != NULL)
        jf_network_destroyUtimer((void **)&piu);

    return u32Ret;
}

void jf_network_dumpUtimerItem(jf_network_utimer_t * pUtimer)
{
#if defined(DEBUG_UTIMER)
    internal_utimer_t * piu = (internal_utimer_t *)pUtimer;
    jf_listhead_t * pos = NULL;
    utimer_item_t * temp = NULL;
    u32 u32Index = 1;

    ol_printf("======= Dump start =======\n");
    
    jf_listhead_forEach(&piu->iu_jlItem, pos)
    {
        temp = jf_listhead_getEntry(pos, utimer_item_t, ui_jlList);

        ol_printf("%02d, expire: %d\n", u32Index, temp->ui_u32Expire);
        u32Index ++;
    }

    ol_printf("======= Dump end =======\n");    
#endif
}


/*------------------------------------------------------------------------------------------------*/

