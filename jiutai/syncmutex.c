/**
 *  @file syncmutex.c
 *
 *  @brief synchronization object implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>
#include <stdio.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "syncmutex.h"
#include "errcode.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */


/* --- public routine section ---------------------------------------------- */

u32 initSyncMutex(sync_mutex_t * pMutex)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert(pMutex != NULL);
    
#if defined(WINDOWS)
    memset(pMutex, 0, sizeof(sync_mutex_t));

    pMutex->sm_hMutex = CreateMutex(NULL, FALSE, NULL);
    if (pMutex->sm_hMutex == NULL)
    {
        u32Ret = OLERR_FAIL_CREATE_MUTEX;
    }
#elif defined(LINUX)
    olint_t nRet;
    pthread_mutexattr_t attr;

    memset(pMutex, 0, sizeof(sync_mutex_t));

    pthread_mutexattr_init(&attr);
    nRet = pthread_mutex_init(&(pMutex->sm_ptmMutex), &attr);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_CREATE_MUTEX;
    }
#endif

    return u32Ret;
}

u32 finiSyncMutex(sync_mutex_t * pMutex)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(WINDOWS)
    BOOL bRet;

    assert(pMutex != NULL);

    if (pMutex->sm_hMutex != NULL)
    {
        bRet = CloseHandle(pMutex->sm_hMutex);
        if (! bRet)
        {
            u32Ret = OLERR_FAIL_DESTROY_MUTEX;
        }
    }
#elif defined(LINUX)

    assert(pMutex != NULL);

    pthread_mutex_destroy(&(pMutex->sm_ptmMutex));
#endif
    return u32Ret;
}

u32 acquireSyncMutex(sync_mutex_t * pMutex)
{
    u32 u32Ret = OLERR_NO_ERROR;

#ifdef DEBUG_SYNCMUTEX
    ol_printf("acquire mutex %p\n", pMutex);
#endif

#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pMutex != NULL);
    
    dwRet = WaitForSingleObject(pMutex->sm_hMutex, INFINITE);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pMutex != NULL);

    nRet = pthread_mutex_lock(&(pMutex->sm_ptmMutex));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
#endif
    
    return u32Ret;
}

u32 tryAcquireSyncMutex(sync_mutex_t * pMutex)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pMutex != NULL);
    
    dwRet = WaitForSingleObject(pMutex->sm_hMutex, 0);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pMutex != NULL);

    nRet = pthread_mutex_trylock(&(pMutex->sm_ptmMutex));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
#endif
    
    return u32Ret;
}

u32 acquireSyncMutexWithTimeout(sync_mutex_t * pMutex, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;
    
    assert(pMutex != NULL);

    dwRet = WaitForSingleObject(pMutex->sm_hMutex, u32Timeout);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pMutex != NULL);

    nRet = pthread_mutex_trylock(&(pMutex->sm_ptmMutex));
    if (nRet != 0)
    {
        usleep(u32Timeout * 1000);
        nRet = pthread_mutex_trylock(&(pMutex->sm_ptmMutex));
        if (nRet != 0)
            u32Ret = OLERR_FAIL_ACQUIRE_MUTEX;
    }
#endif
    
    return u32Ret;
}

u32 releaseSyncMutex(sync_mutex_t * pMutex)
{
    u32 u32Ret = OLERR_NO_ERROR;

#ifdef DEBUG_SYNCMUTEX
    ol_printf("release mutex %p\n", pMutex);    
#endif

#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pMutex != NULL);
    
    bRet = ReleaseMutex(pMutex->sm_hMutex);
    if (bRet == FALSE)
    {
        u32Ret = OLERR_FAIL_RELEASE_MUTEX;
    }
#elif defined(LINUX)
    olint_t nRet = 0;

    assert(pMutex != NULL);

    nRet = pthread_mutex_unlock(&(pMutex->sm_ptmMutex));
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_RELEASE_MUTEX;
    }
#endif
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


