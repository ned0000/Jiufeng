/**
 *  @file jf_sqlite.c
 *
 *  @brief Implementation file for sqlite object. 
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */


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
#define JF_SQLITE_DB_LOCK_WAIT                           (100)

/** The retry count when transaction fails.
 */
#define JF_SQLITE_MAX_TRANSACTION_RETRY                  (20)

/** Minimal time in millisecond for transaction retry.
 */
#define JF_SQLITE_TRANSACTION_RETRY_MIN_TIME             (100)

/** Maximum time in millisecond for transaction retry.
 */
#define JF_SQLITE_TRANSACTION_RETRY_MAX_TIME             (2000)

/* --- private routine section ------------------------------------------------------------------ */

static u32 _jtSqliteEvalSqlStmt(
    jf_sqlite_t * pjs, boolean_t bTransaction, jf_sqlite_fnHandleRowData_t fnHandleRowData,
    void * pArg, sqlite3_stmt * pStmt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bKeepOnTrying = TRUE;
    olint_t nRet = SQLITE_OK;

    while (bKeepOnTrying && (u32Ret == JF_ERR_NO_ERROR))
    {
        nRet = sqlite3_step(pStmt);
        switch (nRet)
        {
        case SQLITE_ROW:
            /*Success and data is returned, after retrieving all data, SQLITE_DONE is returned.*/
#if defined(DEBUG_SQLITE)
            JF_LOGGER_DEBUG("sqlite3_step return SQLITE_ROW");
#endif
            if (fnHandleRowData != NULL)
                u32Ret = fnHandleRowData(pStmt, pArg);
            break;
        case SQLITE_BUSY:
        case SQLITE_LOCKED:
#if defined(DEBUG_SQLITE)
            JF_LOGGER_DEBUG("sqlite3_step return %d", nRet);
#endif
            if (bTransaction || pjs->js_bTransactionStarted)
            {
                /*Do not retry for trasaction.*/
                bKeepOnTrying = FALSE;
                u32Ret = JF_ERR_BUSY;
            }
            else /*Sleep certain time.*/
            {
                jf_time_milliSleep(JF_SQLITE_DB_LOCK_WAIT);
            }
            /*Reset the statement.*/
            sqlite3_reset(pStmt);
            break;
        case SQLITE_DONE:
            /*Success and no data is returned.*/
#if defined(DEBUG_SQLITE)
            JF_LOGGER_DEBUG("sqlite3_step return SQLITE_DONE");
#endif
            bKeepOnTrying = FALSE;
            break;
        default:
            JF_LOGGER_INFO("sqlite3_step return %d", nRet);
            sqlite3_reset(pStmt);
            bKeepOnTrying = FALSE;
            u32Ret = JF_ERR_SQL_EVAL_ERROR;
            break;
        }
    }

    return u32Ret;
}

static u32 _jtSqliteExecSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, boolean_t bTransaction,
    jf_sqlite_fnHandleRowData_t fnHandleRowData, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite3_stmt * pStmt = NULL;
    olint_t nRet = SQLITE_OK;

#if defined(DEBUG_SQLITE)
    JF_LOGGER_DEBUG("sql: %s", pstrSql);
#endif

    /*Compile the SQL statement into a byte-code program.*/
    nRet = sqlite3_prepare_v2(pjs->js_psSqlite, pstrSql, -1, &pStmt, NULL);
    if (nRet != SQLITE_OK)
    {
        u32Ret = JF_ERR_SQL_COMPILE_ERROR;
        JF_LOGGER_ERR(u32Ret, "sqlite3_prepare_v2 return %d", nRet);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _jtSqliteEvalSqlStmt(pjs, bTransaction, fnHandleRowData, pArg, pStmt);
    }

    /*Finilize the statement.*/
    if (pStmt != NULL)
        sqlite3_finalize(pStmt);

    return u32Ret;
}

static u32 _jtSqliteExecSqlTransaction(jf_sqlite_t * pjs, olchar_t * pSql)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nAttempt = 0;
    u32 u32Value = 0;
    
    do
    {
        u32Ret = _jtSqliteExecSql(pjs, pSql, TRUE, NULL, NULL);

        /*Quit the loop if no error.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            break;

        /*Quit the loop if the error is not busy.*/
        if (u32Ret != JF_ERR_BUSY)
            break;

		/*Get random number for sleep time in millisecond.*/
        u32Value = jf_rand_getU32InRange(
            JF_SQLITE_TRANSACTION_RETRY_MIN_TIME, JF_SQLITE_TRANSACTION_RETRY_MAX_TIME);
        JF_LOGGER_DEBUG("attemp: %d, sleep: %u", nAttempt, u32Value);

		jf_time_milliSleep(u32Value);
		nAttempt++;
    } while (nAttempt < JF_SQLITE_MAX_TRANSACTION_RETRY);

    return u32Ret;
}    

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_sqlite_init(jf_sqlite_t * pjs, jf_sqlite_init_param_t * param)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t ret = 0;

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
        u32Ret = _jtSqliteExecSql(pjs, "PRAGMA journal_mode=PERSIST;", FALSE, NULL, NULL);

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
    jf_sqlite_t * pjs, olchar_t * pstrSql, jf_sqlite_fnHandleRowData_t fnHandleRowData, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (! pjs->js_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    u32Ret = _jtSqliteExecSql(pjs, pstrSql, FALSE, fnHandleRowData, pArg);

    return u32Ret;
}

olchar_t * jf_sqlite_getColumnText(jf_sqlite_stmt_t * pStmt, olint_t nCol)
{
    return (olchar_t *)sqlite3_column_text(pStmt, nCol);
}

olint_t jf_sqlite_getColumnInt(jf_sqlite_stmt_t * pStmt, olint_t nCol)
{
    return sqlite3_column_int(pStmt, nCol);
}

oldouble_t jf_sqlite_getColumnDouble(jf_sqlite_stmt_t * pStmt, olint_t nCol)
{
    return sqlite3_column_double(pStmt, nCol);
}

/*------------------------------------------------------------------------------------------------*/
