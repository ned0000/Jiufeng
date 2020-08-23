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

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bScanString = FALSE;

static boolean_t ls_bParseString = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printStringparseTestUsage(void)
{
    ol_printf("\
Usage: stringparse-test [-s] [-p] [logger options] \n\
    -s test scan string.\n\
    -p test parse string.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");


    ol_printf("\n");
}

static u32 _parseStringparseTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "sph?")) != -1))
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
            _printStringparseTestUsage();
            exit(u32Ret);
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
    ol_printf("Test string parse\n");

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
        ol_printf("Test adv string parse\n");

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

static u32 _testScanString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	char * pstrDouble[] = {
        "226303136636.85",
        "2298363628138.857",
        "230189685431.55",
    };
	oldouble_t db;
    u32 index = 0;
    char * pstrBoolean[] = {
        "enabled",
        "disabled",
        "yes",
        "no",
        "true",
        "false",
    };
    boolean_t bValue;
    char * pstrByteSize[] = {
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

    for (index = 0; index < ARRAY_SIZE(pstrDouble); index ++)
    {
        u32Ret = jf_string_getDoubleFromString(pstrDouble[index], ol_strlen(pstrDouble[index]), &db);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("%s, %.2f\n", pstrDouble[index], db);
        }
    }

    for (index = 0; index < ARRAY_SIZE(pstrBoolean); index ++)
    {
        u32Ret = jf_string_getBooleanFromString(
            pstrBoolean[index], ol_strlen(pstrBoolean[index]), &bValue);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("%s, %d\n", pstrBoolean[index], bValue);
        }
    }

    for (index = 0; index < ARRAY_SIZE(pstrByteSize); index ++)
    {
        u32Ret = jf_string_getSizeFromByteString(
            pstrByteSize[index], ol_strlen(pstrByteSize[index]), &u64Size);

        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("%s, %llu\n", pstrByteSize[index], u64Size);
        else
            ol_printf("%s, error\n", pstrByteSize[index]);
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
    jlipParam.jlip_bLogToStdout = TRUE;
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
            if (ls_bParseString)
            {
                u32Ret = _testParseString();
            }
            else if (ls_bScanString)
            {
                u32Ret = _testScanString();
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
