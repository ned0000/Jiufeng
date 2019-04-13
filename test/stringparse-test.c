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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bScanString = FALSE;
static boolean_t ls_bParseString = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUsage(void)
{
    ol_printf("\
Usage: stringparse-test [-s] [-p] [logger options] \n\
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

static void _printParseResult(jf_string_parse_result_t * result)
{
    jf_string_parse_result_field_t * field;
    u32 u32Index;
    olchar_t str[64];

    ol_printf("Number of result: %d\n", result->jspr_u32NumOfResult);

    field = result->jspr_pjsprfFirst;
    for (u32Index = 0; u32Index < result->jspr_u32NumOfResult; u32Index ++)
    {
        ol_printf("%u, size: %d, data: ", u32Index, field->jsprf_sData);
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

static u32 _testParseStringWithDelimiter(
    olchar_t * pstr, olchar_t * pdelimiter)
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
        ol_printf("\n");
    }
    
    return u32Ret;
}

static u32 _testParseStringAdvWithDelimiter(
    olchar_t * pstr, olchar_t * pdelimiter)
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

    ol_printf("----------------------------------------------------\n");
    ol_printf("Test string parse\n");

    u32Ret = _testParseStringWithDelimiter(pstrXml, "<");
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, "=");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, ">");

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testParseStringWithDelimiter(pstrXml, "m");

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

    return u32Ret;
}

static u32 _testScanString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	char * sdb = "226303136636.85";
	char * sdb2 = "2298363628138.857";
	char * sdb3 = "230189685431.55";
	oldouble_t db;

	u32Ret = jf_string_getDoubleFromString(sdb, ol_strlen(sdb), &db);
	if (u32Ret == JF_ERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb, db);
	}

	u32Ret = jf_string_getDoubleFromString(sdb2, ol_strlen(sdb2), &db);
	if (u32Ret == JF_ERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb2, db);
	}

	u32Ret = jf_string_getDoubleFromString(sdb3, ol_strlen(sdb3), &db);
	if (u32Ret == JF_ERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb3, db);
	}

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

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

        if (ls_bParseString)
            u32Ret = _testParseString();
        else if (ls_bScanString)
            u32Ret = _testScanString();

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


