/**
 *  @file dispatcher/daemon/main.c
 *
 *  @brief The main file of dispatcher service
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
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_option.h"

#include "dispatcher.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bForeground = FALSE;

static olchar_t ls_strProgramName[64];
static olchar_t * ls_pstrVersion = "1.0.0";

/* --- private routine section ------------------------------------------------------------------ */

static void _printDispatcherUsage(void)
{
    ol_printf("\
Usage: %s [-f] [-s config dir] [-V] [logger options]\n\
    -f running in foreground.\n\
    -s specify the directory containing configuration file.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error only, 2: info, 3: debug.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_strProgramName);

    ol_printf("\n");
}

static u32 _parseDispatcherCmdLineParam(
    olint_t argc, olchar_t ** argv, 
    dispatcher_param_t * pdp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "fs:VT:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            ls_bForeground = TRUE;
            break;
        case 's':
            pdp->dp_pstrConfigDir = optarg;
            break;
        case '?':
        case 'h':
            _printDispatcherUsage();
            exit(0);
        case 'V':
            ol_printf("%s %s\n", ls_strProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(optarg, &pjlip->jlip_sLogFile);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _initAndStartDispatcher(dispatcher_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = initDispatcher(pdp);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcher();

    finiDispatcher();

    return u32Ret;
}

static u32 _serviceDispatcher(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_param_t dp;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    jf_file_getFileName(ls_strProgramName, sizeof(ls_strProgramName), argv[0]);

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "DISPATCHER";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;
    jlipParam.jlip_bLogToStdout = TRUE;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    setDefaultDispatcherParam(&dp);
    dp.dp_pstrCmdLine = argv[0];

    u32Ret = _parseDispatcherCmdLineParam(argc, argv, &dp, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (! ls_bForeground)
            u32Ret = jf_process_switchToDaemon();
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_process_isAlreadyRunning(ls_strProgramName))
        {
            fprintf(stderr, "Another %s is running\n", ls_strProgramName);
            u32Ret = JF_ERR_ALREADY_RUNNING;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Init the logger.*/
        jf_logger_init(&jlipParam);

        /*Init jiukun library for memory allocation.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Init socket library.*/
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _initAndStartDispatcher(&dp);

                jf_process_finiSocket();
            }

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "quit dispatcher");
        
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _serviceDispatcher(argc, argv);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

