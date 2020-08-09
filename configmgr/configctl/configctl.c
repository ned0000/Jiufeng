/**
 *  @file configctl.c
 *
 *  @brief Utility for config control.
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
#include "jf_err.h"
#include "jf_config.h"
#include "jf_jiukun.h"
#include "jf_option.h"
#include "jf_file.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** List config.
 */
static boolean_t ls_bConfigCtlListConfig = FALSE;

/** Get config operation.
 */
static boolean_t ls_bConfigCtlGetConfig = FALSE;

/** Set config operation.
 */
static boolean_t ls_bConfigCtlSetConfig = FALSE;

/** The config name string.
 */
static olchar_t * ls_pstrConfigCtlConfigName = NULL;

/** Use transaction.
 */
static boolean_t ls_bTransaction = FALSE;

/** The transaction ID.
 */
static u32 ls_u32ConfigCtlTransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;

/** The config value string.
 */
static olchar_t * ls_pstrConfigCtlConfigValue = NULL;

/** The name of the executable file for this utility.
 */
static olchar_t ls_strConfigCtlProgramName[64];

/** The version of the program.
 */
static const olchar_t * ls_pstrConfigCtlVersion = "1.0.0";

/* --- private routine section ------------------------------------------------------------------ */

static void _printConfigCtlUsage(void)
{
    ol_printf("\
Usage: %s [-l] [-g] [-s] [-n config-name] [-v config-value] [-t] [-V] [logger options]\n\
  -l: list all configs.\n\
  -g: get config.\n\
  -s: set config.\n\
  -n: specify the config name.\n\
  -v: specify the config value.\n\
  -t: start a transaction to get or set config.\n\
  -V: show version information.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
  -O: output the log to stdout.\n\
  -F: the log file.\n\
  -S: the size of log file. No limit if not specified.\n",
              ls_strConfigCtlProgramName);

    ol_printf("\n");

}

static u32 _parseConfigCtlCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "lgsn:v:VOT:F:S:h?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printConfigCtlUsage();
            exit(0);
            break;
        case 'g':
            ls_bConfigCtlGetConfig = TRUE;
            break;
        case 's':
            ls_bConfigCtlSetConfig = TRUE;
            break;
        case 'l':
            ls_bConfigCtlListConfig = TRUE;
            break;
        case 'n':
            ls_pstrConfigCtlConfigName = optarg;
            break;
        case 't':
            ls_bTransaction = TRUE;
            break;
        case 'v':
            ls_pstrConfigCtlConfigValue = optarg;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'V':
            ol_printf("%s %s\n", ls_strConfigCtlProgramName, ls_pstrConfigCtlVersion);
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

static u32 _listConfigs(void)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

static u32 _getConfig(olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t value[JF_CONFIG_MAX_CONFIG_VALUE_LEN];

    if (pstrName == NULL)
    {
        ol_printf("Config name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(value, sizeof(value));

        u32Ret = jf_config_get(ls_u32ConfigCtlTransactionId, pstrName, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Name : %s\n", pstrName);
        ol_printf("Value: %s\n", value);
    }

    return u32Ret;
}

static u32 _setConfig(olchar_t * pstrName, olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrName == NULL)
    {
        ol_printf("Config name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (! ls_bTransaction)
        {
            u32Ret = jf_config_set(ls_u32ConfigCtlTransactionId, pstrName, pstrValue);
        }
        else
        {
            u32Ret = jf_config_startTransaction(&ls_u32ConfigCtlTransactionId);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_config_set(ls_u32ConfigCtlTransactionId, pstrName, pstrValue);

                jf_config_commitTransaction(ls_u32ConfigCtlTransactionId);
            }
        }
    }

    return u32Ret;
}

static u32 _processConfigCtlCommand(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_config_init_param_t jcip;

    ol_bzero(&jcip, sizeof(jcip));

    /*Initialize the service library.*/
    u32Ret = jf_config_init(&jcip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bConfigCtlGetConfig)
        {
            u32Ret = _getConfig(ls_pstrConfigCtlConfigName);
        }
        else if (ls_bConfigCtlSetConfig)
        {
            u32Ret = _setConfig(ls_pstrConfigCtlConfigName, ls_pstrConfigCtlConfigValue);
        }
        else if (ls_bConfigCtlListConfig)
        {
            u32Ret = _listConfigs();
        }
        else
        {
            ol_printf("Operation is not specified!\n");
            _printConfigCtlUsage();
            u32Ret = JF_ERR_INVALID_PARAM;
        }

        jf_config_fini();
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
    jf_file_getFileName(ls_strConfigCtlProgramName, sizeof(ls_strConfigCtlProgramName), argv[0]);

    /*Initialize the logger parameter.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "CONFIGCTL";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseConfigCtlCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the logger library.*/
        jf_logger_init(&jlipParam);

        /*Initialize the jiukun library.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Process the config control command.*/
            u32Ret = _processConfigCtlCommand();

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
