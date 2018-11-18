/**
 *  @file syncsem.h
 *
 *  @brief synchronization semaphore object header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUTAI_SYNCSEM_H
#define JIUTAI_SYNCSEM_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
#if defined(LINUX)
    olint_t is_nSem;
#elif defined(WINDOWS)
    HANDLE is_hSem;
#endif
} sync_sem_t;

/* --- functional routines ------------------------------------------------- */

/** Initialize a semaphore 
 *
 *  @param pSem [in] the semaphore to be initiablized
 *  @param u32InitialCount [in] the initial value of the semaphore
 *  @param u32MaxCount [in] the maximum value of the semaphore
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 initSyncSem(sync_sem_t * pSem, u32 u32InitialCount, u32 u32MaxCount);

/** Finalize a semaphore
 *
 *  @param pSem [in] the semaphore to be finilized
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 finiSyncSem(sync_sem_t * pSem);

/** Down a semaphore
 *
 *  @note if the semaphore is 0, this routine suspends the calling thread 
 *   until the semaphore is up.
 *
 *  @param pSem [in] the semaphore to be downed
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 downSyncSem(sync_sem_t * pSem);

/** Try to down a semaphore
 *
 *  @note it does not block the calling thread if the semaphore is 0
 *
 *  @param pSem [in] the semaphore to be downed
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 tryDownSyncSem(sync_sem_t * pSem);

/** Down a semaphore with time out
 *
 *  @param pSem [in] the semaphore to be downed
 *  @param u32Timeout [in] the maximum waiting period if the semophore has
 *   been up by another thread in milliseconds.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 downSyncSemWithTimeout(sync_sem_t * pSem, u32 u32Timeout);

/** Up a semaphore.
 *
 *  @param pSem [in] the semaphore to be uped
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 upSyncSem(sync_sem_t * pSem);

#endif /*JIUTAI_SYNCSEM_H*/

/*---------------------------------------------------------------------------*/

