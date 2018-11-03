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

u32 initSyncSem(sync_sem_t * pSem, u32 u32InitialCount, u32 u32MaxCount);

u32 finiSyncSem(sync_sem_t * pSem);

u32 downSyncSem(sync_sem_t * pSem);

u32 tryDownSyncSem(sync_sem_t * pSem);

u32 downSyncSemWithTimeout(sync_sem_t * pSem, u32 u32Timeout);

u32 upSyncSem(sync_sem_t * pSem);

#endif /*JIUTAI_SYNCSEM_H*/

/*---------------------------------------------------------------------------*/

