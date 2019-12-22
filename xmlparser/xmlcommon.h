/**
 *  @file xmlcommon.h
 *
 *  @brief Header file defines internal data structures and routines.
 *
 *  @author Min Zhang
 *
 */

#ifndef XMLPARSER_XMLCOMMON_H
#define XMLPARSER_XMLCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_hashtree.h"
#include "jf_xmlparser.h"
#include "jf_linklist.h"
#include "jf_ptree.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define internal XML node data type.
 */
typedef struct internal_xmlparser_xml_node
{
    /**The element name string.*/
    olchar_t * ixxn_pstrName;
    /**Size of the element name string.*/
    olsize_t ixxn_sName;
    u32 ixxn_u32Reserved;
    /**The name space string.*/
    olchar_t * ixxn_pstrNs;
    /**Size of the name space string.*/
    olsize_t ixxn_sNs;
    u32 ixxn_u32Reserved2;

    /**The content string of the element.*/
    olchar_t * ixxn_pstrContent;
    /**Size of the content string.*/
    olsize_t ixxn_sContent;

    /**It's a start tag if TRUE. It's a close tag is FALSE.*/
    boolean_t ixxn_bStartTag;
    /**It's an empty tag if TRUE.*/
    boolean_t ixxn_bEmptyTag;
    u8 ixxn_u8Reserved[6];

    /**XML attributes of this XML node.*/
    jf_linklist_t ixxn_jlAttribute;

    /**The name space hash tree.*/
    jf_hashtree_t ixxn_jhNameSpace;

    /**The parent XML node.*/
    struct internal_xmlparser_xml_node * ixxn_pixxnParent;
    /**The children XML node.*/
    struct internal_xmlparser_xml_node * ixxn_pixxnChildren;
    /**The sibling XML node.*/
    struct internal_xmlparser_xml_node * ixxn_pixxnSibling;

    /**The next XML node in the list.*/
    struct internal_xmlparser_xml_node * ixxn_pixxnNext;
    /**The pointer to the close tag if this is a start tag. The pointer to the start tag if this
       is a close tag.*/
    struct internal_xmlparser_xml_node * ixxn_pixxnPairTag;
    /**Pointer to data segment.*/
    olchar_t * ixxn_pstrSegment;
} internal_xmlparser_xml_node_t;

/** Define XML document data type.
 */
typedef struct internal_xmlparser_xml_doc
{
    /**XML attributes in declaration.*/
    jf_linklist_t ixxd_jlDeclarationAttribute;
    /**The root of the XML node list.*/
    internal_xmlparser_xml_node_t * ixxd_pixxnRoot;
    u8 ixxd_u8Reserved[32];
} internal_xmlparser_xml_doc_t;

/* --- functional routines ---------------------------------------------------------------------- */

boolean_t isAllSpaceInXmlBuffer(olchar_t * pstr, olsize_t size);

void initXmlErrMsg(void);

/** Generate error message.
 */
void genXmlErrMsg(u32 u32Err, const olchar_t * pData, olsize_t sData);

/** Try to generate error message in case the erro message is not set.
 */
void tryGenXmlErrMsg(u32 u32Err, const olchar_t * pData, olsize_t sData);

#endif /*XMLPARSER_XMLCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


