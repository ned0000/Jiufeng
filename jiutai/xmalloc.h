/**
 *  @file xmalloc.h
 *
 *  @brief memory allocation header file
 *  	 provide some functional routine to allocate memeory
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef JIUTAI_XMALLOC_H
#define JIUTAI_XMALLOC_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
u32 xmalloc(void ** pptr, olsize_t size);

/*allocate the memory and clear to 0*/
u32 xcalloc(void ** pptr, olsize_t size);

u32 xrealloc(void ** pptr, olsize_t size);

u32 xfree(void ** pptr);

u32 dupMemory(void ** pptr, const u8 * pu8Buffer, const olsize_t size);

#endif /*JIUTAI_XMALLOC_H*/

/*---------------------------------------------------------------------------*/


