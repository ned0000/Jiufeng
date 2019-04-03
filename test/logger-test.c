/**
 *  @file logger-test.c
 *
 *  @brief The test file for logger library
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
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "errcode.h"
//#include "files.h"

/* --- private data/data structure section --------------------------------- */
typedef struct
{
    boolean_t ts_bInitialized;
    u8 ts_u8Reserved[7];
    olint_t ts_nData1;
    olint_t ts_nData2;
    olint_t ts_nData3;
    olint_t ts_nData4;
} test_struct_t;

static test_struct_t ls_tsStruct;
static boolean_t ls_bDump = FALSE;
static boolean_t ls_bErrorCode = FALSE;
static boolean_t ls_bTestLogger = FALSE;

static u32 ls_u32ErrorCode;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: logger-test [-l] [-c error code] \n\
    [-l]: test the logger. \n\
    [-c error code]: prolint_t error message for the error coce.\n");

    ol_printf("\n");
}

static u32 _scanErrorCode(olchar_t * strcode)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (strlen(strcode) > 2)
    {
        if (strncasecmp(strcode, "0x", 2) == 0)
            sscanf(strcode + 2, "%x", &ls_u32ErrorCode);
        else
            sscanf(strcode, "%d", &ls_u32ErrorCode);
    }
    else
        sscanf(strcode, "%d", &ls_u32ErrorCode);

    return u32Ret;
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "c:lh")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
            break;
        case 'd':
            ls_bDump = TRUE;
            break;
        case 'c':
            ls_bErrorCode = TRUE;
            _scanErrorCode(optarg);
            break;
        case 'l':
            ls_bTestLogger = TRUE;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _testLogger(void)
{
//    u32 u32Ret = OLERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    u8 u8Data[94];
    test_struct_t * pts = &ls_tsStruct;
//    file_t fd;
//    dir_t * pDir;

    ol_printf("%d, %d, %d, %d\n", pts->ts_nData1, pts->ts_nData2, pts->ts_nData3,
           pts->ts_nData4);

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "LOGGER-TEST";
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_INFO;

//    checkErrCode();

    jf_logger_init(&jlipParam);

#define E_HELLO_WORLD (OLERR_VENDOR_SPEC_ERROR_START + 0x0)
#define E_SUPER_MAN (OLERR_VENDOR_SPEC_ERROR_START + 0x1)
#define E_IRON_MAN (OLERR_VENDOR_SPEC_ERROR_START + 0x2)
#define E_BAT_MAN (OLERR_VENDOR_SPEC_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)

    addErrCode(E_HELLO_WORLD, "Hello world");
    addErrCode(E_SUPER_MAN, "Super man");
    addErrCode(E_IRON_MAN, "Iron man");
    addErrCode(E_BAT_MAN, "Bat man");

    logErrMsg(E_HELLO_WORLD, "Vendor error code");
    logErrMsg(E_SUPER_MAN, "Vendor error code");
    logErrMsg(E_IRON_MAN, "Vendor error code");
    logErrMsg(E_BAT_MAN, "Vendor error code");

    logInfoMsg("logger test, info msg");

    logErrMsg(OLERR_NOT_FOUND, "logger test, err msg");

    logDataMsg(u8Data, sizeof(u8Data), "logger test, err msg");

//        u32Ret = openDir("shit", &pDir);
//    u32Ret = OLERR_OPERATION_FAIL;

//    logErrMsg(u32Ret, "Quit");

    jf_logger_fini();
}

static void _printErrorCode(void)
{
    olchar_t msg[512];
    olint_t module, code;

    module = (ls_u32ErrorCode & ERRCODE_MODULE_MASK) >> ERRCODE_MODULE_SHIFT;
    code = ls_u32ErrorCode & ERRCODE_CODE_MASK;

    ol_printf("Module 0x%x\n", module);
    ol_printf("Code 0x%x\n", code);

    getErrMsg(ls_u32ErrorCode, msg, sizeof(msg));

    ol_printf("%s\n", msg);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_bTestLogger)
            _testLogger();
        else if (ls_bErrorCode)
            _printErrorCode();
        else
            _printUsage();
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


