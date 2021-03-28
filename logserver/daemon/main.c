/**
 *  @file logserver/daemon/main.c
 *
 *  @brief The main file of log server service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The service needs to be started with root privilege otherwise the PID file will fail to be
 *   created in directory "/var/run".
 *  -# The logs of the service itself can only be outputted to file. The logs from other module
 *   can be outputted to stdout, file (not the log file of this service) or tty.
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
#include "jf_time.h"

#include "logserver.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The name of the executable file for this service.
 */
static olchar_t ls_strLogServerProgramName[64];

/** The version of this service.
 */
static olchar_t * ls_pstrLogServerVersion = "1.0.0";

#if defined(LINUX)
    #define SERVICE_RETURN_VALUE  int
    #define SERVICE_RETURN(value) return value
#elif defined(WINDOWS)
    #define SERVICE_RETURN_VALUE  void
    #define SERVICE_RETURN(value) return
#endif

/* --- private routine section ------------------------------------------------------------------ */

static void _printLogServerUsage(void)
{
    ol_printf("\
Usage: %s [-a address] [-p port] [-o] [-f log file] [-s log file size] [-t tty file] \n\
  [-c max-client] [-V] [logger options]\n\
  -a: the address on which the log server should listen, by default, it's set to accept any\n\
      incoming messages.\n\
  -p: the port of the log server, default port is %d.\n\
  -c: maximum number of log client supported.\n\
  -o: output the log to stdout.\n\
  -f: output the log to file, default log file is \"jiufeng.log\".\n\
  -s: the size of log file. No limit if not specified.\n\
  -t: output the log to tty.\n\
  -V: show version information.\n\
logger options: [-T <0|1|2|3|4|5>] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -F: output the log to file, default log file is \"%s.log\".\n\
  -S: size of log file. No limit if not specified.\n",
              ls_strLogServerProgramName, JF_LOGGER_DEFAULT_SERVER_PORT, LOG_SERVER_NAME);

    ol_printf("\n");

}

static u32 _parseLogServerCmdLineParam(
    olint_t argc, olchar_t ** argv, log_server_init_param_t * plsip, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "a:p:c:of:s:t:VT:F:S:h")) != -1))
    {
        switch (nOpt)
        {
        case 'a':
            plsip->lsip_pstrServerAddress = jf_option_getArg();
            break;
        case 'p':
            u32Ret = jf_option_getU16FromString(jf_option_getArg(), &plsip->lsip_u16ServerPort);
            break;
        case 'c':
            u32Ret = jf_option_getU16FromString(jf_option_getArg(), &plsip->lsip_u16MaxLogClient);
            break;
        case 'o':
            plsip->lsip_bLogToStdout = TRUE;
            break;
        case 'f':
            plsip->lsip_bLogToFile = TRUE;
            plsip->lsip_pstrLogFile = jf_option_getArg();
            break;
        case 's':
            u32Ret = jf_option_getS32FromString(jf_option_getArg(), &plsip->lsip_sLogFile);
            break;
        case 't':
            plsip->lsip_bLogToTty = TRUE;
            plsip->lsip_pstrTtyFile = jf_option_getArg();
            break;
        case ':':
        case '?':
        case 'h':
            _printLogServerUsage();
            exit(0);
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strLogServerProgramName, ls_pstrLogServerVersion);
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
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

static u32 _initAndStartLogServer(log_server_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Initialize the log server service.*/
    u32Ret = initLogServer(plsip);

    /*Start the log server service.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startLogServer();

    /*Log server is stopped in the signal handler. Sleep several second to wait for the finish of
      network chain and log save thread.*/
    jf_time_sleep(3);

    /*Finalize the log server service.*/
    finiLogServer();

    return u32Ret;
}

static u32 _serviceLogServer(olint_t argc, char** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_server_init_param_t lsip;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    /*Get program name of the service.*/
    jf_file_getFileName(ls_strLogServerProgramName, sizeof(ls_strLogServerProgramName), argv[0]);

    /*Initialize the parameter for logger library.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = LOG_SERVER_NAME;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;
    jlipParam.jlip_bLogToFile = TRUE;

    /*Initialize the parameter for jiukun library.*/
    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    /*Set default parameter for log server.*/
    setDefaultLogServerParam(&lsip);
    lsip.lsip_pstrCmdLine = argv[0];
    lsip.lsip_bLogToFile = TRUE;

    /*Parse the parameter.*/
    u32Ret = _parseLogServerCmdLineParam(argc, argv, &lsip, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check if the service is alreay started.*/
        if (jf_process_isAlreadyRunning(ls_strLogServerProgramName))
        {
            ol_fprintf(stderr, "Another %s is running\n", ls_strLogServerProgramName);
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

            /*Initialize and start log server.*/
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _initAndStartLogServer(&lsip);

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
        JF_LOGGER_ERR(u32Ret, "quit log server");
        
    return u32Ret;
}

SERVICE_RETURN_VALUE _serviceMain(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    u32Ret = _serviceLogServer(argc, argv);
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

