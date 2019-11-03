/**
 *  @file xmlparser-test.c
 *
 *  @brief test file for xmlparser library
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

/* An example of XML file for testing purpose:

<?xml version="1.0" encoding="UTF-8" ?>
<servMgmtSetting>
  <version xmlns:h="http://www.w3.org/" class="am" language="en">1.0</version>
  <serviceSetting>
    <service>
      <name>zeus</name>
      <startupType>automatic</startupType>
      <role>external</role>
      <cmdPath>olzeus</cmdPath>
      <restartDelay>0</restartDelay>
      <cmdParam></cmdParam>
    </service>
    <service>
      <name>bio</name>
      <startupType>automatic</startupType>
      <role>wakeup</role>
      <cmdPath>olbio</cmdPath>
      <restartDelay>0</restartDelay>
      <cmdParam></cmdParam>
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
<person>\n\
<name>Jack</name>\n\
<age>27</age>\n\
<gender>male</gender>\n\
</person>\n\
<person>\n\
<name>Tom</name>\n\
<age>30</age>\n\
<gender>male</gender>\n\
</person>\n\
"

typedef struct
{
    olchar_t * pstrXml;
    u32 u32ErrCode;
} test_xml_parser_t;

static u32 _testXmlParser_1()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_doc_t xmldoc;
    olchar_t strErrMsg[300];
    test_xml_parser_t txp[] = {
        {"", JF_ERR_CORRUPTED_XML_FILE},
        {"corrupted xml content", JF_ERR_CORRUPTED_XML_FILE},
        {"<name>", JF_ERR_INVALID_XML_DECLARATION},
        {"<?xml version=\"1.0\" ?>", JF_ERR_CORRUPTED_XML_FILE},
        {XML_DOCUMENT_1, JF_ERR_UNMATCHED_CLOSE_TAG},
        {XML_DOCUMENT_2, JF_ERR_NO_ERROR},
        {XML_DOCUMENT_3, JF_ERR_NO_ERROR},
        {XML_DOCUMENT_4, JF_ERR_NO_ERROR},
    };
    u32 u32NumOfCase = sizeof(txp) / sizeof(test_xml_parser_t);
    u32 u32Index;

    for (u32Index = 0; u32Index < u32NumOfCase; u32Index ++)
    {
        ol_printf("Parse following XML document:\n%s\n", txp[u32Index].pstrXml);
        u32Ret = jf_xmlparser_parseXML(
            txp[u32Index].pstrXml, 0, strlen(txp[u32Index].pstrXml), &xmldoc);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("Parse result:\n");            
            jf_xmlparser_printXMLNodeList(xmldoc.jxxd_pjxxnRoot, 4);
        }
        else if (u32Ret != txp[u32Index].u32ErrCode)
        {
            u32Ret = JF_ERR_PROGRAM_ERROR;
            break;
        }
        else
        {
            ol_printf("Parse result:\n");
            jf_err_getMsg(u32Ret, strErrMsg, 300);
            ol_printf("%s\n", strErrMsg);
            if (xmldoc.jxxd_pjxxnError != NULL)
            {
                strErrMsg[0] = '\0';
                if (xmldoc.jxxd_pjxxnError->jxxn_pstrNs != NULL)
                    ol_strncat(
                        strErrMsg, xmldoc.jxxd_pjxxnError->jxxn_pstrNs,
                        xmldoc.jxxd_pjxxnError->jxxn_sNs);
                ol_strcat(strErrMsg, ":");
                ol_strncat(
                    strErrMsg, xmldoc.jxxd_pjxxnError->jxxn_pstrName,
                    xmldoc.jxxd_pjxxnError->jxxn_sName);
                ol_printf("Error node: %s\n", strErrMsg);
            }
            u32Ret = JF_ERR_NO_ERROR;
        }
        ol_printf("\n");

        if (xmldoc.jxxd_pjxxnRoot != NULL)
            jf_xmlparser_destroyXMLNodeList(&xmldoc.jxxd_pjxxnRoot);
    }

    return u32Ret;
}

static u32 _printXmlNodeAttributes(jf_xmlparser_xml_attribute_t * pAttr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t value[64];

    while (pAttr != NULL)
    {
        if (pAttr->jxxa_sPrefix > 0)
        {
            memcpy(value, pAttr->jxxa_pstrPrefix, pAttr->jxxa_sPrefix);
            value[pAttr->jxxa_sPrefix] = '\0';
            ol_printf("Attribute Name Prefix: %s\n", value);
        }

        memcpy(value, pAttr->jxxa_pstrName, pAttr->jxxa_sName);
        value[pAttr->jxxa_sName] = '\0';
        ol_printf("Attribute Name: %s\n", value);

        memcpy(value, pAttr->jxxa_pstrValue, pAttr->jxxa_sValue);
        value[pAttr->jxxa_sValue] = '\0';
        ol_printf("Attribute Value: %s\n", value);

        ol_printf("\n");

        pAttr = pAttr->jxxa_pjxxaNext;
    }

    return u32Ret;
}

static u32 _printXmlNodeAttr(jf_xmlparser_xml_node_t * node)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t * temp, * child;
    olchar_t tagname[64];
    olchar_t value[64];
    olchar_t * pData;
    olsize_t sData;
    jf_xmlparser_xml_attribute_t * pAttr;

    ol_printf("\n----------------------------------------------------\n");

    temp = node;
    while (temp != NULL)
    {
        memcpy(tagname, temp->jxxn_pstrName, temp->jxxn_sName);
        tagname[temp->jxxn_sName] = '\0';

        ol_printf("%s", tagname);
        ol_printf("\n");

        child = temp->jxxn_pjxxnChildren;
        if (child != NULL)
        {
            memcpy(tagname, child->jxxn_pstrName, child->jxxn_sName);
            tagname[child->jxxn_sName] = '\0';

            u32Ret = jf_xmlparser_readInnerXML(child, &pData, &sData);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_memcpy(value, pData, sData);
                value[sData] = '\0';
            }
            
            ol_printf("%s: %s\n", tagname, value);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_xmlparser_getXMLAttributes(child, &pAttr);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    _printXmlNodeAttributes(pAttr);
                    jf_xmlparser_destroyXMLAttributeList(&pAttr);
                }
            }

            ol_printf("\n");
        }
        
        temp = temp->jxxn_pjxxnSibling;
    }

    return u32Ret;
}

static u32 _testXmlParser_2(olchar_t * pstrXmlFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	char strErrMsg[300];
    olchar_t strTagName[100];
    jf_xmlparser_xml_file_t * pjxxf = NULL;

    u32Ret = jf_xmlparser_parseXMLFile(pstrXmlFile, &pjxxf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_xmlparser_printXMLNodeList(pjxxf->jxxf_jxxdDoc.jxxd_pjxxnRoot, 2);
        _printXmlNodeAttr(pjxxf->jxxf_jxxdDoc.jxxd_pjxxnRoot);
    }
    else
	{
		jf_err_getMsg(u32Ret, strErrMsg, 300);
        printf("%s\n", strErrMsg);
		if ((pjxxf != NULL) && (pjxxf->jxxf_jxxdDoc.jxxd_pjxxnError == NULL))
		{
            ol_strncpy(
                strTagName, pjxxf->jxxf_jxxdDoc.jxxd_pjxxnError->jxxn_pstrName,
                pjxxf->jxxf_jxxdDoc.jxxd_pjxxnError->jxxn_sName);
            strTagName[pjxxf->jxxf_jxxdDoc.jxxd_pjxxnError->jxxn_sName] = '\0';
            ol_printf("%s, %s\n", strErrMsg, strTagName);
		}
	}

    if (pjxxf != NULL)
        jf_xmlparser_destroyXMLFile(&pjxxf);

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
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;

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
                u32Ret = _testXmlParser_2(ls_pstrXmlFileName);
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

