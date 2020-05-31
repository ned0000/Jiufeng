/**
 *  @file xmlparser-test.c
 *
 *  @brief Test file for jf_xmlparser library.
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
#include "jf_xmlparser.h"
#include "jf_option.h"
#include "jf_jiukun.h"
#include "jf_ptree.h"

/* An example of XML file for testing purpose:

<?xml version="1.0" encoding="UTF-8" ?>
<servMgmtSetting>
  <version xmlns:h="http://www.w3.org/" class="am" language="en">1.0</version>
  <serviceSetting>
    <service type="bg">
      <!-- zeus service -->
      <name type="bin" format="elf">zeus</name>
      <startupType nochange="yes">automatic</startupType>
      <role>external</role>
      <cmdPath>olzeus</cmdPath>
      <cmdParam path="/usr/bin"></cmdParam>
    </service>
    <service type="fg">
      <name>bio</name>
      <startupType nochange="no">automatic</startupType>
      <role>wakeup</role>
      <cmdPath>olbio</cmdPath>
      <cmdParam path="/usr/bin"></cmdParam>
    </service>
  </serviceSetting>
</servMgmtSetting>

 */

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrXmlFileName = NULL;

/* --- private routine section ------------------------------------------------------------------ */
static void _printXmlparserTestUsage(void)
{
    ol_printf("\
Usage: xmlparser-test [-f xml-file] [-h] [logger options] \n\
    -f specify the XML file.\n\
    -h print the usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parseXmlparserTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "f:T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printXmlparserTestUsage();
            exit(0);
            break;
        case 'f':
            ls_pstrXmlFileName = optarg;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
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

#define XML_DOCUMENT_1 "\
<?xml version=\"1.0\" ?>\
<person>Tom</persion>\
"

#define XML_DOCUMENT_2 "\
<?xml version=\"1.0\" ?>\
<person></person>\
"

#define XML_DOCUMENT_3 "\
<?xml version=\"1.0\" ?>\n\
<person>Jack</person>\n\
<person>Tom</person>\n\
"

#define XML_DOCUMENT_4 "\
<?xml version=\"1.0\" ?>\n\
<!-- comment for school -->\n\
<school>\n\
<!-- comment for person -->\n\
<person>\n\
<!-- comment for name -->\n\
<name>Jack</name>\n\
<age>27</age>\n\
<gender>male</gender>\n\
</person>\n\
<person>\n\
<name>Tom</name>\n\
<age>30</age>\n\
<gender>male</gender>\n\
</person>\n\
</school>\n\
"

#define XML_DOCUMENT_5 "\
<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\
<school name='yuying' location='ab district'>\n\
<!-- comment for person -->\n\
<person>\n\
<!-- comment for name -->\n\
<name>Jack</name>\n\
<age>27</age>\n\
<gender>male</gender>\n\
</person>\n\
<person>\n\
<name>Tom</name>\n\
<age>30</age>\n\
<gender>male</gender>\n\
</person>\n\
</school>\n\
"

typedef struct
{
    olchar_t * pstrXml;
    u32 u32ErrCode;
} test_xml_parser_t;

static u32 _testXmlParser_1()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_t * pjpXml = NULL;
    const olchar_t * pstrErrMsg = NULL;
    olchar_t strErrMsg[300];
    test_xml_parser_t txp[] = {
        {"", JF_ERR_INCOMPLETE_XML_DOCUMENT},
        {"corrupted xml content", JF_ERR_INCOMPLETE_XML_DOCUMENT},
        {"<name></name>", JF_ERR_INVALID_XML_DECLARATION},
        {"<?xml version=\"1.0\" ?>", JF_ERR_INCOMPLETE_XML_DOCUMENT},
        {XML_DOCUMENT_1, JF_ERR_UNMATCHED_XML_CLOSE_TAG},
        {XML_DOCUMENT_2, JF_ERR_NO_ERROR},
        {XML_DOCUMENT_3, JF_ERR_NOT_UNIQUE_XML_ROOT_ELEMENT},
        {XML_DOCUMENT_4, JF_ERR_NO_ERROR},
        {XML_DOCUMENT_5, JF_ERR_NO_ERROR},
    };
    u32 u32NumOfCase = sizeof(txp) / sizeof(test_xml_parser_t);
    u32 u32Index;

    for (u32Index = 0; (u32Index < u32NumOfCase) && (u32Ret == JF_ERR_NO_ERROR); u32Index ++)
    {
        ol_printf("----------------------------------------------------------------\n");
        ol_printf("Parse following XML document:\n%s\n", txp[u32Index].pstrXml);
        u32Ret = jf_xmlparser_parseXmlDoc(
            txp[u32Index].pstrXml, 0, strlen(txp[u32Index].pstrXml), &pjpXml);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("\nParse result:\n");
            jf_ptree_dump(pjpXml);
        }
        else if (u32Ret != txp[u32Index].u32ErrCode)
        {
            ol_printf("\nParse result:\n");
            jf_err_getMsg(u32Ret, strErrMsg, 300);
            ol_printf("%s\n", strErrMsg);

            u32Ret = JF_ERR_PROGRAM_ERROR;
        }
        else
        {
            ol_printf("\nParse result:\n");
            pstrErrMsg = jf_xmlparser_getErrMsg();

            ol_printf("%s\n", pstrErrMsg);

            u32Ret = JF_ERR_NO_ERROR;
        }
        ol_printf("\n");

        if (pjpXml != NULL)
            jf_ptree_destroy(&pjpXml);
    }

    return u32Ret;
}

static boolean_t _isSpecifiedService(
    jf_ptree_t * pjpXml, jf_ptree_node_t * pNode, olchar_t * pstrService)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bRet = FALSE;
    jf_ptree_node_t * temp = NULL;
    olchar_t * pstr = NULL;
    olchar_t * name = "name";

    u32Ret = jf_ptree_findChildNode(pjpXml, pNode, NULL, 0, name, ol_strlen(name), &temp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_getNodeValue(temp, &pstr, NULL);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ol_strcmp(pstr, pstrService) == 0)
                bRet = TRUE;
        }
    }

    return bRet;
}

static u32 _changeStartupType(jf_ptree_t * pjpXml, jf_ptree_node_t * pNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * temp = NULL;
    olchar_t * name = "startupType";
    olchar_t * value = "manual";

    u32Ret = jf_ptree_findChildNode(pjpXml, pNode, NULL, 0, name, ol_strlen(name), &temp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_changeNodeValue(temp, value, ol_strlen(value));
    }

    return u32Ret;
}

static u32 _modifyXmlDocument(jf_ptree_t * pjpXml)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * nodename = "servMgmtSetting.serviceSetting.service";
    jf_ptree_node_t * pNode[10];
    u16 u16Num = 10, u16Index = 0;

    u32Ret = jf_ptree_findAllNode(pjpXml, nodename, ol_strlen(nodename), pNode, &u16Num);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16Num; u16Index ++)
        {
            if (_isSpecifiedService(pjpXml, pNode[u16Index], "bio"))
                u32Ret = _changeStartupType(pjpXml, pNode[u16Index]);
        }
    }
    else if (u32Ret == JF_ERR_PTREE_NODE_NOT_FOUND)
    {
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

static u32 _testXmlParserFile(olchar_t * pstrXmlFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	const char * pstrErrMsg;
    jf_ptree_t * pjpXml = NULL;

    u32Ret = jf_xmlparser_parseXmlFile(pstrXmlFile, &pjpXml);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pjpXml);

        u32Ret = _modifyXmlDocument(pjpXml);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pjpXml);

        u32Ret = jf_xmlparser_saveXmlFile(pjpXml, "temp.xml");
    }
    
    if (u32Ret != JF_ERR_NO_ERROR)
	{
		pstrErrMsg = jf_xmlparser_getErrMsg();
        printf("%s\n", pstrErrMsg);
	}

    if (pjpXml != NULL)
        jf_ptree_destroy(&pjpXml);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
	char strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "XMLPARSER";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseXmlparserTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_pstrXmlFileName != NULL)
            {
                u32Ret = _testXmlParserFile(ls_pstrXmlFileName);
            }
            else
            {
                u32Ret = _testXmlParser_1();
            }

            jf_jiukun_fini();
        }

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

