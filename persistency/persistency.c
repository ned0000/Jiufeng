/**
 *  @file persistency/persistency.c
 *
 *  @brief Persistency implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_persistency.h"
#include "jf_jiukun.h"

#include "persistencycommon.h"
#include "filepersistency.h"
#if defined(LINUX)
    #include "sqlitepersistency.h"
#endif

/* --- private data/data structure section ------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */


/* --- private routine section ------------------------------------------------------------------ */

u32 jf_persistency_create(
    jf_persistency_type_t type, jf_persistency_config_t * ppc, jf_persistency_t ** ppPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = NULL;

    JF_LOGGER_INFO("type: %s", getStringPersistencyType(type));

    /*Allocate memory for persistency manager.*/
    u32Ret = jf_jiukun_allocMemory((void **)&ppm, sizeof(persistency_manager_t));

    /*Initialize persistency manager.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ppm->pm_jptType = type;
    
        switch (type)
        {
#if defined(LINUX)
        case JF_PERSISTENCY_TYPE_SQLITE:
            u32Ret = initSqlitePersistency(ppm, &ppc->jpc_jpcsConfigSqlite);
            break;
#endif
        case JF_PERSISTENCY_TYPE_FILE:
            u32Ret = initFilePersistency(ppm, &ppc->jpc_jpcfConfigFile);
            break;
        default:
            u32Ret = JF_ERR_UNSUPPORTED_PERSISTENCY_TYPE;
            break;
        }
    }

    /*Initialize the mutex.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mutex_init(&ppm->pm_jmLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPersist = ppm;
    else if (ppm != NULL)
        jf_persistency_destroy((jf_persistency_t **)&ppm);
    
    return u32Ret;
}

u32 jf_persistency_destroy(jf_persistency_t ** ppPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)*ppPersist;

    if (ppm->pm_fnFini != NULL)
        u32Ret = ppm->pm_fnFini(ppm);

    jf_mutex_fini(&ppm->pm_jmLock);
    
    jf_jiukun_freeMemory((void **)ppPersist);

    return u32Ret;
}

u32 jf_persistency_getValue(
    jf_persistency_t * pPersist, const olchar_t * key, olchar_t * value, olsize_t valuelen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    value[0] = '\0';

    jf_mutex_acquire(&ppm->pm_jmLock);

    u32Ret = ppm->pm_fnGetValue(ppm, key, value, valuelen);

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

u32 jf_persistency_setValue(
    jf_persistency_t * pPersist, const olchar_t * key, const olchar_t * value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    jf_mutex_acquire(&ppm->pm_jmLock);
    
    u32Ret = ppm->pm_fnSetValue(ppm, key, value);

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

u32 jf_persistency_startTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    jf_mutex_acquire(&ppm->pm_jmLock);

    u32Ret = ppm->pm_fnStartTransaction(ppm);

    if (u32Ret == JF_ERR_NO_ERROR)
        ppm->pm_bTransactionStarted = TRUE;

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

u32 jf_persistency_commitTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    jf_mutex_acquire(&ppm->pm_jmLock);
    
    u32Ret = ppm->pm_fnCommitTransaction(ppm);

    ppm->pm_bTransactionStarted = FALSE;

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

u32 jf_persistency_rollbackTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    jf_mutex_acquire(&ppm->pm_jmLock);

    u32Ret = ppm->pm_fnRollbackTransaction(ppm);

    ppm->pm_bTransactionStarted = FALSE;

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

u32 jf_persistency_traverse(
    jf_persistency_t * pPersist, jf_persistency_fnHandleKeyValue_t fnHandleKeyValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    persistency_manager_t * ppm = (persistency_manager_t *)pPersist;

    jf_mutex_acquire(&ppm->pm_jmLock);

    u32Ret = ppm->pm_fnTraverse(ppm, fnHandleKeyValue, pArg);

    jf_mutex_release(&ppm->pm_jmLock);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
