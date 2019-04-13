/**
 *  @file jf_sharedmemory.h
 *
 *  @brief Shared memory header file. Provide some functional routine for
 *   shared memory
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_sharedmemory library
 *  @note Link with xmalloc common object
 *  @note Link with oluuid.lib on Windows platform
 *  
 */

#ifndef JIUTAI_SHAREDMEMORY_H
#define JIUTAI_SHAREDMEMORY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/*the shared memory identifier is a string with NULL-terminated*/
typedef olchar_t  jf_sharedmemory_id_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_sharedmemory_create(jf_sharedmemory_id_t ** ppShmId, u32 u32MemorySize);

u32 jf_sharedmemory_attach(jf_sharedmemory_id_t * pShmId, void ** ppMapAddress);

u32 jf_sharedmemory_detach(void ** ppMapAddress);

u32 jf_sharedmemory_destroy(jf_sharedmemory_id_t ** ppShmId);

#endif /*JIUTAI_SHAREDMEMORY_H*/

/*------------------------------------------------------------------------------------------------*/


