/**
 *  @file syncsem.c
 *
 *  @brief synchronization semaphore object implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>
#if defined(LINUX)
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/sem.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "syncsem.h"
#include "errcode.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
#if defined(LINUX)
    #define SV_W    0400
    #define SV_R    0200

    #define SEM_FLAG  (SV_W | SV_R | (SV_W >> 3) | (SV_R >> 3) | (SV_W >> 6) | (SV_R >> 6))
#endif

#if defined(LINUX)
union semun
{
    olint_t val;
    struct semid_ds *buf;
    ushort * array;
} ;
#endif

/* --- private routine section ------------------------------------------------ */

/* --- public routine section ---------------------------------------------- */

/** initialize a semaphore 
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] pss, the semaphore to be initiablized
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 initSyncSem(sync_sem_t * pSem, u32 u32InitialCount, u32 u32MaxCount)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(WINDOWS)
    assert(pSem != NULL);
    assert(u32InitialCount <= u32MaxCount);

    memset(pSem, 0, sizeof(sync_sem_t));

    pSem->is_hSem = CreateSemaphore(NULL, u32InitialCount, u32MaxCount, NULL);
    if (pSem->is_hSem == NULL)
    {
        u32Ret = OLERR_FAIL_CREATE_SEM;
    }
#elif defined(LINUX)
    olint_t nRet;
    union semun init;

    assert(pSem != NULL);
    assert(u32InitialCount <= u32MaxCount);

    memset(pSem, 0, sizeof(sync_sem_t));

    /* create 1 semaphore */
    pSem->is_nSem = semget(IPC_PRIVATE, 1, SEM_FLAG);
    if (pSem->is_nSem == -1)
    {
        u32Ret = OLERR_FAIL_CREATE_SEM;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        init.val = u32InitialCount;
        nRet = semctl(pSem->is_nSem, 0, SETVAL, init);
        if (nRet == -1)
        {
            u32Ret = OLERR_FAIL_CREATE_SEM;
        }
    }
#endif
    
    return u32Ret;
}

/** finalize a semaphore
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] the semaphore to be finilized
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 *
 */
u32 finiSyncSem(sync_sem_t * pSem)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pSem != NULL);

    if (pSem->is_hSem != NULL)
    {
        bRet = CloseHandle(pSem->is_hSem);
        if (! bRet)
        {
            u32Ret = OLERR_FAIL_DESTROY_SEM;
        }
    }
#elif defined(LINUX)
    olint_t nRet;

    assert(pSem != NULL);

    if ((pSem->is_nSem != 0) && (pSem->is_nSem != -1))
    {
        nRet = semctl(pSem->is_nSem, 0, IPC_RMID);
        if (nRet == -1)
        {
            u32Ret = OLERR_FAIL_DESTROY_SEM;
        }
    }
#endif
    
    return u32Ret;
}

/** up a semaphore
 *
 *  Notes.
 *    - If the semaphore is 0, this routine suspends the calling thread 
 *      until the semaphore is up.
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] the semaphore to be downed
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 *
 */
u32 downSyncSem(sync_sem_t * pSem)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pSem != NULL);

    dwRet = WaitForSingleObject(pSem->is_hSem, INFINITE);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;
    struct sembuf semlock;

    assert(pSem != NULL);

    semlock.sem_num = 0;
    semlock.sem_op = -1;
    semlock.sem_flg = SEM_UNDO;
    nRet = semop(pSem->is_nSem, &semlock, 1);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
#endif
    
    return u32Ret;
}

/** try to down a semaphore
 *
 *  Notes:
 *    - it does not block the calling thread if the semaphore is 0
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] the semaphore to be downed
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 tryDownSyncSem(sync_sem_t * pSem)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pSem != NULL);

    dwRet = WaitForSingleObject(pSem->is_hSem, 0);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;
    struct sembuf semlock;

    assert(pSem != NULL);

    semlock.sem_num = 0;
    semlock.sem_op = -1;
    semlock.sem_flg = SEM_UNDO | IPC_NOWAIT;
    nRet = semop(pSem->is_nSem, &semlock, 1);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
#endif
    
    return u32Ret;
}

/** down a mutex with time out
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] the semaphore to be downed
 *  @param u32Timeout: u32 <BR>
 *     @b [in] the maximum waiting period if the semophore has been uped
 *           by another thread. In milliseconds.
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 downSyncSemWithTimeout(sync_sem_t * pSem, u32 u32Timeout)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    DWORD dwRet = 0;

    assert(pSem != NULL);

    dwRet = WaitForSingleObject(pSem->is_hSem, u32Timeout);
    if (dwRet == WAIT_FAILED)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
    else if (dwRet == WAIT_TIMEOUT)
    {
        u32Ret = OLERR_TIMEOUT;
    }
#elif defined(LINUX)
    olint_t nRet = 0;
    struct sembuf semlock;
    struct timespec timeout;

    assert(pSem != NULL);

    memset(&timeout, 0, sizeof(struct timespec));

    timeout.tv_sec = u32Timeout / 1000;
    timeout.tv_nsec = (u32Timeout % 1000) * 1000000;

    semlock.sem_num = 0;
    semlock.sem_op = -1;
    semlock.sem_flg = SEM_UNDO;
    nRet = semtimedop(pSem->is_nSem, &semlock, 1, &timeout);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_ACQUIRE_SEM;
    }
#endif
    
    return u32Ret;
}

/** up a semaphore.
 *
 *  @param pSem: sync_sem_t * <BR>
 *     @b [in] the semaphore to be uped
 *
 *  @return return OLERR_NO_ERROR on success; otherwise the error code
 */
u32 upSyncSem(sync_sem_t * pSem)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
#if defined(WINDOWS)
    BOOL bRet = TRUE;

    assert(pSem != NULL);

    bRet = ReleaseSemaphore(pSem->is_hSem, 1, NULL);
    if (! bRet)
    {
        u32Ret = OLERR_FAIL_RELEASE_SEM;
    }
#elif defined(LINUX)
    olint_t nRet = 0;
    struct sembuf semunlock;

    assert(pSem != NULL);

    semunlock.sem_num = 0;
    semunlock.sem_op = 1;
    semunlock.sem_flg = SEM_UNDO;
    nRet = semop(pSem->is_nSem, &semunlock, 1);
    if (nRet != 0)
    {
        u32Ret = OLERR_FAIL_RELEASE_SEM;
    }
#endif
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


