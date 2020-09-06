/**
 *  @file thread-test.c
 *
 *  @brief test file for thread operation defined in jf_thread object.
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
#include "jf_thread.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bToTerminateThreadTest = FALSE;

static boolean_t ls_bThreadTestCreate = FALSE;

static boolean_t ls_bThreadTestBasic = FALSE;

#define MAX_THREAD_COUNT  5

/* --- private routine section ------------------------------------------------------------------ */

static void _printThreadTestUsage(void)
{
    ol_printf("\
Usage: thread-test [-c] [-b] [-h] \n\
  -c: test thread create.\n\
  -b: test basic thread function.\n\
  -h: print this usage.");

    ol_printf("\n");
}

static u32 _parseThreadTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "cbh")) != -1))
    {
        switch (nOpt)
        {
        case 'c':
            ls_bThreadTestCreate = TRUE;
            break;
        case 'b':
            ls_bThreadTestBasic = TRUE;
            break;
        case ':':
        case '?':
        case 'h':
            _printThreadTestUsage();
            exit(0);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _threadTestThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Id = *(u32 *)pArg;

    ol_printf("thread %u starts\n", u32Id);

    while ((! ls_bToTerminateThreadTest) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("thread %u shout\n", u32Id);
        ol_sleep(5);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("thread %u quits\n", u32Id);

    JF_THREAD_RETURN(u32Ret);
}

static u32 _threadTestCreate(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;
    u32 u32Id[MAX_THREAD_COUNT];

    for (u32Index = 0;
         ((u32Index < MAX_THREAD_COUNT) && (u32Ret == JF_ERR_NO_ERROR));
         u32Index ++)
    {
        u32Id[u32Index] = u32Index + 1;
        ol_printf("main tread, create thread %u\n", u32Id[u32Index]);
        u32Ret = jf_thread_create(NULL, NULL, _threadTestThread, &u32Id[u32Index]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("main thread, sleeping for 5 minutes\n");
        ol_sleep(300);
        ol_printf("prepare to exit\n");
    }

    ls_bToTerminateThreadTest = TRUE;

    ol_sleep(20);

    ol_printf("main thread quits\n");

    return u32Ret;
}

static u32 _threadTestBasic(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bRet = FALSE;
    jf_thread_id_t id;

    ol_printf("thread id: %ld\n", jf_thread_getCurrentId());

    jf_thread_initId(&id);
    bRet = jf_thread_isValidId(&id);
    if (bRet)
        ol_printf("valid thread id\n");
    else
        ol_printf("invalid thread id\n");


    return u32Ret;
}

static void _threadTestSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "THREAD-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    u32Ret = _parseThreadTestCmdLineParam(argc, argv, &jlipParam);

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_registerSignalHandlers(_threadTestSignalHandler);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        if (ls_bThreadTestCreate)
            u32Ret = _threadTestCreate();
        else if (ls_bThreadTestBasic)
            u32Ret = _threadTestBasic();
        else
            _printThreadTestUsage();

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

