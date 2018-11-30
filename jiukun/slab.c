/**
 *  @file slab.c
 *
 *  @brief The slab memory allocation system
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "slab.h"
#include "xmalloc.h"
#include "syncmutex.h"
#include "common.h"
#include "syncmutex.h"

/* --- private data/data structure section --------------------------------- */

/** Maximum size of an obj (in 2^order pages) and absolute limit for the page
 *  order. The size should be more than the maximum size of general cache.
 *  Up to 8Mb.
 */
#define MAX_JP_ORDER      (11)

/** Bufctl's are used for linking objs within a slab linked offsets.
 *
 *  This implementation relies on "struct jiukun_page" for locating the cache &
 *  slab an object belongs to.
 *  This allows the bufctl structure to be small (one int), but limits the
 *  number of objects a slab (not a cache) can contain when off-slab bufctls
 *  are used. The limit is the size of the largest general cache that does not
 *  use off-slab slabs.
 */
typedef u32 slab_bufctl_t;
#define BUFCTL_END  (((slab_bufctl_t)(~0U)) - 0)
#define BUFCTL_FREE (((slab_bufctl_t)(~0U)) - 1)
#define SLAB_LIMIT  (((slab_bufctl_t)(~0U)) - 2)

/** Manages the objs in a slab. Placed either at the beginning of mem allocated
 *  for a slab, or allocated from an general cache.
 *  Slabs are chained into three list: fully used, partial, fully free slabs.
 */
typedef struct
{
    struct list_head s_lhList;
    /** pointer to objects */
    void * s_pMem;
    /** num of objs active in slab */
    u32 s_u32InUse;
    slab_bufctl_t s_sbFree;
} slab_t;

#define slab_bufctl(slabp) \
    ((slab_bufctl_t *)(((slab_t*)slabp) + 1))

/** Max name length for a slab cache
 *
 */
#define CACHE_NAME_LEN     (20)

/** Internal flags for slab_cache
 *
 */
typedef enum slab_cache_flag
{
    SC_FLAG_OFF_SLAB = 32,/**< Slab management in own cache */
    SC_FLAG_GROWN,        /**< Cache is growing */
    SC_FLAG_DESTROY,      /**< Cache is being destroyed */
    SC_FLAG_RED_ZONE,     /**< Red zone objs in a cache, available when
                             DEBUG_JIUKUN is true, the size of obj should less
                             than (BUDDY_PAGE_SIZE >> 3) */
    SC_FLAG_POISON,       /**< Poison objects */
    SC_FLAG_LOCKED,       /**< Cache is locked */
} slab_cache_flag_t;

typedef struct slab_cache
{
    u32 sc_u32ObjSize;
    u32 sc_u32RealObjSize;
    /** cache flags */
    olflag_t sc_fCache;
    /** num of objs per slab */
    u32 sc_u32Num;

    /** lock for the full, partial and free list*/
    sync_mutex_t sc_smCache;
    /** list for fully used slab */
    struct list_head sc_lhFull;
    /** list for partial slab */
    struct list_head sc_lhPartial;
    /** list for fully free slab */
    struct list_head sc_lhFree;

    /** order of pages per slab (2^n) */
    u32 sc_u32Order;

    /** page flags */
    olflag_t sc_fPage;

    /** cache for slab_t */
    struct slab_cache * sc_pscSlab;

    /** cache name */
    olchar_t sc_strName[CACHE_NAME_LEN];
    /** linked in cache_cache */
    struct list_head sc_lhNext;

#if DEBUG_JIUKUN_STAT
    ulong sc_ulNumActive;
    ulong sc_ulNumAlloced;
    ulong sc_ulNumHighMark;
    ulong sc_ulNumGrown;
    ulong sc_ulNumReaped;
    ulong sc_ulNumErrors;
#endif
} slab_cache_t;


#define OFF_SLAB(x) (GET_FLAG((x)->sc_fCache, SC_FLAG_OFF_SLAB))
#define GROWN(x)    (GET_FLAG((x)->sc_fCache, AF_FLAGS_GROWN))

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
/** Magic nums for obj red zoning.
 *  Placed in the first word before and the first word after an obj.
 */
#define RED_MAGIC1  0x5A2CF071UL    /* when obj is active */
#define RED_MAGIC2  0x170FC2A5UL    /* when obj is inactive */

/** For poisoning
 */
#define POISON_BYTE 0x5a        /* byte value for poisoning */
#define POISON_END  0xa5        /* end-byte of poisoning */

#endif

/** General caches.
 */
typedef struct general_cache
{
    olsize_t gc_sSize;
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
} ;

#define MAX_NUM_OF_GENERAL_CACHE  20

typedef struct internal_jiukun_slab
{
    boolean_t ijs_bInitialized;
    u8 ijs_u8Reserved[7];
    /** New caches are linked to ijs_scCacheCache.sc_lhNext*/
    slab_cache_t ijs_scCacheCache;

    u32 ijs_u32OffSlabLimit;

    sync_mutex_t ijs_smLock;

    u8 ijs_u8Reserved2[16];

    u16 ijs_u16Reserved[4];
    general_cache_t ijs_gcGeneral[MAX_NUM_OF_GENERAL_CACHE];
} internal_jiukun_slab_t;

/** Byte aligned size
 */
#define SLAB_ALIGN_SIZE  BYTES_PER_WORD

/* Macros for storing/retrieving the cache and slab from the page structure.
 * These are used to find the cache and slab an obj belongs to.
 */
#define SET_PAGE_CACHE(pg, x) \
    ((pg)->jp_lhLru.lh_plhNext = (struct list_head *)(x))
#define GET_PAGE_CACHE(pg)    ((slab_cache_t *)(pg)->jp_lhLru.lh_plhNext)
#define SET_PAGE_SLAB(pg, x)  \
    ((pg)->jp_lhLru.lh_plhPrev = (struct list_head *)(x))
#define GET_PAGE_SLAB(pg)     ((slab_t *)(pg)->jp_lhLru.lh_plhPrev)

static internal_jiukun_slab_t ls_iasSlab;

/* --- private routine section---------------------------------------------- */
#if defined(DEBUG_JIUKUN)
void _dumpSlabCache(slab_cache_t * pCache)
{
    struct list_head * q;
    slab_t * slabp;
    u32 full_objs = 0, partial_objs = 0, free_objs = 0;
    u32 full_slabs = 0, partial_slabs = 0, free_slabs = 0;
    u32 full_use_objs = 0, partial_use_objs = 0, free_use_objs = 0;

    logInfoMsg(
        "cachep name %s, order %u, num %u",
        pCache->sc_strName, pCache->sc_u32Order, pCache->sc_u32Num);

    listForEach(&pCache->sc_lhFull, q)
    {
        slabp = listEntry(q, slab_t, s_lhList);
        assert(slabp->s_u32InUse == pCache->sc_u32Num);
        full_objs += pCache->sc_u32Num;
        full_use_objs += slabp->s_u32InUse;
        full_slabs ++;
    }
    listForEach(&pCache->sc_lhPartial, q)
    {
        slabp = listEntry(q, slab_t, s_lhList);
        assert((slabp->s_u32InUse != pCache->sc_u32Num) && (slabp->s_u32InUse != 0));
        partial_objs += pCache->sc_u32Num;
        partial_use_objs += slabp->s_u32InUse;
        partial_slabs ++;
    }
    listForEach(&pCache->sc_lhFree, q)
    {
        slabp = listEntry(q, slab_t, s_lhList);
        assert(slabp->s_u32InUse == 0);
        free_objs += pCache->sc_u32Num;
        free_use_objs += slabp->s_u32InUse;
        free_slabs ++;
    }

    logInfoMsg(
        "full slabs %u, full objs %u, full use objs %u, "
        "partial slabs %u, partial objs %u, partial use objs %u, "
        "free slabs %u, free objs %u, free use objs %u",
        full_slabs, full_objs, full_use_objs,
        partial_slabs, partial_objs, partial_use_objs,
        free_slabs, free_objs, free_use_objs);
#if DEBUG_JIUKUN_STAT
    logInfoMsg(
        "high mark %u, active %u, allocation %u, "
        "grown %u, reaped %u, error %u",
        pCache->sc_ulNumHighMark, pCache->sc_ulNumActive,
        pCache->sc_ulNumAlloced, pCache->sc_ulNumGrown, pCache->sc_ulNumReaped,
        pCache->sc_ulNumErrors);
#endif
}
#endif

static inline u32 _allocObj(internal_jiukun_slab_t * pijs,
    slab_cache_t * pCache, void ** ppObj, olflag_t flag);


/* Cal the num objs, wastage, and bytes left over for a given slab size. */
static void _slabCacheEstimate(
    ulong jporder, olsize_t size,
    olflag_t flag, olsize_t * left_over, u32 * num)
{
    olint_t i;
    olsize_t wastage = BUDDY_PAGE_SIZE << jporder;
    olsize_t extra = 0;
    olsize_t base = 0;

    if (! GET_FLAG(flag, SC_FLAG_OFF_SLAB))
    {
        base = sizeof(slab_t);
        extra = sizeof(slab_bufctl_t);
    }
    i = 0;
    while (i * size + ALIGN(base + i * extra, SLAB_ALIGN_SIZE) <= wastage)
        i++;
    if (i > 0)
        i--;

    if (i > SLAB_LIMIT)
        i = SLAB_LIMIT;

    *num = i;
    wastage -= i * size;
    wastage -= ALIGN(base + i * extra, SLAB_ALIGN_SIZE);
    *left_over = wastage;
}

/** Free pages to buddy page allocator
 */
static inline void _freePages(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void * addr)
{
    ulong i = (1 << pCache->sc_u32Order);
    jiukun_page_t * page = addrToJiukunPage(addr);
    void * pAddr = addr;

#if defined(DEBUG_JIUKUN)
    logInfoMsg("slab free page, %p", addr);
#endif

    while (i--)
    {
        clearJpSlab(page);
        page++;
    }

    freeJiukunPage(&pAddr);
}

#if DEBUG_JIUKUN
static inline void _poisonObj(slab_cache_t * pCache, u8 * addr)
{
    olint_t size = pCache->sc_u32ObjSize;

    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
    {
        addr += SLAB_ALIGN_SIZE;
        size -= 2 * SLAB_ALIGN_SIZE;
    }
    memset(addr, POISON_BYTE, size);
    *(u8 *)(addr + size - 1) = POISON_END;
}

static inline boolean_t _checkPoisonObj(slab_cache_t * pCache, u8 * addr)
{
    olint_t size = pCache->sc_u32ObjSize;
    void * end;

    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
    {
        addr += SLAB_ALIGN_SIZE;
        size -= 2 * SLAB_ALIGN_SIZE;
    }

    end = memchr(addr, POISON_END, size);
    if (end != (addr + size - 1))
        return TRUE;

    return FALSE;
}
#endif

static inline void * _allocOneObjFromTail(
    slab_cache_t * pCache, slab_t * slabp)
{
    u8 * objp;

    STATS_INC_ALLOCED(pCache);
    STATS_INC_ACTIVE(pCache);
    STATS_SET_HIGH(pCache);

    /* get obj pointer */
    slabp->s_u32InUse ++;
    objp = (u8 *)slabp->s_pMem + slabp->s_sbFree * pCache->sc_u32ObjSize;
    slabp->s_sbFree = slab_bufctl(slabp)[slabp->s_sbFree];

    if (slabp->s_sbFree == BUFCTL_END)
    {
        listDel(&(slabp->s_lhList));
        listAdd(&(pCache->sc_lhFull), &(slabp->s_lhList));
    }

#if DEBUG_JIUKUN
    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_POISON))
        if (_checkPoisonObj(pCache, objp))
        {
            logInfoMsg("Out of bound access");
            abort();
        }

    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
    {
        /* check old one. */
        if ((*((ulong *)(objp)) != RED_MAGIC1) ||
            (*((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) !=
             RED_MAGIC1))
        {
            logInfoMsg("Invalid red zone");
            abort();
        }

        /* Set alloc red-zone. */
        *((ulong *)(objp)) = RED_MAGIC2;
        *((ulong *)(objp + pCache->sc_u32ObjSize -
                             SLAB_ALIGN_SIZE)) = RED_MAGIC2;

        objp += SLAB_ALIGN_SIZE;
    }
#endif

    return objp;
}

/* Get the memory for a slab management obj. */
static inline slab_t * _slabMgmt(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, u8 * objp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olflag_t flags = 0;
    slab_t * slabp = NULL;
    olint_t offset = 0;

    if (OFF_SLAB(pCache))
    {
        /* Slab management obj is off-slab. */
        u32Ret = _allocObj(
            pijs, pCache->sc_pscSlab, (void **)&slabp, flags);
        if (u32Ret != OLERR_NO_ERROR)
            return NULL;
    }
    else
    {
        slabp = (slab_t *)objp;
        offset = ALIGN(
            pCache->sc_u32Num * sizeof(slab_bufctl_t) + sizeof(slab_t),
            SLAB_ALIGN_SIZE);
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

        if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
        {
            *((ulong*)(objp)) = RED_MAGIC1;
            *((ulong*)(objp + pCache->sc_u32ObjSize -
                               SLAB_ALIGN_SIZE)) = RED_MAGIC1;
            objp += SLAB_ALIGN_SIZE;
        }
#endif

#if DEBUG_JIUKUN
        if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
            objp -= SLAB_ALIGN_SIZE;

        if (GET_FLAG(pCache->sc_fCache, SC_FLAG_POISON))
            /* need to poison the objs */
            _poisonObj(pCache, objp);

        if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
        {
            if (*((ulong *)(objp)) != RED_MAGIC1)
                abort();
            if (*((ulong *)(objp + pCache->sc_u32ObjSize -
                SLAB_ALIGN_SIZE)) != RED_MAGIC1)
                abort();
        }
#endif
        slab_bufctl(slabp)[i] = i + 1;
    }
    slab_bufctl(slabp)[i - 1] = BUFCTL_END;
    slabp->s_sbFree = 0;
}

/** Grow the number of slabs within a cache. This is called by allocObj() when
 *  there are no active objs left in a cache.
 */
static u32 _growSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, olflag_t jpflag)
{
    u32 u32Ret = OLERR_NO_ERROR;
    slab_t * slabp;
    jiukun_page_t * page;
    void * objp;
    u32 i;

    SET_FLAG(pCache->sc_fCache, SC_FLAG_GROWN);

    /* Get mem for the objs. */
    jpflag |= pCache->sc_fPage;
    u32Ret = allocJiukunPage(&objp, pCache->sc_u32Order, jpflag);
    if (u32Ret == OLERR_NO_ERROR)
    {
#if DEBUG_JIUKUN
        logInfoMsg("grow cache, %p", objp);
#endif
        /* Get slab management. */
        slabp = _slabMgmt(pijs, pCache, objp);
        if (slabp == NULL)
        {
            _freePages(pijs, pCache, objp);
            u32Ret = OLERR_FAIL_GROW_JIUKUN_CACHE;    
            logErrMsg(u32Ret, "grow cache, slab error");
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        i = 1 << pCache->sc_u32Order;
        page = addrToJiukunPage(objp);
        do
        {
            SET_PAGE_CACHE(page, pCache);
            SET_PAGE_SLAB(page, slabp);
            setJpSlab(page);
            page++;
        } while (--i);

        _initSlabCacheObjs(pCache, slabp);

        /* Make slab active. */
        listAddTail(&(pCache->sc_lhFree), &(slabp->s_lhList));
        STATS_INC_GROWN(pCache);

#if DEBUG_JIUKUN
        _dumpSlabCache(pCache);
#endif
    }

    CLEAR_FLAG(pCache->sc_fCache, SC_FLAG_GROWN);

    return u32Ret;
}

static inline void _lockSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    acquireSyncMutex(&pijs->ijs_smLock);
    SET_FLAG(pCache->sc_fCache, SC_FLAG_LOCKED);
    releaseSyncMutex(&pijs->ijs_smLock);
}

static inline void _unlockSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    acquireSyncMutex(&pijs->ijs_smLock);
    CLEAR_FLAG(pCache->sc_fCache, SC_FLAG_LOCKED);
    releaseSyncMutex(&pijs->ijs_smLock);
}

static inline u32 _allocObj(
    internal_jiukun_slab_t * pijs,
    slab_cache_t * pCache, void ** ppObj, olflag_t flag)
{
    u32 u32Ret = OLERR_NO_ERROR;
    struct list_head * entry;
    slab_t * slabp;
    olflag_t jpflag = 0;

#if defined(DEBUG_JIUKUN_VERBOSE)
    logInfoMsg("alloc obj from %s", pCache->sc_strName);
#endif

    *ppObj = NULL;
    _lockSlabCache(pijs, pCache);

    acquireSyncMutex(&pCache->sc_smCache);

    while (*ppObj == NULL)
    {
        entry = pCache->sc_lhPartial.lh_plhNext;
        if (listIsEmpty(&pCache->sc_lhPartial))
        {
            entry = pCache->sc_lhFree.lh_plhNext;
            if (listIsEmpty(&pCache->sc_lhFree))
            {
                if (GET_FLAG(flag, MAF_NOWAIT))
                    SET_FLAG(jpflag, PAF_NOWAIT);

                u32Ret = _growSlabCache(pijs, pCache, jpflag);
                if (u32Ret != OLERR_NO_ERROR)
                    break;

                entry = pCache->sc_lhFree.lh_plhNext;
            }
            listDel(entry);
            listAdd(&pCache->sc_lhPartial, entry);
        }

        slabp = listEntry(entry, slab_t, s_lhList);
        *ppObj = _allocOneObjFromTail(pCache, slabp);
    }

    releaseSyncMutex(&pCache->sc_smCache);

    _unlockSlabCache(pijs, pCache);

    return u32Ret;
}

/*
 * Release an obj back to its cache. If the obj has a constructed
 * state, it should be in this state _before_ it is released.
 * - caller is responsible for the synchronization
 */

#if DEBUG_JIUKUN
    #define CHECK_PAGE(page)           \
    do                                 \
    {                                  \
        if (! isJpSlab(page))          \
        {                              \
            logInfoMsg("check page, bad ptr %lxh.", (ulong)objp);   \
            abort();                         \
        }                                    \
    } while (0)
#else
    #define CHECK_PAGE(pg) do { } while (0)
#endif

#if DEBUG_JIUKUN
static olint_t _extraFreeChecks (slab_cache_t * pCache,
    slab_t * slabp, u8 * objp)
{
    olint_t i;
    u32 objnr = (objp - (u8 *)slabp->s_pMem) / pCache->sc_u32ObjSize;

    if (objnr >= pCache->sc_u32Num)
        abort();

    if (objp != (u8 *)slabp->s_pMem + objnr * pCache->sc_u32ObjSize)
        abort();

    /* Check slab's freelist to see if this obj is there. */
    for (i = slabp->s_sbFree; i != BUFCTL_END; i = slab_bufctl(slabp)[i])
    {
        if (i == objnr)
        {
            logInfoMsg("Double free is detected");
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

    CHECK_PAGE(addrToJiukunPage(objp));

    slabp = GET_PAGE_SLAB(addrToJiukunPage(objp));

#if DEBUG_JIUKUN
    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
    {
        objp -= SLAB_ALIGN_SIZE;

        /* check old one. */
        if ((*((ulong *)(objp)) != RED_MAGIC2) ||
            (*((ulong *)(objp + pCache->sc_u32ObjSize - SLAB_ALIGN_SIZE)) !=
             RED_MAGIC2))
        {
            logInfoMsg("Free an unallocated memory");
            abort();
        }

        /* Set alloc red-zone. */
        *((ulong *)(objp)) = RED_MAGIC1;
        *((ulong *)(objp + pCache->sc_u32ObjSize -
                             SLAB_ALIGN_SIZE)) = RED_MAGIC1;
    }

    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_POISON))
        _poisonObj(pCache, objp);

    if (_extraFreeChecks(pCache, slabp, objp))
        return;
#endif
    {
        u32 objnr = (objp - (u8 *)slabp->s_pMem) / pCache->sc_u32ObjSize;

        slab_bufctl(slabp)[objnr] = slabp->s_sbFree;
        slabp->s_sbFree = objnr;
    }
    STATS_DEC_ACTIVE(pCache);
    /* fixup slab chains */
    {
        u32 inuse = slabp->s_u32InUse;
        slabp->s_u32InUse --;
        if (slabp->s_u32InUse == 0)
        {
            /* Was partial or full, now empty. */
            listDel(&(slabp->s_lhList));
            listAdd(&(pCache->sc_lhFree), &(slabp->s_lhList));
        }
        else if (inuse == pCache->sc_u32Num)
        {
            /* Was full. */
            listDel(&(slabp->s_lhList));
            listAdd(&(pCache->sc_lhPartial), &(slabp->s_lhList));
        }
    }
}

static inline void _freeObj(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, void ** pptr)
{
    assert(! GET_FLAG(pCache->sc_fCache, SC_FLAG_DESTROY));

    _lockSlabCache(pijs, pCache);

    acquireSyncMutex(&pCache->sc_smCache);
    _freeOneObj(pijs, pCache, *pptr);
    releaseSyncMutex(&pCache->sc_smCache);

    _unlockSlabCache(pijs, pCache);

    *pptr = NULL;
}

/* Destroy all the objs in a slab, and release the mem back to the buddy.
 * Before calling the slab must have been unlinked from the cache.
 * The cache-lock is not held/needed.
 */
static void _destroySlab(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache, slab_t * slabp)
{
    slab_t * pSlab = slabp;

#if DEBUG_JIUKUN
    if (GET_FLAG(pCache->sc_fCache, SC_FLAG_POISON) ||
        GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
    {
        olint_t i;
        for (i = 0; i < pCache->sc_u32Num; i++)
        {
            u8 * objp = (u8 *)slabp->s_pMem + pCache->sc_u32ObjSize * i;

            if (GET_FLAG(pCache->sc_fCache, SC_FLAG_RED_ZONE))
            {
                assert(*((ulong*)(objp)) == RED_MAGIC1);
                assert(*((ulong*)(objp + pCache->sc_u32ObjSize
                          - SLAB_ALIGN_SIZE)) == RED_MAGIC1);
            }

            if (GET_FLAG(pCache->sc_fCache, SC_FLAG_POISON) &&
                _checkPoisonObj(pCache, objp))
                abort();
        }
    }
#endif

    _freePages(pijs, pCache, (u8 *)slabp->s_pMem);
    if (OFF_SLAB(pCache))
        _freeObj(pijs, pCache->sc_pscSlab, (void **)&pSlab);
}

static u32 _destroySlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * psc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    slab_t * slabp;
    struct list_head * p;

    logInfoMsg("destroy jiukun cache %s", psc->sc_strName);

    acquireSyncMutex(&(psc->sc_smCache));

    while (u32Ret == OLERR_NO_ERROR)
    {
        p = psc->sc_lhFree.lh_plhPrev;
        if (p == &(psc->sc_lhFree))
            break;

        slabp = listEntry(psc->sc_lhFree.lh_plhPrev, slab_t, s_lhList);
#if DEBUG_JIUKUN
        assert(slabp->s_u32InUse == 0);
#endif
        listDel(&(slabp->s_lhList));

        _destroySlab(pijs, psc, slabp);
    }

    releaseSyncMutex(&(psc->sc_smCache));

    return u32Ret;
}

static slab_cache_t * _findGeneralSlabCache(
    internal_jiukun_slab_t * pijs, olsize_t size, olint_t gfpflags)
{
    general_cache_t * pgc = &(pijs->ijs_gcGeneral[0]);

    /* This function could be moved to the header file, and
     * made inline so consumers can quickly determine what
     * cache pointer they require.
     */
    for ( ; pgc->gc_sSize != OLSIZE_MAX; pgc ++)
    {
        if (size > pgc->gc_sSize)
            continue;
        break;
    }
    return pgc->gc_pscCache;
}

static u32 _createSlabCache(
    internal_jiukun_slab_t * pijs,
    jiukun_cache_t ** ppCache, jiukun_cache_param_t * pjcp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t left_over, slab_size;
    slab_cache_t * pCache = NULL;
    u32 realobjsize = pjcp->jcp_sObj;
#ifdef DEBUG_JIUKUN
    struct list_head * plh;
#endif

    logInfoMsg(
        "create jiukun cache, %s, size: %u, flag: 0x%llX",
        pjcp->jcp_pstrName, pjcp->jcp_sObj, pjcp->jcp_fCache);

#if DEBUG_JIUKUN
    /* do not red zone large object, causes severe fragmentation.
     */
    if (pjcp->jcp_sObj < (BUDDY_PAGE_SIZE >> 3))
        SET_FLAG(pjcp->jcp_fCache, SC_FLAG_RED_ZONE);

    SET_FLAG(pjcp->jcp_fCache, SC_FLAG_POISON);
#endif

    /* Check that size is in terms of words. This is needed to avoid
     * unaligned accesses for some archs when redzoning is used, and makes
     * sure any on-slab bufctl's are also correctly aligned.
     */
    pjcp->jcp_sObj = ALIGN(pjcp->jcp_sObj, SLAB_ALIGN_SIZE);

    /* Get cache's description obj. */
    u32Ret = _allocObj(
        pijs, &(pijs->ijs_scCacheCache), (void **)&pCache, 0);
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(pCache, 0, sizeof(slab_cache_t));

#if DEBUG_JIUKUN
        if (GET_FLAG(pjcp->jcp_fCache, SC_FLAG_RED_ZONE))
        {
            pjcp->jcp_sObj += 2 * SLAB_ALIGN_SIZE;   /* words for redzone */
        }
#endif
        /* Determine if the slab management is 'on' or 'off' slab. */
        if (pjcp->jcp_sObj >= (BUDDY_PAGE_SIZE >> 3))
            /* Size is large, assume best to place the slab management obj
             * off-slab (should allow better packing of objs).
             */
            SET_FLAG(pjcp->jcp_fCache, SC_FLAG_OFF_SLAB);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Cal size (in pages) of slabs, and the num of objs per slab.
         */
        do
        {
            u32 break_flag = 0;

            _slabCacheEstimate(
                pCache->sc_u32Order, pjcp->jcp_sObj,
                pjcp->jcp_fCache, &left_over, &pCache->sc_u32Num);
            if (break_flag)
                break;
            if (pCache->sc_u32Order >= MAX_JP_ORDER)
                break;
            if (pCache->sc_u32Num == 0)
            {
                pCache->sc_u32Order++;
                continue;
            }
            if (GET_FLAG(pjcp->jcp_fCache, SC_FLAG_OFF_SLAB) &&
                (pCache->sc_u32Num > pijs->ijs_u32OffSlabLimit))
            {
                /* this num of objs will cause problems. */
                pCache->sc_u32Order--;
                break_flag++;
                continue;
            }
            if ((left_over * 8) <= (BUDDY_PAGE_SIZE << pCache->sc_u32Order))
                break;  /* Acceptable internal fragmentation. */
        } while (1);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg(
            "create slab cache, %s, size: %u, order: %u, num: %u",
            pjcp->jcp_pstrName, pjcp->jcp_sObj, pCache->sc_u32Order,
            pCache->sc_u32Num);

        if (pCache->sc_u32Num == 0)
        {
            u32Ret = OLERR_FAIL_CREATE_JIUKUN_CACHE;
            logErrMsg(u32Ret, "create slab cache, couldn't create cache");
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        slab_size = ALIGN(
            pCache->sc_u32Num * sizeof(slab_bufctl_t) + sizeof(slab_t),
            SLAB_ALIGN_SIZE);

        /* If the slab has been placed off-slab, and we have enough space then
         * move it on-slab.
         */
        if (GET_FLAG(pjcp->jcp_fCache, SC_FLAG_OFF_SLAB) &&
            left_over >= slab_size)
        {
            CLEAR_FLAG(pjcp->jcp_fCache, SC_FLAG_OFF_SLAB);
            left_over -= slab_size;
        }

        pCache->sc_fCache = pjcp->jcp_fCache;
        pCache->sc_fPage = 0;

        u32Ret = initSyncMutex(&(pCache->sc_smCache));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pCache->sc_u32ObjSize = pjcp->jcp_sObj;
        pCache->sc_u32RealObjSize = realobjsize;

        listInit(&(pCache->sc_lhFull));
        listInit(&(pCache->sc_lhPartial));
        listInit(&(pCache->sc_lhFree));

        if (GET_FLAG(pjcp->jcp_fCache, SC_FLAG_OFF_SLAB))
            pCache->sc_pscSlab = _findGeneralSlabCache(pijs, slab_size, 0);
        ol_strncpy(pCache->sc_strName, pjcp->jcp_pstrName, CACHE_NAME_LEN - 1);

        acquireSyncMutex(&(pijs->ijs_smLock));
#ifdef DEBUG_JIUKUN
        listForEach(&(pijs->ijs_scCacheCache.sc_lhNext), plh)
        {
            slab_cache_t * pc = listEntry(plh, slab_cache_t, sc_lhNext);
            assert(strcmp(pc->sc_strName, pjcp->jcp_pstrName) != 0);
        }
#endif
        listAdd(&(pijs->ijs_scCacheCache.sc_lhNext), &(pCache->sc_lhNext));
        releaseSyncMutex(&(pijs->ijs_smLock));
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppCache = pCache;
    else if (pCache != NULL)
    {
        _freeObj(pijs, &(pijs->ijs_scCacheCache), (void **)&pCache);
    }

    return u32Ret;
}

static u32 _initSlabCache(internal_jiukun_slab_t * pijs)
{
    u32 u32Ret = OLERR_NO_ERROR;
    slab_cache_t * pkc = &(pijs->ijs_scCacheCache);
    olsize_t left_over;
    general_cache_t *sizes;
    olchar_t name[20];
    jiukun_cache_param_t jcp;
    u16 u16NumOfSize = 0;

    listInit(&(pkc->sc_lhNext));

    listInit(&(pkc->sc_lhFull));
    listInit(&(pkc->sc_lhPartial));
    listInit(&(pkc->sc_lhFree));

    pkc->sc_u32ObjSize = sizeof(slab_cache_t);
    SET_FLAG(pkc->sc_fCache, JC_FLAG_NOREAP);
    ol_strcpy(pkc->sc_strName, "cache_cache");

    _slabCacheEstimate(
        0, pkc->sc_u32ObjSize, 0, &left_over, &(pkc->sc_u32Num));

    sizes = &(pijs->ijs_gcGeneral[0]);

    while ((ls_sCacheSize[u16NumOfSize] != OLSIZE_MAX) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        sizes->gc_sSize = ls_sCacheSize[u16NumOfSize];
        ol_snprintf(name, sizeof(name), "size-%d", sizes->gc_sSize);

        memset(&jcp, 0, sizeof(jiukun_cache_param_t));

        jcp.jcp_pstrName = name;
        jcp.jcp_sObj = sizes->gc_sSize;

        u32Ret = _createSlabCache(
            pijs, (jiukun_cache_t **)&(sizes->gc_pscCache), &jcp);

        /* Inc off-slab bufctl limit until the ceiling is hit. */
        if (u32Ret == OLERR_NO_ERROR)
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

    if (u32Ret == OLERR_NO_ERROR)
    {
        assert(u16NumOfSize < MAX_NUM_OF_GENERAL_CACHE);

        sizes->gc_sSize = OLSIZE_MAX;

        u32Ret = initSyncMutex(&(pkc->sc_smCache));
    }

    return u32Ret;
}

/** Shrink slab cache
 * 
 */
static olint_t _shrinkSlabCache(
    internal_jiukun_slab_t * pijs, slab_cache_t * pCache)
{
    slab_t * slabp;
    olint_t ret = 0;
    struct list_head * p;

    while (1)
    {
        p = pCache->sc_lhFree.lh_plhPrev;
        if (p == &(pCache->sc_lhFree))
            break;

        slabp = listEntry(pCache->sc_lhFree.lh_plhPrev, slab_t, s_lhList);
#if DEBUG_JIUKUN
        assert(slabp->s_u32InUse == 0);
#endif
        listDel(&(slabp->s_lhList));

        _destroySlab(pijs, pCache, slabp);
        ret++;
    }

    return ret;
}

/* --- public routine section ---------------------------------------------- */
u32 initJiukunSlab(slab_param_t * psp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    assert(psp != NULL);
    assert(! pijs->ijs_bInitialized);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&(pijs->ijs_smLock));

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _initSlabCache(pijs);

    if (u32Ret == OLERR_NO_ERROR)
        pijs->ijs_bInitialized = TRUE;
    else if (pijs != NULL)
        finiJiukunSlab();

    return u32Ret;
}

u32 finiJiukunSlab(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    general_cache_t * pgc;
    slab_cache_t * psc;

    logInfoMsg("fini jiukun slab");

    pgc = pijs->ijs_gcGeneral;

    while ((pgc->gc_sSize != OLSIZE_MAX) && (pgc->gc_pscCache != NULL))
        pgc ++;

    for ( ; pgc != pijs->ijs_gcGeneral; )
    {
        pgc --;
        destroyJiukunCache((void **)&(pgc->gc_pscCache));
    }

    psc = &(pijs->ijs_scCacheCache);
    _destroySlabCache(pijs, psc);

    finiSyncMutex(&(pijs->ijs_smLock));

    pijs->ijs_bInitialized = FALSE;

    return u32Ret;
}

/** Create a jiukun cache.
 *  
 *  @param ppCache [out] a ptr to the cache on success, NULL on failure.
 *
 *  @return the error code
 *
 */
u32 createJiukunCache(jiukun_cache_t ** ppCache, jiukun_cache_param_t * pjcp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    /*
     * Sanity checks
     */
    assert(pijs->ijs_bInitialized);
    assert(pjcp != NULL);
    assert((pjcp->jcp_pstrName != NULL) &&
           (pjcp->jcp_sObj >= SLAB_ALIGN_SIZE) &&
           (pjcp->jcp_sObj <= (1 << MAX_JP_ORDER) * BUDDY_PAGE_SIZE));

    u32Ret = _createSlabCache(pijs, ppCache, pjcp);

    return u32Ret;
}

/** Delete a cache. Remove a slab_cache_t object from the slab cache.
 *
 *  @param ppCache [in/out] the cache to destroy
 * 
 *  @return the error coce
 *
 */
u32 destroyJiukunCache(jiukun_cache_t ** ppCache)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * psc;

    assert((ppCache != NULL) && (*ppCache != NULL));

    psc = (slab_cache_t *) *ppCache;
    *ppCache = NULL;

    /* Find the cache in the chain of caches. */
    acquireSyncMutex(&(pijs->ijs_smLock));
    /* the chain is never empty, cache_cache is never destroyed */
    listDel(&(psc->sc_lhNext));
    SET_FLAG(psc->sc_fCache, SC_FLAG_DESTROY);
    releaseSyncMutex(&(pijs->ijs_smLock));

    _destroySlabCache(pijs, psc);
    _freeObj(pijs, &pijs->ijs_scCacheCache, (void **)&psc);

    return u32Ret;
}

/** Reclaim memory from caches.
 * 
 *  @param bNoWait [in] if it should wait for lock
 *
 */
u32 reapJiukunSlab(boolean_t bNoWait)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * searchp;
    olint_t ret = 0;
    struct list_head * plh;

    assert(pijs->ijs_bInitialized);

    logInfoMsg("reap slab, nowait %d", bNoWait);

    if (bNoWait)
    {
        u32Ret = tryAcquireSyncMutex(&(pijs->ijs_smLock));
        if (u32Ret != OLERR_NO_ERROR)
            return u32Ret;
    }
    else
    {
        acquireSyncMutex(&(pijs->ijs_smLock));
    }
    logInfoMsg("reap slab, start");

    listForEach(&(pijs->ijs_scCacheCache.sc_lhNext), plh)
    {
        searchp = listEntry(plh, slab_cache_t, sc_lhNext);

        if (GET_FLAG(searchp->sc_fCache, JC_FLAG_NOREAP))
        {
            logInfoMsg("reap cache, %s no reap", searchp->sc_strName);
            continue;
        }

        if (GET_FLAG(searchp->sc_fCache, SC_FLAG_GROWN))
        {
            logInfoMsg("reap cache, %s is growing", searchp->sc_strName);
            continue;
        }

        if (GET_FLAG(searchp->sc_fCache, SC_FLAG_LOCKED))
        {
            logInfoMsg("reap cache, %s is locked", searchp->sc_strName);
            continue;
        }

        acquireSyncMutex(&(searchp->sc_smCache));
#if DEBUG_JIUKUN
        logInfoMsg(
            "cache %p, name %s, flag 0x%llx", searchp, searchp->sc_strName,
            searchp->sc_fCache);
        _dumpSlabCache(searchp);
#endif

        ret += _shrinkSlabCache(pijs, searchp);

        releaseSyncMutex(&(searchp->sc_smCache));
    }

    if ((u32Ret == OLERR_NO_ERROR) && (ret == 0))
    {
        /*no cache is reaped*/
        u32Ret = OLERR_FAIL_REAP_JIUKUN;
    }

    return u32Ret;
}

void freeObject(jiukun_cache_t * pCache, void ** pptr)
{
    internal_jiukun_slab_t * pijs = &ls_iasSlab;

    assert(pijs->ijs_bInitialized);
    assert((pCache != NULL) && (pptr != NULL) && (*pptr != NULL));

    _freeObj(pijs, (slab_cache_t *)pCache, pptr);
    *pptr = NULL;
}

/** Allocate an object from this cache.
 *
 *  @param pCache [in] the cache to allocate from.
 *  @param pptr [out] the pointer to object
 *  @param flag [in] flags
 */
u32 allocObject(jiukun_cache_t * pCache, void ** pptr, olflag_t flag)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    slab_cache_t * cache = (slab_cache_t *)pCache;

    assert(pijs->ijs_bInitialized);
    assert((pCache != NULL) && (pptr != NULL));

    u32Ret = _allocObj(pijs, cache, pptr, flag);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (GET_FLAG(cache->sc_fCache, JC_FLAG_ZERO) ||
            GET_FLAG(flag, MAF_ZERO))
            memset(*pptr, 0, cache->sc_u32RealObjSize);
    }

    return u32Ret;
}

/** Allocate memory
 * 
 *  @param size [in] bytes of memory are required
 *  @param flags [in] the flags for memory allocation
 *
 */
u32 allocMemory(void ** pptr, olsize_t size, olflag_t flag)
{
    u32 u32Ret = OLERR_UNSUPPORTED_MEMORY_SIZE;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    general_cache_t * pgc = pijs->ijs_gcGeneral;

    assert(pijs->ijs_bInitialized);

    *pptr = NULL;

    for ( ; pgc->gc_sSize != OLSIZE_MAX; pgc++) 
    {
        if (size > pgc->gc_sSize)
            continue;

        u32Ret = _allocObj(pijs, pgc->gc_pscCache, pptr, flag);
        break;
    }

#if defined(DEBUG_JIUKUN_VERBOSE)
    logDebugMsg("alloc sized obj %p, size: %u, flags: 0x%llX",
                *pptr, size, flag);
#endif

    if (u32Ret == OLERR_NO_ERROR && GET_FLAG(flag, MAF_ZERO))
        memset(*pptr, 0, size);

    return u32Ret;
}

/** Free previously allocated memory
 *
 *  @param pptr [out] pointer to memory
 *
 */
void freeMemory(void ** pptr)
{
    slab_cache_t * pCache;
    internal_jiukun_slab_t * pijs = &ls_iasSlab;
    void * objp = * pptr;

    assert(pijs->ijs_bInitialized);
    assert((pptr != NULL) && (*pptr != NULL));

#if defined(DEBUG_JIUKUN_VERBOSE)
    logInfoMsg("free sized obj, %p", *pptr);
#endif

    pCache = GET_PAGE_CACHE(addrToJiukunPage(objp));

    _freeObj(pijs, pCache, pptr);
}

u32 copyMemory(void ** pptr, u8 * pu8Buffer, olsize_t size)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert((pptr != NULL) && (pu8Buffer != NULL) && (size > 0));

    u32Ret = allocMemory(pptr, size, 0);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memcpy(*pptr, pu8Buffer, size);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


