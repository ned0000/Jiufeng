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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "persistency.h"
#include "persistencycommon.h"
#include "sqlitepersistency.h"
#include "xtime.h"
#include "randnum.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static u32 _rollbackSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = rollbackJtSqliteTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _initSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_config_sqlite_t * ppcs =
        &ppm->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    jt_sqlite_param_t jsp;
    olchar_t strRet[128];
    olchar_t strSql[256];

    ol_bzero(&jsp, sizeof(jt_sqlite_param_t));
    jsp.jsp_pstrDbName = ppcs->pcs_strDbName;
    
    u32Ret = initJtSqlite(&pSqlite->sp_jsSqlite, &jsp);
    if (u32Ret != OLERR_NO_ERROR)
    {
        ol_sprintf(
            strSql,
            "CREATE TABLE IF NOT EXISTS"
            " %s(%s TEXT PRIMARY KEY, %s TEXT);",
            ppcs->pcs_strTableName, ppcs->pcs_strKeyColumnName,
            ppcs->pcs_strValueColumnName);
        u32Ret = execJtSqliteSql(
            &pSqlite->sp_jsSqlite, strSql, strRet, sizeof(strRet));
    }

    return u32Ret;
}

static u32 _finiSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = finiJtSqlite(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _getSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    persistency_config_sqlite_t * ppcs =
        &ppm->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite;
    olchar_t strSql[512];

    ol_snprintf(
        strSql, sizeof(strSql), "SELECT %s FROM %s WHERE %s='%s';",
        ppcs->pcs_strValueColumnName,
        ppcs->pcs_strTableName, ppcs->pcs_strKeyColumnName, pKey);
    u32Ret = execJtSqliteSql(&pSqlite->sp_jsSqlite, strSql, pValue, sValue);

    return u32Ret;
}


static u32 _setSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey, olchar_t * pValue)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    persistency_config_sqlite_t * ppcs =
        &ppm->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite;
    olchar_t * pstrSql = NULL;
    olchar_t strRet[128];
    olsize_t nsize = ol_strlen(pValue) + ol_strlen(pKey) + 256;

    u32Ret = xmalloc((void **)&pstrSql, nsize);
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*update or insert the value into the DB*/
        ol_snprintf(
            pstrSql, nsize,
            "REPLACE INTO %s(%s, %s) VALUES ('%s', '%s');",
            ppcs->pcs_strTableName, ppcs->pcs_strKeyColumnName,
            ppcs->pcs_strValueColumnName, pKey, pValue);
        u32Ret = execJtSqliteSql(
            &pSqlite->sp_jsSqlite, pstrSql, strRet, sizeof(strRet));
    }
    
    if (pstrSql != NULL)
        xfree((void **)&pstrSql);

    return u32Ret;
}

static u32 _startSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = startJtSqliteTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

static u32 _commitSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    u32Ret = commitJtSqliteTransaction(&pSqlite->sp_jsSqlite);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 initSqlitePersistency(
    persistency_manager_t * pManager, persistency_config_sqlite_t * pConfig)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("init sqlite persistency");

    ol_memcpy(
        &pManager->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite, pConfig,
        sizeof(persistency_config_t));

    pManager->pm_fnFini = _finiSqlite;
    pManager->pm_fnGetValue = _getSqliteValue;
    pManager->pm_fnSetValue = _setSqliteValue;
    pManager->pm_fnStartTransaction = _startSqliteTransaction;
    pManager->pm_fnCommitTransaction = _commitSqliteTransaction;
    pManager->pm_fnRollbackTransaction = _rollbackSqliteTransaction;

    u32Ret = _initSqlite(pManager);
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/

