/**
 *  @file dongyuan/main.c
 *
 *  @brief The main file of dongyuan service
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

#include "dongyuan.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bForeground = FALSE;

static olchar_t ls_strProgramName[64];
static olchar_t * ls_pstrVersion = "1.0.0";

#if defined(LINUX)
    #define SERVICE_RETURN_VALUE  int
    #define SERVICE_RETURN(value) return value
#elif defined(WINDOWS)
    #define SERVICE_RETURN_VALUE  void
    #define SERVICE_RETURN(value) return
#endif

/* --- private routine section ------------------------------------------------------------------ */
static void _printDongyuanUsage(void)
{
    ol_printf("\
Usage: %s [-f] [-s setting file] [-V] [logger options]\n\
    -f running in foreground.\n\
    -s specify the setting file.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_strProgramName);

    ol_printf("\n");

    exit(0);
}

static u32 _parseDongyuanCmdLineParam(
    olint_t argc, olchar_t ** argv, 
    dongyuan_param_t * pdp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "fs:VT:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            ls_bForeground = TRUE;
            break;
        case 's':
            pdp->dp_pstrSettingFile = optarg;
            break;
        case '?':
        case 'h':
            _printDongyuanUsage();
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strProgramName, ls_pstrVersion);
            exit(0);
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
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_sLogFile = u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _startDongyuan(dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = initDongyuan(pdp);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDongyuan();

    finiDongyuan();

    return u32Ret;
}

static u32 _serviceDongyuan(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dongyuan_param_t dp;
    jf_logger_init_param_t jlipParam;

    jf_file_getFileName(ls_strProgramName, sizeof(ls_strProgramName), argv[0]);

    ol_memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "DONGYUAN";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;
    jlipParam.jlip_bLogToStdout = TRUE;

    setDefaultDongyuanParam(&dp);
    dp.dp_pstrCmdLine = argv[0];

    u32Ret = _parseDongyuanCmdLineParam(argc, argv, &dp, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);
        jf_process_initSocket();

        if (! ls_bForeground)
            u32Ret = jf_process_switchToDaemon(ls_strProgramName);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (jf_process_isAlreadyRunning(ls_strProgramName))
            {
                fprintf(stderr, "Another %s is running\n", ls_strProgramName);
                u32Ret = JF_ERR_ALREADY_RUNNING;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _startDongyuan(&dp);

        jf_process_finiSocket();
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        jf_logger_logErrMsg(u32Ret, "quit dongyuan");
        
    return u32Ret;
}

SERVICE_RETURN_VALUE _serviceMain(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    u32Ret = _serviceDongyuan(argc, argv);
#endif

    SERVICE_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    u32Ret = _serviceMain(argc, argv);
#elif defined(WINDOWS)
    _serviceMain(argc, argv);
#endif

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

