/**
 *  @file stringparse-test.c
 *
 *  @brief The test file for stringparse library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_hex.h"

/* --- private data/data structure section --------------------------------- */

static boolean_t ls_bScanString = FALSE;
static boolean_t ls_bParseString = FALSE;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: hex-test [-s] [-p] [logger options] \n\
    -s test scan string.\n\
    -p test parse string.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");


    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "sph?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 's':
            ls_bScanString = TRUE;
            break;
        case 'p':
            ls_bParseString = TRUE;
            break;
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
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
        datasize = jf_hexstr_convertByteData(
            u8Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Word Hex String:\n");
    index = 0;
    while (index < TEST_HEXSTR_DATA_SIZE)
    {
        datasize = jf_hexstr_convertWordData(
            u16Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Byte Hex String With ASCII:\n");
    index = 0;
    while (index < TEST_HEXSTR_DATA_SIZE)
    {
        datasize = jf_hexstr_convertByteDataWithAscii(
            u8Data, TEST_HEXSTR_DATA_SIZE, index, str, sizeof(str));

        ol_printf("%s\n", str);
        index += datasize;
    }
    ol_printf("\n");

    ol_printf("Dump Data:\n");
    jf_hexstr_dumpByteDataBuffer(u8Data, TEST_HEXSTR_DATA_SIZE);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
	olchar_t strErrMsg[300];

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "XMLPARSER";
//    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = _testHexstr();

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


