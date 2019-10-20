/**
 *  @file buddy.c
 *
 *  @brief The buddy system for page allocation
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_listhead.h"
#include "jf_err.h"
#include "jf_mem.h"
#include "jf_mutex.h"
#include "jf_thread.h"

#include "common.h"
#include "buddy.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct free_area
{
    /** free list */
    jf_listhead_t fa_jlFree;
    /** number of item in free list */
    u32 fa_u32Free;
} free_area_t;

typedef struct buddy_zone
{
    u32 bz_u32FreePages;

    free_area_t bz_faFreeArea[JF_JIUKUN_MAX_PAGE_ORDER + 1];

    u32 bz_u32MaxOrder;
    u32 bz_u32NumOfPage;
    jiukun_page_t * bz_papPage;

    u8 * bz_pu8Pool;
    u8 * bz_pu8PoolEnd;
} buddy_zone_t;

#define MAX_BUDDY_ZONES  20

typedef struct
{
    boolean_t ijb_bInitialized;
    boolean_t ijb_bNoGrow;
    u8 ijb_u8Reserved[6];

    u32 ijb_u32MaxOrder;
    u32 ijb_u32Reserved[2];

    u32 ijb_u32NumOfZone;
    buddy_zone_t * ijb_pbzZone[MAX_BUDDY_ZONES];

    jf_mutex_t ijb_jmLock;
} internal_jiukun_buddy_t;

static internal_jiukun_buddy_t ls_ijbBuddy;

/* --- private routine section ------------------------------------------------------------------ */

static inline void _setPageOrder(jiukun_page_t * page, olint_t order)
{
    setJpOrder(page, order);
}

static inline void _clearPageOrder(jiukun_page_t * page)
{
    setJpOrder(page, 0);
}

/** Get page order
 */
static inline ulong _getPageOrder(jiukun_page_t * page)
{
    return getJpOrder(page);
}

static jiukun_page_t * _expand(
    buddy_zone_t * zone, jiukun_page_t * page, olint_t low, olint_t high, free_area_t * area)
{
    ulong size = 1 << high;

    while (high > low)
    {
        area--;
        high--;
        size >>= 1;
        /** Add the last half part of the page to the free list */
        jf_listhead_add(&(area->fa_jlFree), &(page[size].jp_jlLru));
        area->fa_u32Free++;
        _setPageOrder(&page[size], high);
    }

    return page;
}

/** Remove an element from the buddy allocator.
 * 
 */
static jiukun_page_t * _rmqueue(buddy_zone_t * zone, u32 order)
{
    free_area_t * area;
    u32 current_order;
    jiukun_page_t * page;

    for (current_order = order; current_order < zone->bz_u32MaxOrder; ++current_order)
    {
        area = zone->bz_faFreeArea + current_order;
        if (jf_listhead_isEmpty(&area->fa_jlFree))
            continue;

        page = jf_listhead_getEntry(area->fa_jlFree.jl_pjlNext, jiukun_page_t, jp_jlLru);
        jf_listhead_del(&page->jp_jlLru);
        _clearPageOrder(page);
        area->fa_u32Free--;
        zone->bz_u32FreePages -= 1UL << order;

        return _expand(zone, page, order, current_order, area);
    }

    return NULL;
}

static inline ulong _getBuddyIndex(ulong page_idx, u32 order)
{
    return page_idx ^ (1 << order);
}

static inline jiukun_page_t * _findBuddyPage(
    jiukun_page_t * page, ulong page_idx, u32 order)
{
    ulong buddy_idx = _getBuddyIndex(page_idx, order);

    return page + (buddy_idx - page_idx);
}

/** This function checks whether a page is free && is the buddy we can do
 *  coalesce a page and its buddy if
 *  1. the buddy is free &&
 *  2. page and its buddy have the same order.
 *
 */
static inline boolean_t _isBuddyPage(jiukun_page_t * page, olint_t order)
{
    if (! isJpAllocated(page) && (_getPageOrder(page) == order))
        return TRUE;

    return FALSE;
}

/** Freeing function for a buddy system allocator.
 *
 */
static inline void _freeOnePage(
    buddy_zone_t * zone, jiukun_page_t * page, u32 order)
{
    ulong page_idx;
    olint_t order_size = 1 << order;
    ulong buddy_idx;
    free_area_t * area;
    jiukun_page_t * buddy;

    clearJpAllocated(page);
    page_idx = pageToIndex(page, zone->bz_papPage) & ((1 << zone->bz_u32MaxOrder) - 1);

    zone->bz_u32FreePages += order_size;
    while (order < zone->bz_u32MaxOrder - 1)
    {
        buddy_idx = _getBuddyIndex(page_idx, order);
        buddy = _findBuddyPage(page, page_idx, order);

        if (! _isBuddyPage(buddy, order))
            break;      /* Move the buddy up one level. */
        jf_listhead_del(&buddy->jp_jlLru);
        area = zone->bz_faFreeArea + order;
        area->fa_u32Free--;
        _clearPageOrder(buddy);
        page = (buddy_idx > page_idx) ? page : buddy;
        page_idx = pageToIndex(page, zone->bz_papPage);
        order++;
    }
    _setPageOrder(page, order);
    jf_listhead_add(&(zone->bz_faFreeArea[order].fa_jlFree), &(page->jp_jlLru));
    zone->bz_faFreeArea[order].fa_u32Free++;
}

static u32 _destroyBuddyZone(buddy_zone_t ** ppZone)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    buddy_zone_t * pbz = NULL;

    assert((ppZone != NULL) && (*ppZone != NULL));

    pbz = (buddy_zone_t *)*ppZone;

    if (pbz->bz_papPage != NULL)
        jf_mem_free((void **)&(pbz->bz_papPage));

    if (pbz->bz_pu8Pool != NULL)
        jf_mem_free((void **)&(pbz->bz_pu8Pool));

    jf_mem_free((void **)ppZone);

    return u32Ret;
}

static void _initBuddyPage(
    jiukun_page_t * papPage, u32 u32NumOfPage, u32 u32ZoneId)
{
    u32 u32Index;

    for (u32Index = 0; u32Index < u32NumOfPage; u32Index ++)
    {
        setJpZoneId(papPage, u32ZoneId);
        papPage ++;
    }

}

static u32 _createBuddyZone(
    buddy_zone_t ** ppZone, u32 u32MaxOrder, u32 u32ZoneId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    buddy_zone_t * pbz = NULL;
    u32 u32Index;

    jf_logger_logInfoMsg(
        "create jiukun zone, order: %u, zoneid: %u", u32MaxOrder, u32ZoneId);

    u32Ret = jf_mem_calloc((void **)&pbz, sizeof(buddy_zone_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pbz->bz_u32MaxOrder = u32MaxOrder;
        pbz->bz_u32NumOfPage = 1UL << (pbz->bz_u32MaxOrder - 1);
        pbz->bz_u32FreePages = pbz->bz_u32NumOfPage;

        u32Ret = jf_mem_calloc(
            (void **)&(pbz->bz_papPage), pbz->bz_u32NumOfPage * sizeof(jiukun_page_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _initBuddyPage(pbz->bz_papPage, pbz->bz_u32NumOfPage, u32ZoneId);

        for (u32Index = 0; u32Index < JF_JIUKUN_MAX_PAGE_ORDER + 1; u32Index ++)
            jf_listhead_init(&(pbz->bz_faFreeArea[u32Index].fa_jlFree));

        pbz->bz_faFreeArea[pbz->bz_u32MaxOrder - 1].fa_u32Free = 1;
        _setPageOrder(pbz->bz_papPage, pbz->bz_u32MaxOrder - 1);

        jf_listhead_add(
            &(pbz->bz_faFreeArea[pbz->bz_u32MaxOrder - 1].fa_jlFree),
            &(pbz->bz_papPage[0].jp_jlLru));

        u32Ret = jf_mem_alloc(
            (void **)&(pbz->bz_pu8Pool), pbz->bz_u32NumOfPage * BUDDY_PAGE_SIZE);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pbz->bz_pu8PoolEnd = pbz->bz_pu8Pool + pbz->bz_u32NumOfPage * BUDDY_PAGE_SIZE;

        jf_logger_logInfoMsg(
            "create jiukun zone, start: %p, end: %p", pbz->bz_pu8Pool, pbz->bz_pu8PoolEnd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppZone = pbz;
    else if (pbz != NULL)
        _destroyBuddyZone(&pbz);

    return u32Ret;
}

static jiukun_page_t * _allocPages(
    internal_jiukun_buddy_t * piab, u32 u32Order, jf_flag_t flag)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Pages = 1UL << u32Order;
    u32 u32Index, u32Left = U32_MAX, u32Id = U32_MAX;
    buddy_zone_t * pbz;
    jiukun_page_t * page;

    for (u32Index = 0; u32Index < piab->ijb_u32NumOfZone; u32Index ++)
    {
        pbz = piab->ijb_pbzZone[u32Index];
        if ((pbz->bz_u32FreePages >= u32Pages) && (u32Left > pbz->bz_u32FreePages - u32Pages))
        {
            u32Left = pbz->bz_u32FreePages - u32Pages;
            u32Id = u32Index;
        }
    }

    if (u32Id != U32_MAX)
    {
        page = _rmqueue(piab->ijb_pbzZone[u32Id], u32Order);
        if (page != NULL)
            return page;
    }

    /*maximum zone is reached*/
    if ((piab->ijb_u32NumOfZone == MAX_BUDDY_ZONES) || piab->ijb_bNoGrow)
        return NULL;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createBuddyZone(
            &(piab->ijb_pbzZone[piab->ijb_u32NumOfZone]),
            piab->ijb_u32MaxOrder, piab->ijb_u32NumOfZone);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pbz = piab->ijb_pbzZone[piab->ijb_u32NumOfZone];
        piab->ijb_u32NumOfZone ++;

        page = _rmqueue(pbz, u32Order);
        if (page != NULL)
            return page;
    }

    return NULL;
}

#if defined(DEBUG_JIUKUN)
static void _dumpBuddyZone(buddy_zone_t * pbz)
{
    u32 u32Index;
    free_area_t * pfa = NULL;
    jf_listhead_t * pjl = NULL;
    jiukun_page_t * pap = NULL;
    boolean_t bNoErrMsg = FALSE;

    jf_logger_logInfoMsg("  max page: %u, %p", pbz->bz_u32NumOfPage, pbz->bz_papPage);
    jf_logger_logInfoMsg("  free page: %u", pbz->bz_u32FreePages);

    for (u32Index = 0; u32Index < pbz->bz_u32MaxOrder; u32Index ++)
    {
        pfa = &(pbz->bz_faFreeArea[u32Index]);

        if (pfa->fa_u32Free != 0)
        {
            jf_logger_logInfoMsg("    area index: %u", u32Index);
            jf_logger_logInfoMsg("      free page in area: %u", pfa->fa_u32Free);

            jf_logger_logInfoMsg("      free page:");
            jf_listhead_forEach(&(pfa->fa_jlFree), pjl)
            {
                pap = jf_listhead_getEntry(pjl, jiukun_page_t, jp_jlLru);
                jf_logger_logInfoMsg("        %u, %p", pageToIndex(pap, pbz->bz_papPage), pap);
            }
        }

        if ((u32Index < pbz->bz_u32MaxOrder - 1) && (pfa->fa_u32Free != 0) && (! bNoErrMsg))
        {
            bNoErrMsg = TRUE;
            jf_logger_logErrMsg(JF_ERR_JIUKUN_MEMORY_LEAK, "buddy page not free");
        }
    }
}

static void _dumpBuddy(internal_jiukun_buddy_t * piab)
{
    u32 u32Index;

    jf_mutex_acquire(&piab->ijb_jmLock);
    for (u32Index = 0; u32Index < piab->ijb_u32NumOfZone; u32Index ++)
    {
        assert(piab->ijb_pbzZone[u32Index] != NULL);

        jf_logger_logInfoMsg("zone: %u", u32Index);
        _dumpBuddyZone(piab->ijb_pbzZone[u32Index]);
    }
    jf_mutex_release(&piab->ijb_jmLock);
    jf_logger_logInfoMsg("");
}
#endif

static inline u32 _u32Log2(u32 x)
{
    olint_t r = 0;

    for (x >>= 1; x > 0; x >>= 1)
        r++;

    return r;
}

/* --- public routine section ------------------------------------------------------------------- */
u32 initJiukunBuddy(buddy_param_t * pbp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;

    assert((pbp != NULL) && (pbp->bp_u8MaxOrder <= JF_JIUKUN_MAX_PAGE_ORDER + 1) &&
           (pbp->bp_u8MaxOrder > 0));
    assert(! piab->ijb_bInitialized);

    jf_logger_logInfoMsg("init jiukun buddy");

    piab->ijb_u32MaxOrder = pbp->bp_u8MaxOrder + 1;
    piab->ijb_bNoGrow = pbp->bp_bNoGrow;

    u32Ret = _createBuddyZone(&(piab->ijb_pbzZone[0]), piab->ijb_u32MaxOrder, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piab->ijb_u32NumOfZone ++;

        u32Ret = jf_mutex_init(&(piab->ijb_jmLock));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        piab->ijb_bInitialized = TRUE;
    else
        finiJiukunBuddy();

    return u32Ret;
}

u32 finiJiukunBuddy(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;

    jf_logger_logInfoMsg("fini jiukun buddy");

#if defined(DEBUG_JIUKUN)
    _dumpBuddy(piab);
#endif

    for (u32Index = 0; u32Index < piab->ijb_u32NumOfZone; u32Index ++)
        _destroyBuddyZone(&(piab->ijb_pbzZone[u32Index]));

    jf_mutex_fini(&(piab->ijb_jmLock));

    piab->ijb_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_jiukun_allocPage(void ** pptr, u32 u32Order, jf_flag_t flag)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jiukun_page_t * pap = NULL;

    *pptr = NULL;

    u32Ret = getJiukunPage(&pap, u32Order, flag);
    if (u32Ret == JF_ERR_NO_ERROR)
        *pptr = jiukunPageToAddr(pap);

    return u32Ret;
}

void jf_jiukun_freePage(void ** pptr)
{
    jiukun_page_t * pap;

    assert(ls_ijbBuddy.ijb_bInitialized);
    assert((pptr != NULL) && (*pptr != NULL));

    pap = addrToJiukunPage(*pptr);
    *pptr = NULL;

    putJiukunPage(&pap);
}

u32 getJiukunPage(jiukun_page_t ** ppPage, u32 u32Order, jf_flag_t flag)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    jiukun_page_t * pap = NULL;
    olint_t retrycount = 0;

    assert(piab->ijb_bInitialized);
    assert(ppPage != NULL);

#if defined(DEBUG_JIUKUN)
    jf_logger_logDebugMsg("get jiukun page, order: %u, flag 0x%llX", u32Order, flag);
#endif

    *ppPage = NULL;
    if (u32Order >= piab->ijb_u32MaxOrder)
        return JF_ERR_INVALID_JIUKUN_PAGE_ORDER;

    do
    {
        jf_mutex_acquire(&(piab->ijb_jmLock));
        pap = _allocPages(piab, u32Order, flag);
        jf_mutex_release(&(piab->ijb_jmLock));

        if (pap == NULL)
        {
            reapJiukun(TRUE);

            if (retrycount > 0)
                sleep(retrycount);

            retrycount ++;
        }
    } while ((pap == NULL) && JF_FLAG_GET(flag, JF_JIUKUN_PAGE_ALLOC_FLAG_WAIT));

    if (pap == NULL)
    {
        u32Ret = JF_ERR_JIUKUN_OUT_OF_MEMORY;
    }
    else
    {
        _setPageOrder(pap, u32Order);
        setJpAllocated(pap);
        *ppPage = pap;
#if defined(DEBUG_JIUKUN)
        jf_logger_logDebugMsg("get jiukun page, page: %p", pap);
#endif
    }

    return u32Ret;
}

void putJiukunPage(jiukun_page_t ** ppPage)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    u32 u32Order, u32ZoneId;

    assert(piab->ijb_bInitialized);
    assert((ppPage != NULL) && (*ppPage != NULL));

    u32ZoneId = getJpZoneId((*ppPage));
    u32Order = getJpOrder((*ppPage));

#if defined(DEBUG_JIUKUN)
    jf_logger_logInfoMsg("put jiukun page, paga addr: %p, order: %u", *ppPage, u32Order);
#endif

    if (! isJpAllocated((*ppPage)))
    {
        jf_logger_logErrMsg(JF_ERR_JIUKUN_FREE_UNALLOCATED, "put jiukun page");
        abort();
    }

    jf_mutex_acquire(&(piab->ijb_jmLock));

    _freeOnePage(piab->ijb_pbzZone[u32ZoneId], *ppPage, u32Order);

    jf_mutex_release(&(piab->ijb_jmLock));

    *ppPage = NULL;
}

void * jiukunPageToAddr(jiukun_page_t * pap)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    buddy_zone_t * pbz;

    assert(piab->ijb_bInitialized);

    pbz = piab->ijb_pbzZone[getJpZoneId(pap)];

    assert(pbz != NULL);

    return pbz->bz_pu8Pool + (pap - pbz->bz_papPage) * BUDDY_PAGE_SIZE;
}

jiukun_page_t * addrToJiukunPage(void * pAddr)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    buddy_zone_t * pbz;
    u32 u32Index;

    assert(piab->ijb_bInitialized);

    for (u32Index = 0; u32Index < piab->ijb_u32NumOfZone; u32Index ++)
    {
        pbz = piab->ijb_pbzZone[u32Index];

        assert(pbz != NULL);

        if (((u8 *)pAddr >= pbz->bz_pu8Pool) && ((u8 *)pAddr < pbz->bz_pu8PoolEnd))
            break;
    }

    if (u32Index == piab->ijb_u32NumOfZone)
    {
        jf_logger_logErrMsg(JF_ERR_JIUKUN_BAD_POINTER, "addr to page");
        abort();
    }

    return pbz->bz_papPage + ((u8 *)pAddr - pbz->bz_pu8Pool) / BUDDY_PAGE_SIZE;
}

#if defined(DEBUG_JIUKUN)
void dumpJiukunBuddy(void)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;

    assert(piab->ijb_bInitialized);

    _dumpBuddy(piab);
}
#endif

/*------------------------------------------------------------------------------------------------*/


