/**
 *  @file syncrwlock.h
 *
 *  @brief synchronizaiton rwlock header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUTAI_SYNCRWLOCK_H
#define JIUTAI_SYNCRWLOCK_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
#if defined(LINUX)
    pthread_rwlock_t sr_ptrRwlock;
#elif defined(WINDOWS)
    boolean_t sr_bWritePriority;        /*true, if writer priority*/
   
    u32 sr_u32ReadCount;                /*number of active readers*/
    u32 sr_u32ReadWaiting;              /*number of waiting readers*/
      
    u32 sr_u32WriteCount;               /*number of active writers*/
    u32 sr_u32WriteWaiting;             /*number of waiting writers*/
      
    HANDLE sr_hReadGreen;               /*reader events*/
    HANDLE sr_hWriteGreen;              /*writer event*/
    CRITICAL_SECTION sr_csLock;         /*R/W lock critical section*/
#endif
} sync_rwlock_t;

/* --- functional routines ------------------------------------------------- */

/** Initialize the rwlock
 *
 *  @param pRwlock [in] the rwlock to be initialized
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 initSyncRwlock(sync_rwlock_t * pRwlock);

/** Finalize a rwlock
 *
 *  @param pRwlock [in] the rwlock to be finalized
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 finiSyncRwlock(sync_rwlock_t * pRwlock);

/** Acquire a read rwlock
 *
 *  @note If the rwlock is already  locked by another thread, this routine
 *   suspends the calling thread until the rwlock is unlocked.
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncReadlock(sync_rwlock_t * pRwlock);

/** Try to acquire a read rwlock
 *
 *  @note It does not block the calling thread if the rwlock is already
 *   locked by another thread
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 tryAcquireSyncReadlock(sync_rwlock_t * pRwlock);

/** Acquire a rwlock with time out
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *  @param u32Timeout [in] the maximum waiting period if the rwlock has been
 *   acquired by another thread. In milliseconds.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncReadlockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout);

/** Release a rwlock.
 *
 *  @param pRwlock [in] the rwlock to be released
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 releaseSyncReadlock(sync_rwlock_t * pRwlock);

/** Acquire a write rwlock
 *
 *  @note If the rwlock is already locked by another thread, this routine
 *   suspends the calling thread until the rwlock is unlocked.
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncWritelock(sync_rwlock_t * pRwlock);

/** Try to acquire a write rwlock
 *
 *  @note It does not block the calling thread if the rwlock is already
 *   locked by another thread
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 tryAcquireSyncWritelock(sync_rwlock_t * pRwlock);

/** Acquire a write rwlock with time out
 *
 *  @param pRwlock [in] the rwlock to be acquired
 *  @param u32Timeout [in] the maximum waiting period if the rwlock has been
 *   acquired by another thread in milliseconds.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 acquireSyncWritelockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout);

/** Release a write rwlock.
 *
 *  @param pRwlock [in] the rwlock to be released
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 releaseSyncWritelock(sync_rwlock_t * pRwlock);

#endif /*JIUTAI_SYNCRWLOCK_H*/

/*---------------------------------------------------------------------------*/


