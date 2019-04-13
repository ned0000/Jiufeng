/**
 *  @file jf_mem.h
 *
 *  @brief Memory allocation header file. Provide some functional routine to
 *   allocate memeory
 *
 *  @author Min Zhang
 *  
 *  @note Routines declared in this file are included in jf_mem object
 *
 */

#ifndef JIUTAI_XMALLOC_H
#define JIUTAI_XMALLOC_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */
u32 jf_mem_alloc(void ** pptr, olsize_t size);

/*allocate the memory and clear to 0*/
u32 jf_mem_calloc(void ** pptr, olsize_t size);

u32 jf_mem_realloc(void ** pptr, olsize_t size);

u32 jf_mem_free(void ** pptr);

u32 jf_mem_duplicate(void ** pptr, const u8 * pu8Buffer, const olsize_t size);

#endif /*JIUTAI_XMALLOC_H*/

/*------------------------------------------------------------------------------------------------*/


