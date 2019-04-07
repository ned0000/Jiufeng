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
#include "jf_basic.h"
#include "syncrwlock.h"
#include "errcode.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
#if defined(WINDOWS)
static u32 _acquireSyncReadlock(jf_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    DWORD dwRet = 0;
    boolean_t wait = FALSE;
   
    do
	{
        EnterCriticalSection(&(pRwlock->jr_csLock));

        /* acquire lock if 
           - there are no active writers and 
           - readers have priority or
           - writers have priority and there are no waiting writers */
        if (! pRwlock->jr_u32WriteCount &&
			(! pRwlock->jr_bWritePriority || ! pRwlock->jr_u32WriteWaiting))
		{
            if (wait)
			{
                pRwlock->jr_u32ReadWaiting --;
                wait = FALSE;
            }
            pRwlock->jr_u32ReadCount ++;
        }
        else
		{
            if (! wait)
			{
                pRwlock->jr_u32ReadWaiting ++;
                wait = TRUE;
            }
            /*always reset the event to avoid 100% CPU usage*/
            ResetEvent(pRwlock->jr_hReadGreen);
        }

        LeaveCriticalSection(&(pRwlock->jr_csLock));
      
        if (wait)
		{
			dwRet = WaitForSingleObject(pRwlock->jr_hReadGreen, u32Timeout);
            if (dwRet != WAIT_OBJECT_0)
			{
                EnterCriticalSection(&(pRwlock->jr_csLock));
                pRwlock->jr_u32ReadWaiting --;
                SetEvent(pRwlock->jr_hReadGreen);
				SetEvent(pRwlock->jr_hWriteGreen);
                LeaveCriticalSection(&(pRwlock->jr_csLock));

                if (dwRet == WAIT_TIMEOUT)
                    u32Ret = JF_ERR_TIMEOUT;
				else
					u32Ret = JF_ERR_FAIL_ACQUIRE_MUTEX;

                return u32Ret;
            } 
        }
   } while (wait);

	return u32Ret;
}

static u32 _acquireSyncWritelock(jf_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    DWORD dwRet = 0;
    boolean_t wait = FALSE;
   
    do
	{
        EnterCriticalSection(&(pRwlock->jr_csLock));

        /* acquire lock if 
            - there are no active readers nor writers and 
            - writers have priority or
            - readers have priority and there are no waiting readers */
        if (! pRwlock->jr_u32ReadCount && ! pRwlock->jr_u32WriteCount &&
			(pRwlock->jr_bWritePriority || ! pRwlock->jr_u32ReadWaiting))
		{
            if (wait)
			{
                pRwlock->jr_u32WriteWaiting --;
                wait = FALSE;
            }
            pRwlock->jr_u32WriteCount ++;
        }
        else
		{
            if (! wait)
			{
                pRwlock->jr_u32WriteWaiting ++;
                wait = TRUE;
            }
            /*always reset the event to avoid 100% CPU usage*/
            ResetEvent(pRwlock->jr_hWriteGreen);
        }

        LeaveCriticalSection(&(pRwlock->jr_csLock));

        if (wait)
		{
			dwRet = WaitForSingleObject(pRwlock->jr_hWriteGreen, u32Timeout);
            if (dwRet != WAIT_OBJECT_0)
			{
                EnterCriticalSection(&(pRwlock->jr_csLock));
                pRwlock->jr_u32WriteWaiting --;
                SetEvent(pRwlock->jr_hReadGreen);
				SetEvent(pRwlock->jr_hWriteGreen);
                LeaveCriticalSection(&(pRwlock->jr_csLock));

				if (dwRet == WAIT_TIMEOUT)
                    u32Ret = JF_ERR_TIMEOUT;
				else
					u32Ret = JF_ERR_FAIL_ACQUIRE_MUTEX;

                return u32Ret;
            }
         }
         
    } while (wait);

	return u32Ret;
}

#endif

/* --- public routine section ---------------------------------------------- */

u32 jf_rwlock_init(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pRwlock != NULL);
    
#if defined(WINDOWS)
    memset(pRwlock, 0, sizeof(jf_rwlock_t));

	pRwlock->jr_bWritePriority = TRUE;
	InitializeCriticalSection(&(pRwlock->jr_csLock));

    pRwlock->jr_hReadGreen = CreateEvent(NULL, FALSE, TRUE, NULL);
    pRwlock->jr_hWriteGreen = CreateEvent(NULL, FALSE, TRUE, NULL);

#elif defined(LINUX)
    olint_t nRet;
    pthread_rwlockattr_t attr;

    memset(pRwlock, 0, sizeof(jf_rwlock_t));

    pthread_rwlockattr_init(&attr);
    nRet = pthread_rwlock_init(&(pRwlock->jr_ptrRwlock), &attr);
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_CREATE_RWLOCK;
    }
#endif

    return u32Ret;
}

u32 jf_rwlock_fini(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(WINDOWS)
    BOOL bRet;

    assert(pRwlock != NULL);

    CloseHandle(pRwlock->jr_hReadGreen);
    CloseHandle(pRwlock->jr_hWriteGreen);
   
    DeleteCriticalSection(&(pRwlock->jr_csLock));

#elif defined(LINUX)

    assert(pRwlock != NULL);

    pthread_rwlock_destroy(&(pRwlock->jr_ptrRwlock));
#endif
    return u32Ret;
}

u32 jf_rwlock_acquireReadlock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(WINDOWS)

	assert(pRwlock != NULL);

	u32Ret = _acquireSyncReadlock(pRwlock, INFINITE);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_rdlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_tryAcquireReadlock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
#if defined(WINDOWS)

	assert(pRwlock != NULL);

	u32Ret = _acquireSyncReadlock(pRwlock, 0);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_tryrdlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_acquireReadlockWithTimeout(jf_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
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

    nRet = pthread_rwlock_timedrdlock(&(pRwlock->jr_ptrRwlock), &ts);
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_releaseReadlock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pRwlock != NULL);
    
    EnterCriticalSection(&(pRwlock->jr_csLock));

    pRwlock->jr_u32ReadCount--;

    /*always release waiting threads (do not check for rdcount == 0)*/
    if (pRwlock->jr_bWritePriority)
	{
        if (pRwlock->jr_u32WriteWaiting)
            SetEvent(pRwlock->jr_hWriteGreen);
        else if (pRwlock->jr_u32ReadWaiting)
            SetEvent(pRwlock->jr_hReadGreen);
    }
    else
	{
        if (pRwlock->jr_u32ReadWaiting)
            SetEvent(pRwlock->jr_hReadGreen);
        else if (pRwlock->jr_u32WriteWaiting)
            SetEvent(pRwlock->jr_hWriteGreen);
    }

    LeaveCriticalSection(&(pRwlock->jr_csLock));

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_unlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_RELEASE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_acquireWritelock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pRwlock != NULL);
    
    u32Ret = _acquireSyncWritelock(pRwlock, INFINITE);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_wrlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_tryAcquireWritelock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pRwlock != NULL);
    
    u32Ret = _acquireSyncWritelock(pRwlock, 0);

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_trywrlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_acquireWritelockWithTimeout(jf_rwlock_t * pRwlock, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
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

    nRet = pthread_rwlock_timedwrlock(&(pRwlock->jr_ptrRwlock), &ts);
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_ACQUIRE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

u32 jf_rwlock_releaseWritelock(jf_rwlock_t * pRwlock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pRwlock != NULL);
    
    EnterCriticalSection(&(pRwlock->jr_csLock));

    pRwlock->jr_u32WriteCount --;

    if (pRwlock->jr_bWritePriority)
	{
        if (pRwlock->jr_u32WriteWaiting)
            SetEvent(pRwlock->jr_hWriteGreen);
        else if(pRwlock->jr_u32ReadWaiting)
            SetEvent(pRwlock->jr_hReadGreen);
    }
    else
	{
        if (pRwlock->jr_u32ReadWaiting)
            SetEvent(pRwlock->jr_hReadGreen);
        else if(pRwlock->jr_u32WriteWaiting)
            SetEvent(pRwlock->jr_hWriteGreen);
    }
      
    LeaveCriticalSection(&(pRwlock->jr_csLock));

#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pRwlock != NULL);

    nRet = pthread_rwlock_unlock(&(pRwlock->jr_ptrRwlock));
    if (nRet != 0)
    {
        u32Ret = JF_ERR_FAIL_RELEASE_RWLOCK;
    }
#endif
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


