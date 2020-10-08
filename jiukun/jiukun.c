/**
 *  @file jiukun.c
 *
 *  @brief Implementation file for the jiukun library.
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
#include "jf_jiukun.h"

#include "buddy.h"
#include "slab.h"
#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The order primes array.
 */
static u32 ls_u32OrderPrimes[] = {
    1, 3, 7, 15, 31,
    63, 127, 255, 511, 1023,
    2047, 4095, 8191, 16383, 32767,
    65535, 131071, 262143, 524287, 1048575,
    2097151, 4194303, 4194303, 16777215, 33554431,
    67108863, 134217727, 268435455, 536870911, 1073741823,
    2147483647, 4294967295UL,
};

/** Define the internal jiukun data type.
 */
typedef struct
{
    /**Jiukun is initialized if it's TRUE.*/
    boolean_t ia_bInitialized;
    u8 ia_u8Reserved[7];

    u32 ia_u32Reserved[4];
} internal_jiukun_t;

/** Declare the internal jiukun object.
 */
static internal_jiukun_t ls_iaJiukun;

/** Minimal pool size.
 */
#define MIN_JIUKUN_POOL_SIZE                      (1UL << 20)

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_jiukun_init(jf_jiukun_init_param_t * pjjip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_t * pia = &ls_iaJiukun;
    u32 u32NumOfPages = 0;
    buddy_param_t bp;
    slab_param_t sp;

    if (pia->ia_bInitialized)
        return u32Ret;

    assert((pjjip->jjip_sPool >= MIN_JIUKUN_POOL_SIZE) &&
           (pjjip->jjip_sPool <= JF_JIUKUN_MAX_POOL_SIZE));

    ol_bzero(pia, sizeof(internal_jiukun_t));
    ol_bzero(&bp, sizeof(buddy_param_t));
    bp.bp_bNoGrow = pjjip->jjip_bNoGrow;
    u32NumOfPages = sizeToPages(pjjip->jjip_sPool);

    while (u32NumOfPages > ls_u32OrderPrimes[bp.bp_u8MaxOrder])
        bp.bp_u8MaxOrder ++;

    JF_LOGGER_INFO(
        "size: %u, num of page: %u, max order: %u", pjjip->jjip_sPool, u32NumOfPages,
        bp.bp_u8MaxOrder);

    /*Initialize jiukun page allocator.*/
    u32Ret = initJiukunBuddy(&bp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&sp, sizeof(slab_param_t));

        /*Initialize the jiukun slab.*/
        u32Ret = initJiukunSlab(&sp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pia->ia_bInitialized = TRUE;
    else
        jf_jiukun_fini();

    return u32Ret;
}

u32 jf_jiukun_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jiukun_t * pia = &ls_iaJiukun;

#if defined(DEBUG_JIUKUN)
    JF_LOGGER_INFO("fini");
#endif

    /*Finalize the jiukun slab.*/
    finiJiukunSlab();

    /*Finalize jiukun page allocator.*/
    finiJiukunBuddy();

    pia->ia_bInitialized = FALSE;

    return u32Ret;
}

u32 reapJiukun(boolean_t bNoWait)
{
    u32 u32Ret;
    olint_t ret;

    assert(ls_iaJiukun.ia_bInitialized);

    /*Reap jiukun slabs.*/
    ret = reapJiukunSlab(bNoWait);

#if defined(DEBUG_JIUKUN)
    JF_LOGGER_INFO("%u slabs are reaped", ret);
    jf_jiukun_dump();
#endif

    if (ret == 0)
    {
        /*no cache is reaped*/
        u32Ret = JF_ERR_FAIL_REAP_JIUKUN;
    }

    return u32Ret;
}

#if defined(DEBUG_JIUKUN)
void jf_jiukun_dump(void)
{
    internal_jiukun_t * pia = &ls_iaJiukun;

    assert(pia->ia_bInitialized);

    dumpJiukunBuddy();
}
#endif

/*------------------------------------------------------------------------------------------------*/
