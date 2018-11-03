/**
 *  @file sharedmemory.h
 *
 *  @brief shared memory header file
 *	provide some functional routine for shared memory
 *
 *  @author Min Zhang
 *
 *  @note
 *  - link with xmalloc common object
 *  - link with oluuid.lib on Windows platform
 *  
 */

#ifndef JIUTAI_SHAREDMEMORY_H
#define JIUTAI_SHAREDMEMORY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
/*the shared memory identifier is a string with NULL-terminated*/
typedef olchar_t  shm_id_t;

/* --- functional routines ------------------------------------------------- */
u32 createSharedMemory(shm_id_t ** ppShmId, u32 u32MemorySize);

u32 attachSharedMemory(shm_id_t * pShmId, void ** ppMapAddress);

u32 detachSharedMemory(void ** ppMapAddress);

u32 destroySharedMemory(shm_id_t ** ppShmId);

#endif /*JIUTAI_SHAREDMEMORY_H*/

/*---------------------------------------------------------------------------*/


