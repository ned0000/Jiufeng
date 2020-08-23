/**
 *  @file process-test.c
 *
 *  @brief test file for process operation defined in jf_process object.
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
#include "jf_process.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define PROCESS_TEST_NAME    "process-test"

static boolean_t ls_bProcessTestCreate = FALSE;

static boolean_t ls_bProcessTestBasic = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printProcessTestUsage(void)
{
    ol_printf("\
Usage: process-test [-c] [-b] [-h] \n\
  -c: test process create.\n\
  -b: test basic process function.\n\
  -h: print this usage.");

    ol_printf("\n");
}

static u32 _parseProcessTestCmdLineParam(
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
            ls_bProcessTestCreate = TRUE;
            break;
        case 'b':
            ls_bProcessTestBasic = TRUE;
            break;
        case ':':
        case '?':
        case 'h':
            _printProcessTestUsage();
            exit(0);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _processTestCreate(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    return u32Ret;
}

static u32 _processTestBasic(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strFile[512];
    boolean_t bRet = FALSE;
    jf_process_handle_t handle;

    ol_printf("process id: %d\n", jf_process_getCurrentId());

    jf_process_getPidFilename(strFile, sizeof(strFile), PROCESS_TEST_NAME);
    ol_printf("pid file: %s\n", strFile);

    jf_process_getCurrentWorkingDirectory(strFile, sizeof(strFile));
    ol_printf("current working directory: %s\n", strFile);

    bRet = jf_process_isAlreadyRunning(PROCESS_TEST_NAME);
    if (bRet)
        ol_printf("another process is running\n");
    else
        ol_printf("no other process is running\n");

    jf_process_initHandle(&handle);
    bRet = jf_process_isValidHandle(&handle);
    if (bRet)
        ol_printf("valid process handle\n");
    else
        ol_printf("invalid process handle\n");


    return u32Ret;
}

static void _processTestSignalHandler(olint_t signal)
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
    jlipParam.jlip_pstrCallerName = PROCESS_TEST_NAME;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    u32Ret = _parseProcessTestCmdLineParam(argc, argv, &jlipParam);

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_processTestSignalHandler);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        if (ls_bProcessTestCreate)
            u32Ret = _processTestCreate();
        else if (ls_bProcessTestBasic)
            u32Ret = _processTestBasic();
        else
            _printProcessTestUsage();

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
