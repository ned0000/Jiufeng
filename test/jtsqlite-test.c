/**
 *  @file jtsqlite.c
 *
 *  @brief The test file for jt sqlite common object
 *
 *  @author Min Zhang
 *
 *  @note Create DB with command: sqlite3 env.db
 *  @note Create table with sql statement: CREATE TABLE env(key TEXT PRIMARY
 *   KEY, value TEXT);
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "bases.h"
#include "errcode.h"
#include "jtsqlite.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: jtsqlite-test [-h] \n\
    -h show this usage. \n\
    ");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "?h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _setJtSqliteValue(
    jf_sqlite_t * pjs, olchar_t * pKey, olchar_t * pValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrSql = NULL;
    olchar_t strRet[128];
    olsize_t nsize = ol_strlen(pValue) + ol_strlen(pKey) + 256;

    u32Ret = jf_mem_alloc((void **)&pstrSql, nsize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*update or insert the value into the DB*/
        ol_snprintf(
            pstrSql, nsize,
            "REPLACE INTO env(key, value) VALUES ('%s', '%s');",
            pKey, pValue);
        u32Ret = jf_sqlite_execSql(pjs, pstrSql, strRet, sizeof(strRet));
    }
    
    if (pstrSql != NULL)
        jf_mem_free((void **)&pstrSql);

    return u32Ret;
}

static u32 _getJtSqliteValue(
    jf_sqlite_t * pjs, olchar_t * pKey, olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strSql[512];

    ol_snprintf(
        strSql, sizeof(strSql), "SELECT value FROM env WHERE key='%s';", pKey);
    u32Ret = jf_sqlite_execSql(pjs, strSql, pValue, sValue);

    return u32Ret;
}

static u32 _testRwJtSqlite(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * key = "today";
    olchar_t * no_such_key = "no-such-key";
    olchar_t * monday = "Monday";
    olchar_t * tuesday = "tuesday";
    olchar_t value[512];

    ol_printf("Testing jt sqlite common object \n");

    ol_printf("set, %s = %s\n", key, monday);
    u32Ret = _setJtSqliteValue(pjs, key, monday);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getJtSqliteValue(pjs, key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        ol_printf("set, %s = %s\n", key, tuesday);
        u32Ret = _setJtSqliteValue(pjs, key, tuesday);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getJtSqliteValue(pjs, key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        u32Ret = _getJtSqliteValue(pjs, no_such_key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", no_such_key, value);
    }

    return u32Ret;
}

static u32 _testJtSqliteTransaction(jf_sqlite_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Start jt sqlite transaction\n");
    u32Ret = jf_sqlite_startTransaction(pjs);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _setJtSqliteValue(pjs, "color", "red");
        _setJtSqliteValue(pjs, "name", "min");
        _setJtSqliteValue(pjs, "age", "32");

        ol_printf("Commit jt sqlite transaction\n");
        jf_sqlite_commitTransaction(pjs);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Start jt sqlite transaction\n");
        u32Ret = jf_sqlite_startTransaction(pjs);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _setJtSqliteValue(pjs, "book1", "1");
            _setJtSqliteValue(pjs, "book2", "2");
            _setJtSqliteValue(pjs, "book3", "3");

            ol_printf("Rollback jt sqlite transaction\n");
            jf_sqlite_rollbackTransaction(pjs);
        }
    }

    return u32Ret;
}

static u32 _testJtSqlite(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_sqlite_t js;
    jf_sqlite_init_param_t config;

    ol_memset(&config, 0, sizeof(jf_sqlite_init_param_t));
    config.jsip_pstrDbName = "env.db";

    u32Ret = jf_sqlite_init(&js, &config);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _testRwJtSqlite(&js);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _testJtSqliteTransaction(&js);

        jf_sqlite_fini(&js);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;

    if (argc < 1)
    {
        _printUsage();
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseCmdLineParam(argc, argv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
        jlipParam.jlip_pstrCallerName = "JTSQLITE";
        jlipParam.jlip_bLogToFile = FALSE;
        jlipParam.jlip_bLogToStdout = TRUE;
        jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;

        jf_logger_init(&jlipParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testJtSqlite();
    }

    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_printf("Err (0x%X) %s\n", u32Ret, jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


