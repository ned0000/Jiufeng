/**
 *  @file buddy.c
 *
 *  @brief Implemenation file for the buddy system for page allocation.
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
#include "jf_listhead.h"
#include "jf_err.h"
#include "jf_mem.h"
#include "jf_mutex.h"
#include "jf_thread.h"

#include "common.h"
#include "buddy.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the free area data type.
 */
typedef struct free_area
{
    /**Free list for jiukun page object.*/
    jf_listhead_t fa_jlFree;
    /**Number of item in free list.*/
    u32 fa_u32Free;
} free_area_t;

/** Define the buddy zone data type.
 */
typedef struct buddy_zone
{
    /**Number of free pages.*/
    u32 bz_u32FreePages;

    /**The free area of pages.*/
    free_area_t bz_faFreeArea[JF_JIUKUN_MAX_PAGE_ORDER + 1];

    /**Maximum page order.*/
    u32 bz_u32MaxOrder;
    /**Number of page in this zone.*/
    u32 bz_u32NumOfPage;
    /**Jiukun page object array.*/
    jiukun_page_t * bz_papPage;

    /**The start memory address of pool for the zone.*/
    u8 * bz_pu8Pool;
    /**The end memory address of pool for the zone.*/
    u8 * bz_pu8PoolEnd;
} buddy_zone_t;

/** Maximum buddy zones.
 */
#define MAX_BUDDY_ZONES            (20)

/** Define the internal jiukun buddy data type.
 */
typedef struct
{
    /**Buddy is Initalized if it's TRUE.*/
    boolean_t ijb_bInitialized;
    /**Donot grow if it's TRUE.*/
    boolean_t ijb_bNoGrow;
    u8 ijb_u8Reserved[6];

    /**Maximum page order.*/
    u32 ijb_u32MaxOrder;
    u32 ijb_u32Reserved[2];

    /**Number of zone in buddy.*/
    u32 ijb_u32NumOfZone;
    /**Buddy zone array.*/
    buddy_zone_t * ijb_pbzZone[MAX_BUDDY_ZONES];

    /**Mutex lock for the jiukun page allocator.*/
    jf_mutex_t ijb_jmLock;
} internal_jiukun_buddy_t;

/** Declare the internal jiukun buddy object.
 */
static internal_jiukun_buddy_t ls_ijbBuddy;

/* --- private routine section ------------------------------------------------------------------ */

/** Set page order.
 */
static inline void _setPageOrder(jiukun_page_t * page, olint_t order)
{
    setJpOrder(page, order);
}

/** Clear page order.
 */
static inline void _clearPageOrder(jiukun_page_t * page)
{
    setJpOrder(page, 0);
}

/** Get page order.
 */
static inline ulong _getPageOrder(jiukun_page_t * page)
{
    return getJpOrder(page);
}

/** Split the page list of high order and add the last half part to low order.
 *
 *  @param zone [in] The zone from which jiukun page is allocated.
 *  @param page [in/out] The page list to split.
 *  @param low [in] The low page order.
 *  @param high [in] The high page order.
 *  @param area [in/out] The free area for the high page order.
 *
 *  @return The jiukun page object.
 */
static jiukun_page_t * _splitPageList(
    buddy_zone_t * zone, jiukun_page_t * page, olint_t low, olint_t high, free_area_t * area)
{
    ulong size = 1 << high;

    while (high > low)
    {
        area--;
        high--;
        size >>= 1;
        /*Add the last half part of the page to the free list.*/
        jf_listhead_add(&(area->fa_jlFree), &(page[size].jp_jlLru));
        area->fa_u32Free++;
        _setPageOrder(&page[size], high);
    }

    return page;
}

/** Remove an element from the buddy allocator.
 */
static jiukun_page_t * _rmqueue(buddy_zone_t * zone, u32 order)
{
    free_area_t * area = NULL;
    u32 current_order = 0;
    jiukun_page_t * page = NULL;

    /*Iterate the linked list head in free area.*/
    for (current_order = order; current_order < zone->bz_u32MaxOrder; ++current_order)
    {
        area = zone->bz_faFreeArea + current_order;
        if (jf_listhead_isEmpty(&area->fa_jlFree))
            continue;

        /*Get the first entry in the list.*/
        page = jf_listhead_getEntry(area->fa_jlFree.jl_pjlNext, jiukun_page_t, jp_jlLru);
        jf_listhead_del(&page->jp_jlLru);
        _clearPageOrder(page);
        area->fa_u32Free--;
        zone->bz_u32FreePages -= 1UL << order;

        /*Split page list if the current order is higher than the expected order.*/
        return _splitPageList(zone, page, order, current_order, area);
    }

    return NULL;
}

/** Get buddy page index according to the page index and order.
 */
static inline olint_t _getBuddyIndex(olint_t page_idx, u32 order)
{
    return page_idx ^ (1 << order);
}

/** Get buddy jiukun page according to the page index and order.
 */
static inline jiukun_page_t * _findBuddyPage(jiukun_page_t * page, olint_t page_idx, u32 order)
{
    olint_t buddy_idx = _getBuddyIndex(page_idx, order);

    return page + (buddy_idx - page_idx);
}

/** Check whether the jiukun page is free and the page order is the same as the specified one.
 */
static inline boolean_t _isBuddyPage(jiukun_page_t * page, olint_t order)
{
    if (! isJpAllocated(page) && (_getPageOrder(page) == order))
        return TRUE;

    return FALSE;
}

/** Free page to jiukun page allocator.
 */
static inline void _freeOnePage(buddy_zone_t * zone, jiukun_page_t * page, u32 order)
{
    olint_t page_idx = 0, buddy_idx = 0;
    olint_t order_size = 1 << order;
    free_area_t * area = NULL;
    jiukun_page_t * buddy = NULL;

    clearJpAllocated(page);
    /*Get page index in the array.*/
    page_idx = pageToIndex(page, zone->bz_papPage) & ((1 << zone->bz_u32MaxOrder) - 1);

    zone->bz_u32FreePages += order_size;
    while (order < zone->bz_u32MaxOrder - 1)
    {
        /*Get buddy page index.*/
        buddy_idx = _getBuddyIndex(page_idx, order);
        buddy = _findBuddyPage(page, page_idx, order);

        /*Check if the buddy page is free and the order is the same as freed page.*/
        if (! _isBuddyPage(buddy, order))
            break;
        /*Move the buddy up one level.*/
        jf_listhead_del(&buddy->jp_jlLru);
        area = zone->bz_faFreeArea + order;
        area->fa_u32Free--;
        _clearPageOrder(buddy);
        page = (buddy_idx > page_idx) ? page : buddy;
        page_idx = pageToIndex(page, zone->bz_papPage);
        order++;
    }
    /*Add the freed page to free area list.*/
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

    /*Free the jiukun page array.*/
    if (pbz->bz_papPage != NULL)
        jf_mem_free((void **)&(pbz->bz_papPage));

    /*Free the memory pool.*/
    if (pbz->bz_pu8Pool != NULL)
        jf_mem_free((void **)&(pbz->bz_pu8Pool));

    jf_mem_free((void **)ppZone);

    return u32Ret;
}

static void _initBuddyPage(
    jiukun_page_t * papPage, u32 u32NumOfPage, u32 u32ZoneId)
{
    u32 u32Index;

    /*Set zone id to the jiukun page.*/
    for (u32Index = 0; u32Index < u32NumOfPage; u32Index ++)
    {
        setJpZoneId(papPage, u32ZoneId);
        papPage ++;
    }

}

static u32 _createBuddyZone(buddy_zone_t ** ppZone, u32 u32MaxOrder, u32 u32ZoneId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    buddy_zone_t * pbz = NULL;
    u32 u32Index;

    JF_LOGGER_INFO("max order: %u, zoneid: %u", u32MaxOrder, u32ZoneId);

    /*Allocate memory for buddy zone.*/
    u32Ret = jf_mem_calloc((void **)&pbz, sizeof(buddy_zone_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pbz->bz_u32MaxOrder = u32MaxOrder;
        pbz->bz_u32NumOfPage = 1UL << (pbz->bz_u32MaxOrder - 1);
        pbz->bz_u32FreePages = pbz->bz_u32NumOfPage;

        /*Allocate memory for jiukun page array.*/
        u32Ret = jf_mem_calloc(
            (void **)&(pbz->bz_papPage), pbz->bz_u32NumOfPage * sizeof(jiukun_page_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _initBuddyPage(pbz->bz_papPage, pbz->bz_u32NumOfPage, u32ZoneId);

        /*Initialize the free area linked list head.*/
        for (u32Index = 0; u32Index < JF_JIUKUN_MAX_PAGE_ORDER + 1; u32Index ++)
            jf_listhead_init(&(pbz->bz_faFreeArea[u32Index].fa_jlFree));

        /*Add the jiukun page to free area, only 1 entry in the free area list.*/
        pbz->bz_faFreeArea[pbz->bz_u32MaxOrder - 1].fa_u32Free = 1;
        _setPageOrder(pbz->bz_papPage, pbz->bz_u32MaxOrder - 1);

        jf_listhead_add(
            &(pbz->bz_faFreeArea[pbz->bz_u32MaxOrder - 1].fa_jlFree),
            &(pbz->bz_papPage[0].jp_jlLru));

        /*Allocate memory pool.*/
        u32Ret = jf_mem_alloc(
            (void **)&(pbz->bz_pu8Pool), pbz->bz_u32NumOfPage * BUDDY_PAGE_SIZE);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pbz->bz_pu8PoolEnd = pbz->bz_pu8Pool + pbz->bz_u32NumOfPage * BUDDY_PAGE_SIZE;

        JF_LOGGER_INFO("start: %p, end: %p", pbz->bz_pu8Pool, pbz->bz_pu8PoolEnd);
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
    u32 u32Index = 0, u32Left = U32_MAX, u32Id = U32_MAX;
    buddy_zone_t * pbz = NULL;
    jiukun_page_t * page = NULL;

    /*Find a zone to allocate pages.*/
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
        /*Allocate page from zone.*/
        page = _rmqueue(piab->ijb_pbzZone[u32Id], u32Order);
        if (page != NULL)
            return page;
    }

    /*Maximum zone is reached, return NULL if grow is not allowed.*/
    if ((piab->ijb_u32NumOfZone == MAX_BUDDY_ZONES) || piab->ijb_bNoGrow)
        return NULL;

    /*Create a new zone.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createBuddyZone(
            &(piab->ijb_pbzZone[piab->ijb_u32NumOfZone]), piab->ijb_u32MaxOrder,
            piab->ijb_u32NumOfZone);

    /*Allocate page from the created zone.*/
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
    u32 u32Index = 0;
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
                jf_logger_logInfoMsg("        %d, %p", pageToIndex(pap, pbz->bz_papPage), pap);
            }
        }

        if ((u32Index < pbz->bz_u32MaxOrder - 1) && (pfa->fa_u32Free != 0) && (! bNoErrMsg))
        {
            bNoErrMsg = TRUE;
            jf_logger_logErrMsg(JF_ERR_JIUKUN_MEMORY_LEAK, "jiukun pages are not free");
        }
    }
}

static void _dumpBuddy(internal_jiukun_buddy_t * piab)
{
    u32 u32Index = 0;

    jf_mutex_acquire(&piab->ijb_jmLock);
    /*Iterate each zone.*/
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

/* --- public routine section ------------------------------------------------------------------- */

u32 initJiukunBuddy(buddy_param_t * pbp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;

    assert((pbp != NULL) && (pbp->bp_u8MaxOrder <= JF_JIUKUN_MAX_PAGE_ORDER + 1) &&
           (pbp->bp_u8MaxOrder > 0));
    assert(! piab->ijb_bInitialized);

    JF_LOGGER_INFO("max order: %u, no grow: %u", pbp->bp_u8MaxOrder, pbp->bp_bNoGrow);

    piab->ijb_u32MaxOrder = pbp->bp_u8MaxOrder + 1;
    piab->ijb_bNoGrow = pbp->bp_bNoGrow;

    /*Create one zone.*/
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
    u32 u32Index = 0;
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;

    JF_LOGGER_INFO("fini");

#if defined(DEBUG_JIUKUN)
    _dumpBuddy(piab);
#endif

    /*Destroy all zones.*/
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

    /*Allocate jiukun page.*/
    u32Ret = getJiukunPage(&pap, u32Order, flag);

    /*Convert the jiukun page the memory address.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        *pptr = jiukunPageToAddr(pap);

    return u32Ret;
}

void jf_jiukun_freePage(void ** pptr)
{
    jiukun_page_t * pap;

    assert(ls_ijbBuddy.ijb_bInitialized);
    assert((pptr != NULL) && (*pptr != NULL));

    /*Convert the memory address to the jiukun page.*/
    pap = addrToJiukunPage(*pptr);
    *pptr = NULL;

    /*Free the jiukun page.*/
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
    JF_LOGGER_DEBUG("order: %u, flag 0x%llX", u32Order, flag);
#endif

    *ppPage = NULL;

    /*Check the page order.*/
    if (u32Order >= piab->ijb_u32MaxOrder)
        return JF_ERR_INVALID_JIUKUN_PAGE_ORDER;

    /*The loop will not stop until the jiukun pages are successfully allocated.*/
    do
    {
        /*Allocate pages.*/
        jf_mutex_acquire(&(piab->ijb_jmLock));
        pap = _allocPages(piab, u32Order, flag);
        jf_mutex_release(&(piab->ijb_jmLock));

        if (pap == NULL)
        {
            /*Failed to allocate jiukun page.*/
            reapJiukun(TRUE);

            if (retrycount > 0)
                ol_sleep(retrycount);

            retrycount ++;
        }
    } while ((pap == NULL) && JF_FLAG_GET(flag, JF_JIUKUN_PAGE_ALLOC_FLAG_WAIT));

    if (pap == NULL)
    {
        u32Ret = JF_ERR_JIUKUN_OUT_OF_MEMORY;
    }
    else
    {
        /*Set page order and allocated.*/
        _setPageOrder(pap, u32Order);
        setJpAllocated(pap);
        *ppPage = pap;
#if defined(DEBUG_JIUKUN)
        JF_LOGGER_DEBUG("page: %p", pap);
#endif
    }

    return u32Ret;
}

void putJiukunPage(jiukun_page_t ** ppPage)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    u32 u32Order = 0, u32ZoneId = 0;

    assert(piab->ijb_bInitialized);
    assert((ppPage != NULL) && (*ppPage != NULL));

    u32ZoneId = getJpZoneId((*ppPage));
    u32Order = getJpOrder((*ppPage));

#if defined(DEBUG_JIUKUN)
    JF_LOGGER_DEBUG("paga: %p, zone id: %u, order: %u", *ppPage, u32ZoneId, u32Order);
#endif

    /*Check if the page is allocated.*/
    if (! isJpAllocated((*ppPage)))
    {
        JF_LOGGER_ERR(JF_ERR_JIUKUN_FREE_UNALLOCATED, "page: %p");
        abort();
    }

    /*Free the page.*/
    jf_mutex_acquire(&(piab->ijb_jmLock));
    _freeOnePage(piab->ijb_pbzZone[u32ZoneId], *ppPage, u32Order);
    jf_mutex_release(&(piab->ijb_jmLock));

    *ppPage = NULL;
}

void * jiukunPageToAddr(jiukun_page_t * pap)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    buddy_zone_t * pbz = NULL;

    assert(piab->ijb_bInitialized);

    /*Get zone.*/
    pbz = piab->ijb_pbzZone[getJpZoneId(pap)];

    assert(pbz != NULL);

    /*Convert the jiukun page object to memory address.*/
    return pbz->bz_pu8Pool + (pap - pbz->bz_papPage) * BUDDY_PAGE_SIZE;
}

jiukun_page_t * addrToJiukunPage(void * pAddr)
{
    internal_jiukun_buddy_t * piab = &ls_ijbBuddy;
    buddy_zone_t * pbz;
    u32 u32Index;

    assert(piab->ijb_bInitialized);

    /*Find the zone with the memory address.*/
    for (u32Index = 0; u32Index < piab->ijb_u32NumOfZone; u32Index ++)
    {
        pbz = piab->ijb_pbzZone[u32Index];

        assert(pbz != NULL);

        if (((u8 *)pAddr >= pbz->bz_pu8Pool) && ((u8 *)pAddr < pbz->bz_pu8PoolEnd))
            break;
    }

    /*The address is invalid if zone is not found.*/
    if (u32Index == piab->ijb_u32NumOfZone)
    {
        JF_LOGGER_ERR(JF_ERR_INVALID_JIUKUN_ADDRESS, "address: %p", pAddr);
        abort();
    }

    /*Get the jiukun page object from array.*/
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
