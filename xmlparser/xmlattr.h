/**
 *  @file xmlattr.h
 *
 *  @brief Header file defines internal data structures for XML attribute.
 *
 *  @author Min Zhang
 *
 */

#ifndef XMLPARSER_XMLATTR_H
#define XMLPARSER_XMLATTR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_hashtree.h"
#include "jf_string.h"
#include "jf_linklist.h"
#include "jf_ptree.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

/** Parse XML attribute string and generate XML attribute list.
 *
 *  @note
 *  -# The first field of the parse result is the attribute string.
 *
 *  @param pElem [in] The element containing the XML attribute.
 *  @param pLinklist [in/out] The link list for XML attribute.
 *
 *  @return The error code.
 */
u32 parseXmlAttributeList(jf_string_parse_result_t * pElem, jf_linklist_t * pLinklist);

u32 destroyXmlAttributeList(jf_linklist_t * pLinklist);

void printXmlAttributeList(jf_linklist_t * pLinklist);

u32 copyXmlDeclarationToPtree(jf_linklist_t * pLinklist, jf_ptree_t * pjpXml);

u32 copyXmlAttributeToPtree(jf_linklist_t * pLinklist, jf_ptree_node_t * pjpn);

#endif /*XMLPARSER_XMLATTR_H*/

/*------------------------------------------------------------------------------------------------*/


