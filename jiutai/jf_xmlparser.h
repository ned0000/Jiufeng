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
 *  <tag-name attribute-name="value">content</tag-name>
 *  @endcode
 *
 *  @par XML Empty Element
 *  @code
 *  <tag-name attribute-name="value"/>
 *  @endcode
 *
 *  @par XML Comment
 *  @code
 *  <!-- comment -->
 *  @endcode
 *
 *  @par XML Attibute
 *  @code
 *  <person ns:name="tom" >content</person>
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
typedef void  jf_xmlparser_xml_node_t;

/** Define XML document data type.
 */
typedef void  jf_xmlparser_xml_doc_t;

/** Define XML file data type.
 */
typedef void  jf_xmlparser_xml_file_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Parse xml document in memory, start from offset.
 *
 *  @note
 *  -# After parse, the buffer is kept unchanged.
 *
 *  @param pstrBuffer [in] The buffer to parse.
 *  @param sOffset [in] The offset in the buffer to start parsing.
 *  @param sBuf [in] The length of the buffer.
 *  @param ppDoc [in/out] A tree of xml node, representing the XML document.
 *
 *  @return The error code.
 *  @retval JF_ERR_CORRUPTED_XML_DOCUMENT Corrupted XML document. 
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXmlDoc(
    olchar_t * pstrBuffer, olsize_t sOffset, olsize_t sBuf, jf_xmlparser_xml_doc_t ** ppDoc);

/** Destory a XML document.
 *
 *  @param ppDoc [in/out] The XML document to destroy.
 *
 *  @return The error code.
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXmlDoc(jf_xmlparser_xml_doc_t ** ppDoc);

/** Resolves a namespace prefix from the scope of the given node.
 *
 *  @param node [in] the node used to start the resolve
 *  @param prefix [in] the namespace prefix to resolve
 *  @param sPrefix [in] the lenght of the prefix
 *  @param ppstr [out] the resolved namespace, NULL if unable to resolve
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_lookupXmlNamespace(
    jf_xmlparser_xml_node_t * pNode, olchar_t * pstrPrefix, olsize_t sPrefix, olchar_t ** ppstr);

/** Print XML document.
 *
 *  @param pDoc [in] The XML document to print.
 *
 *  @return Void.
 */
XMLPARSERAPI void XMLPARSERCALL jf_xmlparser_printXmlDoc(jf_xmlparser_xml_doc_t * pDoc);

/** Get XML error message in case there are error during parse.
 *
 *  @note
 *  -# The error message is in static memory, so it's NOT thread safe.
 *
 *  @return The error message.
 */
XMLPARSERAPI const olchar_t * XMLPARSERCALL jf_xmlparser_getErrMsg(void);

/** Parse XML file.
 *
 *  @param pstrFilename [in] The XML file to be parsed.
 *  @param ppFile [in/out] The XML file data type.
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXmlFile(
    const olchar_t * pstrFilename, jf_xmlparser_xml_file_t ** ppFile);

/** Destroy XML file data type.
 *
 *  @param ppFile [in/out] The file data type to be destroyed.
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_destroyXmlFile(jf_xmlparser_xml_file_t ** ppFile);

#endif /*JIUFENG_XMLPARSER_H*/

/*------------------------------------------------------------------------------------------------*/


