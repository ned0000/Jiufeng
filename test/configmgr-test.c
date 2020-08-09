/**
 *  @file configmgr-test.c
 *
 *  @brief Test file for config manager.
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
#include "jf_config.h"
#include "jf_jiukun.h"
#include "jf_option.h"
#include "jf_file.h"
#include "jf_time.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** List config.
 */
static boolean_t ls_bConfigMgrTestListConfig = FALSE;

/** Get config operation.
 */
static boolean_t ls_bConfigMgrTestGetConfig = FALSE;

/** Set config operation.
 */
static boolean_t ls_bConfigMgrTestSetConfig = FALSE;

/** The config name string.
 */
static olchar_t * ls_pstrConfigMgrTestConfigName = NULL;

/** The config value string.
 */
static olchar_t * ls_pstrConfigMgrTestConfigValue = NULL;

/** Use transaction.
 */
static boolean_t ls_bConfigMgrTestTransaction = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printConfigMgrTestUsage(void)
{
    ol_printf("\
Usage: configmgr-test [-l] [-g] [-s] [-n config-name] [-v config-value] [-t] [logger options]\n\
  -l: list all configs.\n\
  -g: get config.\n\
  -s: set config.\n\
  -n: specify the config name.\n\
  -v: specify the config value.\n\
  -t: test the transaction.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
  -O: output the log to stdout.\n\
  -F: the log file.\n\
  -S: the size of log file. No limit if not specified.\n");

    ol_printf("\n");

}

static u32 _parseConfigMgrTestCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "lgsn:v:tOT:F:S:h?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printConfigMgrTestUsage();
            exit(0);
            break;
        case 'g':
            ls_bConfigMgrTestGetConfig = TRUE;
            break;
        case 's':
            ls_bConfigMgrTestSetConfig = TRUE;
            break;
        case 'l':
            ls_bConfigMgrTestListConfig = TRUE;
            break;
        case 'n':
            ls_pstrConfigMgrTestConfigName = optarg;
            break;
        case 't':
            ls_bConfigMgrTestTransaction = TRUE;
            break;
        case 'v':
            ls_pstrConfigMgrTestConfigValue = optarg;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
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

static u32 _listConfigMgrTestConfig(void)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

static u32 _getConfigMgrTestConfig(olchar_t * pstrName)
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

        u32Ret = jf_config_get(JF_CONFIG_INVALID_TRANSACTION_ID, pstrName, value, sizeof(value));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Name : %s\n", pstrName);
        ol_printf("Value: %s\n", value);
    }

    return u32Ret;
}

static u32 _setConfigMgrTestConfig(olchar_t * pstrName, olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrName == NULL)
    {
        ol_printf("Config name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_config_set(JF_CONFIG_INVALID_TRANSACTION_ID, pstrName, pstrValue);
    }

    return u32Ret;
}

static u32 _testConfigMgrCommitTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32TransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;
    olchar_t * pstrName = "system.aa";
    olchar_t strValue[128];

    ol_printf("Start transaction\n");

    u32Ret = jf_config_startTransaction(&u32TransactionId);

    if (u32Ret == JF_ERR_NO_ERROR)
    {

        u32Ret = jf_config_set(u32TransactionId, pstrName, "88");

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, pstrName, strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, "sysctl.cc", strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_set(u32TransactionId, "sysctl.aa", "nb");

        ol_printf("Commit transaction, id: %u\n", u32TransactionId);

        jf_config_commitTransaction(u32TransactionId);
    }

    return u32Ret;
}

static u32 _testConfigMgrRollbackTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32TransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;
    olchar_t * pstrName = "system.aa";
    olchar_t strValue[128];

    ol_printf("Start transaction\n");

    u32Ret = jf_config_startTransaction(&u32TransactionId);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_config_set(u32TransactionId, pstrName, "88");

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, pstrName, strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, "sysctl.cc", strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_set(u32TransactionId, "sysctl.aa", "nb");

        ol_printf("Rollback transaction, id: %u\n", u32TransactionId);

        jf_config_rollbackTransaction(u32TransactionId);
    }

    return u32Ret;
}

static u32 _testConfigMgrTimeoutTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32TransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;
    olchar_t * pstrName = "system.aa";
    olchar_t strValue[128];

    ol_printf("Start transaction\n");

    u32Ret = jf_config_startTransaction(&u32TransactionId);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_config_set(u32TransactionId, pstrName, "88");

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, pstrName, strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_get(u32TransactionId, "sysctl.cc", strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_config_set(u32TransactionId, "sysctl.aa", "nb");

        jf_time_sleep(JF_CONFIG_DEFAULT_TRANSACTION_TIME_OUT + 1);

        u32Ret = jf_config_commitTransaction(u32TransactionId);

        ol_printf(
            "Commit time out transaction, id: %u, %s\n", u32TransactionId,
            jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

static u32 _testConfigMgrTransaction(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _testConfigMgrRollbackTransaction();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testConfigMgrCommitTransaction();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testConfigMgrTimeoutTransaction();

    return u32Ret;
}

static u32 _processConfigMgrTestCommand(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_config_init_param_t jcip;

    ol_bzero(&jcip, sizeof(jcip));

    /*Initialize the service library.*/
    u32Ret = jf_config_init(&jcip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bConfigMgrTestGetConfig)
        {
            u32Ret = _getConfigMgrTestConfig(ls_pstrConfigMgrTestConfigName);
        }
        else if (ls_bConfigMgrTestSetConfig)
        {
            u32Ret = _setConfigMgrTestConfig(
                ls_pstrConfigMgrTestConfigName, ls_pstrConfigMgrTestConfigValue);
        }
        else if (ls_bConfigMgrTestListConfig)
        {
            u32Ret = _listConfigMgrTestConfig();
        }
        else if (ls_bConfigMgrTestTransaction)
        {
            u32Ret = _testConfigMgrTransaction();
        }
        else
        {
            ol_printf("Operation is not specified!\n");
            _printConfigMgrTestUsage();
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

    /*Initialize the logger parameter.*/
    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "CONFIGMGR-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseConfigMgrTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the logger library.*/
        jf_logger_init(&jlipParam);

        /*Initialize the jiukun library.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Process the config manager test command.*/
            u32Ret = _processConfigMgrTestCommand();

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
