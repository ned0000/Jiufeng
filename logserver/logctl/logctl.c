/**
 *  @file logctl.c
 *
 *  @brief Utility for log control.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_option.h"
#include "jf_file.h"

#include "logservermsg.h"
#include "logservercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Get log server setting operation.
 */
static boolean_t ls_bGetLogServerSetting = FALSE;

/** The name of the executable file for this utility.
 */
static olchar_t ls_strLogCtlProgramName[64];

/** The version of the program.
 */
static const olchar_t * ls_pstrLogCtlVersion = "1.0.0";

/* --- private routine section ------------------------------------------------------------------ */

static void _printLogCtlUsage(void)
{
    ol_printf("\
Usage: %s [-g] [-V] [logger options]\n\
  -g: get log server setting.\n\
  -V: show version information.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: the log file.\n\
  -S: the size of log file. No limit if not specified.\n",
              ls_strLogCtlProgramName);

    ol_printf("\n");

}

static u32 _parseLogCtlCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "gVOT:F:S:h?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printLogCtlUsage();
            exit(0);
            break;
        case 'g':
            ls_bGetLogServerSetting = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strLogCtlProgramName, ls_pstrLogCtlVersion);
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
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

static u32 _getLogServerSetting(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static u32 _processLogCtlCommand(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ls_bGetLogServerSetting)
    {
        u32Ret = _getLogServerSetting();
    }
    else
    {
        ol_printf("Operation is not specified!\n");
        _printLogCtlUsage();
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    return u32Ret;
}
/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    /*Get programe name of the service.*/
    jf_file_getFileName(ls_strLogCtlProgramName, sizeof(ls_strLogCtlProgramName), argv[0]);

    /*Initialize the logger parameter.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "LOGCTL";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseLogCtlCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the logger library.*/
        jf_logger_init(&jlipParam);

        /*Initialize the jiukun library.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Process the service control command.*/
            u32Ret = _processLogCtlCommand();

            jf_jiukun_fini();
        }

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
