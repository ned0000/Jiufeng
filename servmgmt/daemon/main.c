/**
 *  @file servmgmt/daemon/main.c
 *
 *  @brief The main file of dongyuan service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The service needs to be started with root privilege otherwise the PID file will fail to be
 *   created in directory "/var/run".
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_jiukun.h"
#include "jf_option.h"

#include "dongyuan.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The service should run in foregroud if it's TRUE, otherwise it runs in background as a daemon.
 */
static boolean_t ls_bDongyuanForeground = FALSE;

/** The name of the executable file for this service.
 */
static olchar_t ls_strDongyuanProgramName[64];

/** The version of this service.
 */
static olchar_t * ls_pstrDongyuanVersion = "1.0.0";

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
  -f: running in foreground.\n\
  -s: specify the setting file.\n\
  -V: show version information.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n",
           ls_strDongyuanProgramName);

    ol_printf("\n");

}

static u32 _parseDongyuanCmdLineParam(
    olint_t argc, olchar_t ** argv, dongyuan_param_t * pdp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = 0;

    while (((nOpt = getopt(argc, argv, "fs:VT:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'f':
            ls_bDongyuanForeground = TRUE;
            break;
        case 's':
            pdp->dp_pstrSettingFile = optarg;
            break;
        case '?':
        case 'h':
            _printDongyuanUsage();
            exit(0);
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strDongyuanProgramName, ls_pstrDongyuanVersion);
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = optarg;
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

static u32 _initAndStartDongyuan(dongyuan_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Initialize the dongyuan service.*/
    u32Ret = initDongyuan(pdp);

    /*Start the dongyuan service.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDongyuan();

    /*Finalize the dongyuan service.*/
    finiDongyuan();

    return u32Ret;
}

static u32 _serviceDongyuan(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dongyuan_param_t dp;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    /*Get programe name of the service.*/
    jf_file_getFileName(ls_strDongyuanProgramName, sizeof(ls_strDongyuanProgramName), argv[0]);

    /*Initialize the parameter for logger library.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = DONGYUAN_SERVER_NAME;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;
//    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_bLogToFile = TRUE;

    /*Initialize the parameter for jiukun library.*/
    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    /*Set default parameter of dongyuan service.*/
    setDefaultDongyuanParam(&dp);
    dp.dp_pstrCmdLine = argv[0];

    /*Parse the parameter.*/
    u32Ret = _parseDongyuanCmdLineParam(argc, argv, &dp, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Switch to daemon if necessary.*/
        if (! ls_bDongyuanForeground)
            u32Ret = jf_process_switchToDaemon();
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check if the service is alreay started.*/
        if (jf_process_isAlreadyRunning(ls_strDongyuanProgramName))
        {
            ol_fprintf(stderr, "Another %s is running\n", ls_strDongyuanProgramName);
            u32Ret = JF_ERR_ALREADY_RUNNING;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the logger.*/
        jf_logger_init(&jlipParam);

        /*Initialize jiukun library for memory allocation.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Initialize socket.*/
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*Initialize and start dongyuan.*/
                u32Ret = _initAndStartDongyuan(&dp);

                /*Finalize socket.*/
                jf_process_finiSocket();
            }

            /*Finalize jiukun library.*/
            jf_jiukun_fini();
        }

        /*Finalize logger library.*/
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

