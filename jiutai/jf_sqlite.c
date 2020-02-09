/**
 *  @file jf_sqlite.c
 *
 *  @brief Implementation file for sqlite object. 
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_sqlite object.
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_sqlite.h"
#include "jf_time.h"
#include "jf_rand.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The wait time in milli-second when DB is locked.
 */
#define DB_LOCK_WAIT                 (100)

/** The retry count when transaction fails.
 */
#define MAX_TRANSACTION_RETRY        (20)

/** Minimal time in millisecond for transaction retry.
 */
#define TRANSACTION_RETRY_MIN_TIME   (100)

/** Maximum time in millisecond for transaction retry.
 */
#define TRANSACTION_RETRY_MAX_TIME   (2000)

/* --- private routine section ------------------------------------------------------------------ */

static u32 _jtSqliteEvalSqlStmt(
    jf_sqlite_t * pjs, boolean_t bTransaction, olchar_t * pstrResult, olsize_t sResult,
    sqlite3_stmt * pStatement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bKeepOnTrying = TRUE;
    olint_t nRet = SQLITE_OK;

    pstrResult[0] = '\0';

    while (bKeepOnTrying)
    {
        nRet = sqlite3_step(pStatement);
        switch (nRet)
        {
        case SQLITE_ROW:
            /*Success and data is returned.*/
            JF_LOGGER_DEBUG("sqlite3_step return SQLITE_ROW");
            ol_strncpy(pstrResult, (olchar_t *)sqlite3_column_text(pStatement, 0), sResult - 1);
            pstrResult[sResult - 1] = '\0';
            bKeepOnTrying = FALSE;
            break;
        case SQLITE_BUSY:
        case SQLITE_LOCKED:
            JF_LOGGER_DEBUG("sqlite3_step return %d", nRet);

            if (bTransaction || pjs->js_bTransactionStarted)
            {
                /*Do not retry for trasaction.*/
                bKeepOnTrying = FALSE;
            }
            else
            {
                jf_time_milliSleep(DB_LOCK_WAIT);
            }
            /*Reset the statement.*/
            sqlite3_reset(pStatement);
            break;
        case SQLITE_DONE:
            /*Success and no data is returned.*/
            JF_LOGGER_DEBUG("sqlite3_step return SQLITE_DONE");
            bKeepOnTrying = FALSE;
            break;
        default:
            JF_LOGGER_INFO("sqlite3_step return %d", nRet);
            sqlite3_reset(pStatement);
            bKeepOnTrying = FALSE;
            u32Ret = JF_ERR_SQL_EVAL_ERROR;
            break;
        }
    }

    return u32Ret;
}

static u32 _jtSqliteExecSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, boolean_t bTransaction, olchar_t * pstrResult,
    olsize_t sResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite3_stmt * pStatement = NULL;
    olint_t nRet = SQLITE_OK;

    JF_LOGGER_DEBUG("sql: %s", pstrSql);

    pstrResult[0] = '\0';

    nRet = sqlite3_prepare_v2(pjs->js_psSqlite, pstrSql, -1, &pStatement, NULL);
    if (nRet != SQLITE_OK)
    {
        u32Ret = JF_ERR_SQL_COMPILE_ERROR;
        JF_LOGGER_ERR(u32Ret, "sqlite3_prepare_v2 return %d", nRet);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _jtSqliteEvalSqlStmt(pjs, bTransaction, pstrResult, sResult, pStatement);
    }

    /*Finilize the statement.*/
    if (pStatement != NULL)
        sqlite3_finalize(pStatement);

    return u32Ret;
}

static u32 _jtSqliteExecSqlTransaction(jf_sqlite_t * pjs, olchar_t * pSql)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nAttempt = 0;
    olchar_t strRet[128];
    u32 u32Value;
    
    do
    {
        u32Ret = _jtSqliteExecSql(pjs, pSql, TRUE, strRet, sizeof(strRet));
        if ((u32Ret == JF_ERR_NO_ERROR) || (nAttempt == MAX_TRANSACTION_RETRY))
            break;

		/*Get random number for sleep time in millisecond.*/
        u32Value = jf_rand_getU32InRange(TRANSACTION_RETRY_MIN_TIME, TRANSACTION_RETRY_MAX_TIME);
        JF_LOGGER_DEBUG("attemp: %d, sleep: %u", nAttempt, u32Value);
		jf_time_milliSleep(u32Value);
		nAttempt++;
	} while (u32Ret != JF_ERR_NO_ERROR);

    return u32Ret;
}    

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_sqlite_init(jf_sqlite_t * pjs, jf_sqlite_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strRet[128];
    olint_t ret;

    ol_bzero(pjs, sizeof(*pjs));

    ret = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    if (ret != SQLITE_OK)
    {
        /*Doesn't matter if failed.*/
        JF_LOGGER_INFO("sqlite3_config failed");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Open the database.*/
        ret = sqlite3_open_v2(
            param->jsip_pstrDbName, &pjs->js_psSqlite,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
        if (ret != SQLITE_OK)
        {
            u32Ret = JF_ERR_PERSISTENCY_INIT_ERROR;
            JF_LOGGER_ERR(u32Ret, "open db failed");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Set journal mode to persist.*/
        u32Ret = _jtSqliteExecSql(
            pjs, "PRAGMA journal_mode=PERSIST;", FALSE, strRet, sizeof(strRet));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ret = sqlite3_busy_timeout(pjs->js_psSqlite, 500);
            if (ret != SQLITE_OK)
            {
                JF_LOGGER_INFO("failed to set busy timeout");
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjs->js_bInitialized = TRUE;
    }
    else
    {
        jf_sqlite_fini(pjs);
    }

    return u32Ret;
}

u32 jf_sqlite_fini(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pjs->js_psSqlite != NULL)
    {
        sqlite3_close(pjs->js_psSqlite);
        pjs->js_psSqlite = NULL;
    }

    pjs->js_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_sqlite_rollbackTransaction(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("rollback transaction");

    if (! pjs->js_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    if (! pjs->js_bTransactionStarted)
        return u32Ret;

    u32Ret = _jtSqliteExecSqlTransaction(pjs, "ROLLBACK TRANSACTION");
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjs->js_bTransactionStarted = FALSE;
    }

    return u32Ret;
}

u32 jf_sqlite_startTransaction(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("start transaction");

    if (! pjs->js_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    u32Ret = _jtSqliteExecSqlTransaction(pjs, "BEGIN IMMEDIATE TRANSACTION");
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjs->js_bTransactionStarted = TRUE;
    }

    return u32Ret;
}

u32 jf_sqlite_commitTransaction(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("commit transaction");

    if (! pjs->js_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    if (! pjs->js_bTransactionStarted)
        return u32Ret;

    u32Ret = _jtSqliteExecSqlTransaction(pjs, "COMMIT TRANSACTION");
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjs->js_bTransactionStarted = FALSE;
    }

    return u32Ret;
}

u32 jf_sqlite_execSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, olchar_t * pstrResult, olsize_t sResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (! pjs->js_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    u32Ret = _jtSqliteExecSql(pjs, pstrSql, FALSE, pstrResult, sResult);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

