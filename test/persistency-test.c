/**
 *  @file persistency-test.c
 *
 *  @brief The test file for persistency library
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "errcode.h"
#include "persistency.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: persistency-test [-h] \n\
    \n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "?h")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testRwPersistency(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * key = "today";
    olchar_t * no_such_key = "no-such-key";
    olchar_t * monday = "Monday";
    olchar_t * tuesday = "tuesday";
    olchar_t value[512];

    ol_printf("Testing persistency library\n");

    ol_printf("set, %s = %s\n", key, monday);
    u32Ret = setPersistencyValue(pPersist, key, monday);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getPersistencyValue(pPersist, key, value, 512);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        ol_printf("set, %s = %s\n", key, tuesday);
        u32Ret = setPersistencyValue(pPersist, key, tuesday);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getPersistencyValue(pPersist, key, value, 512);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", key, value);

        u32Ret = getPersistencyValue(pPersist, no_such_key, value, 512);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("get, %s = %s\n", no_such_key, value);
    }

    return u32Ret;
}

static u32 _testPersistencyTransaction(persistency_t * pPersist)
{
    u32 u32Ret = OLERR_NO_ERROR;

    ol_printf("Start transaction\n");
    u32Ret = startPersistencyTransaction(pPersist);
    if (u32Ret == OLERR_NO_ERROR)
    {
        setPersistencyValue(pPersist, "color", "red");
        setPersistencyValue(pPersist, "name", "min");
        setPersistencyValue(pPersist, "age", "32");

        ol_printf("Commit transaction\n");
        commitPersistencyTransaction(pPersist);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Start transaction\n");
        u32Ret = startPersistencyTransaction(pPersist);
        if (u32Ret == OLERR_NO_ERROR)
        {
            setPersistencyValue(pPersist, "book1", "1");
            setPersistencyValue(pPersist, "book2", "2");
            setPersistencyValue(pPersist, "book3", "3");

            ol_printf("Rollback transaction\n");
            rollbackPersistencyTransaction(pPersist);
        }
    }

    return u32Ret;
}

static u32 _testPersistency(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    persistency_t * pPersist = NULL;
    persistency_config_t config;

    memset(&config, 0, sizeof(persistency_config_t));
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strDbName, "env.db");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strTableName, "env");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strKeyColumnName, "key");
    ol_strcpy(config.pc_pcsConfigSqlite.pcs_strValueColumnName, "value");

    u32Ret = createPersistency(SQLITE_PERSISTENCY, &config, &pPersist);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testRwPersistency(pPersist);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testPersistencyTransaction(pPersist);

    if (pPersist != NULL)
        destroyPersistency(&pPersist);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    logger_param_t lpParam;

    if (argc < 1)
    {
        _printUsage();
        u32Ret = OLERR_MISSING_PARAM;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _parseCmdLineParam(argc, argv);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&lpParam, 0, sizeof(logger_param_t));
        lpParam.lp_pstrCallerName = "PERSISTENCY";
        lpParam.lp_bLogToFile = FALSE;
        lpParam.lp_bLogToStdout = TRUE;
        lpParam.lp_u8TraceLevel = LOGGER_TRACE_DATA;

        initLogger(&lpParam);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _testPersistency();
    }

    finiLogger();

    if (u32Ret != OLERR_NO_ERROR)
    {
        ol_printf("Err (0x%X) %s\n", u32Ret, getErrorDescription(u32Ret));
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


