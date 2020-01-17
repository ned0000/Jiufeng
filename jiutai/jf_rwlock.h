/**
 *  @file jf_rwlock.h
 *
 *  @brief Header file defines the interface for synchronizaiton read-write lock.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_rwlock object.
 *
 */

#ifndef JIUTAI_RWLOCK_H
#define JIUTAI_RWLOCK_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the read-write lock data type.
 */
typedef struct
{
#if defined(LINUX)
    /**The rwlock from pthread library.*/
    pthread_rwlock_t jr_ptrRwlock;
#elif defined(WINDOWS)
    /**True if writer priority.*/
    boolean_t jr_bWritePriority;
    /**Number of active readers.*/   
    u32 jr_u32ReadCount;
    /**Number of waiting readers.*/
    u32 jr_u32ReadWaiting;
    /**Number of active writers.*/
    u32 jr_u32WriteCount;
    /**Number of waiting writers.*/
    u32 jr_u32WriteWaiting;

    /**Reader events.*/
    HANDLE jr_hReadGreen;
    /**Writer event.*/
    HANDLE jr_hWriteGreen;
    /**R/W lock critical section.*/
    CRITICAL_SECTION jr_csLock;
#endif
} jf_rwlock_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the read-write lock.
 *
 *  @param pRwlock [in] The read-write lock to be initialized.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_init(jf_rwlock_t * pRwlock);

/** Finalize a read-write lock.
 *
 *  @param pRwlock [in] The read-write lock to be finalized.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_fini(jf_rwlock_t * pRwlock);

/** Acquire a read lock.
 *
 *  @note
 *  -# The calling thread acquires the read lock if a writer does not hold the lock and there are
 *   no writers blocked on the lock. If the read-write lock is already locked, this routine suspends
 *   the calling thread until the rwlock is unlocked.
 *
 *  @param pRwlock [in] The rwlock to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_acquireReadlock(jf_rwlock_t * pRwlock);

/** Try to acquire a read lock.
 *
 *  @note
 *  -# It does not block the calling thread if the read-write lock is already locked by another
 *   thread.
 *
 *  @param pRwlock [in] The rwlock to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_tryAcquireReadlock(jf_rwlock_t * pRwlock);

/** Acquire a read lock with time out.
 *
 *  @param pRwlock [in] The read-write lock to be acquired.
 *  @param u32Timeout [in] The maximum waiting period if the read-write lock has been acquired by
 *   another thread. The timeout value is in milliseconds.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_acquireReadlockWithTimeout(jf_rwlock_t * pRwlock, u32 u32Timeout);

/** Release a read lock.
 *
 *  @param pRwlock [in] The read-write lock to be released.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_releaseReadlock(jf_rwlock_t * pRwlock);

/** Acquire a write lock.
 *
 *  @note
 *  -# If the read-write lock is already locked by another thread, this routine suspends the
 *   calling thread until the read-write lock is unlocked.
 *
 *  @param pRwlock [in] The read-write lock to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_acquireWritelock(jf_rwlock_t * pRwlock);

/** Try to acquire a write lock.
 *
 *  @note
 *  -# It does not block the calling thread if the read-write lock is already locked by another
 *   thread.
 *
 *  @param pRwlock [in] The rwlock to be acquired.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_tryAcquireWritelock(jf_rwlock_t * pRwlock);

/** Acquire a write lock with time out.
 *
 *  @param pRwlock [in] The rwlock to be acquired.
 *  @param u32Timeout [in] The maximum waiting period in millisecond if the read-write lock has
 *   been acquired by another thread.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_acquireWritelockWithTimeout(jf_rwlock_t * pRwlock, u32 u32Timeout);

/** Release a write rwlock.
 *
 *  @param pRwlock [in] The rwlock to be released.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_rwlock_releaseWritelock(jf_rwlock_t * pRwlock);

#endif /*JIUTAI_RWLOCK_H*/

/*------------------------------------------------------------------------------------------------*/


