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

/** Initialize the mutex
 *
 *  @param pMutex [in] the mutex to be initialized
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 initSyncMutex(sync_mutex_t * pMutex);

/** Finalize a mutex
 *
 *  @param pMutex [in] the mutex to be finalized
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 finiSyncMutex(sync_mutex_t * pMutex);

/** Acquire a mutex
 *
 *  @note If the mutex is already locked by another thread, this routine
 *   suspends the calling thread until the mutex is unlocked.
 *
 *  @param pMutex [in] the mutex to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncMutex(sync_mutex_t * pMutex);

/** Try to acquire a mutex
 *
 *  @note It does not block the calling thread if the mutex is already
 *   locked by another thread
 *
 *  @param pMutex [in] the mutex to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 tryAcquireSyncMutex(sync_mutex_t * pMutex);

/** Acquire a mutex with time out
 *
 *  @param pMutex [in] the mutex to be acquired
 *  @param u32Timeout [in] the maximum waiting period if the mutex has been
 *   acquired by another thread in milliseconds.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncMutexWithTimeout(sync_mutex_t * pMutex, u32 u32Timeout);

/** Release a mutex.
 *
 *  @param pMutex [in] the mutex to be released
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 releaseSyncMutex(sync_mutex_t * pMutex);

#endif /*JIUTAI_SYNCMUTEX_H*/

/*---------------------------------------------------------------------------*/

