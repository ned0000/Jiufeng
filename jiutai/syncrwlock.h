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
    boolean_t sr_bWritePriority;              /*true, if writer priority*/
   
    u32 sr_u32ReadCount;                /*number of active readers*/
    u32 sr_u32ReadWaiting;              /*number of waiting readers*/
      
    u32 sr_u32WriteCount;                /*number of active writers*/
    u32 sr_u32WriteWaiting;              /*number of waiting writers*/
      
    HANDLE sr_hReadGreen;              /*reader events*/
    HANDLE sr_hWriteGreen;              /*writer event*/
    CRITICAL_SECTION sr_csLock;        /*R/W lock critical section*/
#endif
} sync_rwlock_t;

/* --- functional routines ------------------------------------------------- */
u32 initSyncRwlock(sync_rwlock_t * pRwlock);

u32 finiSyncRwlock(sync_rwlock_t * pRwlock);

u32 acquireSyncReadlock(sync_rwlock_t * pRwlock);

u32 tryAcquireSyncReadlock(sync_rwlock_t * pRwlock);

u32 acquireSyncReadlockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout);

u32 releaseSyncReadlock(sync_rwlock_t * pRwlock);

u32 acquireSyncWritelock(sync_rwlock_t * pRwlock);

u32 tryAcquireSyncWritelock(sync_rwlock_t * pRwlock);

u32 acquireSyncWritelockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout);

u32 releaseSyncWritelock(sync_rwlock_t * pRwlock);

#endif /*JIUTAI_SYNCRWLOCK_H*/

/*---------------------------------------------------------------------------*/


