/**
 *  @file slab.c
 *
 *  @brief Implentation file for the slab memory allocation system.
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
#include "jf_mem.h"
#include "jf_mutex.h"

#include "common.h"
#include "slab.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum size of an obj (in 2^order pages) and absolute limit for the page order. The size
 *  should be more than the maximum size of general cache. Up to 8Mb.
 */
#define MAX_JP_ORDER             (11)

/** Bufctl's are used for linking objs within a slab linked offsets.
 *
 *  This implementation relies on "struct jiukun_page" for locating the cache & slab an object
 *  belongs to.
 *  This allows the bufctl structure to be small (one int), but limits the number of objects a slab
 *  (not a cache) can contain when off-slab bufctls are used. The limit is the size of the largest
 *  general cache that does not use off-slab slabs.
 */
typedef u32                      slab_bufctl_t;
#define BUFCTL_END               (((slab_bufctl_t)(~0U)) - 0)
#define BUFCTL_FREE              (((slab_bufctl_t)(~0U)) - 1)
#define SLAB_LIMIT               (((slab_bufctl_t)(~0U)) - 2)

/** Manages the objects in a slab. Placed either at the beginning of memory allocated for a slab,
 *  or allocated from an general cache.
 */
typedef struct
{
    /**List entry, chained into three list: fully used, partial, fully free slabs.*/
    jf_listhead_t s_jlList;
    /**Pointer to objects.*/
    void * s_pMem;
    /**Number of objs active in slab.*/
    u32 s_u32InUse;
    /**Free object number.*/
    slab_bufctl_t s_sbFree;
} slab_t;

#define slab_bufctl(slabp)       ((slab_bufctl_t *)(((slab_t*)slabp) + 1))

/** Maximum name length for a slab cache.
 */
#define CACHE_NAME_LEN           (24)

/** Define the internal flags for slab cache.
 */
typedef enum slab_cache_flag
{
    /**Slab management in own cache.*/
    SC_FLAG_OFF_SLAB = 32,
    /**Cache is growing.*/
    SC_FLAG_GROWN,
    /**Cache is being destroyed.*/
    SC_FLAG_DESTROY,
    /**Red zone objs in a cache, available when DEBUG_JIUKUN is true, the size of obj should less
       than (BUDDY_PAGE_SIZE >> 3)*/
    SC_FLAG_RED_ZONE,
    /**Cache is locked.*/
    SC_FLAG_LOCKED,
} slab_cache_flag_t;

/** Define the internal slab cache data type.
 */
typedef struct slab_cache
{
    /**Object size including red zone.*/
    u32 sc_u32ObjSize;
    /**Real object size user requests.*/
    u32 sc_u32RealObjSize;
    /**Cache flags.*/
    jf_flag_t sc_jfCache;
    /**Num of objs per slab.*/
    u32 sc_u32Num;

    /**Lock for the full, partial and free list.*/
    jf_mutex_t sc_jmCache;
    /**List for fully used slab.*/
    jf_listhead_t sc_jlFull;
    /**List for partial slab.*/
    jf_listhead_t sc_jlPartial;
    /**List for fully free slab.*/
    jf_listhead_t sc_jlFree;

    /**Order of pages per slab (2^n).*/
    u32 sc_u32Order;

    /**Page flags.*/
    jf_flag_t sc_jfPage;

    /**Cache for slab_t.*/
    struct slab_cache * sc_pscSlab;

    /**Cache name.*/
    olchar_t sc_strName[CACHE_NAME_LEN];
    /**Linked in cache_cache.*/
    jf_listhead_t sc_jlNext;

#if DEBUG_JIUKUN_STAT
    /**Number of object in use.*/
    ulong sc_ulNumActive;
    /**Number of times with object allocation.*/
    ulong sc_ulNumAlloced;
    /**The high mark for object allocation.*/
    ulong sc_ulNumHighMark;
    /**Number of times with grown operation.*/
    ulong sc_ulNumGrown;
    /**Number of times with reap operation.*/
    ulong sc_ulNumReaped;
    /**Number of times with error.*/
    ulong sc_ulNumErrors;
#endif
} slab_cache_t;


#define OFF_SLAB(x)              (JF_FLAG_GET((x)->sc_jfCache, SC_FLAG_OFF_SLAB))
#define GROWN(x)                 (JF_FLAG_GET((x)->sc_jfCache, AF_FLAGS_GROWN))

#if DEBUG_JIUKUN_STAT
    #define STATS_INC_ACTIVE(x)  ((x)->sc_ulNumActive++)
    #define STATS_DEC_ACTIVE(x)  ((x)->sc_ulNumActive--)
    #define STATS_INC_ALLOCED(x) ((x)->sc_ulNumAlloced++)
    #define STATS_INC_GROWN(x)   ((x)->sc_ulNumGrown++)
    #define STATS_INC_REAPED(x)  ((x)->sc_ulNumReaped++)
    #define STATS_SET_HIGH(x)   \
        do { if ((x)->sc_ulNumActive > (x)->sc_ulNumHighMark)  \
                (x)->sc_ulNumHighMark = (x)->sc_ulNumActive;   \
        } while (0)
    #define STATS_INC_ERR(x)     ((x)->sc_ulNumErrors++)
#else
    #define STATS_INC_ACTIVE(x)  do { } while (0)
    #define STATS_DEC_ACTIVE(x)  do { } while (0)
    #define STATS_INC_ALLOCED(x) do { } while (0)
    #define STATS_INC_GROWN(x)   do { } while (0)
    #define STATS_INC_REAPED(x)  do { } while (0)
    #define STATS_SET_HIGH(x)    do { } while (0)
    #define STATS_INC_ERR(x)     do { } while (0)
#endif

#if DEBUG_JIUKUN

/** Magic number of active object for red zoning. Placed in the first word before and the first
 *  word after an object.
 */
#define RED_MAGIC1               (0x5A2CF071UL)

/** Magic number of inactive object for red zoning. Placed in the first word before and the first
 *  word after an object.
 */
#define RED_MAGIC2               (0x170FC2A5UL)

#endif

/** Define the general cache data type.
 */
typedef struct general_cache
{
    /**Size of the cache.*/
    olsize_t gc_sSize;
    /**The cache object.*/
    slab_cache_t * gc_pscCache;
} general_cache_t;

/** These are the size for general cache. Custom caches can have other sizes.
 */
static olsize_t ls_sCacheSize[] =
{
#define CACHE(x) x,
#include "cachesizes.h"
    CACHE(OLSIZE_MAX)
#undef CACHE
};

/** Maximum number of general cache.
 */
#define MAX_NUM_OF_GENERAL_CACHE (20)

/** Define the internal jiukun slab data type.
 */
typedef struct internal_jiukun_slab
{
    /**Slab system is initialized if it's TRUE.*/
    boolean_t ijs_bInitialized;
    u8 ijs_u8Reserved[7];
    /**The cache for internal use. New caches are linked to ijs_scCacheCache.sc_jlNext.
       The cache objects are allocated from here.*/
    slab_cache_t ijs_scCacheCache;

    /**Limit for off-slab cache.*/
    u32 ijs_u32OffSlabLimit;

    /**Mutex lock for slab system.*/
    jf_mutex_t ijs_smLock;

    u8 ijs_u8Reserved2[16];

    u16 ijs_u16Reserved[4];

    /*The general cache.*/
    general_cache_t ijs_gcGeneral[MAX_NUM_OF_GENERAL_CACHE];
} internal_jiukun_slab_t;

/** Byte aligned size.
 */
#define SLAB_ALIGN_SIZE          (BYTES_PER_POINTER)

/* Store the cache object pointer to page object.
 */
#define SET_PAGE_CACHE(pg, x)    ((pg)->jp_jlLru.jl_pjlNext = (jf_listhead_t *)(x))

/* Retrieve the cache object pointer from page object.
 */
#define GET_PAGE_CACHE(pg)       ((slab_cache_t *)(pg)->jp_jlLru.jl_pjlNext)

/* Store the slab object pointer to page object.
 */
#define SET_PAGE_SLAB(pg, x)     ((pg)->jp_jlLru.jl_pjlPrev = (jf_listhead_t *)(x))

/* Retrieve the slab object pointer from page object.
 */
#define GET_PAGE_SLAB(pg)        ((slab_t *)(pg)->jp_jlLru.jl_pjlPrev)

/** Declare the internal jiukun slab object.
 */
static internal_jiukun_slab_t ls_iasSlab;

/* --- private routine section ------------------------------------------------------------------ */

#if defined(DEBUG_JIUKUN)

void _dumpSlabCache(slab_cache_t * pCache)
{
    jf_listhead_t * q = NULL;
    slab_t * slabp = NULL;
    u32 full_objs = 0, partial_objs = 0, free_objs = 0;
    u32 full_slabs = 0, partial_slabs = 0, free_slabs = 0;
    u32 full_use_objs = 0, partial_use_objs = 0, free_use_objs = 0;

    jf_logger_logInfoMsg(
        "cache name: %s, order: %u, num: %u",
        pCache->sc_strName, pCache->sc_u32Order, pCache->sc_u32Num);

    jf_listhead_forEach(&pCache->sc_jlFull, q)
    {
        slabp = jf_listhead_getEntry(q, slab_t, s_jlList);
        assert(slabp->s_u32InUse == pCache->sc_u32Num);
        full_objs += pCache->sc_u32Num;
        full_use_objs += slabp->s_u32InUse;
        full_slabs ++;
    }
    jf_listhead_forEach(&pCache->sc_jlPartial, q)
    {
        slabp = jf_listhead_getEntry(q, slab_t, s_jlList);
        assert((slabp->s_u32InUse != pCache->sc_u32Num) && (slabp->s_u32InUse != 0));
        partial_objs += pCache->sc_u32Num;
        partial_use_objs += slabp->s_u32InUse;
        partial_slabs ++;
    }
    jf_listhead_forEach(&pCache->sc_jlFree, q)
    {
        slabp = jf_listhead_getEntry(q, slab_t, s_jlList);
        assert(slabp->s_u32InUse == 0);
        free_objs += pCache->sc_u32Num;
        free_use_objs += slabp->s_u32InUse;
        free_slabs ++;
    }

    jf_logger_logInfoMsg(
        "full slabs %u, full objs %u, full use objs %u, partial slabs %u, partial objs %u, "
        "partial use objs %u, free slabs %u, free objs %u, free use objs %u",
        full_slabs, full_objs, full_use_objs, partial_slabs, partial_objs, partial_use_objs,
        free_slabs, free_objs, free_use_objs);
#if DEBUG_JIUKUN_STAT
    jf_logger_logInfoMsg(
        "high mark %u, active %u, allocation %u, grown %u, reaped %u, error %u",
        pCache->sc_ulNumHighMark, pCache->sc_ulNumActive, pCache->sc_ulNumAlloced,
        pCache->sc_ulNumGrown, pCache->sc_ulNumReaped, pCache->sc_ulNumErrors);
#endif
}
#endif

static inline u32 _allocObj(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void ** ppObj);


/** Calulate the number of objects, wastage, and bytes left over for a given slab size.
 */
static void _slabCacheEstimate(
    ulong jporder, olsize_t size, jf_flag_t flag, olsize_t * left_over, u32 * num)
{
    olint_t i = 0;
    olsize_t wastage = BUDDY_PAGE_SIZE << jporder;
    olsize_t extra = 0;
    olsize_t base = 0;

    if (! JF_FLAG_GET(flag, SC_FLAG_OFF_SLAB))
    {
        base = sizeof(slab_t);
        extra = sizeof(slab_bufctl_t);
    }
    i = 0;
    while (i * size + ALIGN_CEIL(base + i * extra, SLAB_ALIGN_SIZE) <= wastage)
        i++;
    if (i > 0)
        i--;

    if (i > SLAB_LIMIT)
        i = SLAB_LIMIT;

    *num = i;
    wastage -= i * size;
    wastage -= ALIGN_CEIL(base + i * extra, SLAB_ALIGN_SIZE);
    *left_over = wastage;
}

/** Free pages to buddy page allocator.
 */
static inline void _freePages(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void * addr)
{
    ulong i = (1 << pCache->sc_u32Order);
    jiukun_page_t * page = addrToJiukunPage(addr);
    void * pAddr = addr;

#if defined(DEBUG_JIUKUN)
    JF_LOGGER_DEBUG("page: %p", addr);
#endif

    /*Clear the flag of all pages.*/
    while (i--)
    {
        clearJpSlab(page);
        page++;
    }

    jf_jiukun_freePage(&pAddr);
}

static inline void * _allocOneObjFromTail(slab_cache_t * pCache, slab_t * slabp)
{
    u8 * objp = NULL;

    STATS_INC_ALLOCED(pCache);
    STATS_INC_ACTIVE(pCache);
    STATS_SET_HIGH(pCache);

    /*Get object pointer.*/
    slabp->s_u32InUse ++;
    objp = (u8 *)slabp->s_pMem + slabp->s_sbFree * pCache->sc_u32ObjSize;
    slabp->s_sbFree = slab_bufctl(slabp)[slabp->s_sbFree];

    /*Remove the slab to full list if it's the end of array.*/
    if (slabp->s_sbFree == BUFCTL_END)
    {
        jf_listhead_del(&(slabp->s_jlList));
        jf_listhead_add(&(pCache->sc_jlFull), &(slabp->s_jlList));
    }

#if DEBUG_JIUKUN
    if (JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_RED_ZONE))
    {
        /*Check old one.*/
        if ((*((ulong *)(objp)) != RED_MAGIC1) ||
            (*((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) != RED_MAGIC1))
        {
            /*Magic number is missing, out of bound.*/
            JF_LOGGER_ERR(JF_ERR_JIUKUN_MEMORY_CORRUPTED, "Invalid red zone");
            abort();
        }

        /*Set alloc red-zone.*/
        *((ulong *)(objp)) = RED_MAGIC2;
        *((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) = RED_MAGIC2;

        objp += SLAB_ALIGN_SIZE;
    }
#endif

    return objp;
}

/** Get the memory for a slab management object.
 */
static inline slab_t * _slabMgmt(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, u8 * objp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    slab_t * slabp = NULL;
    olint_t offset = 0;

    if (OFF_SLAB(pCache))
    {
        /*Slab management object is off-slab.*/
        u32Ret = _allocObj(pijs, pCache->sc_pscSlab, (void **)&slabp);
        if (u32Ret != JF_ERR_NO_ERROR)
            return NULL;
    }
    else
    {
        /*Slab management object is located at the start of the page.*/
        slabp = (slab_t *)objp;
        offset = ALIGN_CEIL(
            pCache->sc_u32Num * sizeof(slab_bufctl_t) + sizeof(slab_t), SLAB_ALIGN_SIZE);
    }
    slabp->s_u32InUse = 0;
    slabp->s_pMem = objp + offset;

    return slabp;
}

static inline void _initSlabCacheObjs(slab_cache_t * pCache, slab_t * slabp)
{
    olint_t i;

    for (i = 0; i < pCache->sc_u32Num; i++)
    {
#if DEBUG_JIUKUN
        u8 * objp = (u8 *)slabp->s_pMem + pCache->sc_u32ObjSize * i;

        /*If red zone is enabled, set magic number at the head and tail of the object.*/
        if (JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_RED_ZONE))
        {
            *((ulong*)(objp)) = RED_MAGIC1;
            *((ulong*)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) = RED_MAGIC1;
        }
#endif
        /*Set the list array.*/
        slab_bufctl(slabp)[i] = i + 1;
    }
    /*Set the end of list array.*/
    slab_bufctl(slabp)[i - 1] = BUFCTL_END;
    /*Set the first free object number to 0.*/
    slabp->s_sbFree = 0;
}

/** Grow the number of slabs within a cache. This is called by allocObj() when there are no active
 *  objects left in a cache.
 */
static u32 _growSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, jf_flag_t jpflag)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    slab_t * slabp = NULL;
    jiukun_page_t * page = NULL;
    void * objp = NULL;
    u32 i = 0;

    JF_FLAG_SET(pCache->sc_jfCache, SC_FLAG_GROWN);

    /*Get free page from jiukun page allocator.*/
    jpflag |= pCache->sc_jfPage;
    u32Ret = jf_jiukun_allocPage(&objp, pCache->sc_u32Order, jpflag);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
#if DEBUG_JIUKUN
        JF_LOGGER_DEBUG("addr: %p", objp);
#endif
        /*Get slab management object.*/
        slabp = _slabMgmt(pijs, pCache, objp);
        if (slabp == NULL)
        {
            _freePages(pijs, pCache, objp);
            u32Ret = JF_ERR_FAIL_GROW_JIUKUN_CACHE;
            JF_LOGGER_ERR(u32Ret, "slab error");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        i = 1 << pCache->sc_u32Order;
        page = addrToJiukunPage(objp);
        do
        {
            /*Set the cache pointer to page object.*/
            SET_PAGE_CACHE(page, pCache);
            /*Set the slab pointer to page object.*/
            SET_PAGE_SLAB(page, slabp);
            /*Set flag in page object.*/
            setJpSlab(page);
            page++;
        } while (--i);

        /*Initalize slab management object.*/
        _initSlabCacheObjs(pCache, slabp);

        /*Make slab active.*/
        jf_listhead_addTail(&(pCache->sc_jlFree), &(slabp->s_jlList));
        STATS_INC_GROWN(pCache);

#if DEBUG_JIUKUN
        _dumpSlabCache(pCache);
#endif
    }

    JF_FLAG_CLEAR(pCache->sc_jfCache, SC_FLAG_GROWN);

    return u32Ret;
}


/** Set the lock flag in cache with the lock in jiukun slab object.
 */
static inline void _lockSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    jf_mutex_acquire(&pijs->ijs_smLock);
    JF_FLAG_SET(pCache->sc_jfCache, SC_FLAG_LOCKED);
    jf_mutex_release(&pijs->ijs_smLock);
}

/** Clear the lock flag in cache with the lock in jiukun slab object.
 */
static inline void _unlockSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    jf_mutex_acquire(&pijs->ijs_smLock);
    JF_FLAG_CLEAR(pCache->sc_jfCache, SC_FLAG_LOCKED);
    jf_mutex_release(&pijs->ijs_smLock);
}

static inline u32 _allocObj(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void ** ppObj)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * entry = NULL;
    slab_t * slabp = NULL;
    jf_flag_t jpflag = 0;

#if defined(DEBUG_JIUKUN_VERBOSE)
    JF_LOGGER_DEBUG("alloc obj from %s", pCache->sc_strName);
#endif

    *ppObj = NULL;
    /*Set lock flag in cache with lock in jiukun slab object.*/
    _lockSlabCache(pijs, pCache);

    jf_mutex_acquire(&pCache->sc_jmCache);

    while (*ppObj == NULL)
    {
        /*First try partial list.*/
        entry = pCache->sc_jlPartial.jl_pjlNext;
        if (jf_listhead_isEmpty(&pCache->sc_jlPartial))
        {
            /*Partial list is empty, try free list.*/
            entry = pCache->sc_jlFree.jl_pjlNext;
            if (jf_listhead_isEmpty(&pCache->sc_jlFree))
            {
                /*free list is empty, need to grow cache.*/
                if (JF_FLAG_GET(pCache->sc_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_WAIT))
                    JF_FLAG_SET(jpflag, JF_JIUKUN_PAGE_ALLOC_FLAG_WAIT);

                u32Ret = _growSlabCache(pijs, pCache, jpflag);
                if (u32Ret != JF_ERR_NO_ERROR)
                    break;

                entry = pCache->sc_jlFree.jl_pjlNext;
            }
            /*Remove from free list and add to partial list.*/
            jf_listhead_del(entry);
            jf_listhead_add(&pCache->sc_jlPartial, entry);
        }

        /*Allocate one object from list.*/
        slabp = jf_listhead_getEntry(entry, slab_t, s_jlList);
        *ppObj = _allocOneObjFromTail(pCache, slabp);
    }

    jf_mutex_release(&pCache->sc_jmCache);

    _unlockSlabCache(pijs, pCache);

    return u32Ret;
}

#if DEBUG_JIUKUN
static olint_t _extraFreeChecks(slab_cache_t * pCache, slab_t * slabp, u8 * objp)
{
    olint_t i = 0;
    u32 objnr = (objp - (u8 *)slabp->s_pMem) / pCache->sc_u32ObjSize;

    /*Check the object number.*/
    if (objnr >= pCache->sc_u32Num)
        abort();

    /*Check the memory(object) address.*/
    if (objp != (u8 *)slabp->s_pMem + objnr * pCache->sc_u32ObjSize)
        abort();

    /*Check slab's freelist to see if this obj is there.*/
    for (i = slabp->s_sbFree; i != BUFCTL_END; i = slab_bufctl(slabp)[i])
    {
        if (i == objnr)
        {
            jf_logger_logErrMsg(JF_ERR_JIUKUN_DOUBLE_FREE, "extra free check");
            abort();
        }
    }

    return 0;
}
#endif

static inline void _freeOneObj(
    internal_jiukun_slab_t * pijs, slab_cache_t *pCache, u8 * objp)
{
    slab_t * slabp;
    jiukun_page_t * page = addrToJiukunPage(objp);

#if DEBUG_JIUKUN
    /*Make sure the page is used by slab, otherwise abort.*/
    if (! isJpSlab(page))
    {
        JF_LOGGER_ERR(JF_ERR_INVALID_JIUKUN_ADDRESS, "address: %p", objp);
        abort();
    }
#endif

    slabp = GET_PAGE_SLAB(page);

#if DEBUG_JIUKUN
    /*Check red zone if it's enabled.*/
    if (JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_RED_ZONE))
    {
        objp -= SLAB_ALIGN_SIZE;

        /*Check old one.*/
        if ((*((ulong *)(objp)) != RED_MAGIC2) ||
            (*((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) != RED_MAGIC2))
        {
            jf_logger_logErrMsg(JF_ERR_JIUKUN_FREE_UNALLOCATED, "red zone check");
            abort();
        }

        /*Set alloc red-zone.*/
        *((ulong *)(objp)) = RED_MAGIC1;
        *((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) = RED_MAGIC1;
    }

    if (_extraFreeChecks(pCache, slabp, objp))
        return;
#endif
    /*Restore the object number.*/
    {
        u32 objnr = (objp - (u8 *)slabp->s_pMem) / pCache->sc_u32ObjSize;

        slab_bufctl(slabp)[objnr] = slabp->s_sbFree;
        slabp->s_sbFree = objnr;
    }
    STATS_DEC_ACTIVE(pCache);
    /*Fixup slab chains.*/
    {
        u32 inuse = slabp->s_u32InUse;
        slabp->s_u32InUse --;
        if (slabp->s_u32InUse == 0)
        {
            /*It was partial or full (in case there is only 1 object in the slab), now empty.*/
            jf_listhead_del(&(slabp->s_jlList));
            jf_listhead_add(&(pCache->sc_jlFree), &(slabp->s_jlList));
        }
        else if (inuse == pCache->sc_u32Num)
        {
            /*It was full, now partial.*/
            jf_listhead_del(&(slabp->s_jlList));
            jf_listhead_add(&(pCache->sc_jlPartial), &(slabp->s_jlList));
        }
    }
}

static inline void _freeObj(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void ** pptr)
{
    assert(! JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_DESTROY));

    _lockSlabCache(pijs, pCache);

    jf_mutex_acquire(&pCache->sc_jmCache);
    _freeOneObj(pijs, pCache, *pptr);
    jf_mutex_release(&pCache->sc_jmCache);

    _unlockSlabCache(pijs, pCache);

    *pptr = NULL;
}

/** Destroy all the objects in a slab, and release the memory back to page allocator. Before calling
 *  the slab must have been unlinked from the cache. The cache-lock is not held/needed.
 */
static void _destroySlab(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, slab_t * slabp)
{
    slab_t * pSlab = slabp;

#if DEBUG_JIUKUN
    /*Check the red zone.*/
    if (JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_RED_ZONE))
    {
        olint_t i;
        for (i = 0; i < pCache->sc_u32Num; i++)
        {
            u8 * objp = (u8 *)slabp->s_pMem + pCache->sc_u32ObjSize * i;

            if ((*((ulong *)(objp)) != RED_MAGIC1) ||
                (*((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) != RED_MAGIC1))
            {
                jf_logger_logErrMsg(
                    JF_ERR_JIUKUN_MEMORY_LEAK, "destroy slab, objp: %p", objp + SLAB_ALIGN_SIZE);
            }
        }
    }
#endif

    /*Free the page.*/
    _freePages(pijs, pCache, (u8 *)slabp->s_pMem);

    /*Free the slab management object if the it's off slab.*/
    if (OFF_SLAB(pCache))
        _freeObj(pijs, pCache->sc_pscSlab, (void **)&pSlab);
}

static u32 _destroySlabCacheSlabs(
    internal_jiukun_slab_t * pijs, slab_cache_t * psc, jf_listhead_t * pjl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos = NULL, * next = NULL;
    slab_t * slabp = NULL;

    jf_listhead_forEachSafe(pjl, pos, next)
    {
        slabp = jf_listhead_getEntry(pos, slab_t, s_jlList);

        /*If there are still objects in use, memory leak occurs.*/
        if (slabp->s_u32InUse != 0)
        {
            jf_logger_logErrMsg(JF_ERR_JIUKUN_MEMORY_LEAK, "destroy cache slabs");
        }

        jf_listhead_del(pos);

        _destroySlab(pijs, psc, slabp);
    }

    return u32Ret;
}

static u32 _destroySlabCache(internal_jiukun_slab_t * pijs, slab_cache_t * psc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if DEBUG_JIUKUN_VERBOSE
    JF_LOGGER_INFO("name: %s", psc->sc_strName);
#endif

    _destroySlabCacheSlabs(pijs, psc, &psc->sc_jlFree);

    _destroySlabCacheSlabs(pijs, psc, &psc->sc_jlPartial);

    _destroySlabCacheSlabs(pijs, psc, &psc->sc_jlFull);

    jf_mutex_fini(&psc->sc_jmCache);

    return u32Ret;
}

static slab_cache_t * _findGeneralSlabCache(
    internal_jiukun_slab_t * pijs, olsize_t size, olint_t gfpflags)
{
    general_cache_t * pgc = &(pijs->ijs_gcGeneral[0]);

    for ( ; pgc->gc_sSize != OLSIZE_MAX; pgc ++)
    {
        if (size > pgc->gc_sSize)
            continue;
        break;
    }
    return pgc->gc_pscCache;
}

static u32 _createSlabCache(
    internal_jiukun_slab_t * pijs, jf_jiukun_cache_t ** ppCache,
    jf_jiukun_cache_create_param_t * pjjccp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t left_over = 0, slab_size = 0;
    slab_cache_t * pCache = NULL;
    u32 realobjsize = pjjccp->jjccp_sObj;
#ifdef DEBUG_JIUKUN
    jf_listhead_t * pjl = NULL;
#endif

#if DEBUG_JIUKUN_VERBOSE
    JF_LOGGER_DEBUG(
        "name, %s, size: %u, flag: 0x%llX",
        pjjccp->jjccp_pstrName, pjjccp->jjccp_sObj, pjjccp->jjccp_jfCache);
#endif

#if DEBUG_JIUKUN
    /*Do not red zone large object, causes severe fragmentation.*/
    if (pjjccp->jjccp_sObj < (BUDDY_PAGE_SIZE >> 3))
        JF_FLAG_SET(pjjccp->jjccp_jfCache, SC_FLAG_RED_ZONE);

#endif

    /*Check that size is in terms of words. This is needed to avoid unaligned accesses for some
      archs when redzoning is used, and makes sure any on-slab bufctl's are also correctly
      aligned.*/
    pjjccp->jjccp_sObj = ALIGN_CEIL(pjjccp->jjccp_sObj, SLAB_ALIGN_SIZE);

    /*Get cache's description object.*/
    u32Ret = _allocObj(pijs, &(pijs->ijs_scCacheCache), (void **)&pCache);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pCache, sizeof(slab_cache_t));

#if DEBUG_JIUKUN
        if (JF_FLAG_GET(pjjccp->jjccp_jfCache, SC_FLAG_RED_ZONE))
        {
            /*Reserve spaces for redzone.*/
            pjjccp->jjccp_sObj += 2 * SLAB_ALIGN_SIZE;
        }
#endif
        /*Determine if the slab management is 'on' or 'off' slab.*/
        if (pjjccp->jjccp_sObj >= (BUDDY_PAGE_SIZE >> 3))
            /*Size is large, assume best to place the slab management object off-slab (should allow
              better packing of objects).*/
            JF_FLAG_SET(pjjccp->jjccp_jfCache, SC_FLAG_OFF_SLAB);
    }

    /*Calculate size (in pages) of slabs, and the number of objects per slab.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        do
        {
            u32 break_flag = 0;

            _slabCacheEstimate(
                pCache->sc_u32Order, pjjccp->jjccp_sObj, pjjccp->jjccp_jfCache, &left_over,
                &pCache->sc_u32Num);
            if (break_flag)
                break;
            if (pCache->sc_u32Order >= MAX_JP_ORDER)
                break;
            /*Number of object is 0, need to increase order and try again.*/
            if (pCache->sc_u32Num == 0)
            {
                pCache->sc_u32Order++;
                continue;
            }
            if (JF_FLAG_GET(pjjccp->jjccp_jfCache, SC_FLAG_OFF_SLAB) &&
                (pCache->sc_u32Num > pijs->ijs_u32OffSlabLimit))
            {
                /*This num of objects will cause problems.*/
                pCache->sc_u32Order--;
                break_flag++;
                continue;
            }
            if ((left_over * 8) <= (BUDDY_PAGE_SIZE << pCache->sc_u32Order))
                break;  /*Acceptable internal fragmentation.*/
        } while (1);
    }

    /*Check the number of object in cache.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
#if DEBUG_JIUKUN_VERBOSE
        JF_LOGGER_DEBUG(
            "name, %s, size: %u, order: %u, num: %u",
            pjjccp->jjccp_pstrName, pjjccp->jjccp_sObj, pCache->sc_u32Order, pCache->sc_u32Num);
#endif

        if (pCache->sc_u32Num == 0)
        {
            u32Ret = JF_ERR_FAIL_CREATE_JIUKUN_CACHE;
            JF_LOGGER_ERR(u32Ret, "couldn't create cache: %s", pjjccp->jjccp_pstrName);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        slab_size = ALIGN_CEIL(
            pCache->sc_u32Num * sizeof(slab_bufctl_t) + sizeof(slab_t), SLAB_ALIGN_SIZE);

        /*If the slab has been placed off-slab, and we have enough space then move it on-slab.*/
        if (JF_FLAG_GET(pjjccp->jjccp_jfCache, SC_FLAG_OFF_SLAB) && (left_over >= slab_size))
        {
            JF_FLAG_CLEAR(pjjccp->jjccp_jfCache, SC_FLAG_OFF_SLAB);
            left_over -= slab_size;
        }

        pCache->sc_jfCache = pjjccp->jjccp_jfCache;
        pCache->sc_jfPage = 0;

        u32Ret = jf_mutex_init(&(pCache->sc_jmCache));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The object size may not be equal to real object size if red zone is enabled.*/
        pCache->sc_u32ObjSize = pjjccp->jjccp_sObj;
        /*Real object size is from user.*/
        pCache->sc_u32RealObjSize = realobjsize;

        jf_listhead_init(&(pCache->sc_jlFull));
        jf_listhead_init(&(pCache->sc_jlPartial));
        jf_listhead_init(&(pCache->sc_jlFree));

        /*If slab management object is off-slab, need to find a general cache to allocate it.*/
        if (JF_FLAG_GET(pjjccp->jjccp_jfCache, SC_FLAG_OFF_SLAB))
            pCache->sc_pscSlab = _findGeneralSlabCache(pijs, slab_size, 0);
        ol_strncpy(pCache->sc_strName, pjjccp->jjccp_pstrName, CACHE_NAME_LEN - 1);

        /*Add cache to list.*/
        jf_mutex_acquire(&(pijs->ijs_smLock));
#ifdef DEBUG_JIUKUN
        /*Make sure there is no other cache which has the same name with current one.*/
        jf_listhead_forEach(&(pijs->ijs_scCacheCache.sc_jlNext), pjl)
        {
            slab_cache_t * pc = jf_listhead_getEntry(pjl, slab_cache_t, sc_jlNext);
            assert(strcmp(pc->sc_strName, pjjccp->jjccp_pstrName) != 0);
        }
#endif
        jf_listhead_add(&(pijs->ijs_scCacheCache.sc_jlNext), &(pCache->sc_jlNext));
        jf_mutex_release(&(pijs->ijs_smLock));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppCache = pCache;
    else if (pCache != NULL)
        _freeObj(pijs, &(pijs->ijs_scCacheCache), (void **)&pCache);

    return u32Ret;
}

static u32 _initSlabCache(internal_jiukun_slab_t * pijs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    slab_cache_t * pkc = &(pijs->ijs_scCacheCache);
    olsize_t left_over = 0;
    general_cache_t * sizes = NULL;
    olchar_t name[20];
    jf_jiukun_cache_create_param_t jjccp;
    u16 u16NumOfSize = 0;

    jf_listhead_init(&(pkc->sc_jlNext));

    jf_listhead_init(&(pkc->sc_jlFull));
    jf_listhead_init(&(pkc->sc_jlPartial));
    jf_listhead_init(&(pkc->sc_jlFree));

    pkc->sc_u32ObjSize = sizeof(slab_cache_t);
    /*The cache cache cannot be reapped.*/
    JF_FLAG_SET(pkc->sc_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_NOREAP);
    ol_strcpy(pkc->sc_strName, "cache_cache");

    _slabCacheEstimate(0, pkc->sc_u32ObjSize, 0, &left_over, &(pkc->sc_u32Num));

    /*Create the general cache.*/
    sizes = &(pijs->ijs_gcGeneral[0]);

    while ((ls_sCacheSize[u16NumOfSize] != OLSIZE_MAX) && (u32Ret == JF_ERR_NO_ERROR))
    {
        sizes->gc_sSize = ls_sCacheSize[u16NumOfSize];
        ol_snprintf(name, sizeof(name), "size-%d", sizes->gc_sSize);

        ol_bzero(&jjccp, sizeof(jjccp));

        jjccp.jjccp_pstrName = name;
        jjccp.jjccp_sObj = sizes->gc_sSize;

        u32Ret = _createSlabCache(pijs, (jf_jiukun_cache_t **)&(sizes->gc_pscCache), &jjccp);

        /*Inc off-slab bufctl limit until the ceiling is hit.*/
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (! OFF_SLAB(sizes->gc_pscCache))
            {
                pijs->ijs_u32OffSlabLimit = sizes->gc_sSize - sizeof(slab_t);
                pijs->ijs_u32OffSlabLimit /= 2;
            }
        }

        sizes ++;
        u16NumOfSize ++;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        assert(u16NumOfSize < MAX_NUM_OF_GENERAL_CACHE);

        sizes->gc_sSize = OLSIZE_MAX;

        u32Ret = jf_mutex_init(&(pkc->sc_jmCache));
    }

    return u32Ret;
}

/** Shrink slab cache.
 *
 *  @note
 *  -# Only free slab can be freed.
 *
 *  @return Number of slab freed.
 */
static olint_t _shrinkSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    slab_t * slabp = NULL;
    olint_t ret = 0;
    jf_listhead_t * p = NULL;

    while (1)
    {
        p = pCache->sc_jlFree.jl_pjlPrev;

        /*Quit if the free list is empty.*/
        if (p == &(pCache->sc_jlFree))
            break;

        /*Get entry from free list and free it.*/
        slabp = jf_listhead_getEntry(pCache->sc_jlFree.jl_pjlPrev, slab_t, s_jlList);
#if DEBUG_JIUKUN
        assert(slabp->s_u32InUse == 0);
#endif
        jf_listhead_del(&(slabp->s_jlList));

        _destroySlab(pijs, pCache, slabp);
        ret++;
    }

    return ret;
}

#if DEBUG_JIUKUN
static void * _getMemoryEndAddr(internal_jiukun_slab_t * pijs, void * pMem)
{
    void * pRet = NULL;
    jiukun_page_t * pap = NULL;
    slab_cache_t * pCache = NULL;
    slab_t * slabp = NULL;
    u32 objnr = 0;

    pap = addrToJiukunPage(pMem);
    pCache = GET_PAGE_CACHE(pap);
    slabp = GET_PAGE_SLAB(pap);

    objnr = ((u8 *)pMem - (u8 *)slabp->s_pMem) / pCache->sc_u32ObjSize;
    /*Start address of the memory.*/
    pRet = (u8 *)slabp->s_pMem + objnr * pCache->sc_u32ObjSize;

    /*End address of the memory.*/
    pRet = (u8 *)pRet + pCache->sc_u32RealObjSize;

    /*Add the space used by red zone.*/
    if (JF_FLAG_GET(pCache->sc_jfCache, SC_FLAG_RED_ZONE))
        pRet = (u8 *)pRet + SLAB_ALIGN_SIZE;

    return pRet;
}
#endif

/* --- public routine section ------------------------------------------------------------------- */

u32 initJiukunSlab(slab_param_t * psp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    assert(psp != NULL);
    assert(! pijs->ijs_bInitialized);

    JF_LOGGER_INFO("init");

    u32Ret = jf_mutex_init(&(pijs->ijs_smLock));

    /*Initialize cache.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initSlabCache(pijs);

    if (u32Ret == JF_ERR_NO_ERROR)
        pijs->ijs_bInitialized = TRUE;
    else if (pijs != NULL)
        finiJiukunSlab();

    return u32Ret;
}

u32 finiJiukunSlab(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    general_cache_t * pgc = NULL;
    slab_cache_t * psc = NULL;

    JF_LOGGER_INFO("fini");

    pgc = pijs->ijs_gcGeneral;

    /*Move to the end of general cache array.*/
    while ((pgc->gc_sSize != OLSIZE_MAX) && (pgc->gc_pscCache != NULL))
        pgc ++;

    /*It's necessary to destroy the general cache from the end to the start in the array. As slab
      management objects are allocated from the first several general cache.*/
    for ( ; pgc != pijs->ijs_gcGeneral; )
    {
        pgc --;
        jf_jiukun_destroyCache((void **)&(pgc->gc_pscCache));
    }

    /*Destroy the cache cache.*/
    psc = &(pijs->ijs_scCacheCache);
    _destroySlabCache(pijs, psc);

    jf_mutex_fini(&(pijs->ijs_smLock));

    pijs->ijs_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_jiukun_createCache(
    jf_jiukun_cache_t ** ppCache, jf_jiukun_cache_create_param_t * pjjccp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    /*Sanity checks.*/
    assert(pijs->ijs_bInitialized);
    assert(pjjccp != NULL);
    assert((pjjccp->jjccp_pstrName != NULL) &&
           (pjjccp->jjccp_sObj >= SLAB_ALIGN_SIZE) &&
           (pjjccp->jjccp_sObj <= (1 << MAX_JP_ORDER) * BUDDY_PAGE_SIZE));

    u32Ret = _createSlabCache(pijs, ppCache, pjjccp);

    return u32Ret;
}

u32 jf_jiukun_destroyCache(jf_jiukun_cache_t ** ppCache)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * psc = NULL;

    assert((ppCache != NULL) && (*ppCache != NULL));

    psc = (slab_cache_t *) *ppCache;
    *ppCache = NULL;

    /*Find the cache in the chain of caches.*/
    jf_mutex_acquire(&(pijs->ijs_smLock));
    /*The chain is never empty, cache_cache is never destroyed.*/
    jf_listhead_del(&(psc->sc_jlNext));
    JF_FLAG_SET(psc->sc_jfCache, SC_FLAG_DESTROY);
    jf_mutex_release(&(pijs->ijs_smLock));

    /*Destory the cache.*/
    _destroySlabCache(pijs, psc);
    /*Free the jiukun slab object.*/
    _freeObj(pijs, &pijs->ijs_scCacheCache, (void **)&psc);

    return u32Ret;
}

olint_t reapJiukunSlab(boolean_t bNoWait)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * searchp = NULL;
    olint_t ret = 0;
    jf_listhead_t * pjl = NULL;

    assert(pijs->ijs_bInitialized);

    JF_LOGGER_DEBUG("nowait: %d", bNoWait);

    if (bNoWait)
    {
        /*Try to acquire the lock if no wait is requested.*/
        u32Ret = jf_mutex_tryAcquire(&(pijs->ijs_smLock));
        if (u32Ret != JF_ERR_NO_ERROR)
            return ret;
    }
    else
    {
        /*Lock jiukun slab object.*/
        jf_mutex_acquire(&(pijs->ijs_smLock));
    }
    JF_LOGGER_DEBUG("start");

    jf_listhead_forEach(&(pijs->ijs_scCacheCache.sc_jlNext), pjl)
    {
        searchp = jf_listhead_getEntry(pjl, slab_cache_t, sc_jlNext);

        /*Skip the cache with no reap flag.*/
        if (JF_FLAG_GET(searchp->sc_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_NOREAP))
        {
            JF_LOGGER_DEBUG("cache %s no reap", searchp->sc_strName);
            continue;
        }

        /*Skip the cache which is growning.*/
        if (JF_FLAG_GET(searchp->sc_jfCache, SC_FLAG_GROWN))
        {
            JF_LOGGER_DEBUG("cache %s is growing", searchp->sc_strName);
            continue;
        }

        /*Skip the cache which is locked.*/
        if (JF_FLAG_GET(searchp->sc_jfCache, SC_FLAG_LOCKED))
        {
            JF_LOGGER_DEBUG("cache %s is locked", searchp->sc_strName);
            continue;
        }

        /*Lock the cache.*/
        jf_mutex_acquire(&(searchp->sc_jmCache));
#if DEBUG_JIUKUN
        JF_LOGGER_DEBUG(
            "cache: %p, name: %s, flag: 0x%llX", searchp, searchp->sc_strName, searchp->sc_jfCache);
        _dumpSlabCache(searchp);
#endif

        /*Shrink the cache.*/
        ret += _shrinkSlabCache(pijs, searchp);

        jf_mutex_release(&(searchp->sc_jmCache));
    }

    jf_mutex_release(&(pijs->ijs_smLock));

    return ret;
}

void jf_jiukun_freeObject(jf_jiukun_cache_t * pCache, void ** pptr)
{
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    assert(pijs->ijs_bInitialized);
    assert((pCache != NULL) && (pptr != NULL) && (*pptr != NULL));

    _freeObj(pijs, (slab_cache_t *)pCache, pptr);
    *pptr = NULL;
}

u32 jf_jiukun_allocObject(jf_jiukun_cache_t * pCache, void ** pptr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * cache = (slab_cache_t *)pCache;

    assert(pijs->ijs_bInitialized);
    assert((pCache != NULL) && (pptr != NULL));

    u32Ret = _allocObj(pijs, cache, pptr);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Clear the memory if zero flag is set.*/
        if (JF_FLAG_GET(cache->sc_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO))
            ol_memset(*pptr, 0, cache->sc_u32RealObjSize);
    }

#if defined(DEBUG_JIUKUN_VERBOSE)
    JF_LOGGER_DEBUG("obj: %p", *pptr);
#endif

    return u32Ret;
}

u32 jf_jiukun_allocMemory(void ** pptr, olsize_t size)
{
    u32 u32Ret = JF_ERR_UNSUPPORTED_MEMORY_SIZE;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    general_cache_t * pgc = pijs->ijs_gcGeneral;

    assert(pijs->ijs_bInitialized);

    *pptr = NULL;

    /*Find a right general cache.*/
    for ( ; pgc->gc_sSize != OLSIZE_MAX; pgc++) 
    {
        if (size > pgc->gc_sSize)
            continue;

        /*Allocate object from the cache.*/
        u32Ret = _allocObj(pijs, pgc->gc_pscCache, pptr);
        break;
    }

#if defined(DEBUG_JIUKUN_VERBOSE)
    JF_LOGGER_DEBUG("memory: %p, size: %u", *pptr, size);
#endif

    return u32Ret;
}

void jf_jiukun_freeMemory(void ** pptr)
{
    slab_cache_t * pCache = NULL;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    void * objp = * pptr;

    assert(pijs->ijs_bInitialized);
    assert((pptr != NULL) && (*pptr != NULL));

#if defined(DEBUG_JIUKUN_VERBOSE)
    JF_LOGGER_DEBUG("obj: %p", *pptr);
#endif

    pCache = GET_PAGE_CACHE(addrToJiukunPage(objp));

    _freeObj(pijs, pCache, pptr);
}

u32 jf_jiukun_cloneMemory(void ** pptr, const u8 * pu8Buffer, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pptr != NULL) && (pu8Buffer != NULL) && (size > 0));

    u32Ret = jf_jiukun_allocMemory(pptr, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memcpy(*pptr, pu8Buffer, size);
    }

    return u32Ret;
}

u32 jf_jiukun_memcpy(void * pDest, const void * pSource, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if DEBUG_JIUKUN
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    void * pEnd = (u8 *)pDest + size;
    void * pMemEnd = NULL;

    assert((pDest != NULL) && (pSource != NULL) && (size > 0));

    /*Get the end address of the allocated memory.*/
    pMemEnd = _getMemoryEndAddr(pijs, pDest);

    if (pEnd > pMemEnd)
    {
        JF_LOGGER_ERR(JF_ERR_JIUKUN_MEMORY_OUT_OF_BOUND, "jiukun memcpy");
        abort();
    }
    else
    {
        ol_memcpy(pDest, pSource, size);
    }
#else
    ol_memcpy(pDest, pSource, size);
#endif

    return u32Ret;
}

u32 jf_jiukun_strncpy(olchar_t * pDest, const olchar_t * pSource, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if DEBUG_JIUKUN
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    void * pEnd = pDest + size;
    void * pMemEnd = NULL;

    assert((pDest != NULL) && (pSource != NULL) && (size > 0));

    /*Get the end address of the allocated memory.*/
    pMemEnd = _getMemoryEndAddr(pijs, pDest);

    if (pEnd > pMemEnd)
    {
        JF_LOGGER_ERR(JF_ERR_JIUKUN_MEMORY_OUT_OF_BOUND, "jiukun strncpy");
        abort();
    }
    else
    {
        ol_strncpy(pDest, pSource, size);
    }
#else
    ol_strncpy(pDest, pSource, size);
#endif

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
