/**
 *  @file jf_mem.h
 *
 *  @brief Memory allocation header file which provide some functional routine to allocate memeory.
 *
 *  @author Min Zhang
 *  
 *  @note
 *  -# Routines declared in this file are included in jf_mem object.
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

/** Allocate memory.
 *
 *  @param pptr [out] The pointer to the allocated memory.
 *  @param size [in] The size of the memory.
 *
 *  @return The error code.
 */
u32 jf_mem_alloc(void ** pptr, olsize_t size);

/** Allocate the memory and clear the memory to all 0.
 *
 *  @param pptr [out] The pointer to the allocated memory.
 *  @param size [in] The size of the memory.
 *
 *  @return The error code.
 */
u32 jf_mem_calloc(void ** pptr, olsize_t size);

/** Change the size of the memory block.
 *
 *  @note
 *  -# The contents will be unchanged in the range from the start of the region up to the minimum
 *   of the old and new sizes. If the new size is larger than the old size, the added memory will
 *   not be initialized.
 *
 *  @param pptr [in/out] The pointer to the allocated memory.
 *  @param size [in] The new size of the memory.
 *
 *  @return The error code.
 */
u32 jf_mem_realloc(void ** pptr, olsize_t size);

/** Free the memory.
 *
 *  @param pptr [in/out] The pointer to the allocated memory.
 *
 *  @return The error code.
 */
u32 jf_mem_free(void ** pptr);

/** Duplicate memory.
 *
 *  @param pptr [out] The pointer to the allocated memory.
 *  @param pu8Buffer [in] The data buffer.
 *  @param size [in] The size of the buffer.
 *
 *  @return The error code.
 */
u32 jf_mem_duplicate(void ** pptr, const u8 * pu8Buffer, const olsize_t size);

#endif /*JIUTAI_XMALLOC_H*/

/*------------------------------------------------------------------------------------------------*/


