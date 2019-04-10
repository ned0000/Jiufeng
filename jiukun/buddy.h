/**
 *  @file buddy.h
 *
 *  @brief Buddy header file, provide some functional routine for buddy
 *   allocator
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUKUN_BUDDY_H
#define JIUKUN_BUDDY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_listhead.h"
#include "jf_err.h"
#include "jf_jiukun.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

#define BUDDY_PAGE_SIZE      (JF_JIUKUN_PAGE_SIZE)

#define sizeToPages(size)    ((size + BUDDY_PAGE_SIZE - 1) / BUDDY_PAGE_SIZE)

/** Jiukun page flag
 */
typedef enum jiukun_page_flag
{
    JP_FLAG_ALLOCATED = 0,/**< page is allocated */
    JP_FLAG_SLAB,         /**< page is used by slab */
} jiukun_page_flag_t;

/** Jiukun page data structure
 */
typedef struct jiukun_page
{
    jf_flag_t jp_jfPage;
    jf_listhead_t jp_jlLru;
    /** private data, the content is determined by flags */
    ulong jp_ulPrivate;
    u32 jp_u32Reserved[2];
} jiukun_page_t;

#define setJpAllocated(page) (JF_FLAG_SET(page->jp_jfPage, JP_FLAG_ALLOCATED))
#define clearJpAllocated(page) (JF_FLAG_CLEAR(page->jp_jfPage, JP_FLAG_ALLOCATED))
#define isJpAllocated(page) (JF_FLAG_GET(page->jp_jfPage, JP_FLAG_ALLOCATED))

#define setJpSlab(page) (JF_FLAG_SET(page->jp_jfPage, JP_FLAG_SLAB))
#define clearJpSlab(page) (JF_FLAG_CLEAR(page->jp_jfPage, JP_FLAG_SLAB))
#define isJpSlab(page)  (JF_FLAG_GET(page->jp_jfPage, JP_FLAG_SLAB))

/** order is at bit 48 ~ 55
 */
#define setJpOrder(page, order) \
    (JF_FLAG_SET_VALUE(page->jp_jfPage, 55, 48, order))
#define getJpOrder(page)   (JF_FLAG_GET_VALUE(page->jp_jfPage, 55, 48))

/** zone id is at bit 56 ~ 63
 */
#define setJpZoneId(page, zoneid) \
    (JF_FLAG_SET_VALUE(page->jp_jfPage, 63, 56, zoneid))
#define getJpZoneId(page)   (JF_FLAG_GET_VALUE(page->jp_jfPage, 63, 56))

#define pageToIndex(page, base) ((u32)(page - base))

typedef struct
{
    u8 bp_u8MaxOrder;
    boolean_t bp_bNoGrow;
    u8 bp_u8Reserved[6];
} buddy_param_t;

/* --- functional routines ------------------------------------------------- */
u32 initJiukunBuddy(buddy_param_t * pbp);

u32 finiJiukunBuddy(void);

#if defined(DEBUG_JIUKUN)
void dumpJiukunBuddy(void);
#endif

u32 getJiukunPage(
    jiukun_page_t ** ppPage, u32 u32Order, jf_flag_t flag);

void putJiukunPage(jiukun_page_t ** ppPage);

void * jiukunPageToAddr(jiukun_page_t * pbp);

jiukun_page_t * addrToJiukunPage(void * pAddr);

#endif /*JIUKUN_BUDDY_H*/

/*---------------------------------------------------------------------------*/


