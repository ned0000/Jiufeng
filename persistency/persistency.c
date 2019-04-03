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

u32 jf_persistency_create(
    jf_persistency_type_t type, jf_persistency_config_t * ppc,
    jf_persistency_t ** ppPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = NULL;

    jf_logger_logInfoMsg("create persistency");

    u32Ret = xmalloc((void **)&ppm, sizeof(persistency_manager_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ppm->pm_jptType = type;
    
        switch (type)
        {
        case JF_PERSISTENCY_TYPE_SQLITE:
            u32Ret = initSqlitePersistency(ppm, &ppc->jpc_pcsConfigSqlite);
            break;
        default:
            u32Ret = JF_ERR_INVALID_PARAM;
            break;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = initSyncMutex(&ppm->pm_smLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ppm->pm_bInitialized = TRUE;
        *ppPersist = ppm;
    }
    else if (ppm != NULL)
    {
        jf_persistency_destroy((jf_persistency_t **)&ppm);
    }
    
    return u32Ret;
}

u32 jf_persistency_destroy(jf_persistency_t ** ppPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)*ppPersist;

    u32Ret = ppm->pm_fnFini(ppm);

    finiSyncMutex(&ppm->pm_smLock);
    
    xfree((void **)ppPersist);

    return u32Ret;
}

u32 jf_persistency_getValue(
    jf_persistency_t * pPersist, olchar_t * key, olchar_t * value,
    olsize_t valuelen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnGetValue(ppm, key, value, valuelen);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 jf_persistency_setValue(
    jf_persistency_t * pPersist, olchar_t * key, olchar_t * value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);
    
    u32Ret = ppm->pm_fnSetValue(ppm, key, value);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 jf_persistency_startTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnStartTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 jf_persistency_commitTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);
    
    u32Ret = ppm->pm_fnCommitTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

u32 jf_persistency_rollbackTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    acquireSyncMutex(&ppm->pm_smLock);

    u32Ret = ppm->pm_fnRollbackTransaction(ppm);

    releaseSyncMutex(&ppm->pm_smLock);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/



