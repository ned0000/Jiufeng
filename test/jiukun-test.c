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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_thread.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define MAX_JIUKUN_TEST_ORDER  (10)

typedef enum
{
    TEST_JIUKUN_TARGET_UNKNOWN = 0,
    TEST_JIUKUN_TARGET_PAGE,
    TEST_JIUKUN_TARGET_MEMORY,
    TEST_JIUKUN_TARGET_OBJECT,
} test_jiukun_target_t;

#define MAX_THREAD_COUNT  2

static boolean_t ls_bToTerminate = FALSE;

boolean_t ls_bMultiThread = FALSE;
boolean_t ls_bStress = FALSE;
boolean_t ls_bOutOfBound = FALSE;

u8 ls_u8TestTarget = TEST_JIUKUN_TARGET_UNKNOWN;

boolean_t ls_bDoubleFree = FALSE;
boolean_t ls_bUnallocatedFree = FALSE;
boolean_t ls_bAllocateWithoutFree = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUsage(void)
{
    ol_printf("\
Usage: jiukun-test [-t] [-j page|memory|object] [stress testing option] [allocate without free] \n\
    [double free option] [unallocated free option] [out of bound option] [logger options]\n\
    -t test in multi-threading environment.\n\
    -j specify the test target.\n\
double free option:\n\
    -d test double free.\n\
unallocated free option:\n\
    -u test unallocated free.\n\
allocate without free:\n\
    -w test allocate without free.\n\
out of bound option:\n\
    -b test out of bound.\n\
stress testing option:\n\
    -s stress testing.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <trace file size> the size of log file. No limit if not specified.\n\
    If no parameter is specified, jiukun basic function is tested.\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "bwj:tsduOT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
            break;
        case 'j':
            if (ol_strcmp(optarg, "page") == 0)
                ls_u8TestTarget = TEST_JIUKUN_TARGET_PAGE;
            else if (ol_strcmp(optarg, "memory") == 0)
                ls_u8TestTarget = TEST_JIUKUN_TARGET_MEMORY;
            else if (ol_strcmp(optarg, "object") == 0)
                ls_u8TestTarget = TEST_JIUKUN_TARGET_OBJECT;
            break;
        case 't':
            ls_bMultiThread = TRUE;
            break;
        case 's':
            ls_bStress = TRUE;
            break;
        case 'b':
            ls_bOutOfBound = TRUE;
            break;
        case 'd':
            ls_bDoubleFree = TRUE;
            break;
        case 'u':
            ls_bUnallocatedFree = TRUE;
            break;
        case 'w':
            ls_bAllocateWithoutFree = TRUE;
            break;
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_sLogFile = u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

#define JIUKUN_STRESS_TEST_LOOP_COUNT  10000

u32 _testAllocMem(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Mem[JIUKUN_STRESS_TEST_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop, u32Size;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    memset(pu8Mem, 0, sizeof(u8 *) * JIUKUN_STRESS_TEST_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();

    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            jf_logger_logInfoMsg("Allocate memory");
        }
        else
        {
            bAlloc = FALSE;
            jf_logger_logInfoMsg("free memory");
        }

        nRand = rand();
        u32Index = nRand % JIUKUN_STRESS_TEST_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();
                u32Size = nRand % JF_JIUKUN_MAX_MEMORY_SIZE;

                jf_logger_logInfoMsg("!!!! Allocate %u", u32Size);

                u32Ret = jf_jiukun_allocMemory((void **)&(pu8Mem[u32Index]), u32Size, 0);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    jf_logger_logInfoMsg("success, at %u, %p\n", u32Index,
                        pu8Mem[u32Index]);
#if defined(DEBUG_JIUKUN)
                    dumpJiukun();
#endif
                }
                else
                {
                    jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    jf_logger_logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    jf_logger_logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    jf_jiukun_freeMemory((void **)&(pu8Mem[u32Idx]));

                    pu8Mem[u32Idx] = NULL;
#if defined(DEBUG_JIUKUN)
                    jf_jiukun_dump();
#endif
                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            jf_jiukun_freeMemory((void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

u32 _testJiukunPage(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Mem[JIUKUN_STRESS_TEST_LOOP_COUNT];
    u32 u32Order[JIUKUN_STRESS_TEST_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    ol_memset(pu8Mem, 0, sizeof(u8 *) * JIUKUN_STRESS_TEST_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();
#if defined(DEBUG_JIUKUN)
    dumpJiukun();
#endif
    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            jf_logger_logInfoMsg("Allocate page");
        }
        else
        {
            bAlloc = FALSE;
            jf_logger_logInfoMsg("free page");
        }

        nRand = rand();
        u32Index = nRand % JIUKUN_STRESS_TEST_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();
                u32Order[u32Index] = nRand % MAX_JIUKUN_TEST_ORDER;

                u32Ret = jf_jiukun_allocPage(
                    (void **)&(pu8Mem[u32Index]), u32Order[u32Index], 0);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
#if defined(DEBUG_JIUKUN)
                    jf_jiukun_dump();
#endif
                    jf_logger_logInfoMsg("success, at %u, %p\n", u32Index, pu8Mem[u32Index]);
                }
                else
                {
                    jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    jf_logger_logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    jf_logger_logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    jf_jiukun_freePage((void **)&(pu8Mem[u32Idx]));
#if defined(DEBUG_JIUKUN)
                    jf_jiukun_dump();
#endif
                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            jf_jiukun_freePage((void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

#define TEST_CACHE "test-cache"
static jf_jiukun_cache_t * ls_pacCache = NULL;

u32 _testJiukunCache(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Mem[JIUKUN_STRESS_TEST_LOOP_COUNT];
    u32 u32Index = 0, u32Idx, u32Loop;
    olchar_t strErrMsg[300];
    olint_t nRand;
    boolean_t bAlloc;

    memset(pu8Mem, 0, sizeof(u8 *) * JIUKUN_STRESS_TEST_LOOP_COUNT);
    srand(time(NULL));

    nRand = rand();

    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
    {
        nRand = rand();

        if ((nRand % 3) != 0)
        {
            bAlloc = TRUE;
            jf_logger_logInfoMsg("Allocate object");
        }
        else
        {
            bAlloc = FALSE;
            jf_logger_logInfoMsg("free object");
        }

        nRand = rand();
        u32Index = nRand % JIUKUN_STRESS_TEST_LOOP_COUNT;

        if (bAlloc)
        {
            if (pu8Mem[u32Index] == NULL)
            {
                nRand = rand();

                u32Ret = jf_jiukun_allocObject(
                    ls_pacCache, (void **)&(pu8Mem[u32Index]));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    jf_logger_logInfoMsg("success, at %u, %p\n", u32Index,
                        pu8Mem[u32Index]);
                }
                else
                {
                    jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
                    jf_logger_logInfoMsg("failed, %s\n", strErrMsg);
                }
            }
        }
        else
        {
            for (u32Idx = u32Index; u32Idx < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Idx ++)
                if (pu8Mem[u32Idx] != NULL)
                {
                    jf_logger_logInfoMsg("!!!! free at %u, %p", u32Idx, pu8Mem[u32Idx]);

                    jf_jiukun_freeObject(ls_pacCache, (void **)&(pu8Mem[u32Idx]));

                    pu8Mem[u32Idx] = NULL;

                    break;
                }
        }
    }

    for (u32Loop = 0; u32Loop < JIUKUN_STRESS_TEST_LOOP_COUNT; u32Loop ++)
        if (pu8Mem[u32Loop] != NULL)
            jf_jiukun_freeObject(ls_pacCache, (void **)&(pu8Mem[u32Loop]));

    return u32Ret;
}

static u32 _stressJiukun(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_u8TestTarget == TEST_JIUKUN_TARGET_PAGE)
        u32Ret = _testJiukunPage();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_MEMORY)
        u32Ret = _testAllocMem();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_OBJECT)
        u32Ret = _testJiukunCache();

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _allocFree(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Index = (u32)(ulong)pArg;

    jf_logger_logInfoMsg("alloc-free thread %u starts", u32Index);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_logger_logInfoMsg("alloc-free thread %u starts testing", u32Index);

        u32Ret = _testJiukunCache();
    }


    if (u32Ret == JF_ERR_NO_ERROR)
        jf_logger_logInfoMsg("alloc-free thread %u quits", u32Index);
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        jf_logger_logInfoMsg("alloc-free thread %u quits, %s", u32Index, strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

static u32 _testJiukunInThread(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;
    jf_jiukun_cache_create_param_t jjccp;

    ol_memset(&jjccp, 0, sizeof(jjccp));
    jjccp.jjccp_pstrName = TEST_CACHE;
    jjccp.jjccp_sObj = 28;

    u32Ret = jf_jiukun_createCache(&ls_pacCache, &jjccp);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0;
             ((u32Index < MAX_THREAD_COUNT) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            u32Ret = jf_thread_create(
                NULL, NULL, _allocFree, (void *)(ulong)(u32Index + 1));
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("main thread, sleeping for 5 minutes\n");
        sleep(300);
        ol_printf("prepare to exit\n");
    }

    ls_bToTerminate = TRUE;
    sleep(1);

    jf_jiukun_destroyCache(&ls_pacCache);

    return u32Ret;
}

static u32 _testJiukunDoubleFreePage(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = NULL, * pu8Temp = NULL;

    ol_printf("testing double free page\n");

    u32Ret = jf_jiukun_allocPage((void **)&pu8Buf, 1, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pu8Temp = pu8Buf;
        ol_printf("free the page\n");
        jf_jiukun_freePage((void **)&pu8Buf);

        ol_printf("double free the page\n");
        jf_jiukun_freePage((void **)&pu8Temp);
    }

    return u32Ret;
}

static u32 _testJiukunDoubleFreeMemory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = NULL, * pu8Temp = NULL;

    ol_printf("testing double free memory\n");

    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buf, 64, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pu8Temp = pu8Buf;
        ol_printf("free the memeory\n");
        jf_jiukun_freeMemory((void **)&pu8Buf);

        ol_printf("double free the memory\n");
        jf_jiukun_freeMemory((void **)&pu8Temp);
    }

    return u32Ret;
}

static u32 _testJiukunDoubleFreeObject(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_cache_create_param_t jjccp;
    jf_jiukun_cache_t * cache;
    void * object = NULL, * pTemp = NULL;

    ol_printf("testing double free object\n");

    ol_printf("create jiukun cache\n");
    ol_memset(&jjccp, 0, sizeof(jjccp));
    jjccp.jjccp_pstrName = "jiukun-test";
    jjccp.jjccp_sObj = 16;
    JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

    u32Ret = jf_jiukun_createCache(&cache, &jjccp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_jiukun_allocObject(cache, &object);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pTemp = object;

            ol_printf("free the object\n");
            jf_jiukun_freeObject(cache, &object);

            ol_printf("double free the object\n");
            jf_jiukun_freeObject(cache, &pTemp);
        }

        jf_jiukun_destroyCache(&cache);
    }

    return u32Ret;
}

static u32 _testJiukunDoubleFree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_u8TestTarget == TEST_JIUKUN_TARGET_PAGE)
        u32Ret = _testJiukunDoubleFreePage();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_MEMORY)
        u32Ret = _testJiukunDoubleFreeMemory();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_OBJECT)
        u32Ret = _testJiukunDoubleFreeObject();

    return u32Ret;
}

static u32 _testJiukunUnallocatedFreePage(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = (u8 *)0x7f7cbf437060;

    ol_printf("testing unallocated free page\n");

    ol_printf("free the unallocated page\n");
    jf_jiukun_freePage((void **)&pu8Buf);

    return u32Ret;
}

static u32 _testJiukunUnallocatedFreeMemory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = (u8 *)0x7fdfe85af0f0;

    ol_printf("testing unallocated free memory\n");

    ol_printf("free the unallocated memeory\n");
    jf_jiukun_freeMemory((void **)&pu8Buf);

    return u32Ret;
}

static u32 _testJiukunUnallocatedFreeObject(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_cache_create_param_t jjccp;
    jf_jiukun_cache_t * cache;
    void * object = (void *)0x7fd8f881f1f0;

    ol_printf("testing unallocated free object\n");

    ol_printf("create jiukun cache\n");
    ol_memset(&jjccp, 0, sizeof(jjccp));
    jjccp.jjccp_pstrName = "jiukun-test";
    jjccp.jjccp_sObj = 16;
    JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

    u32Ret = jf_jiukun_createCache(&cache, &jjccp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("free the unallocated object\n");
        jf_jiukun_freeObject(cache, &object);

        jf_jiukun_destroyCache(&cache);
    }

    return u32Ret;
}

static u32 _testJiukunUnallocatedFree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_u8TestTarget == TEST_JIUKUN_TARGET_PAGE)
        u32Ret = _testJiukunUnallocatedFreePage();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_MEMORY)
        u32Ret = _testJiukunUnallocatedFreeMemory();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_OBJECT)
        u32Ret = _testJiukunUnallocatedFreeObject();

    return u32Ret;
}

static u32 _testJiukunAllocateWithoutFreePage(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = NULL;

    ol_printf("testing allocate without free page\n");

    ol_printf("allocate page\n");
    u32Ret = jf_jiukun_allocPage((void **)&pu8Buf, 1, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _testJiukunAllocateWithoutFreeMemory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buf = NULL;

    ol_printf("testing allocate without free memory\n");

    ol_printf("allocate memory\n");
    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buf, 64, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _testJiukunAllocateWithoutFreeObject(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_cache_create_param_t jjccp;
    jf_jiukun_cache_t * cache;
    void * object = NULL;

    ol_printf("testing allocate without free object\n");

    ol_printf("create jiukun cache\n");
    ol_memset(&jjccp, 0, sizeof(jjccp));
    jjccp.jjccp_pstrName = "jiukun-test";
    jjccp.jjccp_sObj = 16;
    JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

    u32Ret = jf_jiukun_createCache(&cache, &jjccp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("allocate object\n");

        u32Ret = jf_jiukun_allocObject(cache, &object);
        if (u32Ret == JF_ERR_NO_ERROR)
        {

        }

        ol_printf("destroy cache\n");
        jf_jiukun_destroyCache(&cache);
    }

    return u32Ret;
}

static u32 _testJiukunAllocateWithoutFree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_u8TestTarget == TEST_JIUKUN_TARGET_PAGE)
        u32Ret = _testJiukunAllocateWithoutFreePage();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_MEMORY)
        u32Ret = _testJiukunAllocateWithoutFreeMemory();
    else if (ls_u8TestTarget == TEST_JIUKUN_TARGET_OBJECT)
        u32Ret = _testJiukunAllocateWithoutFreeObject();

    return u32Ret;
}

static u32 _testJiukunOutOfBound(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    void * pMem = NULL;
    olsize_t size = 32;

    ol_printf("allocate jiukun memory\n");
    u32Ret = jf_jiukun_allocMemory((void **)&pMem, size, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pMem, size);
        jf_jiukun_memcpy(pMem, "12345678901234567890123456789012", 33);

        ol_printf("mem: %s\n", (olchar_t *)pMem);

        jf_jiukun_freeMemory((void **)&pMem);
    }

    return u32Ret;
}

static u32 _baseJiukunFunc(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * page = NULL;
    olint_t order = 5;
    jf_jiukun_cache_create_param_t jjccp;
    jf_jiukun_cache_t * cache;
    void * object = NULL, * pMem = NULL;

    ol_printf("get jiukun page: ");
    u32Ret = jf_jiukun_allocPage((void **)&page, order, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("success\n");
        jf_jiukun_freePage((void **)&page);
    }
    else
    {
        ol_printf("fail\n");
        return u32Ret;
    }

    ol_printf("allocate jiukun memory: ");
    u32Ret = jf_jiukun_allocMemory((void **)&pMem, 10, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("success\n");
        jf_jiukun_freeMemory((void **)&pMem);
    }
    else
    {
        ol_printf("fail\n");
    }

    ol_printf("create jiukun cache: ");
    ol_memset(&jjccp, 0, sizeof(jjccp));
    jjccp.jjccp_pstrName = "jiukun-test";
    jjccp.jjccp_sObj = 16;
    JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

    u32Ret = jf_jiukun_createCache(&cache, &jjccp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_jiukun_allocObject(cache, &object);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("success\n");
            jf_jiukun_freeObject(cache, &object);
        }
        else
        {
            ol_printf("fail\n");
        }

        jf_jiukun_destroyCache(&cache);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));

    jlipParam.jlip_pstrCallerName = "JIUKUN-TEST";
//    jlipParam.jlip_pstrLogFilePath = "jiukun-test.log";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&jjip, 0, sizeof(jjip));
        jjip.jjip_sPool = (1 << MAX_JIUKUN_TEST_ORDER) * JF_JIUKUN_PAGE_SIZE;
//        jjip.jjip_bNoGrow = TRUE;

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bMultiThread)
                u32Ret = _testJiukunInThread();
            else if (ls_bStress)
                u32Ret = _stressJiukun();
            else if (ls_bDoubleFree)
                u32Ret = _testJiukunDoubleFree();
            else if (ls_bUnallocatedFree)
                u32Ret = _testJiukunUnallocatedFree();
            else if (ls_bAllocateWithoutFree)
                u32Ret = _testJiukunAllocateWithoutFree();
            else if (ls_bOutOfBound)
                u32Ret = _testJiukunOutOfBound();
            else
                u32Ret = _baseJiukunFunc();

            jf_jiukun_fini();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


