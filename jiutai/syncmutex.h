/**
 *  @file syncmutex.h
 *
 *  @brief synchronization mutex object header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUTAI_SYNCMUTEX_H
#define JIUTAI_SYNCMUTEX_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
#if defined(LINUX)
    pthread_mutex_t sm_ptmMutex;
#elif defined(WINDOWS)
    HANDLE sm_hMutex;
#endif
} sync_mutex_t;

/* --- functional routines ------------------------------------------------- */

u32 initSyncMutex(sync_mutex_t * pMutex);

u32 finiSyncMutex(sync_mutex_t * pMutex);

u32 acquireSyncMutex(sync_mutex_t * pMutex);

u32 tryAcquireSyncMutex(sync_mutex_t * pMutex);

u32 acquireSyncMutexWithTimeout(sync_mutex_t * pMutex, u32 u32Timeout);

u32 releaseSyncMutex(sync_mutex_t * pMutex);

#endif /*JIUTAI_SYNCMUTEX_H*/

/*---------------------------------------------------------------------------*/

