/**
 *  @file hex-test.c
 *
 *  @brief Test file for hex function defined in jf_hex common object.
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
#include "jf_hex.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestHexString = FALSE;


/* --- private routine section ------------------------------------------------------------------ */

static void _printHexTestUsage(void)
{
    ol_printf("\
Usage: hex-test [-s] [logger options] \n\
  -s: test hex string.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");


    ol_printf("\n");
}

static u32 _parseHexTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "sh?T:F:S:O")) != -1))
    {
        switch (nOpt)
        {
        case 's':
            ls_bTestHexString = TRUE;
            break;
        case '?':
        case 'h':
            _printHexTestUsage();
            exit(0);
            break;
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

static u32 _testHexstr(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#define TEST_HEXSTR_DATA_SIZE  (97)
    u8 u8Data[TEST_HEXSTR_DATA_SIZE];
    u16 u16Data[TEST_HEXSTR_DATA_SIZE];
    olchar_t str[4 * TEST_HEXSTR_DATA_SIZE];
    olsize_t datasize, index = 0;

    ol_printf("Byte Hex String:\n");
    index = 0;
    while (index < TEST_HEXSTR_DATA_SIZE)
    {
        datasize = jf_hex_convertByteDataToString(
            u8Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Word Hex String:\n");
    index = 0;
    while (index < TEST_HEXSTR_DATA_SIZE)
    {
        datasize = jf_hex_convertWordDataToString(
            u16Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Byte Hex String With ASCII:\n");
    index = 0;
    while (index < TEST_HEXSTR_DATA_SIZE)
    {
        datasize = jf_hex_convertByteDataToStringWithAscii(
            u8Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Dump Data:\n");
    jf_hex_dumpByteDataBuffer(u8Data, TEST_HEXSTR_DATA_SIZE);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
	olchar_t strErrMsg[300];

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "HEXTEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    u32Ret = _parseHexTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        if (ls_bTestHexString)
            u32Ret = _testHexstr();
        else
            _printHexTestUsage();
        
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
