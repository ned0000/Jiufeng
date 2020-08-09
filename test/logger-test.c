/**
 *  @file logger-test.c
 *
 *  @brief The test file for logger function defined in jf_logger library.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_option.h"
#include "jf_thread.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestLoggerVendorErrorCode = FALSE;


/* --- private routine section ------------------------------------------------------------------ */

static void _printLoggerTestUsage(void)
{
    ol_printf("\
Usage: logger-test [-o] [-f log-file] [-t tty-file] [-s] [-a address] [-p port] [-v]\n\
  -o: test the logger with logging to stdout.\n\
  -f: the file name for testing the logger with logging to file.\n\
  -t: the tty file name for testing the logger with logging to tty.\n\
  -s: test the logger with logging to system log.\n\
  -a: the address of the log server for testing the logger with logging to server.\n\
  -p: the port of the log server.\n\
  -v: test the vendor error code.");

    ol_printf("\n");
}

static u32 _parseLoggerTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "of:t:sa:p:vh")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'o':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'f':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = optarg;
            break;
        case 't':
            pjlip->jlip_bLogToTty = TRUE;
            pjlip->jlip_pstrTtyFile = optarg;
            break;
        case 's':
            pjlip->jlip_bLogToSystemLog = TRUE;
            break;
        case 'a':
            pjlip->jlip_bLogToServer = TRUE;
            pjlip->jlip_pstrServerAddress = optarg;
            break;
        case 'p':
            pjlip->jlip_bLogToServer = TRUE;
            u32Ret = jf_option_getU16FromString(optarg, &pjlip->jlip_u16ServerPort);
            break;
        case 'v':
            ls_bTestLoggerVendorErrorCode = TRUE;
            break;
        case '?':
        case 'h':
            _printLoggerTestUsage();
            exit(0);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testLoggerVendorErrCode(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Data[94];

#define E_HELLO_WORLD (JF_ERR_VENDOR_SPEC_ERROR_START + 0x0)
#define E_SUPER_MAN (JF_ERR_VENDOR_SPEC_ERROR_START + 0x1)
#define E_IRON_MAN (JF_ERR_VENDOR_SPEC_ERROR_START + 0x2)
#define E_BAT_MAN (JF_ERR_VENDOR_SPEC_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)

    jf_err_addCode(E_HELLO_WORLD, "Hello world");
    jf_err_addCode(E_SUPER_MAN, "Super man");
    jf_err_addCode(E_IRON_MAN, "Iron man");
    jf_err_addCode(E_BAT_MAN, "Bat man");

    jf_logger_logErrMsg(E_HELLO_WORLD, "Vendor error code");
    jf_logger_logErrMsg(E_SUPER_MAN, "Vendor error code");
    jf_logger_logErrMsg(E_IRON_MAN, "Vendor error code");
    jf_logger_logErrMsg(E_BAT_MAN, "Vendor error code");

    jf_logger_logInfoMsg("logger test, info msg");
    JF_LOGGER_INFO("logger test, info msg");

    jf_logger_logErrMsg(JF_ERR_NOT_FOUND, "logger test, err msg");
    JF_LOGGER_ERR(JF_ERR_FAIL_CREATE_MUTEX, "err msg for testing logger");

    jf_logger_logDataMsg(u8Data, sizeof(u8Data), "logger test, data");

    return u32Ret;
}

static u32 _testLoggerLogMsg(olint_t index)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Data[94];
    olchar_t str[JF_LOGGER_MAX_MSG_SIZE + 16], * pstr = str;
    olchar_t strnum[32];
    olint_t num = 1;
    olsize_t snum = 0;

    jf_logger_logInfoMsg("logger test, info msg, write %d msg", index + 1);
    JF_LOGGER_INFO("logger test, info msg");

    str[0] = '\0';
    while (pstr - str < JF_LOGGER_MAX_MSG_SIZE)
    {
        snum = ol_sprintf(strnum, "%d,", num);
        ol_strcat(pstr, strnum);
        pstr += snum;
        num += snum;
    }

    str[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';
    jf_logger_logInfoMsg("logger test, info msg, maximum msg size, %d, %s", JF_LOGGER_MAX_MSG_SIZE, str);

    jf_logger_logErrMsg(JF_ERR_NOT_FOUND, "logger test, err msg");
    JF_LOGGER_ERR(JF_ERR_FAIL_CREATE_MUTEX, "err msg for testing logger");
    jf_logger_logErrMsg(JF_ERR_FAIL_SEND_DATA, "logger test, err msg, maximum msg size, %d, %s", JF_LOGGER_MAX_MSG_SIZE, str);

    JF_LOGGER_ERR(JF_ERR_INVALID_IP, "logger test, err msg, maximum msg size, %d, %s", JF_LOGGER_MAX_MSG_SIZE, str);

    jf_logger_logDataMsg(u8Data, sizeof(u8Data), "logger test, data msg");
    JF_LOGGER_DATA(u8Data, sizeof(u8Data), "logger test, data msg, maximum msg size, %d, %s", JF_LOGGER_MAX_MSG_SIZE, str);

    return u32Ret;
}

static u32 _testLoggerToStdout(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 100;

    while (index < count)
    {
        _testLoggerLogMsg(index);

        index ++;
    }

    return u32Ret;
}

static u32 _testLoggerToFile(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 100;

    while (index < count)
    {
        _testLoggerLogMsg(index);

        index ++;
    }

    return u32Ret;
}

static u32 _testLoggerToSystemLog(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _testLoggerLogMsg(0);

    return u32Ret;
}

static u32 _testLoggerToTty(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t index = 0, count = 100;

    while (index < count)
    {
        _testLoggerLogMsg(index);

        index ++;
    }

    return u32Ret;
}

static u32 _testLoggerToServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _testLoggerLogMsg(0);

    return u32Ret;
}

static void _loggerTestSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "LOGGER-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    u32Ret = _parseLoggerTestCmdLineParam(argc, argv, &jlipParam);

    /*Register the signal handlers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_registerSignalHandlers(_loggerTestSignalHandler);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        if (jlipParam.jlip_bLogToStdout)
            u32Ret = _testLoggerToStdout();
        else if (jlipParam.jlip_bLogToFile)
            u32Ret = _testLoggerToFile();
        else if (jlipParam.jlip_bLogToSystemLog)
            u32Ret = _testLoggerToSystemLog();
        else if (jlipParam.jlip_bLogToTty)
            u32Ret = _testLoggerToTty();
        else if (jlipParam.jlip_bLogToServer)
            u32Ret = _testLoggerToServer();
        else if (ls_bTestLoggerVendorErrorCode)
            u32Ret = _testLoggerVendorErrCode();
        else
            _printLoggerTestUsage();

        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
