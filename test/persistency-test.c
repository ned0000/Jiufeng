/**
 *  @file persistency-test.c
 *
 *  @brief The test file for persistency library
 *
 *  @author Min Zhang
 *  
 *  @note Create DB with command: sqlite3 env.db
 *  @note Create table with sql statement: CREATE TABLE env(key TEXT PRIMARY KEY, value TEXT);
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_persistency.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static void _printPersistencyTestUsage(void)
{
    ol_printf("\
Usage: persistency-test [-h] \n\
    -h show this usage.\n\
    ");

    ol_printf("\n");
}

static u32 _parsePersistencyTestCmdLineParam(olint_t argc, olchar_t ** argv)
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
            _printPersistencyTestUsage();
            exit(0);
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testRwPersistency(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * key = "today";
    olchar_t * no_such_key = "no-such-key";
    olchar_t * monday = "Monday";
    olchar_t * tuesday = "tuesday";
    olchar_t value[512];

    ol_printf("Testing persistency library\n");

    ol_printf("set, %s = %s\n", key, monday);
    u32Ret = jf_persistency_setValue(pPersist, key, monday);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_persistency_getValue(pPersist, key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        ol_printf("set, %s = %s\n", key, tuesday);
        u32Ret = jf_persistency_setValue(pPersist, key, tuesday);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_persistency_getValue(pPersist, key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        u32Ret = jf_persistency_getValue(pPersist, no_such_key, value, 512);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", no_such_key, value);
    }

    return u32Ret;
}

static u32 _testPersistencyTransaction(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Start transaction\n");
    u32Ret = jf_persistency_startTransaction(pPersist);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_persistency_setValue(pPersist, "color", "red");
        jf_persistency_setValue(pPersist, "name", "min");
        jf_persistency_setValue(pPersist, "age", "32");

        ol_printf("Commit transaction\n");
        jf_persistency_commitTransaction(pPersist);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Start transaction\n");
        u32Ret = jf_persistency_startTransaction(pPersist);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_persistency_setValue(pPersist, "book1", "1");
            jf_persistency_setValue(pPersist, "book2", "2");
            jf_persistency_setValue(pPersist, "book3", "3");

            ol_printf("Rollback transaction\n");
            jf_persistency_rollbackTransaction(pPersist);
        }
    }

    return u32Ret;
}

static u32 _testPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = NULL;
    jf_persistency_config_t config;

    memset(&config, 0, sizeof(jf_persistency_config_t));
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strDbName, "env.db");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strTableName, "env");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strKeyColumnName, "key");
    ol_strcpy(config.jpc_pcsConfigSqlite.jpcs_strValueColumnName, "value");

    u32Ret = jf_persistency_create(JF_PERSISTENCY_TYPE_SQLITE, &config, &pPersist);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testRwPersistency(pPersist);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testPersistencyTransaction(pPersist);

    if (pPersist != NULL)
        jf_persistency_destroy(&pPersist);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    if (argc < 1)
    {
        _printPersistencyTestUsage();
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jlipParam, sizeof(jf_logger_init_param_t));
        jlipParam.jlip_pstrCallerName = "PERSISTENCY";
        jlipParam.jlip_bLogToStdout = TRUE;
        jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;

        u32Ret = _parsePersistencyTestCmdLineParam(argc, argv);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        ol_bzero(&jjip, sizeof(jjip));
        jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _testPersistency();

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_printf("Err (0x%X) %s\n", u32Ret, jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


