/**
 *  @file configmgr/daemon/main.c
 *
 *  @brief The main file of config manager service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The service needs to be started with root privilege otherwise the PID file will fail to be
 *   created in directory "/var/run".
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

#include "configmgr.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The name of the executable file for this service.
 */
static olchar_t ls_strConfigMgrProgramName[64];

/** The version of this service.
 */
static olchar_t * ls_pstrConfigMgrVersion = "1.0.0";

#if defined(LINUX)
    #define SERVICE_RETURN_VALUE  int
    #define SERVICE_RETURN(value) return value
#elif defined(WINDOWS)
    #define SERVICE_RETURN_VALUE  void
    #define SERVICE_RETURN(value) return
#endif

/* --- private routine section ------------------------------------------------------------------ */

static void _printConfigMgrUsage(void)
{
    ol_printf("\
Usage: %s [-s setting file] [-V] [logger options]\n\
    -s specify the setting file.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3> the log level. 0: no log, 1: error only, 2: info, 3: all, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_strConfigMgrProgramName);

    ol_printf("\n");

}

static u32 _parseConfigMgrCmdLineParam(
    olint_t argc, olchar_t ** argv, config_mgr_param_t * pcmp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "s:VT:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            pcmp->cmp_pstrSettingFile = optarg;
            break;
        case '?':
        case 'h':
            _printConfigMgrUsage();
            exit(0);
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strConfigMgrProgramName, ls_pstrConfigMgrVersion);
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

static u32 _initAndStartConfigMgr(config_mgr_param_t * pcmp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Initialize the config manager service.*/
    u32Ret = initConfigMgr(pcmp);

    /*Start the config manager service.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startConfigMgr();

    /*Finalize the config manager service.*/
    finiConfigMgr();

    return u32Ret;
}

static u32 _serviceConfigMgr(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    config_mgr_param_t cmp;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    /*Get programe name of the service.*/
    jf_file_getFileName(ls_strConfigMgrProgramName, sizeof(ls_strConfigMgrProgramName), argv[0]);

    /*Initialize the parameter for logger library.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "CONFIGMGR";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_bLogToFile = TRUE;

    /*Initialize the parameter for jiukun library.*/
    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    setDefaultConfigMgrParam(&cmp);
    cmp.cmp_pstrCmdLine = argv[0];

    /*Parse the parameter.*/
    u32Ret = _parseConfigMgrCmdLineParam(argc, argv, &cmp, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check if the service is alreay started.*/
        if (jf_process_isAlreadyRunning(ls_strConfigMgrProgramName))
        {
            ol_fprintf(stderr, "Another %s is running\n", ls_strConfigMgrProgramName);
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
                u32Ret = _initAndStartConfigMgr(&cmp);

                jf_process_finiSocket();
            }

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        JF_LOGGER_ERR(u32Ret, "quit configmgr");
        
    return u32Ret;
}

SERVICE_RETURN_VALUE _serviceMain(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    u32Ret = _serviceConfigMgr(argc, argv);
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

