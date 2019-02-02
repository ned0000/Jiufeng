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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "xmlparser.h"

/** An example of XML file for testing purpose:

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

/* --- private data/data structure section --------------------------------- */

static olchar_t * ls_pstrXmlFileName = NULL;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: xmlparser-test [-f xml-file] [-h] [logger options] \n\
    -f specify the XML file.\n\
    -h print the usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "f:T:F:S:h")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 'f':
            ls_pstrXmlFileName = optarg;
            break;
        case 'T':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                plp->lp_u8TraceLevel = (u8)u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                plp->lp_sLogFile = u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
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
    u32 u32Ret = OLERR_NO_ERROR;
    xml_doc_t xmldoc;
    olchar_t strErrMsg[300];
    test_xml_parser_t txp[] = {
        {"", OLERR_CORRUPTED_XML_FILE},
        {"corrupted xml content", OLERR_CORRUPTED_XML_FILE},
        {"<name>", OLERR_INVALID_XML_DECLARATION},
        {"<?xml version=\"1.0\" ?>", OLERR_CORRUPTED_XML_FILE},
        {XML_DOCUMENT_1, OLERR_UNMATCHED_CLOSE_TAG},
        {XML_DOCUMENT_2, OLERR_NO_ERROR},
        {XML_DOCUMENT_3, OLERR_NO_ERROR},
        {XML_DOCUMENT_4, OLERR_NO_ERROR},
    };
    u32 u32NumOfCase = sizeof(txp) / sizeof(test_xml_parser_t);
    u32 u32Index;

    for (u32Index = 0; u32Index < u32NumOfCase; u32Index ++)
    {
        ol_printf("Parse following XML document:\n%s\n", txp[u32Index].pstrXml);
        u32Ret = parseXML(
            txp[u32Index].pstrXml, 0, strlen(txp[u32Index].pstrXml), &xmldoc);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("Parse result:\n");            
            printXMLNodeList(xmldoc.xd_pxnRoot, 4);
        }
        else if (u32Ret != txp[u32Index].u32ErrCode)
        {
            u32Ret = OLERR_PROGRAM_ERROR;
            break;
        }
        else
        {
            ol_printf("Parse result:\n");
            getErrMsg(u32Ret, strErrMsg, 300);
            ol_printf("%s\n", strErrMsg);
            if (xmldoc.xd_pxnError != NULL)
            {
                strErrMsg[0] = '\0';
                if (xmldoc.xd_pxnError->xn_pstrNs != NULL)
                    ol_strncat(
                        strErrMsg, xmldoc.xd_pxnError->xn_pstrNs,
                        xmldoc.xd_pxnError->xn_sNs);
                ol_strcat(strErrMsg, ":");
                ol_strncat(
                    strErrMsg, xmldoc.xd_pxnError->xn_pstrName,
                    xmldoc.xd_pxnError->xn_sName);
                ol_printf("Error node: %s\n", strErrMsg);
            }
            u32Ret = OLERR_NO_ERROR;
        }
        ol_printf("\n");

        if (xmldoc.xd_pxnRoot != NULL)
            destroyXMLNodeList(&xmldoc.xd_pxnRoot);
    }

    return u32Ret;
}

static u32 _printXmlNodeAttributes(xml_attribute_t * pAttr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t value[64];

    while (pAttr != NULL)
    {
        if (pAttr->xa_sPrefix > 0)
        {
            memcpy(value, pAttr->xa_pstrPrefix, pAttr->xa_sPrefix);
            value[pAttr->xa_sPrefix] = '\0';
            ol_printf("Attribute Name Prefix: %s\n", value);
        }

        memcpy(value, pAttr->xa_pstrName, pAttr->xa_sName);
        value[pAttr->xa_sName] = '\0';
        ol_printf("Attribute Name: %s\n", value);

        memcpy(value, pAttr->xa_pstrValue, pAttr->xa_sValue);
        value[pAttr->xa_sValue] = '\0';
        ol_printf("Attribute Value: %s\n", value);

        ol_printf("\n");

        pAttr = pAttr->xa_pxaNext;
    }

    return u32Ret;
}

static u32 _printXmlNodeAttr(xml_node_t * node)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t * temp, * child;
    olchar_t tagname[64];
    olchar_t value[64];
    olchar_t * pData;
    olsize_t sData;
    xml_attribute_t * pAttr;

    ol_printf("\n----------------------------------------------------\n");

    temp = node;
    while (temp != NULL)
    {
        memcpy(tagname, temp->xn_pstrName, temp->xn_sName);
        tagname[temp->xn_sName] = '\0';

        ol_printf("%s", tagname);
        ol_printf("\n");

        child = temp->xn_pxnChildren;
        if (child != NULL)
        {
            memcpy(tagname, child->xn_pstrName, child->xn_sName);
            tagname[child->xn_sName] = '\0';

            u32Ret = readInnerXML(child, &pData, &sData);
            if (u32Ret == OLERR_NO_ERROR)
            {
                memcpy(value, pData, sData);
                value[sData] = '\0';
            }
            
            ol_printf("%s: %s\n", tagname, value);

            if (u32Ret == OLERR_NO_ERROR)
            {
                u32Ret = getXMLAttributes(child, &pAttr);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    _printXmlNodeAttributes(pAttr);
                    destroyXMLAttributeList(&pAttr);
                }
            }

            ol_printf("\n");
        }
        
        temp = temp->xn_pxnSibling;
    }

    return u32Ret;
}

static u32 _testXmlParser_2(olchar_t * pstrXmlFile)
{
    u32 u32Ret = OLERR_NO_ERROR;
	char strErrMsg[300];
    olchar_t strTagName[100];
    xml_file_t * pxf = NULL;

    u32Ret = parseXMLFile(pstrXmlFile, &pxf);
    if (u32Ret == OLERR_NO_ERROR)
    {
        printXMLNodeList(pxf->xf_xdDoc.xd_pxnRoot, 2);
        _printXmlNodeAttr(pxf->xf_xdDoc.xd_pxnRoot);
    }
    else
	{
		getErrMsg(u32Ret, strErrMsg, 300);
        printf("%s\n", strErrMsg);
		if ((pxf != NULL) && (pxf->xf_xdDoc.xd_pxnError == NULL))
		{
            ol_strncpy(
                strTagName, pxf->xf_xdDoc.xd_pxnError->xn_pstrName,
                pxf->xf_xdDoc.xd_pxnError->xn_sName);
            strTagName[pxf->xf_xdDoc.xd_pxnError->xn_sName] = '\0';
            ol_printf("%s, %s\n", strErrMsg, strTagName);
		}
	}

    if (pxf != NULL)
        destroyXMLFile(&pxf);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    logger_param_t lpParam;
	char strErrMsg[300];
    
    memset(&lpParam, 0, sizeof(logger_param_t));
    lpParam.lp_pstrCallerName = "XMLPARSER";
//    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = LOGGER_TRACE_DEBUG;

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);

        if (ls_pstrXmlFileName != NULL)
        {
            u32Ret = _testXmlParser_2(ls_pstrXmlFileName);
        }
        else
        {
            u32Ret = _testXmlParser_1();
        }

        finiLogger();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

