/**
 *  @file xmlprint.c
 *
 *  @brief Implementation file for XML routines for printing purpose.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_xmlparser.h"

#include "xmlcommon.h"
#include "xmlattr.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static void _printXmlNodeIndentSpace(u16 u16Indent)
{
    u16 u16Index = 0;

    for (u16Index = 0; u16Index < u16Indent; u16Index ++)
        ol_printf(" ");
}

static void _printXmlNodeList(internal_xmlparser_xml_node_t * pixxn, u16 u16Indent)
{
    olchar_t value[128];
    internal_xmlparser_xml_node_t * temp = NULL;

    temp = pixxn;
    while (temp != NULL)
    {
        _printXmlNodeIndentSpace(u16Indent);

        ol_printf("%s", temp->ixxn_pstrName);
        printXmlAttributeList(&temp->ixxn_jlAttribute);
        ol_printf(": ");

        if (temp->ixxn_pixxnChildren != NULL)
        {
            ol_printf("\n");
            _printXmlNodeList(temp->ixxn_pixxnChildren, u16Indent + 4);
        }
        else if (temp->ixxn_pstrContent != NULL)
        {
            ol_memcpy(value, temp->ixxn_pstrContent, temp->ixxn_sContent);
            value[temp->ixxn_sContent] = '\0';
            ol_printf("%s\n", value);
        }

        temp = temp->ixxn_pixxnSibling;
    }
}

/* --- public routine section ------------------------------------------------------------------- */
#if 0
void jf_xmlparser_printXmlAttribute(jf_xmlparser_xml_attribute_t * pAttribute)
{
    internal_xmlparser_xml_attribute_t * pixxa;

    _printXmlNodeAttributes(pixxa);
}
#endif
void jf_xmlparser_printXmlDoc(jf_xmlparser_xml_doc_t * pDoc)
{
    internal_xmlparser_xml_doc_t * pixxd = (internal_xmlparser_xml_doc_t *)pDoc;

    printXmlAttributeList(&pixxd->ixxd_jlDeclarationAttribute);
    ol_printf("\n");

    _printXmlNodeList(pixxd->ixxd_pixxnRoot, 0);
}

/*------------------------------------------------------------------------------------------------*/

