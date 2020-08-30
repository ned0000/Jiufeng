/**
 *  @file persistency-test.c
 *
 *  @brief The test file for persistency library.
 *
 *  @author Min Zhang
 *  
 *  @note
 *  -# Create DB with command: sqlite3 env.db
 *  -# Create table with sql statement: CREATE TABLE env(key TEXT PRIMARY KEY, value TEXT);
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_persistency.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bPersistencyTestSqlite = FALSE;

static boolean_t ls_bPersistencyTestFile = FALSE;

static olchar_t * ls_pstrPersistencyFile = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static void _printPersistencyTestUsage(void)
{
    ol_printf("\
Usage: persistency-test [-s sqlite-db] [-f file] [-h] [logger options] \n\
  -s: test sqlite persistency.\n\
  -f: test file persistency.\n\
  -h: show this usage.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parsePersistencyTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = 0;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "s:f:?hT:F:S:O")) != -1))
    {
        switch (nOpt)
        {
        case 's':
            ls_bPersistencyTestSqlite = TRUE;
            ls_pstrPersistencyFile = jf_option_getArg();
            break;
        case 'f':
            ls_bPersistencyTestFile = TRUE;
            ls_pstrPersistencyFile = jf_option_getArg();
            break;
        case '?':
        case 'h':
            _printPersistencyTestUsage();
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(jf_option_getArg(), &pjlip->jlip_sLogFile);
            break;
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

    ol_printf("-------------------------------------------------\n");
    ol_printf("Test getting/setting key-value...\n");

    value[0] = '\0';

    ol_printf("set, %s = %s\n", key, value);
    u32Ret = jf_persistency_setValue(pPersist, key, value);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_persistency_getValue(pPersist, key, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);
    }

    ol_printf("set, %s = %s\n", key, monday);
    u32Ret = jf_persistency_setValue(pPersist, key, monday);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_persistency_getValue(pPersist, key, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        ol_printf("set, %s = %s\n", key, tuesday);
        u32Ret = jf_persistency_setValue(pPersist, key, tuesday);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_persistency_getValue(pPersist, key, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        u32Ret = jf_persistency_getValue(pPersist, no_such_key, value, sizeof(value));
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

    ol_printf("-------------------------------------------------\n");
    ol_printf("Test transaction...\n");

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

static u32 _handlePersistencyKeyValue(
    olchar_t * pstrKey, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("%s=%s\n", pstrKey, pstrValue);

    return u32Ret;
}

static u32 _testTraversePersistency(jf_persistency_t * pPersist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("-------------------------------------------------\n");
    ol_printf("Test traversing persistency...\n");

    u32Ret = jf_persistency_traverse(pPersist, _handlePersistencyKeyValue, NULL);

    return u32Ret;
}

static u32 _testPersistency(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersist = NULL;
    jf_persistency_config_t config;
    u8 u8Type = JF_PERSISTENCY_TYPE_UNKNOWN;

    ol_bzero(&config, sizeof(config));

    if (ls_bPersistencyTestSqlite)
    {
        u8Type = JF_PERSISTENCY_TYPE_SQLITE;
        ol_strcpy(config.jpc_jpcsConfigSqlite.jpcs_strSqliteDb, ls_pstrPersistencyFile);
    }
    else if (ls_bPersistencyTestFile)
    {
        u8Type = JF_PERSISTENCY_TYPE_FILE;
        ol_strcpy(config.jpc_jpcfConfigFile.jpcf_strFile, ls_pstrPersistencyFile);
    }
    else
    {
        u32Ret = JF_ERR_UNSUPPORTED_PERSISTENCY_TYPE;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_persistency_create(u8Type, &config, &pPersist);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testRwPersistency(pPersist);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testPersistencyTransaction(pPersist);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testTraversePersistency(pPersist);

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

    ol_bzero(&jlipParam, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "PERSISTENCY-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    u32Ret = _parsePersistencyTestCmdLineParam(argc, argv, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        ol_bzero(&jjip, sizeof(jjip));
        jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testPersistency();

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_printf("Err: (0x%X) %s\n", u32Ret, jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
