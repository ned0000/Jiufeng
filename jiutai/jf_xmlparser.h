/**
 *  @file jf_xmlparser.h
 *
 *  @brief Header file defines interface of XML parser library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_xmlparser library.
 *
 *  <HR>
 *
 *  @par XML Declaration
 *  @code
 *  <?xml version="1.0"?>
 *  @endcode
 *
 *  @par XML Element
 *  @code
 *  <name>content</name>
 *  @endcode
 *
 *  @par XML Empty Element
 *  @code
 *  <name/>
 *  @endcode
 *
 *  @par XML Comment
 *  @code
 *  <!-- comment -->
 *  @endcode
 *
 *  @par XML Attibute
 *  @code
 *  <person ns:name="tom">content</person>
 *  @endcode
 *  <HR>
 */

#ifndef JIUFENG_XMLPARSER_H
#define JIUFENG_XMLPARSER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
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

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define XML node data type.
 */
typedef struct jf_xmlparser_xml_node
{
    /**The tag name string.*/
    olchar_t * jxxn_pstrName;
    /**Size of tag name string.*/
    olsize_t jxxn_sName;
    u32 jxxn_u32Reserved;
    /**The name space string.*/
    olchar_t * jxxn_pstrNs;
    /**Size of name space string.*/
    olsize_t jxxn_sNs;
    u32 jxxn_u32Reserved2;

    /**It's a start tag if TRUE.*/
    boolean_t jxxn_bStartTag;
    /**It's an empty tag if TRUE.*/
    boolean_t jxxn_bEmptyTag;
    u8 jxxn_u8Reserved[6];

    void * jxxn_pReserved;
    jf_hashtree_t jxxn_jhNameSpace;

    struct jf_xmlparser_xml_node * jxxn_pjxxnNext;
    struct jf_xmlparser_xml_node * jxxn_pjxxnParent;
    /**The children XML node.*/
    struct jf_xmlparser_xml_node * jxxn_pjxxnChildren;
    /**The sibling XML node.*/
    struct jf_xmlparser_xml_node * jxxn_pjxxnSibling;

    struct jf_xmlparser_xml_node * jxxn_pjxxnClosingTag;
    struct jf_xmlparser_xml_node * jxxn_pjxxnStartingTag;
} jf_xmlparser_xml_node_t;

/** Define XML attribute data type.
 */
typedef struct jf_xmlparser_xml_attribute
{
    /**The string of attribute name.*/
    olchar_t * jxxa_pstrName;
    /**Size of name string.*/
    olsize_t jxxa_sName;

    /**The prefix string of name space.*/
    olchar_t * jxxa_pstrPrefix;
    /**Size of name space string.*/
    olsize_t jxxa_sPrefix;

    /**The string of attribute value.*/
    olchar_t * jxxa_pstrValue;
    /**Size of value string.*/
    olsize_t jxxa_sValue;
    /**The next XML attribute.*/
    struct jf_xmlparser_xml_attribute * jxxa_pjxxaNext;
} jf_xmlparser_xml_attribute_t;

/** Define XML attribute document data type.
 */
typedef struct jf_xmlparser_xml_doc
{
    /**XML attributes in declaration.*/
    jf_xmlparser_xml_attribute_t * jxxd_jxxaDeclaration;
    /**The root of the XML node list.*/
    jf_xmlparser_xml_node_t * jxxd_pjxxnRoot;
    /**The node with error during parse.*/
    jf_xmlparser_xml_node_t * jxxd_pjxxnError;
    u8 jxxd_u8Reserved[32];
} jf_xmlparser_xml_doc_t;

/** Define XML file data type.
 */
typedef struct jf_xmlparser_xml_file
{
    /**The buffer containing XML file content.*/
    olchar_t * jxxf_pstrBuf;
    /**The XML document data type.*/
    jf_xmlparser_xml_doc_t jxxf_jxxdDoc;
} jf_xmlparser_xml_file_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Parse xml string in memory, start from offset.
 *
 *  @note
 *  -# After parse, the buffer is kept unchanged. Everything is referenced via pointers into the
 *     original buffer.
 *
 *  @param pstrBuffer [in] The buffer to parse.
 *  @param sOffset [in] The offset in the buffer to start parsing.
 *  @param sBuf [in] The length of the buffer.
 *  @param pjxxd [out] A tree of xml node, representing the XML document.
 *
 *  @return The error code.
 *  @retval JF_ERR_CORRUPTED_XML_FILE Corrupted XML file. 
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXML(
    olchar_t * pstrBuffer, olsize_t sOffset, olsize_t sBuf, jf_xmlparser_xml_doc_t * pjxxd);

/** Destroy XML node list.
 *
 *  @param ppNode [in/out] The node list to destroy.
 *
 *  @return The error code.
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXMLNodeList(jf_xmlparser_xml_node_t ** ppNode);

/** Read the attributes from an XML node.
 *
 *  @param node [in] The node to read the attributes from.
 *  @param ppAttribute [out] A linked list of attributes.
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

/** Reads the data segment from an xml node.
 *
 *  @note
 *  -# The data is a pointer into the original string that the XML was read from.
 *
 *  @param node [in] the node to read the data from
 *  @param ppstrData [out] the data of the node
 *  @param psData [out] the size of the data
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_readInnerXML(
    jf_xmlparser_xml_node_t * node, olchar_t ** ppstrData, olsize_t * psData);

/** Parse XML file.
 *
 *  @param pstrFilename [in] The XML file to be parsed.
 *  @param ppFile [in/out] The XML file data type.
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXMLFile(
    const olchar_t * pstrFilename, jf_xmlparser_xml_file_t ** ppFile);

/** Destroy XML file data type.
 *
 *  @param ppFile [in/out] The file data type to be destroyed.
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXMLFile(jf_xmlparser_xml_file_t ** ppFile);

/** Print XML node list.
 *
 *  @param pjxxn [in] The node to print.
 *  @param u8Indent [in] The indent when printing XML node.
 *
 *  @return the error code
 */
XMLPARSERAPI void XMLPARSERCALL jf_xmlparser_printXMLNodeList(
    jf_xmlparser_xml_node_t * pjxxn, u8 u8Indent);

#endif /*JIUFENG_XMLPARSER_H*/

/*------------------------------------------------------------------------------------------------*/


