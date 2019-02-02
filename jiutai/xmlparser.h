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
#include "olbasic.h"
#include "bases.h"

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
typedef struct xml_node
{
    /** Tag name */
    olchar_t * xn_pstrName;
    olsize_t xn_sName;
    u32 xn_u32Reserved;
    /** Namespace */
    olchar_t * xn_pstrNs;
    olsize_t xn_sNs;
    u32 xn_u32Reserved2;

    boolean_t xn_bStartTag;
    boolean_t xn_bEmptyTag;
    u8 xn_u8Reserved[6];

    void * xn_pReserved;
    hashtree_t xn_hNameSpace;

    struct xml_node * xn_pxnNext;
    struct xml_node * xn_pxnParent;
    struct xml_node * xn_pxnChildren;
    struct xml_node * xn_pxnSibling;

    struct xml_node * xn_pxnClosingTag;
    struct xml_node * xn_pxnStartingTag;
} xml_node_t;

typedef struct xml_attribute
{
    olchar_t * xa_pstrName;
    olsize_t xa_sName;

    olchar_t * xa_pstrPrefix;
    olsize_t xa_sPrefix;

    olchar_t * xa_pstrValue;
    olsize_t xa_sValue;
    struct xml_attribute * xa_pxaNext;
} xml_attribute_t;

typedef struct xml_doc
{
    xml_node_t * xd_pxnRoot;
    xml_node_t * xd_pxnError;
    u8 xd_u8Reserved[32];
} xml_doc_t;

typedef struct xml_file
{
    olchar_t * xf_pstrBuf;
    xml_doc_t xf_xdDoc;
} xml_file_t;

/* --- functional routines ------------------------------------------------- */

/** Parse xml string in memory, start from sOffset with sBuf
 *
 *  @param pstrBuffer [in] the buffer to parse
 *  @param sOffset [in] the offset in the buffer to start parsing
 *  @param sBuf [in] the length of the buffer
 *  @param pxd [out] a tree of xml node, representing the XML document
 *
 *  @return the error code
 *
 *  @note The strings are never copied. Everything is referenced via pointers
 *   into the original buffer
 */
XMLPARSERAPI u32 XMLPARSERCALL parseXML(
    olchar_t * pstrBuffer, olsize_t sOffset, olsize_t sBuf, xml_doc_t * pxd);

XMLPARSERAPI u32 XMLPARSERCALL destroyXMLNodeList(xml_node_t ** ppNode);

/** Read the attributes from an XML node
 *
 *  @param node [in] the node to read the attributes from
 *  @param ppAttribute [out] a linked list of attributes
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL getXMLAttributes(
    xml_node_t * node, xml_attribute_t ** ppAttribute);

/** Frees resources from an attribute list returned from getXMLAttributes
 *
 *  @param ppAttribute [in/out] the attribute tree to clean up
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL destroyXMLAttributeList(
    xml_attribute_t ** ppAttribute);

/** Reads the data segment from an xml node. The data is a pointer into the
 *  original string that the XML was read from.
 *
 *  @param node [in] the node to read the data from
 *  @param ppstrData [out] the data
 *  @param psBuf [out] the size of the data
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL readInnerXML(
    xml_node_t * node, olchar_t ** ppstrData, olsize_t * psData);

XMLPARSERAPI u32 XMLPARSERCALL parseXMLFile(
    const olchar_t * pstrFilename, xml_file_t ** ppFile);

XMLPARSERAPI u32 XMLPARSERCALL destroyXMLFile(xml_file_t ** ppFile);

XMLPARSERAPI void XMLPARSERCALL printXMLNodeList(xml_node_t * pxn, u8 u8Indent);

#endif /*JIUFENG_XMLPARSER_H*/

/*---------------------------------------------------------------------------*/


