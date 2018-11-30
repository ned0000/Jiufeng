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

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "buddy.h"
#include "jiukun.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u8 sp_u8Reserved[16];
} slab_param_t;

/* --- functional routines ------------------------------------------------- */
u32 initJiukunSlab(slab_param_t * pbp);

u32 finiJiukunSlab(void);

u32 reapJiukunSlab(boolean_t bNoWait);

#endif /*JIUKUN_SLAB_H*/

/*---------------------------------------------------------------------------*/


