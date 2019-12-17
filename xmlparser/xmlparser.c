/**
 *  @file xmlparser.c
 *
 *  @brief Implementation file for XML parse routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The end element starts with "</" like "</name>".
 *  -# The empty element ends with "/>" like "<name/>".
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_xmlparser.h"
#include "jf_stack.h"
#include "jf_jiukun.h"

#include "xmlcommon.h"
#include "xmlattr.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */

static u32 _destroyXmlNode(internal_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * pixxn = *ppNode;
    
    /*If there was a namespace table, delete it*/
    jf_hashtree_fini(&pixxn->ixxn_jhNameSpace);
    destroyXmlAttributeList(&pixxn->ixxn_jlAttribute);

    if (pixxn->ixxn_pstrName != NULL)
        jf_string_free(&pixxn->ixxn_pstrName);

    if (pixxn->ixxn_pstrNs != NULL)
        jf_string_free(&pixxn->ixxn_pstrNs);

    if (pixxn->ixxn_pstrContent != NULL)
        jf_string_free(&pixxn->ixxn_pstrContent);

    jf_jiukun_freeMemory((void **)ppNode);
    
    return u32Ret;
}

static u32 _createXmlNode(
    internal_xmlparser_xml_node_t ** ppNode, olchar_t * pstrTagName, olsize_t sTagName,
    olchar_t * pstrNsTag, olsize_t sNsTag)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * pixxn = NULL;
    
    *ppNode = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pixxn, sizeof(*pixxn));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pixxn, sizeof(*pixxn));

        /*Init the namespace hash table.*/
        jf_hashtree_init(&pixxn->ixxn_jhNameSpace);
        jf_linklist_init(&pixxn->ixxn_jlAttribute);
        pixxn->ixxn_sNs = sNsTag;
        pixxn->ixxn_sName = sTagName;

        if (sTagName > 0)
            u32Ret = jf_string_duplicateWithLen(&pixxn->ixxn_pstrName, pstrTagName, sTagName);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sNsTag > 0))
        u32Ret = jf_string_duplicateWithLen(&pixxn->ixxn_pstrNs, pstrNsTag, sNsTag);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppNode = pixxn;
    else if (pixxn != NULL)
        _destroyXmlNode(&pixxn);

    return u32Ret;
}

/** Frees XML node list.
 *
 *  @param ppNode [in/out] The XML node to clean up.
 */
static u32 _destroyXmlNodeList(internal_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * temp;
    internal_xmlparser_xml_node_t * node = *ppNode;

    while (node != NULL)
    {
        temp = node->ixxn_pixxnNext;

        _destroyXmlNode(&node);

        node = temp;
    }

    return u32Ret;
}

/** New XML node.
 *  
 *  @note
 *  -# A xml element is a node. All the XML node are linked with ixxn_pixxnNext. 
 *
 *  @param ppRoot [out] The root of the XML document, it's also the first XML node created.
 *  @param ppCurrent [in/out] The current XML node.  
 *  @param field [in] The content between 2 "<".
 *
 *  @return The error code.
 */
static u32 _newXmlNode(
    internal_xmlparser_xml_node_t ** ppRoot, internal_xmlparser_xml_node_t ** ppCurrent,
    olchar_t * pstrNsTag, olsize_t sNsTag, olchar_t * pstrTagName, olsize_t sTagName,
    boolean_t bStartTag, boolean_t bEmptyTag, jf_linklist_t * pLinklist,
    jf_string_parse_result_field_t * field, jf_string_parse_result_t * pElem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * pixxn = NULL;

    u32Ret = _createXmlNode(&pixxn, pstrTagName, sTagName, pstrNsTag, sNsTag);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pixxn->ixxn_bStartTag = bStartTag;

        if (! bStartTag)
        {
            /*End element.*/
            /*The pstrData in "field" points to the first character after "<".*/
            pixxn->ixxn_pstrSegment = field->jsprf_pstrData;
            /*The segment field of end element points to the first character before "</".*/
            pixxn->ixxn_pstrSegment -= 2;
        }
        else
        {
            /*Start element.*/
            ol_memcpy(&pixxn->ixxn_jlAttribute, pLinklist, sizeof(*pLinklist));
            /*The segment field of start element point to the end of the element (the
              first character after ">").*/
            pixxn->ixxn_pstrSegment = pElem->jspr_pjsprfLast->jsprf_pstrData;
        }

        if (*ppRoot == NULL)
            *ppRoot = pixxn;
        else
            (*ppCurrent)->ixxn_pixxnNext = pixxn;

        *ppCurrent = pixxn;
        if (bEmptyTag)
        {
            /*If this was an empty element, we need to create a bogus end element, just so the
              tree is consistent.*/
            u32Ret = _createXmlNode(&pixxn, pstrTagName, sTagName, pstrNsTag, sNsTag);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pixxn->ixxn_pstrSegment = (*ppCurrent)->ixxn_pstrSegment;
                (*ppCurrent)->ixxn_bEmptyTag = TRUE;
                (*ppCurrent)->ixxn_pixxnNext = pixxn;
                *ppCurrent = pixxn;
            }
        }
    }

    return u32Ret;
}

static u32 _findXmlTagAngleBracket(
    jf_string_parse_result_field_t * field, olsize_t sOffset, olchar_t * pstrDelimiter,
    olsize_t sDelimiter, jf_string_parse_result_t ** ppElem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Look for the '-->' to find the end of comment element.*/
    u32Ret = jf_string_parse(
        ppElem, field->jsprf_pstrData, sOffset, field->jsprf_sData, pstrDelimiter, sDelimiter);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*If number of result is 1, delimiter is not found, the xml element is not complete.*/
        if ((*ppElem)->jspr_u32NumOfResult == 1)
            u32Ret = JF_ERR_INCOMPLETE_XML_DOCUMENT;
    }

    return u32Ret;
}

/** Parse XML element.
 *  
 *  @note
 *  -# For non empty start element, the first field of the element result contains the content
 *     between "<" and ">".
 *  -# For empty start element, the first field of the element result contains the content between
 *     "<" and "/>".
 *  -# For end element, the first field of the element result contains the content between "</"
 *     and ">".
 *  -# For comment element, the first field of the element result contains the content between
 *     "<!--" and "-->".
 *
 *  @param field [in] The parse result with delimiter "<".
 *  @param pbEmptyTag [out] It's an empty tag if TRUE.
 *  @param pbCommentTag [out] It's a comment tag if TRUE.
 *  @param pbStartTag [out] It's a start tag if TRUE. It's an end tag if FALSE.
 *  @param ppElem [out] The parse result with delimiter ">".
 *
 *  @return The error code.
 */
static u32 _parseXmlElement(
    jf_string_parse_result_field_t * field, boolean_t * pbEmptyTag, boolean_t * pbCommentTag,
    boolean_t * pbStartTag, jf_string_parse_result_t ** ppElem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pjspr = NULL;

    *ppElem = NULL;
    *pbEmptyTag = FALSE;
    *pbCommentTag = FALSE;    

    if (ol_memcmp(field->jsprf_pstrData, "!--", 3) == 0)
    {
        *pbCommentTag = TRUE;

        /*Look for the '-->' to find the end of comment element.*/
        u32Ret = _findXmlTagAngleBracket(field, 3, "-->", 3, &pjspr);
    }
    else if (ol_memcmp(field->jsprf_pstrData, "/", 1) == 0)
    {
        /*The first character after the '<' was a '/', so it is the end element.*/
        *pbStartTag = FALSE;

        /*Look for the '>' to find the end of this element.*/
        u32Ret = _findXmlTagAngleBracket(field, 1, ">", 1, &pjspr);
    }
    else
    {
        /*The first character after the '<' was not a '/', so this is a start element.*/
        *pbStartTag = TRUE;

        /*Look for the '>' to find the end of this element.*/
        u32Ret = _findXmlTagAngleBracket(field, 0, ">", 1, &pjspr);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pjspr->jspr_pjsprfFirst->
                jsprf_pstrData[pjspr->jspr_pjsprfFirst->jsprf_sData - 1] == '/')
            {
                /*If this element ended with a '/' this is an empty element.*/
                *pbEmptyTag = TRUE;
                /*Decrease data size by 1 to ignore the '/'.*/
                pjspr->jspr_pjsprfFirst->jsprf_sData --;
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppElem = pjspr;
    else if (pjspr != NULL)
        jf_string_destroyParseResult(&pjspr);
    
    return u32Ret;
}

static u32 _parseXmlElementName(
    jf_string_parse_result_t * pElem, olchar_t ** ppNs, olsize_t * psNs,
    olchar_t ** ppName, olsize_t * psName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pTag = NULL;
    jf_string_parse_result_t * pAttr = NULL;

    /*Parsing on the ' ', isolate the element name from the attributes. The first token is the
      element name.*/
    u32Ret = jf_string_parse(
        &pAttr, pElem->jspr_pjsprfFirst->jsprf_pstrData, 0, pElem->jspr_pjsprfFirst->jsprf_sData,
        " ", 1);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The first token is the element name, it cannot be NULL.*/
        if (pAttr->jspr_pjsprfFirst == NULL)
            u32Ret = JF_ERR_CORRUPTED_XML_DOCUMENT;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Now that we have the token that contains the element name, we need to parse on the ":"
          because we need to figure out what the namespace qualifiers are.*/
        u32Ret = jf_string_parse(
            &pTag, pAttr->jspr_pjsprfFirst->jsprf_pstrData, 0, pAttr->jspr_pjsprfFirst->jsprf_sData,
            ":", 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pTag->jspr_u32NumOfResult == 1)
        {
            /*If there is only one token, there was no namespace prefix. The whole token is the
              attribute name.*/
            *ppNs = NULL;
            *psNs = 0;
            *ppName = pTag->jspr_pjsprfFirst->jsprf_pstrData;
            *psName = pTag->jspr_pjsprfFirst->jsprf_sData;
        }
        else
        {
            /*The first token is the namespace prefix, the second is the attribute name.*/
            *ppNs = pTag->jspr_pjsprfFirst->jsprf_pstrData;
            *psNs = pTag->jspr_pjsprfFirst->jsprf_sData;
            *ppName = pTag->jspr_pjsprfFirst->jsprf_pjsprfNext->jsprf_pstrData;
            *psName = pTag->jspr_pjsprfFirst->jsprf_pjsprfNext->jsprf_sData;
        }
    }

    if (pTag != NULL)
        jf_string_destroyParseResult(&pTag);

    if (pAttr != NULL)
        jf_string_destroyParseResult(&pAttr);

    return u32Ret;
}

static u32 _parseXmlDeclaration(
    internal_xmlparser_xml_doc_t * pixxd, jf_string_parse_result_t * xml,
    jf_string_parse_result_field_t ** ppField)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_field_t * field = NULL;
    jf_string_parse_result_t * pElem = NULL;

    /*At least 3 field.*/
    if (xml->jspr_u32NumOfResult < 3)
        u32Ret = JF_ERR_INCOMPLETE_XML_DOCUMENT;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The field point to "?xml".*/
        field = xml->jspr_pjsprfFirst->jsprf_pjsprfNext;

        if (ol_memcmp(field->jsprf_pstrData, "?xml", 4) != 0)
        {
            u32Ret = JF_ERR_INVALID_XML_DECLARATION;
            genXmlErrMsg(u32Ret, field->jsprf_pstrData, field->jsprf_sData);
        }
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Find the end of declaration.*/
        u32Ret = _findXmlTagAngleBracket(field, 0, "?>", 2, &pElem);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Parse the attribute in the declaration.*/
        u32Ret = parseXmlAttributeList(pElem, &pixxd->ixxd_jlDeclarationAttribute);
    }

    if (pElem != NULL)
        jf_string_destroyParseResult(&pElem);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppField = field->jsprf_pjsprfNext;

    return u32Ret;
}

static u32 _parseXmlNode(
    internal_xmlparser_xml_node_t ** ppRoot, internal_xmlparser_xml_node_t ** ppCurrent,
    jf_string_parse_result_field_t * field, internal_xmlparser_xml_doc_t * pixxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pElem = NULL;
    olchar_t * tagName = NULL;
    olsize_t sTagName = 0;
    boolean_t bStartTag = FALSE, bCommentTag = FALSE, bEmptyTag = FALSE;
    olchar_t * nsTag = NULL;
    olsize_t sNsTag = 0;
    jf_linklist_t jlAttribute;

    u32Ret = _parseXmlElement(field, &bEmptyTag, &bCommentTag, &bStartTag, &pElem);

    /*Parse the XML element name if it's not a comment element*/
    if ((u32Ret == JF_ERR_NO_ERROR) && (! bCommentTag))
    {
        u32Ret = _parseXmlElementName(pElem, &nsTag, &sNsTag, &tagName, &sTagName);
    }

    /*Parse the XML attribute if it's a start element*/
    if ((u32Ret == JF_ERR_NO_ERROR) && bStartTag)
    {
        jf_linklist_init(&jlAttribute);

        u32Ret = parseXmlAttributeList(pElem, &jlAttribute);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sTagName != 0))
    {
        /*All the tags are linked into the retval with next field in XML node.*/
        u32Ret = _newXmlNode(
            ppRoot, ppCurrent, nsTag, sNsTag, tagName, sTagName, bStartTag, bEmptyTag,
            &jlAttribute, field, pElem);
    }

    if (pElem != NULL)
        jf_string_destroyParseResult(&pElem);

    return u32Ret;
}

/** Parse the XML document.
 *
 *  @note
 *  -# The comment element are ignored.
 */
static u32 _parseXml(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, internal_xmlparser_xml_doc_t * pixxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * xml = NULL;
    jf_string_parse_result_field_t * field = NULL;
    internal_xmlparser_xml_node_t * retval = NULL;
    internal_xmlparser_xml_node_t * current = NULL;

    /* All XML elements start with a '<' character */
    u32Ret = jf_string_parse(&xml, pstrBuf, sOffset, sBuf, "<", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseXmlDeclaration(pixxd, xml, &field);
    }

    while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _parseXmlNode(&retval, &current, field, pixxd);

        field = field->jsprf_pjsprfNext;
    }

    if (xml != NULL)
        jf_string_destroyParseResult(&xml);

    if (u32Ret == JF_ERR_NO_ERROR)
        pixxd->ixxd_pixxnRoot = retval;
    else if (retval != NULL)
        _destroyXmlNodeList(&retval);

    return u32Ret;
}

/** Process XML node list and link the node to a tree.
 *
 *  @param pixxd [in] The XML tree to process.
 *
 *  @return The error code.
 */
static u32 _processXmlNodeList(internal_xmlparser_xml_doc_t * pixxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * current = pixxd->ixxd_pixxnRoot;
    internal_xmlparser_xml_node_t * temp = NULL, * parent = NULL;
    jf_stack_t * tagStack = NULL;

    jf_stack_init(&tagStack);
    /*Iterate through the node list, and setup all the pointers such that all start elements have
      pointers to end elements, and all start elements have pointers to siblings and parents.*/
    while (current != NULL)
    {
        if (current->ixxn_bStartTag)
        {
            /*Start Tag.*/
            parent = jf_stack_peek(&tagStack);
            current->ixxn_pixxnParent = parent;
            if ((parent != NULL) && (parent->ixxn_pixxnChildren == NULL))
                parent->ixxn_pixxnChildren = current;
            jf_stack_push(&tagStack, current);
        }
        else
        {
            /*End Tag.*/
            /*There is supposed to be an start element.*/
            temp = (internal_xmlparser_xml_node_t *)jf_stack_pop(&tagStack);
            if (temp != NULL)
            {
                /*Checking to see if the start element and end element are correct in scope */
                if ((temp->ixxn_sName == current->ixxn_sName) &&
                    (ol_memcmp(
                        temp->ixxn_pstrName, current->ixxn_pstrName, current->ixxn_sName) == 0))
                {
                    /*The start element is correct, set the next XML node to the sibling of
                      start element.*/
                    if (current->ixxn_pixxnNext != NULL)
                    {
                        /*The next XML node must be a start element*/
                        if (current->ixxn_pixxnNext->ixxn_bStartTag)
                        {
                            temp->ixxn_pixxnSibling = current->ixxn_pixxnNext;
                        }
                    }
                    temp->ixxn_pixxnPairTag = current;
                    current->ixxn_pixxnPairTag = temp;
                }
                else
                {
                    /*The start element is not correct, unmatched close tag.*/
                    u32Ret = JF_ERR_UNMATCHED_XML_CLOSE_TAG;
                    genXmlErrMsg(u32Ret, temp->ixxn_pstrName, temp->ixxn_sName);
                    break;
                }
            }
            else
            {
                /*No start element is found, illegal close tag.*/
                u32Ret = JF_ERR_ILLEGAL_XML_CLOSE_TAG;
                break;
            }
        }
        current = current->ixxn_pixxnNext;
    }

    /* If there are still elements in the stack, that means not all the start elements have
       associated end elements, which means this XML is not valid XML */
    if (tagStack != NULL)
    {
        /* Incomplete XML */
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = JF_ERR_INCOMPLETE_XML_DOCUMENT;
        jf_stack_clear(&tagStack);
    }

    return u32Ret;
}

static u32 _validateXmlDoc(internal_xmlparser_xml_doc_t * pixxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * root = pixxd->ixxd_pixxnRoot;

    /*make sure the document has only 1 root element*/
    if (root->ixxn_pixxnSibling != NULL)
        u32Ret = JF_ERR_NOT_UNIQUE_XML_ROOT_ELEMENT;

    return u32Ret;
}

static u32 _readXmlNodeContent(internal_xmlparser_xml_node_t * pixxn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBuf = 0;
    internal_xmlparser_xml_node_t * temp = NULL;

    temp = pixxn;
    while ((temp != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if (temp->ixxn_pixxnChildren != NULL)
        {
            _readXmlNodeContent(temp->ixxn_pixxnChildren);
        }
        else
        {
            /*The segment fields of the start element and end element are used as pointers
              representing the data segment of the XML*/
            sBuf = temp->ixxn_pixxnPairTag->ixxn_pstrSegment - temp->ixxn_pstrSegment + 1;
            if (sBuf > 0)
            {
                temp->ixxn_sContent = sBuf;
                u32Ret = jf_string_duplicateWithLen(
                    &temp->ixxn_pstrContent, temp->ixxn_pstrSegment, temp->ixxn_sContent);
            }
        }

        temp = temp->ixxn_pixxnSibling;
    }

    return u32Ret;
}

/** Builds the namespace hash table.
 *
 *  @param root [in] This root node of the XML tree.
 *
 *  @return The error code.
 */
static u32 _buildXmlNamespaceTable(internal_xmlparser_xml_node_t * root)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if 0
    internal_xmlparser_xml_attribute_t * attr = NULL;
    internal_xmlparser_xml_node_t * current = root;
    jf_linklist_node_t * pNode = NULL;

    /*Iterate through all the start elements, and build a table of the declared namespaces.*/
    while (current != NULL)
    {
        /*Ignore the end element.*/
        if (! current->ixxn_bStartTag)
        {
            current = current->ixxn_pixxnNext;
            continue;
        }

        /*Iterate through all the attributes to find namespace declarations.*/
        pNode = jf_linklist_getFirstNode(&current->ixxn_jlAttribute);
        while (pNode != NULL)
        {
            attr = jf_linklist_getDataFromNode(pNode);

            if (attr->ixxa_sName == 5 &&
                ol_memcmp(attr->ixxa_pstrName, "xmlns", 5) == 0)
            {
                /*Default namespace declaration.*/
                attr->ixxa_pstrValue[attr->ixxa_sValue] = 0;
                jf_hashtree_addEntry(
                    &current->ixxn_jhNameSpace, "xmlns", 5, attr->ixxa_pstrValue);
            }
            else if (attr->ixxa_sPrefix == 5 &&
                     ol_memcmp(attr->ixxa_pstrPrefix, "xmlns", 5) == 0)
            {
                /*Other namespace declaration.*/
                attr->ixxa_pstrValue[attr->ixxa_sValue] = 0;
                jf_hashtree_addEntry(
                    &current->ixxn_jhNameSpace, attr->ixxa_pstrName, attr->ixxa_sName,
                    attr->ixxa_pstrValue);
            }

            pNode = jf_linklist_getNextNode(pNode);
        }

        current = current->ixxn_pixxnNext;
    }
#endif
    return u32Ret;
}

static u32 _findXmlNode(
    internal_xmlparser_xml_node_t * pixxn, olchar_t * pstrName, olsize_t sName,
    internal_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_XML_NODE_NOT_FOUND;

    *ppNode = NULL;
    while (pixxn != NULL)
    {
        if ((pixxn->ixxn_sName == sName) && (ol_memcmp(pixxn->ixxn_pstrName, pstrName, sName) == 0))
        {
            *ppNode = pixxn;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pixxn = pixxn->ixxn_pixxnSibling;
    }

    return u32Ret;
}

static u32 _locateXmlNode(
    internal_xmlparser_xml_doc_t * pixxd, olchar_t * pstrNodeName,
    jf_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pName = NULL;
    jf_string_parse_result_field_t * field = NULL;
    u32 u32Index = 0;
    internal_xmlparser_xml_node_t * pixxn = pixxd->ixxd_pixxnRoot, * pNode = NULL;

    *ppNode = NULL;

    u32Ret = jf_string_parse(&pName, pstrNodeName, 0, ol_strlen(pstrNodeName), ".", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = pName->jspr_pjsprfFirst;
        for (u32Index = 0; (u32Index < pName->jspr_u32NumOfResult) && (pixxn != NULL); u32Index ++)
        {
            u32Ret = _findXmlNode(pixxn, field->jsprf_pstrData, field->jsprf_sData, &pNode);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pixxn = pNode->ixxn_pixxnChildren;

                field = field->jsprf_pjsprfNext;
            }
            else
            {
                break;
            }
        }
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (u32Index == pName->jspr_u32NumOfResult))
        *ppNode = pNode;

    if (pName != NULL)
        jf_string_destroyParseResult(&pName);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_xmlparser_lookupXmlNamespace(
    jf_xmlparser_xml_node_t * pNode, olchar_t * pstrPrefix, olsize_t sPrefix, olchar_t ** ppstr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * temp = (internal_xmlparser_xml_node_t *)pNode;

    /*If the specified prefix is zero length, we interpret that to mean they want to lookup the
      default namespace.*/
    if (sPrefix == 0)
    {
        /*This is the default namespace prefix.*/
        pstrPrefix = "xmlns";
        sPrefix = 5;
    }

    /*From the current node, keep traversing up the parents, until we find a match. Each step we go
      up, is a step wider in scope.*/
    do
    {
        if (jf_hashtree_hasEntry(&temp->ixxn_jhNameSpace, pstrPrefix, sPrefix))
        {
            /*As soon as we find the namespace declaration, stop iterating the tree, as it would be
              a waste of time.*/
            jf_hashtree_getEntry(
                &temp->ixxn_jhNameSpace, pstrPrefix, sPrefix, (void **)ppstr);
            break;
        }

        temp = temp->ixxn_pixxnParent;
    } while ((temp != NULL) && (u32Ret == JF_ERR_NO_ERROR));

    return u32Ret;
}

u32 jf_xmlparser_parseXmlDoc(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, jf_xmlparser_xml_doc_t ** ppDoc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_doc_t * pixxd = NULL;

    *ppDoc = NULL;
    initXmlErrMsg();

    u32Ret = jf_jiukun_allocMemory((void **)&pixxd, sizeof(*pixxd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pixxd, sizeof(*pixxd));
        jf_linklist_init(&pixxd->ixxd_jlDeclarationAttribute);

        u32Ret = _parseXml(pstrBuf, sOffset, sBuf, pixxd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _processXmlNodeList(pixxd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _validateXmlDoc(pixxd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _readXmlNodeContent(pixxd->ixxd_pixxnRoot);
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _buildXmlNamespaceTable(pixxd->ixxd_pixxnRoot);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        tryGenXmlErrMsg(u32Ret, NULL, 0);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDoc = pixxd;
    else if (pixxd != NULL)
        jf_xmlparser_destroyXmlDoc((jf_xmlparser_xml_doc_t **)&pixxd);

    return u32Ret;
}

u32 jf_xmlparser_destroyXmlDoc(jf_xmlparser_xml_doc_t ** ppDoc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_doc_t * pixxd = (internal_xmlparser_xml_doc_t *)*ppDoc;

    destroyXmlAttributeList(&pixxd->ixxd_jlDeclarationAttribute);

    if (pixxd->ixxd_pixxnRoot != NULL)
        u32Ret = _destroyXmlNodeList(&pixxd->ixxd_pixxnRoot);

    jf_jiukun_freeMemory(ppDoc);

    return u32Ret;
}

u32 jf_xmlparser_getXmlNode(
    jf_xmlparser_xml_doc_t * pjxxd, olchar_t * pstrNodeName, jf_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_doc_t * pixxd = (internal_xmlparser_xml_doc_t *) pjxxd;

    u32Ret = _locateXmlNode(pixxd, pstrNodeName, ppNode);
    
    return u32Ret;
}

u32 jf_xmlparser_getContentOfNode(jf_xmlparser_xml_node_t * pNode, olchar_t ** ppStr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_node_t * pixxn = (internal_xmlparser_xml_node_t *) pNode;

    *ppStr = pixxn->ixxn_pstrContent;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

