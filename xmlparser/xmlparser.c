/**
 *  @file xmlparser.c
 *
 *  @brief Implementation file for XML parse routines.
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
#include "jf_string.h"
#include "jf_xmlparser.h"
#include "jf_stack.h"
#include "jf_jiukun.h"
#include "jf_file.h"
#include "jf_filestream.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/** Frees resources from an XML node list that was returned from parseXML
 *
 *  @param ppNode [in/out] The XML node to clean up.
 */
static u32 _destroyXMLNodeList(jf_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t * temp;
    jf_xmlparser_xml_node_t * node = *ppNode;

    while (node != NULL)
    {
        temp = node->jxxn_pjxxnNext;

        /*If there was a namespace table, delete it*/
        jf_hashtree_fini(&node->jxxn_jhNameSpace);

        jf_jiukun_freeMemory((void **)&node);
        node = temp;
    }

    return u32Ret;
}

/** The pjsprfContent is the content between 2 '<'.
 */
static u32 _newXMLNode(
    jf_xmlparser_xml_node_t ** ppRoot, jf_xmlparser_xml_node_t ** ppCurrent,
    olchar_t * pstrTagName, olsize_t sTagName, boolean_t bStartTag,
    boolean_t bEmptyTag, olchar_t * pstrNSTag, olsize_t sNSTag,
    jf_string_parse_result_field_t * pjsprfContent, jf_string_parse_result_t * pElem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t * jxxnNew = NULL;
    olchar_t * pu8Reserved;

    u32Ret = jf_jiukun_allocMemory((void **)&jxxnNew, sizeof(jf_xmlparser_xml_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(jxxnNew, sizeof(jf_xmlparser_xml_node_t));

        jxxnNew->jxxn_pstrName = pstrTagName;
        jxxnNew->jxxn_sName = sTagName;
        jxxnNew->jxxn_bStartTag = bStartTag;
        jxxnNew->jxxn_pstrNs = pstrNSTag;
        jxxnNew->jxxn_sNs = sNSTag;

        if (! bStartTag)
        {
            /* The jxxn_pReserved field of end elements point to the first character before the
               '<' */
            jxxnNew->jxxn_pReserved = pjsprfContent->jsprf_pstrData;
            do
            {
                pu8Reserved = (olchar_t *) jxxnNew->jxxn_pReserved;
                pu8Reserved --;
                jxxnNew->jxxn_pReserved = pu8Reserved;
            } while (*((olchar_t *) jxxnNew->jxxn_pReserved) == '<');
        }
        else
        {
            /* The jxxn_pReserved field of start elements point to the end of the element (the
               data segment) */
            jxxnNew->jxxn_pReserved = pElem->jspr_pjsprfLast->jsprf_pstrData;
        }

        if (*ppRoot == NULL)
        {
            *ppRoot = jxxnNew;
        }
        else
        {
            (*ppCurrent)->jxxn_pjxxnNext = jxxnNew;
        }
        *ppCurrent = jxxnNew;
        if (bEmptyTag)
        {
            /* If this was an empty element, we need to create a bogus EndElement, 
               just so the tree is consistent. No point in introducing unnecessary complexity */
            u32Ret = jf_jiukun_allocMemory((void **)&jxxnNew, sizeof(jf_xmlparser_xml_node_t));
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_memset(jxxnNew, 0, sizeof(jf_xmlparser_xml_node_t));
                jxxnNew->jxxn_pstrName = pstrTagName;
                jxxnNew->jxxn_sName = sTagName;
                jxxnNew->jxxn_pstrNs = pstrNSTag;
                jxxnNew->jxxn_sNs = sNSTag;

                jxxnNew->jxxn_pReserved = (*ppCurrent)->jxxn_pReserved;
                (*ppCurrent)->jxxn_bEmptyTag = TRUE;
                (*ppCurrent)->jxxn_pjxxnNext = jxxnNew;
                *ppCurrent = jxxnNew;
            }
        }
    }

    return u32Ret;
}

static u32 _parseXmlDeclaration(
    jf_xmlparser_xml_doc_t * pjxxd, jf_string_parse_result_t * xml,
    jf_string_parse_result_field_t ** ppField)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_field_t * field = NULL;

    if (xml->jspr_u32NumOfResult < 2)
        u32Ret = JF_ERR_CORRUPTED_XML_FILE;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = xml->jspr_pjsprfFirst->jsprf_pjsprfNext;
        if (field == NULL)
            u32Ret = JF_ERR_CORRUPTED_XML_FILE;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (field->jsprf_sData < 1)
            u32Ret = JF_ERR_INVALID_XML_DECLARATION;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ol_memcmp(field->jsprf_pstrData, "?", 1) != 0)
            u32Ret = JF_ERR_INVALID_XML_DECLARATION;
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*parse the attribute in the declaration*/

    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppField = field->jsprf_pjsprfNext;
    }

    return u32Ret;
}

static u32 _parseXmlElement(
    jf_string_parse_result_field_t * field, boolean_t * pbEmptyTag,
    boolean_t * pbStartTag, jf_string_parse_result_t ** ppElem)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *pbEmptyTag = FALSE;
    if (ol_memcmp(field->jsprf_pstrData, "/", 1) == 0)
    {
        /* The first character after the '<' was a '/', so it is the end element */
        *pbStartTag = FALSE;
        field->jsprf_pstrData ++;
        field->jsprf_sData --;

        /* If we look for the '>' we can find the end of this element */
        u32Ret = jf_string_parse(
            ppElem, field->jsprf_pstrData, 0, field->jsprf_sData, ">", 1);
    }
    else
    {
        /* The first character after the '<' was not a '/', so this is a start element */
        *pbStartTag = TRUE;

        /* If we look for the '>' we can find the end of this element */
        u32Ret = jf_string_parse(
            ppElem, field->jsprf_pstrData, 0, field->jsprf_sData, ">", 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((*ppElem)->jspr_pjsprfFirst->
                jsprf_pstrData[(*ppElem)->jspr_pjsprfFirst->jsprf_sData - 1] == '/')
            {
                /* If this element ended with a '/' this is an EmptyElement */
                *pbEmptyTag = TRUE;
            }
        }
    }

    return u32Ret;
}

static u32 _parseXmlAttribute(
    jf_string_parse_result_t * pElem, jf_string_parse_result_t ** ppAttr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* Parsing on the ' ', isolate the element name from the attributes. The first token is the
       element name */
    u32Ret = jf_string_parse(
        ppAttr, pElem->jspr_pjsprfFirst->jsprf_pstrData, 0, pElem->jspr_pjsprfFirst->jsprf_sData,
        " ", 1);

    return u32Ret;
}

static u32 _parseXmlElementName(
    jf_string_parse_result_t * pAttr, olchar_t ** ppNs, olsize_t * psNs,
    olchar_t ** ppName, olsize_t * psName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pTag = NULL;

    /* Now that we have the token that contains the element name, we need to parse on the ":"
       because we need to figure out what the namespace qualifiers are */
    u32Ret = jf_string_parse(
        &pTag, pAttr->jspr_pjsprfFirst->jsprf_pstrData, 0, pAttr->jspr_pjsprfFirst->jsprf_sData,
        ":", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pTag->jspr_u32NumOfResult == 1)
        {
            /* If there is only one token, there was no namespace prefix. The whole token is the
               attribute name */
            *ppNs = NULL;
            *psNs = 0;
            *ppName = pTag->jspr_pjsprfFirst->jsprf_pstrData;
            *psName = pTag->jspr_pjsprfFirst->jsprf_sData;
        }
        else
        {
            /* The first token is the namespace prefix, the second is the attribute name */
            *ppNs = pTag->jspr_pjsprfFirst->jsprf_pstrData;
            *psNs = pTag->jspr_pjsprfFirst->jsprf_sData;
            *ppName = pTag->jspr_pjsprfFirst->jsprf_pjsprfNext->jsprf_pstrData;
            *psName = pTag->jspr_pjsprfFirst->jsprf_pjsprfNext->jsprf_sData;
        }

        jf_string_destroyParseResult(&pTag);
    }

    return u32Ret;
}

/** Parse the XML document.
 */
static u32 _parseXML(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, jf_xmlparser_xml_doc_t * pjxxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * xml = NULL;
    jf_string_parse_result_field_t * field = NULL;
    jf_string_parse_result_t * pAttr = NULL;
    jf_string_parse_result_t * pElem = NULL;
    olchar_t * tagName = NULL;
    olsize_t sTagName = 0;
    boolean_t bStartTag = FALSE;
    boolean_t bEmptyTag = FALSE;
    jf_xmlparser_xml_node_t * retval = NULL;
    jf_xmlparser_xml_node_t * current = NULL;
    olchar_t * nsTag = NULL;
    olsize_t sNsTag = 0;

    /* All XML elements start with a '<' character */
    u32Ret = jf_string_parse(&xml, pstrBuf, sOffset, sBuf, "<", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseXmlDeclaration(pjxxd, xml, &field);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (field == NULL)
            u32Ret = JF_ERR_CORRUPTED_XML_FILE;
    }

    while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _parseXmlElement(field, &bEmptyTag, &bStartTag, &pElem);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseXmlAttribute(pElem, &pAttr);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _parseXmlElementName(
                pAttr, &nsTag, &sNsTag, &tagName, &sTagName);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (sTagName != 0)
            {
                u32Ret = _newXMLNode(
                    &retval, &current, tagName, sTagName, bStartTag, bEmptyTag, nsTag, sNsTag,
                    field, pElem);
            }
        }

        if (pElem != NULL)
            jf_string_destroyParseResult(&pElem);

        if (pAttr != NULL)
            jf_string_destroyParseResult(&pAttr);

        field = field->jsprf_pjsprfNext;
    }

    if (xml != NULL)
        jf_string_destroyParseResult(&xml);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjxxd->jxxd_pjxxnRoot = retval;
    }
    else if (retval != NULL)
    {
        _destroyXMLNodeList(&retval);
    }

    return u32Ret;
}

/** Checks XML for validity, while at the same time populate helper properties on each node.
 *  This method call will populate various helper properties such as Parent, Peer, etc, to aid in
 *  XML parsing.
 *
 *  @param pjxxd [in] The XML tree to process.
 *
 *  @return the error code
 */
static u32 _processXMLNodeList(jf_xmlparser_xml_doc_t * pjxxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t * current = pjxxd->jxxd_pjxxnRoot;
    jf_xmlparser_xml_node_t * temp, * parent;
    void * TagStack = NULL;

    jf_stack_init(&TagStack);
    /* Iterate through the node list, and setup all the pointers such that all StartElements have
       pointers to EndElements, And all StartElements have pointers to siblings and parents. */
    while (current != NULL)
    {
        if (current->jxxn_bStartTag)
        {
            /* Start Tag */
            parent = jf_stack_peek(&TagStack);
            current->jxxn_pjxxnParent = parent;
            if ((parent != NULL) && (parent->jxxn_pjxxnChildren == NULL))
                parent->jxxn_pjxxnChildren = current;
            jf_stack_push(&TagStack, current);
        }
        else
        {
            /* Close Tag */
            /* Check to see if there is supposed to be an EndElement */
            temp = (jf_xmlparser_xml_node_t *)jf_stack_pop(&TagStack);
            if (temp != NULL)
            {
                /* Checking to see if this end element is correct in scope */
                if ((temp->jxxn_sName == current->jxxn_sName) &&
                    (ol_memcmp(
                        temp->jxxn_pstrName, current->jxxn_pstrName,
                        current->jxxn_sName) == 0))
                {
                    /* The end element is correct, set the peer pointers of the previous sibling */
                    if (current->jxxn_pjxxnNext != NULL)
                    {
                        if (current->jxxn_pjxxnNext->jxxn_bStartTag)
                        {
                            temp->jxxn_pjxxnSibling = current->jxxn_pjxxnNext;
                        }
                    }
                    temp->jxxn_pjxxnClosingTag = current;
                    current->jxxn_pjxxnStartingTag = temp;
                }
                else
                {
                    /* Illegal Close Tag Order */
                    pjxxd->jxxd_pjxxnError = temp;
                    u32Ret = JF_ERR_UNMATCHED_CLOSE_TAG;
                    break;
                }
            }
            else
            {
                /* Illegal Close Tag */
                pjxxd->jxxd_pjxxnError = current;
                u32Ret = JF_ERR_ILLEGAL_CLOSE_TAG;
                break;
            }
        }
        current = current->jxxn_pjxxnNext;
    }

    /* If there are still elements in the stack, that means not all the start elements have
       associated end elements, which means this XML is not valid XML */
    if (TagStack != NULL)
    {
        /* Incomplete XML */
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = JF_ERR_INCOMPLETE_XML;
        jf_stack_clear(&TagStack);
    }

    return u32Ret;
}

static u32 _getXmlAttribute(
    jf_string_parse_result_field_t * field, jf_xmlparser_xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_attribute_t * retval = NULL;
    jf_xmlparser_xml_attribute_t * current = NULL;
    jf_string_parse_result_t * pAttr = NULL;
    jf_string_parse_result_t * pValue = NULL;

    /* Iterate through all the other tokens, as these are all attributes */
    while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pAttr = pValue = NULL;
        if (retval == NULL)
        {
            /* If we haven't already created an attribute node, create it now */
            u32Ret = jf_jiukun_allocMemory((void **)&retval, sizeof(jf_xmlparser_xml_attribute_t));
        }
        else
        {
            ol_bzero(retval, sizeof(jf_xmlparser_xml_attribute_t));
            /* We already created an attribute node, so simply create a new one, and attach it on
               the beginning of the old one. */
            u32Ret = jf_jiukun_allocMemory((void **)&current, sizeof(jf_xmlparser_xml_attribute_t));
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_bzero(current, sizeof(jf_xmlparser_xml_attribute_t));

                current->jxxa_pjxxaNext = retval;
                retval = current;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /* Parse each token by the ':'. If this results is more than one token, we can figure
               that the first token is the namespace prefix */
            u32Ret = jf_string_parseAdv(
                &pAttr, field->jsprf_pstrData, 0, field->jsprf_sData, ":", 1);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pAttr->jspr_u32NumOfResult == 1)
            {
                /* This attribute has no prefix, so just parse on the '='. The first token is the
                   attribute name, the other is the value */
                retval->jxxa_pstrPrefix = NULL;
                retval->jxxa_sPrefix = 0;
                u32Ret = jf_string_parseAdv(
                    &pValue, field->jsprf_pstrData, 0, field->jsprf_sData, "=", 1);
            }
            else
            {
                /* Since there is a namespace prefix, seperate that out, and parse the remainder on
                   the '=' to figure out what the attribute name and value are */
                retval->jxxa_pstrPrefix = pAttr->jspr_pjsprfFirst->jsprf_pstrData;
                retval->jxxa_sPrefix = pAttr->jspr_pjsprfFirst->jsprf_sData;
                u32Ret = jf_string_parseAdv(
                    &pValue, field->jsprf_pstrData, retval->jxxa_sPrefix + 1,
                    field->jsprf_sData - retval->jxxa_sPrefix - 1, "=", 1);
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            retval->jxxa_pstrName = pValue->jspr_pjsprfFirst->jsprf_pstrData;
            retval->jxxa_sName = pValue->jspr_pjsprfFirst->jsprf_sData;
            retval->jxxa_pstrValue = pValue->jspr_pjsprfLast->jsprf_pstrData;
            retval->jxxa_sValue = pValue->jspr_pjsprfLast->jsprf_sData;
        }

        if (pAttr != NULL)
            jf_string_destroyParseResult(&pAttr);

        if (pValue != NULL)
            jf_string_destroyParseResult(&pValue);

        field = field->jsprf_pjsprfNext;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAttribute = retval;
    else if (retval != NULL)
        jf_xmlparser_destroyXMLAttributeList(&retval);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_xmlparser_destroyXMLAttributeList(jf_xmlparser_xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_attribute_t * temp;
    jf_xmlparser_xml_attribute_t * attribute = *ppAttribute;

    while (attribute != NULL)
    {
        temp = attribute->jxxa_pjxxaNext;
        jf_jiukun_freeMemory((void **)&attribute);
        attribute = temp;
    }

    return u32Ret;
}

/** Resolves a namespace prefix from the scope of the given node.
 *
 *  @param node [in] the node used to start the resolve
 *  @param prefix [in] the namespace prefix to resolve
 *  @param sPrefix [in] the lenght of the prefix
 *  @param ppstr [out] the resolved namespace, NULL if unable to resolve
 *
 *  @return the error code
 */
u32 jf_xmlparser_lookupXMLNamespace(
    jf_xmlparser_xml_node_t * node, olchar_t * prefix, olsize_t sPrefix, olchar_t ** ppstr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t * temp = node;

    /*If the specified prefix is zero length, we interpret that to mean they want to lookup the
      default namespace*/
    if (sPrefix == 0)
    {
        /*This is the default namespace prefix*/
        prefix = "xmlns";
        sPrefix = 5;
    }

    /*From the current node, keep traversing up the parents, until we find a match. Each step we go
      up, is a step wider in scope.*/
    do
    {
        if (jf_hashtree_hasEntry(&temp->jxxn_jhNameSpace, prefix, sPrefix))
        {
            /*As soon as we find the namespace declaration, stop iterating the tree, as it would be
              a waste of time*/
            jf_hashtree_getEntry(
                &temp->jxxn_jhNameSpace, prefix, sPrefix, (void **)ppstr);
            break;
        }

        temp = temp->jxxn_pjxxnParent;
    } while ((temp != NULL) && (u32Ret == JF_ERR_NO_ERROR));

    return u32Ret;
}

/** Builds the lookup table used by lookupXMLNamespace.
 *
 *  @param node [in] This node will be the highest scoped.
 *
 *  @return the error code
 */
u32 jf_xmlparser_buildXMLNamespaceLookupTable(jf_xmlparser_xml_node_t * node)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_attribute_t *attr, *currentAttr;
    jf_xmlparser_xml_node_t *current = node;

    /*Iterate through all the start elements, and build a table of the declared namespaces*/
    while (current != NULL)
    {
        if (! current->jxxn_bStartTag)
        {
            current = current->jxxn_pjxxnNext;
            continue;
        }

        /*jxxn_jhNameSpace is the hash table containing the fully qualified namespace keyed by the
          namespace prefix*/
        jf_hashtree_init(&current->jxxn_jhNameSpace);

        u32Ret = jf_xmlparser_getXMLAttributes(current, &attr);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            currentAttr = attr;
            /*Iterate through all the attributes to find namespace declarations*/
            while (currentAttr != NULL)
            {
                if (currentAttr->jxxa_sName == 5 &&
                    ol_memcmp(currentAttr->jxxa_pstrName, "xmlns", 5) == 0)
                {
                    /*default namespace declaration*/
                    currentAttr->jxxa_pstrValue[currentAttr->jxxa_sValue] = 0;
                    jf_hashtree_addEntry(
                        &current->jxxn_jhNameSpace, "xmlns", 5,
                        currentAttr->jxxa_pstrValue);
                }
                else if (currentAttr->jxxa_sPrefix == 5 &&
                         ol_memcmp(currentAttr->jxxa_pstrPrefix, "xmlns", 5) == 0)
                {
                    /*Other Namespace Declaration*/
                    currentAttr->jxxa_pstrValue[currentAttr->jxxa_sValue] = 0;
                    jf_hashtree_addEntry(
                        &current->jxxn_jhNameSpace, currentAttr->jxxa_pstrName,
                        currentAttr->jxxa_sName, currentAttr->jxxa_pstrValue);
                }
                currentAttr = currentAttr->jxxa_pjxxaNext;
            }
            jf_xmlparser_destroyXMLAttributeList(&attr);
        }

        current = current->jxxn_pjxxnNext;
    }

    return u32Ret;
}

u32 jf_xmlparser_readInnerXML(
    jf_xmlparser_xml_node_t * node, olchar_t ** ppstrData, olsize_t * psData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_node_t *x = node;
    olsize_t sBuf = 0;
    jf_stack_t *tagStack;

    /*Starting with the current start element, we use this stack to find the matching end element,
      so we can figure out what we need to return*/
    jf_stack_init(&tagStack);
    do
    {
        if (x->jxxn_bStartTag)
        {
            jf_stack_push(&tagStack, x);
        }
        x = x->jxxn_pjxxnNext;

        if (x->jxxn_sName != node->jxxn_sName)
            continue;
        
        if (ol_memcmp(x->jxxn_pstrName, node->jxxn_pstrName, node->jxxn_sName) != 0)
            continue;
    } while (! ((! x->jxxn_bStartTag) && (jf_stack_pop(&tagStack) == node)));

    jf_stack_clear(&tagStack);

    /*The jxxn_pReserved fields of the start element and end element are used as pointers
      representing the data segment of the XML*/
    sBuf = (olsize_t)((olchar_t *)x->jxxn_pReserved - (olchar_t *)node->jxxn_pReserved - 1);
    if (sBuf < 0)
    {
        sBuf = 0;
    }
    *ppstrData = (olchar_t *) node->jxxn_pReserved;
    *psData = sBuf;

    return u32Ret;
}

u32 jf_xmlparser_getXMLAttributes(
    jf_xmlparser_xml_node_t * node, jf_xmlparser_xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t *c;
    olint_t nEndReserved = (! node->jxxn_bEmptyTag) ? 1 : 2;
    jf_string_parse_result_t * xml;
    jf_string_parse_result_field_t * field;

    /* The reserved field is used to show where the data segments start and stop. We can also use
       them to figure out where the attributes start and stop */
    c = (olchar_t *) node->jxxn_pReserved - 1;
    while (*c != '<')
    {
        /* The jxxn_pReserved field of the start element points to the first character after the
           '>' of the start element. Just work our way backwards to find the start of the start
           element */
        c = c - 1;
    }
    c = c + 1;

    /* Now that we isolated the string between the '<' and the '>', we can parse the string as
       delimited by ' '. Use jf_string_parseAdv because these attributes can be within quotation
       marks */
    u32Ret = jf_string_parseAdv(
        &xml, c, 0, ((olchar_t *) node->jxxn_pReserved - c - nEndReserved), " ", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = xml->jspr_pjsprfFirst;

        /* Skip the first token, because the first token is the Element name */
        if (field != NULL)
        {
            field = field->jsprf_pjsprfNext;
        }

        u32Ret = _getXmlAttribute(field, ppAttribute);
    }

    return u32Ret;
}

u32 jf_xmlparser_parseXML(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, jf_xmlparser_xml_doc_t * pjxxd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pjxxd, sizeof(jf_xmlparser_xml_doc_t));

    u32Ret = _parseXML(pstrBuf, sOffset, sBuf, pjxxd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _processXMLNodeList(pjxxd);
    }

    return u32Ret;
}

u32 jf_xmlparser_destroyXMLNodeList(jf_xmlparser_xml_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _destroyXMLNodeList(ppNode);

    return u32Ret;
}

u32 jf_xmlparser_parseXMLFile(
    const olchar_t * pstrFilename, jf_xmlparser_xml_file_t ** ppFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olsize_t sSize;
    jf_file_stat_t filestat;
    jf_xmlparser_xml_file_t * pjxxf = NULL;

    assert((pstrFilename != NULL) && (ppFile != NULL));

    u32Ret = jf_jiukun_allocMemory((void **)&pjxxf, sizeof(*pjxxf));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pjxxf, sizeof(*pjxxf));
        u32Ret = jf_file_getStat(pstrFilename, &filestat);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sSize = (olsize_t)filestat.jfs_u64Size;
        u32Ret = jf_jiukun_allocMemory((void **)&(pjxxf->jxxf_pstrBuf), sSize);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_open(pstrFilename, "r", &fp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_readn(fp, pjxxf->jxxf_pstrBuf, &sSize);
        if (sSize != (u32)filestat.jfs_u64Size)
            u32Ret = JF_ERR_CORRUPTED_XML_FILE;
    }

    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_xmlparser_parseXML(pjxxf->jxxf_pstrBuf, 0, sSize, &(pjxxf->jxxf_jxxdDoc));
    }

    if (pjxxf != NULL)
        *ppFile = pjxxf;

    return u32Ret;
}

u32 jf_xmlparser_destroyXMLFile(jf_xmlparser_xml_file_t ** ppFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_xmlparser_xml_file_t * pjxxf;

    assert((ppFile != NULL) && (*ppFile != NULL));

    pjxxf = *ppFile;

    if (pjxxf->jxxf_jxxdDoc.jxxd_pjxxnRoot != NULL)
        _destroyXMLNodeList(&(pjxxf->jxxf_jxxdDoc.jxxd_pjxxnRoot));

    if (pjxxf->jxxf_pstrBuf != NULL)
        jf_jiukun_freeMemory((void **)&(pjxxf->jxxf_pstrBuf));

    return u32Ret;
}

void jf_xmlparser_printXMLNodeList(jf_xmlparser_xml_node_t * pjxxn, u8 u8Indent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t tagname[128];
    olchar_t value[128];
    olchar_t * pstrValue = NULL;
    olsize_t sBuf = 0;
    jf_xmlparser_xml_node_t * temp = NULL;
    u8 u8Index = 0;

    temp = pjxxn;
    while (temp != NULL)
    {
        memcpy(tagname, temp->jxxn_pstrName, temp->jxxn_sName);
        tagname[temp->jxxn_sName] = '\0';

        for (u8Index = 0; u8Index < u8Indent; u8Index ++)
            ol_printf(" ");

        ol_printf("%s", tagname);

        if (temp->jxxn_pjxxnChildren != NULL)
        {
            ol_printf("\n");
            jf_xmlparser_printXMLNodeList(temp->jxxn_pjxxnChildren, u8Indent + 2);
        }
        else
        {
            u32Ret = jf_xmlparser_readInnerXML(temp, &pstrValue, &sBuf);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                memcpy(value, pstrValue, sBuf);
                value[sBuf] = '\0';
                ol_printf("  %s\n", value);
            }
        }

        temp = temp->jxxn_pjxxnSibling;
    }
}

/*------------------------------------------------------------------------------------------------*/

