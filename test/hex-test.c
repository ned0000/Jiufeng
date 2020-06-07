/**
 *  @file hex-test.c
 *
 *  @brief Test file for hex common object.
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
#include "jf_hex.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestHexString = FALSE;


/* --- private routine section ------------------------------------------------------------------ */

static void _printHexTestUsage(void)
{
    ol_printf("\
Usage: hex-test [-s] [logger options] \n\
    -s test hex string.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");


    ol_printf("\n");
}

static u32 _parseHexTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "sh?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
