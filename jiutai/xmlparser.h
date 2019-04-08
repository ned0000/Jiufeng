/**
 *  @file xmlparser.h
 *
 *  @brief xml parser header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_XMLPARSER_H
#define JIUFENG_XMLPARSER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_hashtree.h"

#undef XMLPARSERAPI
#undef XMLPARSERCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_XMLPARSER_DLL)
        #define XMLPARSERAPI  __declspec(dllexport)
        #define XMLPARSERCALL
    #else
        #define XMLPARSERAPI
        #define XMLPARSERCALL __cdecl
    #endif
#else
    #define XMLPARSERAPI
    #define XMLPARSERCALL
#endif

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef struct jf_xmlparser_xml_node
{
    /** Tag name */
    olchar_t * jxxn_pstrName;
    olsize_t jxxn_sName;
    u32 jxxn_u32Reserved;
    /** Namespace */
    olchar_t * jxxn_pstrNs;
    olsize_t jxxn_sNs;
    u32 jxxn_u32Reserved2;

    boolean_t jxxn_bStartTag;
    boolean_t jxxn_bEmptyTag;
    u8 jxxn_u8Reserved[6];

    void * jxxn_pReserved;
    jf_hashtree_t jxxn_jhNameSpace;

    struct jf_xmlparser_xml_node * jxxn_pjxxnNext;
    struct jf_xmlparser_xml_node * jxxn_pjxxnParent;
    struct jf_xmlparser_xml_node * jxxn_pjxxnChildren;
    struct jf_xmlparser_xml_node * jxxn_pjxxnSibling;

    struct jf_xmlparser_xml_node * jxxn_pjxxnClosingTag;
    struct jf_xmlparser_xml_node * jxxn_pjxxnStartingTag;
} jf_xmlparser_xml_node_t;

typedef struct jf_xmlparser_xml_attribute
{
    olchar_t * jxxa_pstrName;
    olsize_t jxxa_sName;

    olchar_t * jxxa_pstrPrefix;
    olsize_t jxxa_sPrefix;

    olchar_t * jxxa_pstrValue;
    olsize_t jxxa_sValue;
    struct jf_xmlparser_xml_attribute * jxxa_pjxxaNext;
} jf_xmlparser_xml_attribute_t;

typedef struct jf_xmlparser_xml_doc
{
    jf_xmlparser_xml_node_t * jxxd_pjxxnRoot;
    jf_xmlparser_xml_node_t * jxxd_pjxxnError;
    u8 jxxd_u8Reserved[32];
} jf_xmlparser_xml_doc_t;

typedef struct jf_xmlparser_xml_file
{
    olchar_t * jxxf_pstrBuf;
    jf_xmlparser_xml_doc_t jxxf_jxxdDoc;
} jf_xmlparser_xml_file_t;

/* --- functional routines ------------------------------------------------- */

/** Parse xml string in memory, start from sOffset with sBuf
 *
 *  @param pstrBuffer [in] the buffer to parse
 *  @param sOffset [in] the offset in the buffer to start parsing
 *  @param sBuf [in] the length of the buffer
 *  @param pjxxd [out] a tree of xml node, representing the XML document
 *
 *  @return the error code
 *
 *  @note The strings are never copied. Everything is referenced via pointers
 *   into the original buffer
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXML(
    olchar_t * pstrBuffer, olsize_t sOffset, olsize_t sBuf,
    jf_xmlparser_xml_doc_t * pjxxd);

XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXMLNodeList(
    jf_xmlparser_xml_node_t ** ppNode);

/** Read the attributes from an XML node
 *
 *  @param node [in] the node to read the attributes from
 *  @param ppAttribute [out] a linked list of attributes
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_getXMLAttributes(
    jf_xmlparser_xml_node_t * node, jf_xmlparser_xml_attribute_t ** ppAttribute);

/** Frees resources from an attribute list returned from getXMLAttributes
 *
 *  @param ppAttribute [in/out] the attribute tree to clean up
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXMLAttributeList(
    jf_xmlparser_xml_attribute_t ** ppAttribute);

/** Reads the data segment from an xml node. The data is a pointer into the
 *  original string that the XML was read from.
 *
 *  @param node [in] the node to read the data from
 *  @param ppstrData [out] the data of the node
 *  @param psData [out] the size of the data
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_readInnerXML(
    jf_xmlparser_xml_node_t * node, olchar_t ** ppstrData, olsize_t * psData);

XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXMLFile(
    const olchar_t * pstrFilename, jf_xmlparser_xml_file_t ** ppFile);

XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXMLFile(
    jf_xmlparser_xml_file_t ** ppFile);

XMLPARSERAPI void XMLPARSERCALL jf_xmlparser_printXMLNodeList(
    jf_xmlparser_xml_node_t * pjxxn, u8 u8Indent);

#endif /*JIUFENG_XMLPARSER_H*/

/*---------------------------------------------------------------------------*/


