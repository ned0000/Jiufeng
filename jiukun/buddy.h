/**
 *  @file buddy.h
 *
 *  @brief Buddy header file, provide some functional routine for buddy page allocator.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUKUN_BUDDY_H
#define JIUKUN_BUDDY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_listhead.h"
#include "jf_err.h"
#include "jf_jiukun.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the buddy page size.
 */
#define BUDDY_PAGE_SIZE      (JF_JIUKUN_PAGE_SIZE)

/** Convert the size to numbers of page.
 */
#define sizeToPages(size)    ((size + BUDDY_PAGE_SIZE - 1) / BUDDY_PAGE_SIZE)

/** Define the jiukun page flags.
 */
typedef enum jiukun_page_flag
{
    /**Page is allocated.*/
    JP_FLAG_ALLOCATED = 0,
    /**Page is used by slab.*/
    JP_FLAG_SLAB,
} jiukun_page_flag_t;

/** Define the jiukun page data type.
 */
typedef struct jiukun_page
{
    /**Flag of the page.*/
    jf_flag_t jp_jfPage;
    /**Linked page list.*/
    jf_listhead_t jp_jlLru;
    /**Private data, the content is determined by flags.*/
    ulong jp_ulPrivate;
    u32 jp_u32Reserved[2];
} jiukun_page_t;

/** Set the page allocated.
 */
#define setJpAllocated(page)        (JF_FLAG_SET(page->jp_jfPage, JP_FLAG_ALLOCATED))

/** Clear the page allocated.
 */
#define clearJpAllocated(page)      (JF_FLAG_CLEAR(page->jp_jfPage, JP_FLAG_ALLOCATED))

/** Test if the page is allocated.
 */
#define isJpAllocated(page)         (JF_FLAG_GET(page->jp_jfPage, JP_FLAG_ALLOCATED))

/** Set the page used by slab.
 */
#define setJpSlab(page)             (JF_FLAG_SET(page->jp_jfPage, JP_FLAG_SLAB))

/** Clear the page used by slab.
 */
#define clearJpSlab(page)           (JF_FLAG_CLEAR(page->jp_jfPage, JP_FLAG_SLAB))

/** Test if the page is used by slab.
 */
#define isJpSlab(page)              (JF_FLAG_GET(page->jp_jfPage, JP_FLAG_SLAB))

/** Set the order to the page, the order is at bit 48 ~ 55.
 */
#define setJpOrder(page, order)     (JF_FLAG_SET_VALUE(page->jp_jfPage, 55, 48, order))

/** Get the order from the page.
 */
#define getJpOrder(page)            (JF_FLAG_GET_VALUE(page->jp_jfPage, 55, 48))

/** Set the zone id to the page , the zone id is at bit 56 ~ 63.
 */
#define setJpZoneId(page, zoneid)   (JF_FLAG_SET_VALUE(page->jp_jfPage, 63, 56, zoneid))

/** Get the zone id from the page.
 */
#define getJpZoneId(page)           (JF_FLAG_GET_VALUE(page->jp_jfPage, 63, 56))

/** Get the page index from the page array.
 */
#define pageToIndex(page, base)     ((olint_t)(page - base))

/** Define the parameter for initializing the buddy.
 */
typedef struct
{
    /**Maximum order.*/
    u8 bp_u8MaxOrder;
    /**Donot grow the memery if it's TRUE.*/
    boolean_t bp_bNoGrow;
    u8 bp_u8Reserved[6];
} buddy_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initalize the buddy page allocator.
 *
 *  @param pbp [in] The parameter for initializing buddy.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 initJiukunBuddy(buddy_param_t * pbp);

/** Finalize the buddy page allocator.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 finiJiukunBuddy(void);

#if defined(DEBUG_JIUKUN)

/** Dump buddy page allocator.
 *
 *  @return Void.
 */
void dumpJiukunBuddy(void);

#endif

/** Allocate page from buddy page allocator.
 *
 *  @param ppPage [out] The page allocated.
 *  @param u32Order [in] The page order.
 *  @param flag [in] The flag for the allocation.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_JIUKUN_PAGE_ORDER Invalid page order.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of jiukun memory.
 */
u32 getJiukunPage(jiukun_page_t ** ppPage, u32 u32Order, jf_flag_t flag);

/** Free page to buddy page allocator.
 *
 *  @param ppPage [in/out] The page to be freed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
void putJiukunPage(jiukun_page_t ** ppPage);

/** Convert the jiukun page object to memery address.
 *
 *  @param pbp [in] The jiukun page object to be checked.
 *
 *  @return The memory address.
 */
void * jiukunPageToAddr(jiukun_page_t * pbp);

/** Convert the memery address to jiukun page object.
 *
 *  @param pAddr [in] The memory address.
 *
 *  @return The jiukun page object.
 */
jiukun_page_t * addrToJiukunPage(void * pAddr);

#endif /*JIUKUN_BUDDY_H*/

/*------------------------------------------------------------------------------------------------*/
