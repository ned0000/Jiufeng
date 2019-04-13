/**
 *  @file slab.h
 *
 *  @brief Slab header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUKUN_SLAB_H
#define JIUKUN_SLAB_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "buddy.h"
#include "jf_jiukun.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
    u8 sp_u8Reserved[16];
} slab_param_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 initJiukunSlab(slab_param_t * pbp);

u32 finiJiukunSlab(void);

/** Reclaim memory from caches.
 * 
 *  @param bNoWait [in] if it should wait for lock
 *
 *  @return the error code
 */
u32 reapJiukunSlab(boolean_t bNoWait);

#endif /*JIUKUN_SLAB_H*/

/*------------------------------------------------------------------------------------------------*/


