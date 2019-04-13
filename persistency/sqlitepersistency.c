/**
 *  @file sqlitepersistency.c
 *
 *  @brief Sqlite persistency implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_persistency.h"
#include "persistencycommon.h"
#include "sqlitepersistency.h"
#include "jf_rand.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _rollbackSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = jf_sqlite_rollbackTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _initSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_config_sqlite_t * pjpcs =
        &ppm->pm_pdData.pd_spSqlite.sp_jpcsConfigSqlite;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    jf_sqlite_init_param_t jsip;
    olchar_t strRet[128];
    olchar_t strSql[256];

    ol_bzero(&jsip, sizeof(jf_sqlite_init_param_t));
    jsip.jsip_pstrDbName = pjpcs->jpcs_strDbName;
    
    u32Ret = jf_sqlite_init(&pSqlite->sp_jsSqlite, &jsip);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_sprintf(
            strSql,
            "CREATE TABLE IF NOT EXISTS"
            " %s(%s TEXT PRIMARY KEY, %s TEXT);",
            pjpcs->jpcs_strTableName, pjpcs->jpcs_strKeyColumnName,
            pjpcs->jpcs_strValueColumnName);
        u32Ret = jf_sqlite_execSql(
            &pSqlite->sp_jsSqlite, strSql, strRet, sizeof(strRet));
    }

    return u32Ret;
}

static u32 _finiSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = jf_sqlite_fini(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _getSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    jf_persistency_config_sqlite_t * pjpcs =
        &ppm->pm_pdData.pd_spSqlite.sp_jpcsConfigSqlite;
    olchar_t strSql[512];

    ol_snprintf(
        strSql, sizeof(strSql), "SELECT %s FROM %s WHERE %s='%s';",
        pjpcs->jpcs_strValueColumnName,
        pjpcs->jpcs_strTableName, pjpcs->jpcs_strKeyColumnName, pKey);
    u32Ret = jf_sqlite_execSql(&pSqlite->sp_jsSqlite, strSql, pValue, sValue);

    return u32Ret;
}


static u32 _setSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey, olchar_t * pValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    jf_persistency_config_sqlite_t * pjpcs =
        &ppm->pm_pdData.pd_spSqlite.sp_jpcsConfigSqlite;
    olchar_t * pstrSql = NULL;
    olchar_t strRet[128];
    olsize_t nsize = ol_strlen(pValue) + ol_strlen(pKey) + 256;

    u32Ret = jf_mem_alloc((void **)&pstrSql, nsize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*update or insert the value into the DB*/
        ol_snprintf(
            pstrSql, nsize,
            "REPLACE INTO %s(%s, %s) VALUES ('%s', '%s');",
            pjpcs->jpcs_strTableName, pjpcs->jpcs_strKeyColumnName,
            pjpcs->jpcs_strValueColumnName, pKey, pValue);
        u32Ret = jf_sqlite_execSql(
            &pSqlite->sp_jsSqlite, pstrSql, strRet, sizeof(strRet));
    }
    
    if (pstrSql != NULL)
        jf_mem_free((void **)&pstrSql);

    return u32Ret;
}

static u32 _startSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = jf_sqlite_startTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _commitSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = jf_sqlite_commitTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initSqlitePersistency(
    persistency_manager_t * pManager, jf_persistency_config_sqlite_t * pConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("init sqlite persistency");

    ol_memcpy(
        &pManager->pm_pdData.pd_spSqlite.sp_jpcsConfigSqlite, pConfig,
        sizeof(jf_persistency_config_t));

    pManager->pm_fnFini = _finiSqlite;
    pManager->pm_fnGetValue = _getSqliteValue;
    pManager->pm_fnSetValue = _setSqliteValue;
    pManager->pm_fnStartTransaction = _startSqliteTransaction;
    pManager->pm_fnCommitTransaction = _commitSqliteTransaction;
    pManager->pm_fnRollbackTransaction = _rollbackSqliteTransaction;

    u32Ret = _initSqlite(pManager);
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

