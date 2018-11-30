/**
 *  @file jiukun-test.c
 *
 *  @brief The test file for jiukun library
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
#include "jiukun.h"
#include "process.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_THREAD_COUNT  2

static boolean_t ls_bToTerminate = FALSE;

boolean_t ls_bMultiThread = FALSE;
boolean_t ls_bStress = FALSE;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: jiukun-test [-t] [logger options]\n\
    -t test in multi-threading environment.\n\
    -s stress testing.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <trace file size> the size of log file. No limit if not specified.\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "tsOT:F:S:h")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
            break;
        case 't':
            ls_bMultiThread = TRUE;
            break;
        case 's':
            ls_bStress = TRUE;
            break;
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_sLogFile = u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'O':
            plp->lp_bLogToStdout = TRUE;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

#define DEBUG_LOOP_COUNT  100
u32 _testAllocMem(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Mem[DEBUG_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop, u32Size;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    memset(pu8Mem, 0, sizeof(u8 *) * DEBUG_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();

    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            logInfoMsg("Allocate memory");
        }
        else
        {
            bAlloc = FALSE;
            logInfoMsg("free memory");
        }

        nRand = rand();
        u32Index = nRand % DEBUG_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();
                u32Size = nRand % MAX_JIUKUN_MEMORY_SIZE;

                logInfoMsg("!!!! Allocate %u", u32Size);

                u32Ret = allocMemory((void **)&(pu8Mem[u32Index]), u32Size, 0);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    logInfoMsg("success, at %u, %p\n", u32Index,
                        pu8Mem[u32Index]);
#if defined(DEBUG_JIUKUN)
                    dumpJiukun();
#endif
                }
                else
                {
                    getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < DEBUG_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    freeMemory((void **)&(pu8Mem[u32Idx]));

                    pu8Mem[u32Idx] = NULL;
#if defined(DEBUG_JIUKUN)
                    dumpJiukun();
#endif
                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            freeMemory((void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

#define MAX_ORDER  8

u32 _testJiukunPage(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Mem[DEBUG_LOOP_COUNT];
    u32 u32Order[DEBUG_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    memset(pu8Mem, 0, sizeof(u8 *) * DEBUG_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();
#if defined(DEBUG_JIUKUN)
    dumpJiukun();
#endif
    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            logInfoMsg("Allocate page");
        }
        else
        {
            bAlloc = FALSE;
            logInfoMsg("free page");
        }

        nRand = rand();
        u32Index = nRand % DEBUG_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();
                u32Order[u32Index] = nRand % MAX_ORDER;

                u32Ret = allocJiukunPage(
                    (void **)&(pu8Mem[u32Index]), u32Order[u32Index], 0);
                if (u32Ret == OLERR_NO_ERROR)
                {
#if defined(DEBUG_JIUKUN)
                    dumpJiukun();
#endif
                    logInfoMsg("success, at %u, %p\n", u32Index,
                        pu8Mem[u32Index]);
                }
                else
                {
                    getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < DEBUG_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    freeJiukunPage((void **)&(pu8Mem[u32Idx]));
#if defined(DEBUG_JIUKUN)
                    dumpJiukun();
#endif
                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            freeJiukunPage((void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

#define TEST_CACHE "test-cache"
static jiukun_cache_t * ls_pacCache = NULL;

u32 _testJiukunCache(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Mem[DEBUG_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    memset(pu8Mem, 0, sizeof(u8 *) * DEBUG_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();

    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            logInfoMsg("Allocate object");
        }
        else
        {
            bAlloc = FALSE;
            logInfoMsg("free object");
        }

        nRand = rand();
        u32Index = nRand % DEBUG_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();

                u32Ret = allocObject(ls_pacCache, (void **)&(pu8Mem[u32Index]), 0);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    logInfoMsg("success, at %u, %p\n", u32Index,
                        pu8Mem[u32Index]);
                }
                else
                {
                    getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < DEBUG_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    freeObject(ls_pacCache, (void **)&(pu8Mem[u32Idx]));

                    pu8Mem[u32Idx] = NULL;

                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            freeObject(ls_pacCache, (void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

static u32 _stressJiukun(void)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _testJiukunPage();

    u32Ret = _testAllocMem();

    u32Ret = _testJiukunCache();

    return u32Ret;
}

THREAD_RETURN_VALUE _allocFree(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Index = (u32)(ulong)pArg;

    logInfoMsg("alloc-free thread %u starts", u32Index);

    while ((! ls_bToTerminate) && (u32Ret == OLERR_NO_ERROR))
    {
        logInfoMsg("alloc-free thread %u starts testing", u32Index);

        u32Ret = _testJiukunCache();
    }


    if (u32Ret == OLERR_NO_ERROR)
        logInfoMsg("alloc-free thread %u quits", u32Index);
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        logInfoMsg("alloc-free thread %u quits, %s", u32Index, strErrMsg);
    }

    THREAD_RETURN(u32Ret);
}

static u32 _testJiukunInThread(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Index;
    jiukun_cache_param_t jcp;

    memset(&jcp, 0, sizeof(jiukun_cache_param_t));
    jcp.jcp_pstrName = TEST_CACHE;
    jcp.jcp_sObj = 28;

    u32Ret = createJiukunCache(&ls_pacCache, &jcp);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (u32Index = 0;
             ((u32Index < MAX_THREAD_COUNT) && (u32Ret == OLERR_NO_ERROR));
             u32Index ++)
        {
            u32Ret = createThread(NULL, NULL, _allocFree, (void *)(ulong)(u32Index + 1));
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("main thread, sleeping for 5 minutes\n");
        sleep(300);
        ol_printf("prepare to exit\n");
    }

    ls_bToTerminate = TRUE;
    sleep(1);

    destroyJiukunCache(&ls_pacCache);

    return u32Ret;
}

static u32 _baseJiukunFunc(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * page = NULL;
    olflag_t flags;
    olint_t order = 5;
    jiukun_cache_param_t jcp;
    jiukun_cache_t * cache;
    void * object;

    ol_printf("get jiukun page memory with wait: ");
    u32Ret = allocJiukunPage((void **)&page, order, 0);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("success\n");
        freeJiukunPage((void **)&page);
    }
    else
    {
        ol_printf("fail\n");
        return u32Ret;
    }

    ol_printf("get jiukun page with nowait: ");
    INIT_FLAG(flags);
    SET_FLAG(flags, PAF_NOWAIT);
    u32Ret = allocJiukunPage((void **)&page, 10, flags);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("success\n");
        freeJiukunPage((void **)&page);
    }
    else
    {
        ol_printf("fail\n");
    }

    ol_printf("create jiukun cache: ");
    memset(&jcp, 0, sizeof(jiukun_cache_param_t));
    jcp.jcp_pstrName = "jiukun-test";
    jcp.jcp_sObj = 16;
    SET_FLAG(jcp.jcp_fCache, JC_FLAG_ZERO);

    u32Ret = createJiukunCache(&cache, &jcp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = allocObject(cache, &object, 0);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("success\n");
            freeObject(cache, &object);
        }
        else
        {
            ol_printf("fail\n");
        }

        destroyJiukunCache(&cache);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jiukun_param_t jp;
    logger_param_t lpParam;

    memset(&lpParam, 0, sizeof(logger_param_t));

    lpParam.lp_pstrCallerName = "JIUKUN-TEST";
//    lpParam.lp_pstrLogFilePath = "jiukun-test.log";
    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = LOGGER_TRACE_DATA;

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&jp, 0, sizeof(jiukun_param_t));
        jp.jp_sPool = MAX_JIUKUN_POOL_SIZE;
        jp.jp_bNoGrow = TRUE;

        u32Ret = initJiukun(&jp);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (ls_bMultiThread)
                u32Ret = _testJiukunInThread();
            else if (ls_bStress)
                u32Ret = _stressJiukun();
            else
                u32Ret = _baseJiukunFunc();

            finiJiukun();
        }
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


