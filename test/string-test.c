/**
 *  @file string-test.c
 *
 *  @brief Test file for string function defined in jf_string library.
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
#include "jf_string.h"
#include "jf_jiukun.h"
#include "jf_option.h"
#include "jf_hex.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestScanString = FALSE;

static boolean_t ls_bTestParseString = FALSE;

static boolean_t ls_bTestOtherStringFunc = FALSE;

static boolean_t ls_bTestPrintString = FALSE;

static boolean_t ls_bTestParseSettingString = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printStringparseTestUsage(void)
{
    ol_printf("\
Usage: string-test [-s] [-p] [-e] [-r] [-o] [logger options] \n\
  -s: test scan string.\n\
  -p: test parse string.\n\
  -e: test parse setting string.\n\
  -r: test print string.\n\
  -o: test other string function.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: the log file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parseStringparseTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "speroh?OT:F:S:")) != -1))
    {
        switch (nOpt)
        {
        case 's':
            ls_bTestScanString = TRUE;
            break;
        case 'p':
            ls_bTestParseString = TRUE;
            break;
        case 'e':
            ls_bTestParseSettingString = TRUE;
            break;
        case 'r':
            ls_bTestPrintString = TRUE;
            break;
        case 'o':
            ls_bTestOtherStringFunc = TRUE;
            break;
        case '?':
        case 'h':
            _printStringparseTestUsage();
            exit(u32Ret);
            break;
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
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _printParseResult(jf_string_parse_result_t * result)
{
    jf_string_parse_result_field_t * field;
    u32 u32Index;
    olchar_t str[64];

    ol_printf("Number of result: %d\n", result->jspr_u32NumOfResult);

    field = result->jspr_pjsprfFirst;
    for (u32Index = 0; u32Index < result->jspr_u32NumOfResult; u32Index ++)
    {
        ol_printf("index: %u, size: %d, data: ", u32Index, field->jsprf_sData);
        if (field->jsprf_sData != 0)
        {
            strncpy(str, field->jsprf_pstrData, field->jsprf_sData);
            str[field->jsprf_sData] = '\0';
            ol_printf("%s", str);
        }
        ol_printf("\n");
        field = field->jsprf_pjsprfNext;
    }

}

static u32 _testParseStringWithDelimiter(olchar_t * pstr, olchar_t * pdelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * xml = NULL;

    u32Ret = jf_string_parse(
        &xml, pstr, 0, strlen(pstr), pdelimiter, strlen(pdelimiter));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("String to parse: %s\n", pstr);
        ol_printf("Delimiter: %s\n", pdelimiter);

        _printParseResult(xml);

        jf_string_destroyParseResult(&xml);

        ol_printf("String after parse: %s\n", pstr);
        ol_printf("\n");
    }
    
    return u32Ret;
}

static u32 _testParseStringAdvWithDelimiter(olchar_t * pstr, olchar_t * pdelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * xml = NULL;

    u32Ret = jf_string_parseAdv(
        &xml, pstr, 0, strlen(pstr), pdelimiter, strlen(pdelimiter));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("String to parse: %s\n", pstr);
        ol_printf("Delimiter: %s\n", pdelimiter);

        _printParseResult(xml);

        jf_string_destroyParseResult(&xml);
        ol_printf("\n");
    }
    
    return u32Ret;
}

static u32 _testParseString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrXml = "<name>";
    olchar_t * pstrAdv = "<My name is\">\" adv>";
    olchar_t * pstrAdv2 = "<My name is\'>\' adv>";
    olchar_t * pstrAdv3 = ">My name is\'>\' adv>";
    olchar_t * pstrAdv4 = "\"My > name is\"> adv>";
    olchar_t * pstrAdv5 = "\'My name > is\'> adv>";

    ol_printf("----------------------------------------------------\n");
    ol_printf("Test string parse...\n\n");

    u32Ret = _testParseStringWithDelimiter(pstrXml, "<");
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, "=");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, "me");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, "no");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrAdv, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrAdv3, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("----------------------------------------------------\n");        
        ol_printf("Test adv string parse...\n\n");

        u32Ret = _testParseStringAdvWithDelimiter(pstrAdv, ">");
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringAdvWithDelimiter(pstrAdv2, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
         u32Ret = _testParseStringAdvWithDelimiter(pstrAdv3, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
         u32Ret = _testParseStringAdvWithDelimiter(pstrAdv4, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
         u32Ret = _testParseStringAdvWithDelimiter(pstrAdv5, ">");

    return u32Ret;
}

static u32 _testParseSettingString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;
    olchar_t * pstrArray[32];
    olsize_t sArray = 0;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t * pstrSetting[] = {
            "",
            " = , hi=yes",
            " aber =TRUE  , ber=876 ,coa=5, drs = 6",
            "aa=2.5,bb=bed,cc=5",
        };
        olchar_t strBuf[128];
        olsize_t idindex = 0;

        ol_printf("----------------------------------------------------\n");
        ol_printf("Test parse setting string...\n");

        for (index = 0; index < ARRAY_SIZE(pstrSetting); index ++)
        {
            sArray = 32;
            /*Need to copy the setting string to buffer.*/
            ol_strcpy(strBuf, pstrSetting[index]);
            u32Ret = jf_string_processSettings(strBuf, pstrArray, &sArray);

            ol_printf("Setting string: %s\n", pstrSetting[index]);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                for (idindex = 0; idindex < sArray; idindex ++)
                {
                    ol_printf(
                        "Setting       : %s (%ld)\n",
                        pstrArray[idindex], ol_strlen(pstrArray[idindex]));
                }
            }
            else
            {
                ol_printf("invalid setting string\n");
            }
            ol_printf("\n");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t * pstrSetting[] = {
            "",
            " = \0hi=yes\0",
            "=\0hi=yes\0",
            " aber =TRUE  \0 ber=876 \0coa=5\0 drs = 6\0",
            "aa=2.5\0bb=bed\0cc=5\0",
        };
        olsize_t sSetting[] = {
            0,
            11,
            9,
            39,
            19,
        };
        olchar_t strBuf[128];
        olsize_t idindex = 0;

        ol_printf("----------------------------------------------------\n");
        ol_printf("Test parse keyword setting string...\n");

        for (index = 0; index < ARRAY_SIZE(pstrSetting); index ++)
        {
            sArray = 32;
            /*Need to copy the setting string to buffer.*/
            ol_memcpy(strBuf, pstrSetting[index], sSetting[index]);
            u32Ret = jf_string_processKeywordSettings((u8 *)strBuf, sSetting[index], pstrArray, &sArray);

            ol_printf("Keyword setting string:\n");
            jf_hex_dumpByteDataBuffer((u8 *)pstrSetting[index], sSetting[index]);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                for (idindex = 0; idindex < sArray; idindex ++)
                {
                    ol_printf(
                        "Setting       : %s (%ld)\n",
                        pstrArray[idindex], ol_strlen(pstrArray[idindex]));
                }
            }
            else
            {
                ol_printf("invalid setting string\n");
            }
            ol_printf("\n");
        }

    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t * pstrSetting =
            " aber =TRUE  , ber=876 ,coa=5, drs = 6";
        olchar_t strBuf[128];
        olchar_t * pstrAber = "aber";
        olchar_t strValue[128];

        ol_printf("----------------------------------------------------\n");
        ol_printf("Test retrieve setting ...\n");

        sArray = 32;
        /*Need to copy the setting string to buffer.*/
        ol_strcpy(strBuf, pstrSetting);
        u32Ret = jf_string_processSettings(strBuf, pstrArray, &sArray);

        ol_printf("Setting string: %s\n", pstrSetting);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_string_retrieveSettings(
                pstrArray, sArray, pstrAber, strValue, sizeof(strValue));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("Setting name  : %s (%ld)\n", pstrAber, ol_strlen(pstrAber));
            ol_printf("Setting value : %s (%ld)\n", strValue, ol_strlen(strValue));
        }
    }

    return u32Ret;
}

static u32 _testScanString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 index = 0;

	olchar_t * pstrDouble[] = {
        "226303136636.85",
        "2298363628138.857",
        "230189685431.55",
    };
	oldouble_t db;

    olchar_t * pstrBoolean[] = {
        "enabled",
        "disabled",
        "yes",
        "no",
        "true",
        "false",
    };
    boolean_t bValue;

    olchar_t * pstrByteSize[] = {
        "xb",
        "2..2B",
        "1B",
        "3MB",
        "3.2MB",
        "100KB",
        "100.98KB",
        "100.98KB",
        "55.48GB",
        "800GB",
    };
    u64 u64Size = 0;

    olchar_t * pstrIdList[] = {
        "2,3,4",
        "5~7,8,9",
        "5~7, 8,9~16 ",
        "5~7, 8a ,9~16 ",
        "12 ~ 5, 8a ,9~16 ",
    };
    olid_t ids[64];
    olsize_t sIds = 0, idindex = 0;

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test scan double from string...\n");

    for (index = 0; index < ARRAY_SIZE(pstrDouble); index ++)
    {
        u32Ret = jf_string_getDoubleFromString(pstrDouble[index], ol_strlen(pstrDouble[index]), &db);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("%s, %.2f\n", pstrDouble[index], db);
        }
    }

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test scan boolean from string...\n");

    for (index = 0; index < ARRAY_SIZE(pstrBoolean); index ++)
    {
        u32Ret = jf_string_getBooleanFromString(
            pstrBoolean[index], ol_strlen(pstrBoolean[index]), &bValue);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("%s, %d\n", pstrBoolean[index], bValue);
        }
    }

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test scan size from string...\n");

    for (index = 0; index < ARRAY_SIZE(pstrByteSize); index ++)
    {
        u32Ret = jf_string_getSizeFromByteString(
            pstrByteSize[index], ol_strlen(pstrByteSize[index]), &u64Size);

        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("%s, %llu\n", pstrByteSize[index], u64Size);
        else
            ol_printf("%s, error\n", pstrByteSize[index]);
    }

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test scan id from string...\n");

    for (index = 0; index < ARRAY_SIZE(pstrIdList); index ++)
    {
        sIds = ARRAY_SIZE(ids);
        u32Ret = jf_string_getIdListFromString(pstrIdList[index], ids, &sIds);

        ol_printf("Id list string: %s\n", pstrIdList[index]);
        ol_printf("Id            :");

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (idindex = 0; idindex < sIds; idindex ++)
            {
                ol_printf(" %d", ids[idindex]);
            }
            ol_printf("\n");
        }
        else
        {
            ol_printf(" error\n");
        }
    }

    return u32Ret;
}

static u32 _testOtherStringFunc(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;
    olchar_t str[128];
    olchar_t * pstr[] = {
        "  this   is  a   test string",
        "this is a test string.",
        "this is        a  test string.",
    };

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test trim string...\n");

    for (index = 0; index < ARRAY_SIZE(pstr); index ++)
    {
        ol_strcpy(str, pstr[index]);

        jf_string_trimBlank(str);

        ol_printf("Source String: %s\n", pstr[index]);
        ol_printf("After trim   : %s\n", str);
    }

    return u32Ret;
}

static u32 _testPrintString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;
    olchar_t str[128];
    u64 u64Size[] = {
        0, 1, 1023,
        1024, 1025, 4096, 4101, 4102, 28917, 28918, 1048575,
        1048576, 202047185, 1073741823,
        1073741824, 1099511627775,
        1099511627776, 349480398473829
    };
    u64 u64Size1000Based[] = {
        0, 1, 999,
        1000, 1001, 9004, 9005, 9094, 9095, 9100, 9984, 9985, 9994, 9995, 9999, 999999,
        1000000, 1004240, 1005560, 300004234, 300005984, 534944834, 534945001, 999999999,
        1000000000, 234344000148, 877465000148, 999999999999,
        1000000000000, 349480398473829
    };

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test 1000 based size string...\n");

    for (index = 0; index < ARRAY_SIZE(u64Size1000Based); index ++)
    {
        jf_string_getByteStringSize1000Based(str, sizeof(str), u64Size1000Based[index]);

        ol_printf("1000 Based size: %llu\n", u64Size1000Based[index]);
        ol_printf("Size string    : %s\n", str);
        ol_printf("\n");
    }

    ol_printf("-------------------------------------------------------------------\n");
    ol_printf("Test size string...\n");

    for (index = 0; index < ARRAY_SIZE(u64Size); index ++)
    {
        ol_printf("size                 : %llu\n", u64Size[index]);

        jf_string_getByteStringSize(str, sizeof(str), u64Size[index]);
        ol_printf("Size string(rounding): %s\n", str);

        jf_string_getByteStringSizeMax(str, sizeof(str), u64Size[index]);
        ol_printf("Size string          : %s\n", str);

        ol_printf("\n");
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
	olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "STRING-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseStringparseTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bTestParseString)
            {
                u32Ret = _testParseString();
            }
            else if (ls_bTestParseSettingString)
            {
                u32Ret = _testParseSettingString();
            }
            else if (ls_bTestScanString)
            {
                u32Ret = _testScanString();
            }
            else if (ls_bTestPrintString)
            {
                u32Ret = _testPrintString();
            }
            else if (ls_bTestOtherStringFunc)
            {
                u32Ret = _testOtherStringFunc();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printStringparseTestUsage();
            }

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
