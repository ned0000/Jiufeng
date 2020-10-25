/**
 *  @file slab.h
 *
 *  @brief Header file for slab of memory allocator.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The slab is a memory allocator for small memory.
 *  -# 2 types of cache are created, one is general cache and another is created by user.
 *  -# The general cache is for small memory like 8 bytes, 16 bytes, 32 bytes, 64 bytes ..., etc.
 *  -# The user's cache is created with fixed size.
 *  -# Cache get memory in pages from page allocator.
 *  -# A cache consists of slabs, a slab containing one or more object. Object is allocated to user.
 */

#ifndef JIUKUN_SLAB_H
#define JIUKUN_SLAB_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"

#include "buddy.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameters for initializing the jiukun slab.
 */
typedef struct
{
    u8 sp_u8Reserved[16];
} slab_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the jiukun slab.
 *
 *  @param pbp [in] Parameters for jiukun slab.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 initJiukunSlab(slab_param_t * pbp);

/** Finalize the jiukun slab.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 finiJiukunSlab(void);

/** Reclaim memory from caches.
 * 
 *  @param bNoWait [in] It should wait for lock if it's TRUE.
 *
 *  @return Number of slab reapped.
 */
olint_t reapJiukunSlab(boolean_t bNoWait);

#endif /*JIUKUN_SLAB_H*/

/*------------------------------------------------------------------------------------------------*/
