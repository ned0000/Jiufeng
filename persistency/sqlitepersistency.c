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
/** The wait time in milli-second when DB is locked
 */
#define DB_LOCK_WAIT                 (100)

/** The retry count when transaction fails
 */
#define MAX_TRANSACTION_RETRY        (20)

/** minimal time in millisecond for transaction retry 
 */
#define TRANSACTION_RETRY_MIN_TIME   (100)
/** maximum time in millisecond for transaction retry 
 */
#define TRANSACTION_RETRY_MAX_TIME   (2000)

/* --- private routine section---------------------------------------------- */

static u32 _sqliteEvalSqlStmt(
    persistency_manager_t * ppm, olchar_t * pstrSql, boolean_t bTransaction,
    olchar_t * pstrResult, olsize_t sResult, sqlite3_stmt * pStatement)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bKeepOnTrying = TRUE;
    olint_t nRet = SQLITE_OK;

    logInfoMsg("sqlite eval sql stmt, %s", pstrSql);

    pstrResult[0] = '\0';

    while (bKeepOnTrying)
    {
        nRet = sqlite3_step(pStatement);
        switch (nRet)
        {
        case SQLITE_ROW:
            logDebugMsg("sqlite eval sql stmt return SQLITE_ROW");
            ol_strncpy(
                pstrResult,
                (olchar_t *)sqlite3_column_text(pStatement, 0), sResult - 1);
            pstrResult[sResult - 1] = '\0';
            bKeepOnTrying = FALSE;
            break;
        case SQLITE_BUSY:
        case SQLITE_LOCKED:
            logDebugMsg("sqlite eval sql stmt, ret: %d", nRet);

            if (bTransaction || ppm->pm_bTransactionStarted)
            {
                /* donot retry for trasaction*/
                bKeepOnTrying = FALSE;
            }
            else
            {
                msleep(DB_LOCK_WAIT);
            }
            /* Reset the statement */
            sqlite3_reset(pStatement);
            break;
        case SQLITE_DONE:
            logDebugMsg("sqlite eval sql stmt, return SQLITE_DONE");
            bKeepOnTrying = FALSE;
            break;
        default:
            logInfoMsg("sqlite eval sql stmt, ret: %d", nRet);
            sqlite3_reset(pStatement);
            bKeepOnTrying = FALSE;
            u32Ret = OLERR_SQL_EVAL_ERROR;
            break;
        }
    }

    return u32Ret;
}

static u32 _sqliteExecSql(
    persistency_manager_t * ppm, olchar_t * pstrSql, boolean_t bTransaction,
    olchar_t * pstrResult, olsize_t sResult)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite3_stmt * pStatement = NULL;
    olint_t nRet = SQLITE_OK;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    logInfoMsg("sqlite exec sql, %s", pstrSql);

    pstrResult[0] = '\0';

    nRet = sqlite3_prepare_v2(
        pSqlite->sp_psSqlite, pstrSql, -1, &pStatement, NULL);
    if (nRet != SQLITE_OK)
    {
        logInfoMsg("sqlite3_prepare_v2 return %d", nRet);
        u32Ret = OLERR_SQL_COMPILE_ERROR;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _sqliteEvalSqlStmt(
            ppm, pstrSql, bTransaction, pstrResult, sResult, pStatement);
    }

    /* finilize the statement */
    if (pStatement != NULL)
        sqlite3_finalize(pStatement);

    return u32Ret;
}

static u32 _sqliteExecSqlTransaction(
    persistency_manager_t * ppm, olchar_t * pSql)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nAttempt = 0;
    olchar_t strRet[128];
    u32 u32Value;
    
    do
    {
        u32Ret = _sqliteExecSql(ppm, pSql, TRUE, strRet, sizeof(strRet));
        if ((u32Ret == OLERR_NO_ERROR) ||
            (nAttempt == MAX_TRANSACTION_RETRY))
            break;

		/* Get random number for sleep time in millisecond */
        u32Value = getRandomU32InRange(
            TRANSACTION_RETRY_MIN_TIME, TRANSACTION_RETRY_MAX_TIME);
        logDebugMsg(
            "retry sqlite trans, attemp: %d, sleep: %u", nAttempt, u32Value);
		msleep(u32Value);
		nAttempt++;
	} while (u32Ret != OLERR_NO_ERROR);

    return u32Ret;
}    

static u32 _rollbackSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _sqliteExecSqlTransaction(ppm, "ROLLBACK TRANSACTION");
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("sqlite rollback transaction");
        ppm->pm_bTransactionStarted = FALSE;
    }

    return u32Ret;
}

static u32 _initSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_config_sqlite_t * ppcs =
        &ppm->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;
    olchar_t strRet[128];
    olchar_t strSql[256];
    olint_t ret;

    ret = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    if (ret != SQLITE_OK)
    {
        /*doesn't matter if failed*/
        logInfoMsg("init sqlite, config failed");
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ret = sqlite3_open_v2(
            ppcs->pcs_strDbName, &pSqlite->sp_psSqlite,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
        if (ret != SQLITE_OK)
        {
            u32Ret = OLERR_PERSISTENCY_INIT_ERROR;
            logErrMsg(u32Ret, "init sqlite, open db failed");
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* set journal mode to persist */
        u32Ret = _sqliteExecSql(
            ppm, "PRAGMA journal_mode=PERSIST;", FALSE, strRet, sizeof(strRet));
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (sqlite3_busy_timeout(pSqlite->sp_psSqlite, 500) == SQLITE_OK)
            {
                /* create the table anyway */
                ol_sprintf(
                    strSql,
                    "CREATE TABLE IF NOT EXISTS"
                    " %s(%s TEXT PRIMARY KEY, %s TEXT);",
                    ppcs->pcs_strTableName, ppcs->pcs_strKeyColumnName,
                    ppcs->pcs_strValueColumnName);
                u32Ret = _sqliteExecSql(ppm, strSql, FALSE, strRet, sizeof(strRet));
            }
        }
    }

    return u32Ret;
}


static u32 _finiSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    sqlite_persistency_t * pSqlite = &ppm->pm_pdData.pd_spSqlite;

    sqlite3_close(pSqlite->sp_psSqlite);

    return u32Ret;
}


static u32 _getSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_config_sqlite_t * ppcs =
        &ppm->pm_pdData.pd_spSqlite.sp_pcsConfigSqlite;
    olchar_t strSql[512];

    ol_snprintf(
        strSql, sizeof(strSql), "SELECT %s FROM %s WHERE %s='%s';",
        ppcs->pcs_strValueColumnName,
        ppcs->pcs_strTableName, ppcs->pcs_strKeyColumnName, pKey);
    u32Ret = _sqliteExecSql(ppm, strSql, FALSE, pValue, sValue);

    return u32Ret;
}


static u32 _setSqliteValue(
    persistency_manager_t * ppm, olchar_t * pKey, olchar_t * pValue)
{
    u32 u32Ret = OLERR_NO_ERROR;
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
        u32Ret = _sqliteExecSql(ppm, pstrSql, FALSE, strRet, sizeof(strRet));
    }
    
    if (u32Ret != OLERR_NO_ERROR)
    {
        if (ppm->pm_bTransactionStarted)
        {
            _rollbackSqliteTransaction(ppm);
        }
    }

    if (pstrSql != NULL)
        xfree((void **)&pstrSql);

    return u32Ret;
}

static u32 _startSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _sqliteExecSqlTransaction(ppm, "BEGIN IMMEDIATE TRANSACTION");
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("sqlite start transaction");
        ppm->pm_bTransactionStarted = TRUE;
    }

    return u32Ret;
}

static u32 _commitSqliteTransaction(persistency_manager_t * ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _sqliteExecSqlTransaction(ppm, "COMMIT TRANSACTION");
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("sqlite commit transaction");
        ppm->pm_bTransactionStarted = FALSE;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 initSqlitePersistency(
    persistency_manager_t * pManager, persistency_config_sqlite_t * pConfig)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("init sqlite persistency");

    memcpy(
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

