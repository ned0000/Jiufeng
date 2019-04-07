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
#include "jf_basic.h"
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

#define JF_JIUKUN_PAGE_SHIFT         (12)
#define JF_JIUKUN_PAGE_SIZE          (1 << JF_JIUKUN_PAGE_SHIFT)
#define JF_JIUKUN_PAGE_MASK          (~(JF_JIUKUN_PAGE_SIZE - 1))

/** maximum order for page allocation, maximum pages in one pool is
 *  (1 << JF_JIUKUN_MAX_PAGE_ORDER)
 */
#define JF_JIUKUN_MAX_PAGE_ORDER     (14)

/** the maximum memory size allocMemory() can allocate*/
#define JF_JIUKUN_MAX_MEMORY_ORDER   (23)
#define JF_JIUKUN_MAX_MEMORY_SIZE    (1 << JF_JIUKUN_MAX_MEMORY_ORDER)

/** the maximum object size createJiukunCache() can specify*/
#define JF_JIUKUN_MAX_OBJECT_ORDER   (20)
#define JF_JIUKUN_MAX_OBJECT_SIZE    (1 << JF_JIUKUN_MAX_OBJECT_ORDER)

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    /** the memory pool size in byte, when the pool is full, jiukun will
        create another pool with the size if the grow of jiukun is allowed.*/
#define JF_JIUKUN_MAX_POOL_SIZE  ((1 << JF_JIUKUN_MAX_PAGE_ORDER) * JF_JIUKUN_PAGE_SIZE)
    olsize_t jjip_sPool;
    /** no grow when the initial pool is full */
    boolean_t jjip_bNoGrow;
    u8 jjip_u8Reserved[3];
    u32 jjip_u32Reserved[7];
} jf_jiukun_init_param_t;

/** Jiukun cache data structure
 */
typedef void  jf_jiukun_cache_t;

/** Jiukun cache flag, possible values for the field 'jjccp_fCache' in
 *  jf_jiukun_cache_create_param_t data structure.
 */
typedef enum jf_jiukun_cache_create_flag
{
    JF_JIUKUN_CACHE_CREATE_FLAG_DEBUG_FREE = 0, /**< Peform (expensive) checks on
                                 free, available when DEBUG_JIUKUN is true */
    JF_JIUKUN_CACHE_CREATE_FLAG_NOREAP,  /**< never reap from the cache */
    JF_JIUKUN_CACHE_CREATE_FLAG_NOGROW,  /**< don't grow a cache */
    JF_JIUKUN_CACHE_CREATE_FLAG_RECLAIM_ACCOUNT,/**< track pages allocated to
                                 indicate what is reclaimable later*/
    JF_JIUKUN_CACHE_CREATE_FLAG_ZERO,    /**< zero the allocated object */
} jf_jiukun_cache_create_flag_t;

typedef struct
{
    olchar_t * jjccp_pstrName;
    u8 jjccp_u8Reserved[4];
    olsize_t jjccp_sObj;
    olsize_t jjccp_sOffset;
    olflag_t jjccp_fCache;
    u32 jjccp_u32Reserved2[4];
} jf_jiukun_cache_create_param_t;

/** Flags for allocating jiukun page memory used by jf_jiukun_allocPage.
 */
typedef enum jf_jiukun_page_alloc_flag
{
    JF_JIUKUN_PAGE_ALLOC_FLAG_NOWAIT = 0, /**< Donot wait, the page may be
                                             failed to be allocated*/
} jf_jiukun_page_alloc_flag_t;

/** Flags for allocting object or memory from jiukun cache used by
 *  jf_jiukun_allocMemory(), jf_jiukun_allocObject()
 */
typedef enum jf_jiukun_mem_alloc_flag
{
    JF_JIUKUN_MEM_ALLOC_FLAG_NOWAIT = 0, /**< donot wait, the memory may be
                                            failed to be allocated */
    JF_JIUKUN_MEM_ALLOC_FLAG_ZERO, /* zero the allocated memory */
} jf_jiukun_mem_alloc_flag_t;

/* --- functional routines ------------------------------------------------- */

JIUKUNAPI u32 JIUKUNCALL jf_jiukun_init(jf_jiukun_init_param_t * pjjip);

JIUKUNAPI u32 JIUKUNCALL jf_jiukun_fini(void);

/** Jiukun page allocator
 */

/** Get memory from jiukun page allocator
 */
JIUKUNAPI u32 JIUKUNCALL jf_jiukun_allocPage(
    void ** pptr, u32 u32Order, olflag_t flag);

/** Free memory to jiukun page allocator
 */
JIUKUNAPI void JIUKUNCALL jf_jiukun_freePage(void ** pptr);

/** jiukun cache
 */

/** Create a jiukun cache.
 *  
 *  @param ppCache [out] a pointer to the cache on success, NULL on failure.
 *  @param pjjccp [in] the parameters for creating a cache
 *
 *  @return the error code
 */
JIUKUNAPI u32 JIUKUNCALL jf_jiukun_createCache(
    jf_jiukun_cache_t ** ppCache, jf_jiukun_cache_create_param_t * pjjccp);

/** Delete a jiukun cache.
 *
 *  @param ppCache [in/out] the cache to destroy
 * 
 *  @return the error coce
 */
JIUKUNAPI u32 JIUKUNCALL jf_jiukun_destroyCache(jf_jiukun_cache_t ** ppCache);

/** Allocate an object from this cache.
 *
 *  @param pCache [in] the cache to allocate from.
 *  @param ppObj [out] the pointer to object
 *  @param flag [in] flags
 *
 *  @return the error code
 */
JIUKUNAPI u32 JIUKUNCALL jf_jiukun_allocObject(
    jf_jiukun_cache_t * pCache, void ** ppObj, olflag_t flag);

JIUKUNAPI void JIUKUNCALL jf_jiukun_freeObject(
    jf_jiukun_cache_t * pCache, void ** ppObj);

/** Allocate memory
 * 
 *  @param pptr [out] the pointer to the allocated memory
 *  @param size [in] bytes of memory are required
 *  @param flag [in] the flags for memory allocation
 *
 *  @return the error code
 */
JIUKUNAPI u32 JIUKUNCALL jf_jiukun_allocMemory(
    void ** pptr, olsize_t size, olflag_t flag);

/** Free previously allocated memory
 *
 *  @param pptr [out] pointer to memory
 */
JIUKUNAPI void JIUKUNCALL jf_jiukun_freeMemory(void ** pptr);

JIUKUNAPI u32 JIUKUNCALL jf_jiukun_copyMemory(
    void ** pptr, u8 * pu8Buffer, olsize_t size);

/*debug*/
#if defined(DEBUG_JIUKUN)
JIUKUNAPI void JIUKUNCALL jf_jiukun_dump(void);
#endif

#endif /*JIUFENG_JIUKUN_H*/

/*---------------------------------------------------------------------------*/


