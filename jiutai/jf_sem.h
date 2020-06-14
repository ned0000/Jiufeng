/**
 *  @file jf_sem.h
 *
 *  @brief Header file defines interface for synchronization semaphore object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_sem object.
 *
 */

#ifndef JIUTAI_SEM_H
#define JIUTAI_SEM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the semaphonre data type.
 */
typedef struct
{
#if defined(LINUX)
    olint_t js_nSem;
#elif defined(WINDOWS)
    HANDLE js_hSem;
#endif
} jf_sem_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize a semaphore.
 *
 *  @param pSem [in] The semaphore to be initiablized.
 *  @param u32InitialCount [in] The initial value of the semaphore.
 *  @param u32MaxCount [in] The maximum value of the semaphore.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_SEM Failed to create semaphore.
 */
u32 jf_sem_init(jf_sem_t * pSem, u32 u32InitialCount, u32 u32MaxCount);

/** Finalize a semaphore.
 *
 *  @param pSem [in] The semaphore to be finilized
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_sem_fini(jf_sem_t * pSem);

/** Down a semaphore.
 *
 *  @note
 *  -# If the semaphore is 0, this routine suspends the calling thread until the semaphore is up.
 *
 *  @param pSem [in] The semaphore to be downed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_DOWN_SEM Failed to down a semaphore.
 */
u32 jf_sem_down(jf_sem_t * pSem);

/** Try to down a semaphore.
 *
 *  @note
 *  -# It does not block the calling thread if the semaphore is 0.
 *
 *  @param pSem [in] The semaphore to be downed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_DOWN_SEM Failed to down a semaphore.
 */
u32 jf_sem_tryDown(jf_sem_t * pSem);

/** Down a semaphore with time out.
 *
 *  @param pSem [in] The semaphore to be downed.
 *  @param u32Timeout [in] The maximum waiting period in milliseconds when the semophore is 0.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_DOWN_SEM Failed to down a semaphore.
 */
u32 jf_sem_downWithTimeout(jf_sem_t * pSem, u32 u32Timeout);

/** Up a semaphore.
 *
 *  @param pSem [in] The semaphore to be uped.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_UP_SEM Failed to up a semaphore.
 */
u32 jf_sem_up(jf_sem_t * pSem);

#endif /*JIUTAI_SEM_H*/

/*------------------------------------------------------------------------------------------------*/
