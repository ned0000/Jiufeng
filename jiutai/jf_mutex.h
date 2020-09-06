/**
 *  @file jf_mutex.h
 *
 *  @brief Header file which defines synchronization mutex object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_mutex object.
 *
 */

#ifndef JIUTAI_MUTEX_H
#define JIUTAI_MUTEX_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the mutex data type.
 */
typedef struct
{
#if defined(LINUX)
    /**The mutex handle from pthread library.*/
    pthread_mutex_t jm_ptmMutex;
#elif defined(WINDOWS)
    /**The mutex handle.*/
    HANDLE jm_hMutex;
#endif
} jf_mutex_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the mutex.
 *
 *  @param pMutex [in] The mutex to be initialized.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_MUTEX Failed to create mutex.
 */
u32 jf_mutex_init(jf_mutex_t * pMutex);

/** Finalize a mutex.
 *
 *  @param pMutex [in] The mutex to be finalized.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_DESTROY_MUTEX Failed to destroy mutex.
 */
u32 jf_mutex_fini(jf_mutex_t * pMutex);

/** Acquire a mutex.
 *
 *  @note
 *  -# If the mutex is already locked by another thread, this routine suspends the calling thread
 *   until the mutex is unlocked.
 *
 *  @param pMutex [in] The mutex to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_ACQUIRE_MUTEX Failed to acquire mutex.
 */
u32 jf_mutex_acquire(jf_mutex_t * pMutex);

/** Try to acquire a mutex.
 *
 *  @note
 *  -# It does not block the calling thread if the mutex is already locked by another thread.
 *
 *  @param pMutex [in] The mutex to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_ACQUIRE_MUTEX Failed to acquire mutex.
 */
u32 jf_mutex_tryAcquire(jf_mutex_t * pMutex);

/** Acquire a mutex with time out.
 *
 *  @param pMutex [in] The mutex to be acquired.
 *  @param u32Timeout [in] The maximum waiting period if the mutex has been acquired by another
 *   thread in milliseconds.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_ACQUIRE_MUTEX Failed to acquire mutex.
 *  @retval JF_ERR_TIMEOUT Timeout.
 */
u32 jf_mutex_acquireWithTimeout(jf_mutex_t * pMutex, u32 u32Timeout);

/** Release a mutex.
 *
 *  @param pMutex [in] The mutex to be released.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RELEASE_MUTEX Failed to release mutex.
 */
u32 jf_mutex_release(jf_mutex_t * pMutex);

#endif /*JIUTAI_MUTEX_H*/

/*------------------------------------------------------------------------------------------------*/

