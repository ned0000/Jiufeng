/**
 *  @file persistency.c
 *
 *  @brief persistency implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "persistency.h"
#include "persistencycommon.h"
#include "sqlitepersistency.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */


/* --- public routine section ---------------------------------------------- */


/* --- private routine section --------------------------------------------- */

u32 createPersistency(
    persistency_type_t type, persistency_config_t * ppc, persistency_t ** ppPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = NULL;

    logInfoMsg("create persistency");

    u32Ret = xmalloc((void **)&ppm, sizeof(persistency_manager_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ppm->pm_ptType = type;
    
        switch (type)
        {
        case SQLITE_PERSISTENCY:
            u32Ret = initSqlitePersistency(ppm, &ppc->pc_pcsConfigSqlite);
            break;
        default:
            u32Ret = OLERR_INVALID_PARAM;
            break;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = initSyncMutex(&ppm->pm_smLock);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ppm->pm_bInitialized = TRUE;
        *ppPersist = ppm;
    }
    else if (ppm != NULL)
    {
        destroyPersistency((persistency_t **)&ppm);
    }
    
    return u32Ret;
}

u32 destroyPersistency(persistency_t ** ppPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)*ppPersist;

    u32Ret = ppm->pm_fnFini(ppm);

    finiSyncMutex(&ppm->pm_smLock);
    
    xfree((void **)ppPersist);

    return u32Ret;
}

u32 getPersistencyValue(
    persistency_t * pPersist, olchar_t * key, olchar_t * value, olsize_t valuelen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnGetValue(ppm, key, value, valuelen);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 setPersistencyValue(
    persistency_t * pPersist, olchar_t * key, olchar_t * value)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);
    
    u32Ret = ppm->pm_fnSetValue(ppm, key, value);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 startPersistencyTransaction(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnStartTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 commitPersistencyTransaction(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);
    
    u32Ret = ppm->pm_fnCommitTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 rollbackPersistencyTransaction(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnRollbackTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/



