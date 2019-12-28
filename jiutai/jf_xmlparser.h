/**
 *  @file jf_xmlparser.h
 *
 *  @brief Header file defines interface of XML parser library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_xmlparser library.
 *  -# After parse, a property tree is generated. All operations to XML document can be done to the
 *     property tree.
 *  -# The library is NOT thread safe.
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
#include "jf_ptree.h"

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


/* --- functional routines ---------------------------------------------------------------------- */

/** Parse xml document in memory, start from offset.
 *
 *  @note
 *  -# After parse, the buffer is kept unchanged.
 *  -# The property tree returned should be destroyed by jf_ptree_destroy().
 *
 *  @param pstrBuffer [in] The buffer to parse.
 *  @param sOffset [in] The offset in the buffer to start parsing.
 *  @param sBuf [in] The length of the buffer.
 *  @param ppPtree [out] The property tree representing the XML document.
 *
 *  @return The error code.
 *  @retval JF_ERR_CORRUPTED_XML_DOCUMENT Corrupted XML document. 
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXmlDoc(
    olchar_t * pstrBuffer, olsize_t sOffset, olsize_t sBuf, jf_ptree_t ** ppPtree);

/** Get XML error message in case there are error during parse.
 *
 *  @note
 *  -# The error message is in static memory which might be overwritten by subsequent calls to
 *  parse XML.
 *
 *  @return The error message.
 */
XMLPARSERAPI const olchar_t * XMLPARSERCALL jf_xmlparser_getErrMsg(void);

/** Parse XML file and return the property tree.
 *
 *  @param pstrFilename [in] The XML file to be parsed.
 *  @param ppPtree [in/out] The property tree returned.
 *
 *  @return the error code
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_parseXmlFile(
    const olchar_t * pstrFilename, jf_ptree_t ** ppPtree);

/** Save the property tree to XML file.
 */
XMLPARSERAPI u32 XMLPARSERCALL jf_xmlparser_saveXmlFile(
    jf_ptree_t * pPtree, const olchar_t * pstrFilename);

#endif /*JIUFENG_XMLPARSER_H*/

/*------------------------------------------------------------------------------------------------*/


