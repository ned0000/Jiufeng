/**
 *  @file jiukun.h
 *
 *  @brief jiukun header file. Provide some functional routine for jiukun
 *   library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_JIUKUN_H
#define JIUFENG_JIUKUN_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "bases.h"
#include "bitarray.h"
#include "olflag.h"

#undef JIUKUNAPI
#undef JIUKUNCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_JIUKUN_DLL)
        #define JIUKUNAPI  __declspec(dllexport)
        #define JIUKUNCALL
    #else
        #define JIUKUNAPI
        #define JIUKUNCALL __cdecl
    #endif
#else
    #define JIUKUNAPI
    #define JIUKUNCALL
#endif

/* --- constant definitions ------------------------------------------------ */
#define JIUKUN_PAGE_SHIFT         (12)
#define JIUKUN_PAGE_SIZE          (1 << JIUKUN_PAGE_SHIFT)
#define JIUKUN_PAGE_MASK          (~(JIUKUN_PAGE_SIZE - 1))

/** maximum order for page allocation, maximum pages in one pool is
 *  (1 << MAX_JIUKUN_PAGE_ORDER)
 */
#define MAX_JIUKUN_PAGE_ORDER     (14)

/** the maximum memory size allocMemory() can allocate*/
#define MAX_JIUKUN_MEMORY_ORDER   (23)
#define MAX_JIUKUN_MEMORY_SIZE    (1 << MAX_JIUKUN_MEMORY_ORDER)

/** the maximum object size createJiukunCache() can specify*/
#define MAX_JIUKUN_OBJECT_ORDER   (20)
#define MAX_JIUKUN_OBJECT_SIZE    (1 << MAX_JIUKUN_OBJECT_ORDER)

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    /** the memory pool size in byte, when the pool is full, jiukun will
        create another pool with the size if the grow of jiukun is allowed.*/
#define MAX_JIUKUN_POOL_SIZE  ((1 << MAX_JIUKUN_PAGE_ORDER) * JIUKUN_PAGE_SIZE)
    olsize_t jp_sPool;
    /** no grow when the initial pool is full */
    boolean_t jp_bNoGrow;
    u8 jp_u8Reserved[3];
    u32 jp_u32Reserved[7];
} jiukun_param_t;

/** Jiukun cache data structure
 */
typedef void  jiukun_cache_t;

/** Jiukun cache flag, possible values for the field 'jcp_fCache' in
 *  jiukun_cache_param_t data structure.
 */
typedef enum jiukun_cache_flag
{
    JC_FLAG_DEBUG_FREE = 0, /**< Peform (expensive) checks on free, available 
                               when DEBUG_JIUKUN is true */
    JC_FLAG_NOREAP,         /**< never reap from the cache */
    JC_FLAG_NOGROW,         /**< don't grow a cache */
    JC_FLAG_RECLAIM_ACCOUNT,/**< track pages allocated to indicate what is
                               reclaimable later*/
    JC_FLAG_ZERO,           /**< zero the allocated object */
} jiukun_cache_flag_t;

typedef struct
{
    olchar_t * jcp_pstrName;
    u8 jcp_u8Reserved[4];
    olsize_t jcp_sObj;
    olsize_t jcp_sOffset;
    olflag_t jcp_fCache;
    u32 jcp_u32Reserved2[4];
} jiukun_cache_param_t;

/* --- functional routines ------------------------------------------------- */

JIUKUNAPI u32 JIUKUNCALL initJiukun(jiukun_param_t * pap);

JIUKUNAPI u32 JIUKUNCALL finiJiukun(void);

/** Jiukun page allocator
 */

/** Flags for allocating jiukun page memory used by allocJiukunPage. PAF means
 *  'page allocation flag'
 */
typedef enum jiukun_pa_flag
{
    PAF_NOWAIT = 0, /**< Donot wait, the page may be failed to be allocated*/
} jiukun_pa_flag_t;

/** Get memory from jiukun page allocator
 */
JIUKUNAPI u32 JIUKUNCALL allocJiukunPage(
    void ** pptr, u32 u32Order, olflag_t flag);

/** Free memory to jiukun page allocator
 */
JIUKUNAPI void JIUKUNCALL freeJiukunPage(void ** pptr);

/** jiukun cache
 */
JIUKUNAPI u32 JIUKUNCALL createJiukunCache(
    jiukun_cache_t ** ppCache, jiukun_cache_param_t * pjcp);

JIUKUNAPI u32 JIUKUNCALL destroyJiukunCache(jiukun_cache_t ** ppCache);

/** Flags for allocting object or memory from jiukun cache used by
 *  allocMemory(), allocObject(), MAF means 'memory allocation flag'
 */
typedef enum jiukun_ma_flag
{
    MAF_NOWAIT = 0, /**< donot wait, the memory may be failed to be allocated */
    MAF_ZERO, /* zero the allocated memory */
} jiukun_ma_flag_t;

JIUKUNAPI u32 JIUKUNCALL allocObject(
    jiukun_cache_t * pCache, void ** ppObj, olflag_t flag);

JIUKUNAPI void JIUKUNCALL freeObject(jiukun_cache_t * pCache, void ** ppObj);

JIUKUNAPI u32 JIUKUNCALL allocMemory(
    void ** pptr, olsize_t size, olflag_t flag);

JIUKUNAPI void JIUKUNCALL freeMemory(void ** pptr);

JIUKUNAPI u32 JIUKUNCALL copyMemory(
    void ** pptr, u8 * pu8Buffer, olsize_t size);

/*debug*/
#if defined(DEBUG_JIUKUN)
JIUKUNAPI void JIUKUNCALL dumpJiukun(void);
#endif

#endif /*JIUFENG_JIUKUN_H*/

/*---------------------------------------------------------------------------*/


