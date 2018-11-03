/**
 *  @file syncrwlock.c
 *
 *  @brief The synchronization rwlock common object
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "syncrwlock.h"
#include "errcode.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
#if defined(WINDOWS)
static u32 _acquireSyncReadlock(sync_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    DWORD dwRet = 0;
    boolean_t wait = FALSE;
   
    do
	{
        EnterCriticalSection(&(pRwlock->sr_csLock));

        /* acquire lock if 
           - there are no active writers and 
           - readers have priority or
           - writers have priority and there are no waiting writers */
        if (! pRwlock->sr_u32WriteCount &&
			(! pRwlock->sr_bWritePriority || ! pRwlock->sr_u32WriteWaiting))
		{
            if (wait)
			{
                pRwlock->sr_u32ReadWaiting --;
                wait = FALSE;
            }
            pRwlock->sr_u32ReadCount ++;
        }
        else
		{
            if (! wait)
			{
                pRwlock->sr_u32ReadWaiting ++;
                wait = TRUE;
            }
            /*always reset the event to avoid 100% CPU usage*/
            ResetEvent(pRwlock->sr_hReadGreen);
        }

        LeaveCriticalSection(&(pRwlock->sr_csLock));
      
        if (wait)
		{
			dwRet = WaitForSingleObject(pRwlock->sr_hReadGreen, u32Timeout);
            if (dwRet != WAIT_OBJECT_0)
			{
                EnterCriticalSection(&(pRwlock->sr_csLock));
                pRwlock->sr_u32ReadWaiting --;
                SetEvent(pRwlock->sr_hReadGreen);
				SetEvent(pRwlock->sr_hWriteGreen);
                LeaveCriticalSection(&(pRwlock->sr_csLock));

                if (dwRet == WAIT_TIMEOUT)
                    u32Ret = OLERR_TIMEOUT;
				else
					u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;

                return u32Ret;
            } 
        }
   } while (wait);

	return u32Ret;
}

static u32 _acquireSyncWritelock(sync_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    DWORD dwRet = 0;
    boolean_t wait = FALSE;
   
    do
	{
        EnterCriticalSection(&(pRwlock->sr_csLock));

        /* acquire lock if 
            - there are no active readers nor writers and 
            - writers have priority or
            - readers have priority and there are no waiting readers */
        if (! pRwlock->sr_u32ReadCount && ! pRwlock->sr_u32WriteCount &&
			(pRwlock->sr_bWritePriority || ! pRwlock->sr_u32ReadWaiting))
		{
            if (wait)
			{
                pRwlock->sr_u32WriteWaiting --;
                wait = FALSE;
            }
            pRwlock->sr_u32WriteCount ++;
        }
        else
		{
            if (! wait)
			{
                pRwlock->sr_u32WriteWaiting ++;
                wait = TRUE;
            }
            /*always reset the event to avoid 100% CPU usage*/
            ResetEvent(pRwlock->sr_hWriteGreen);
        }

        LeaveCriticalSection(&(pRwlock->sr_csLock));

        if (wait)
		{
			dwRet = WaitForSingleObject(pRwlock->sr_hWriteGreen, u32Timeout);
            if (dwRet != WAIT_OBJECT_0)
			{
                EnterCriticalSection(&(pRwlock->sr_csLock));
                pRwlock->sr_u32WriteWaiting --;
                SetEvent(pRwlock->sr_hReadGreen);
				SetEvent(pRwlock->sr_hWriteGreen);
                LeaveCriticalSection(&(pRwlock->sr_csLock));

				if (dwRet == WAIT_TIMEOUT)
                    u32Ret = OLERR_TIMEOUT;
				else
					u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;

                return u32Ret;
            }
         }
         
    } while (wait);

	return u32Ret;
}

#endif

/* --- public routine section ---------------------------------------------- */
/** initialize the rwlock
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be initialized
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 initSyncRwlock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert(pRwlock != NULL);
    
#if defined(WINDOWS)
    memset(pRwlock, 0, sizeof(sync_rwlock_t));

	pRwlock->sr_bWritePriority = TRUE;
	InitializeCriticalSection(&(pRwlock->sr_csLock));

    pRwlock->sr_hReadGreen = CreateEvent(NULL, FALSE, TRUE, NULL);
    pRwlock->sr_hWriteGreen = CreateEvent(NULL, FALSE, TRUE, NULL);

#elif defined(LINUX)
    olint_t nRet;
    pthread_rwlockattr_t attr;

    memset(pRwlock, 0, sizeof(sync_rwlock_t));

    pthread_rwlockattr_init(&attr);
    nRet = pthread_rwlock_init(&(pRwlock->sr_ptrRwlock), &attr);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_CREATE_RWLOCK;
    }
#endif

    return u32Ret;
}

/** finalize a rwlock
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be finalized
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 finiSyncRwlock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(WINDOWS)
    BOOL bRet;

    assert(pRwlock != NULL);

    CloseHandle(pRwlock->sr_hReadGreen);
    CloseHandle(pRwlock->sr_hWriteGreen);
   
    DeleteCriticalSection(&(pRwlock->sr_csLock));

#elif defined(LINUX)

    assert(pRwlock != NULL);

    pthread_rwlock_destroy(&(pRwlock->sr_ptrRwlock));
#endif
    return u32Ret;
}

/** acquire a read rwlock
 *
 *  Notes.
 *    - If the rwlock is already  locked by another thread, this routine suspends
 *      the calling thread until the rwlock is unlocked.
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 acquireSyncReadlock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(WINDOWS)

	assert(pRwlock != NULL);

	u32Ret = _acquireSyncReadlock(pRwlock, INFINITE);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_rdlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** try to acquire a read rwlock
 *
 *  Notes:
 *    - it does not block the calling thread if the rwlock is already
 *      locked by another thread
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 tryAcquireSyncReadlock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)

	assert(pRwlock != NULL);

	u32Ret = _acquireSyncReadlock(pRwlock, 0);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_tryrdlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** acquire a rwlock with time out
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *  @param u32Timeout: u32 <BR>
 *     @b [in] the maximum waiting period if the rwlock has been acquired
 *           by another thread. In milliseconds.
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 acquireSyncReadlockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    
    assert(pRwlock != NULL);

	u32Ret = _acquireSyncReadlock(pRwlock, u32Timeout);

#elif defined(LINUX)
    olint_t nRet = 0;
    struct timespec ts;

    assert(pRwlock != NULL);

    memset(&ts, 0, sizeof(ts));

    ts.tv_sec = u32Timeout / 1000;
    ts.tv_nsec = (u32Timeout % 1000) * 1000000;

    nRet = pthread_rwlock_timedrdlock(&(pRwlock->sr_ptrRwlock), &ts);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** release a rwlock.
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be released
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 releaseSyncReadlock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pRwlock != NULL);
    
    EnterCriticalSection(&(pRwlock->sr_csLock));

    pRwlock->sr_u32ReadCount--;

    /*always release waiting threads (do not check for rdcount == 0)*/
    if (pRwlock->sr_bWritePriority)
	{
        if (pRwlock->sr_u32WriteWaiting)
            SetEvent(pRwlock->sr_hWriteGreen);
        else if (pRwlock->sr_u32ReadWaiting)
            SetEvent(pRwlock->sr_hReadGreen);
    }
    else
	{
        if (pRwlock->sr_u32ReadWaiting)
            SetEvent(pRwlock->sr_hReadGreen);
        else if (pRwlock->sr_u32WriteWaiting)
            SetEvent(pRwlock->sr_hWriteGreen);
    }

    LeaveCriticalSection(&(pRwlock->sr_csLock));

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_unlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_RELEASE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** acquire a write rwlock
 *
 *  Notes.
 *    - If the rwlock is already  locked by another thread, this routine suspends
 *      the calling thread until the rwlock is unlocked.
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 acquireSyncWritelock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pRwlock != NULL);
    
    u32Ret = _acquireSyncWritelock(pRwlock, INFINITE);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_wrlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** try to acquire a write rwlock
 *
 *  Notes:
 *    - it does not block the calling thread if the rwlock is already
 *      locked by another thread
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 tryAcquireSyncWritelock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pRwlock != NULL);
    
    u32Ret = _acquireSyncWritelock(pRwlock, 0);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_trywrlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** acquire a write rwlock with time out
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be acquired
 *  @param u32Timeout: u32 <BR>
 *     @b [in] the maximum waiting period if the rwlock has been acquired
 *           by another thread. In milliseconds.
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 acquireSyncWritelockWithTimeout(sync_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;
    
    assert(pRwlock != NULL);

    u32Ret = _acquireSyncWritelock(pRwlock, u32Timeout);

#elif defined(LINUX)
    olint_t nRet = 0;
    struct timespec ts;

    assert(pRwlock != NULL);

    memset(&ts, 0, sizeof(ts));

    ts.tv_sec = u32Timeout / 1000;
    ts.tv_nsec = (u32Timeout % 1000) * 1000000;

    nRet = pthread_rwlock_timedwrlock(&(pRwlock->sr_ptrRwlock), &ts);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/** release a rwlock.
 *
 *  @param pRwlock: sync_rwlock_t * <BR>
 *     @b [in] the rwlock to be released
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 releaseSyncWritelock(sync_rwlock_t * pRwlock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pRwlock != NULL);
    
    EnterCriticalSection(&(pRwlock->sr_csLock));

    pRwlock->sr_u32WriteCount --;

    if (pRwlock->sr_bWritePriority)
	{
        if (pRwlock->sr_u32WriteWaiting)
            SetEvent(pRwlock->sr_hWriteGreen);
        else if(pRwlock->sr_u32ReadWaiting)
            SetEvent(pRwlock->sr_hReadGreen);
    }
    else
	{
        if (pRwlock->sr_u32ReadWaiting)
            SetEvent(pRwlock->sr_hReadGreen);
        else if(pRwlock->sr_u32WriteWaiting)
            SetEvent(pRwlock->sr_hWriteGreen);
    }
      
    LeaveCriticalSection(&(pRwlock->sr_csLock));

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_unlock(&(pRwlock->sr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_RELEASE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


