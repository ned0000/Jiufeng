/**
 *  @file cli/main.c
 *
 *  @brief The main file of CLI utility.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_string.h"
#include "jf_err.h"
#include "jf_clieng.h"
#include "jf_jiukun.h"
#include "jf_file.h"
#include "jf_option.h"

#include "clicmd.h"
#include "main.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jiufeng_cli_master_t ls_jcmCliMaster;

static olchar_t ls_strCliProgramName[64];

static const olchar_t * ls_pstrCliVersion = "1.0.0";

static const olchar_t * ls_pstrCliBuildData = "2/30/2018";

/* --- private routine section ------------------------------------------------------------------ */

static void _printCliUsage(void)
{
    ol_printf("\
Usage: %s [-s command] [-o output-file] [-h] [logger options] \n\
  -s: specify the input command for script engine.\n\
  -o: specify the file to save output for script engine. By default, the output is written to\n\
      stdout.\n\
  -h: print the usage.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n",
              ls_strCliProgramName);

    ol_printf("\n");
}

static u32 _parseCliCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_clieng_init_param_t * pjcip, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = 0;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "s:o:T:F:S:Oh")) != -1))

    {
        switch (nOpt)
        {
        case 's':
            pjcip->jcip_bEnableScriptEngine = TRUE;
            ol_strncpy(pjcip->jcip_strInputCmd, jf_option_getArg(), JF_CLIENG_MAX_COMMAND_LINE_LEN);
            pjcip->jcip_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_LEN - 1] = JF_STRING_NULL_CHAR;
            break;
        case 'o':
            pjcip->jcip_pstrOutputFile = jf_option_getArg();
            break;
        case '?':
        case 'h':
            _printCliUsage();
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

static u32 _printShellGreeting(void * pMaster)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("--------------------------------------------------------------------");
    jf_clieng_outputLine("Jiufeng Command Line Interface (CLI) Utility");
    jf_clieng_outputLine("Version: %s Build Date: %s", ls_pstrCliVersion, ls_pstrCliBuildData);
    jf_clieng_outputLine("--------------------------------------------------------------------");

    return u32Ret;
}

static u32 _initAndRunJfCliEngine(
    jiufeng_cli_master_t * pjcmCliEngine, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_param_t cliParam;

    u32Ret = jf_clieng_init(pjcip);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = addCmd(pjcmCliEngine, &cliParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_clieng_run();
    }

    jf_clieng_fini();

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_clieng_init_param_t jcip;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    jf_file_getFileName(ls_strCliProgramName, sizeof(ls_strCliProgramName), argv[0]);

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "JF_CLI";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_INFO;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    ol_bzero(&jcip, sizeof(jcip));
    ol_strcpy(jcip.jcip_strCliName, "Jiufeng CLI");
    jcip.jcip_pstrNewLine = "\n";
    jcip.jcip_pMaster = &ls_jcmCliMaster;
    jcip.jcip_fnPrintGreeting = _printShellGreeting;

    u32Ret = _parseCliCmdLineParam(argc, argv, &jcip, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _initAndRunJfCliEngine(&ls_jcmCliMaster, &jcip);

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        olchar_t strErrMsg[300];

        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
