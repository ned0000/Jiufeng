/**
 *  @file xmlparser.c
 *
 *  @brief The xml parser library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "stringparse.h"
#include "xmlparser.h"
#include "bases.h"
#include "xmalloc.h"
#include "files.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/** Frees resources from an XML node list that was returned from parseXML
 *
 *  @param ppNode [in/out] the XML tree to clean up
 */
static u32 _destroyXMLNodeList(xml_node_t ** ppNode)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t * temp;
    xml_node_t * node = *ppNode;

    while (node != NULL)
    {
        temp = node->xn_pxnNext;

        /*If there was a namespace table, delete it*/
        finiHashtree(&node->xn_hNameSpace);

        xfree((void **)&node);
        node = temp;
    }

    return u32Ret;
}

/** The pprfContent is the content between 2 '<'
 */
static u32 _newXMLNode(
    xml_node_t ** ppRoot, xml_node_t ** ppCurrent,
    olchar_t * pstrTagName, olsize_t sTagName, boolean_t bStartTag,
    boolean_t bEmptyTag, olchar_t * pstrNSTag, olsize_t sNSTag,
    parse_result_field_t * pprfContent, parse_result_t * pElem)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t * xnNew = NULL;
    olchar_t * pu8Reserved;

    u32Ret = xmalloc((void **)&xnNew, sizeof(xml_node_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(xnNew, 0, sizeof(xml_node_t));

        xnNew->xn_pstrName = pstrTagName;
        xnNew->xn_sName = sTagName;
        xnNew->xn_bStartTag = bStartTag;
        xnNew->xn_pstrNs = pstrNSTag;
        xnNew->xn_sNs = sNSTag;

        if (! bStartTag)
        {
            /* The xn_pReserved field of end elements point to the
               first character before the '<' */
            xnNew->xn_pReserved = pprfContent->prf_pstrData;
            do
            {
                pu8Reserved = (olchar_t *) xnNew->xn_pReserved;
                pu8Reserved --;
                xnNew->xn_pReserved = pu8Reserved;
            } while (*((olchar_t *) xnNew->xn_pReserved) == '<');
        }
        else
        {
            /* The xn_pReserved field of start elements point to the
               end of the element (the data segment) */
            xnNew->xn_pReserved = pElem->pr_pprfLast->prf_pstrData;
        }

        if (*ppRoot == NULL)
        {
            *ppRoot = xnNew;
        }
        else
        {
            (*ppCurrent)->xn_pxnNext = xnNew;
        }
        *ppCurrent = xnNew;
        if (bEmptyTag)
        {
            /* If this was an empty element, we need to create
               a bogus EndElement, 
               just so the tree is consistent. No point in 
               introducing unnecessary complexity */
            u32Ret = xmalloc((void **)&xnNew, sizeof(xml_node_t));
            if (u32Ret == OLERR_NO_ERROR)
            {
                memset(xnNew, 0, sizeof(xml_node_t));
                xnNew->xn_pstrName = pstrTagName;
                xnNew->xn_sName = sTagName;
                xnNew->xn_pstrNs = pstrNSTag;
                xnNew->xn_sNs = sNSTag;

                xnNew->xn_pReserved = (*ppCurrent)->xn_pReserved;
                (*ppCurrent)->xn_bEmptyTag = TRUE;
                (*ppCurrent)->xn_pxnNext = xnNew;
                *ppCurrent = xnNew;
            }
        }
    }

    return u32Ret;
}

static u32 _parseXmlDeclaration(
    parse_result_t * xml, parse_result_field_t ** ppDoc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_field_t * field;

    if (xml->pr_u32NumOfResult < 2)
        u32Ret = OLERR_CORRUPTED_XML_FILE;

    if (u32Ret == OLERR_NO_ERROR)
    {
        field = xml->pr_pprfFirst->prf_pprfNext;
        if (field == NULL)
            u32Ret = OLERR_CORRUPTED_XML_FILE;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (field->prf_sData < 1)
            u32Ret = OLERR_INVALID_XML_DECLARATION;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (memcmp(field->prf_pstrData, "?", 1) != 0)
            u32Ret = OLERR_INVALID_XML_DECLARATION;
    }
    
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*parse the attribute in the declaration*/

    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppDoc = field->prf_pprfNext;
    }

    return u32Ret;
}

static u32 _parseXmlElement(
    parse_result_field_t * field, boolean_t * pbEmptyTag,
    boolean_t * pbStartTag, parse_result_t ** ppElem)
{
    u32 u32Ret = OLERR_NO_ERROR;

    *pbEmptyTag = FALSE;
    if (memcmp(field->prf_pstrData, "/", 1) == 0)
    {
        /* The first character after the '<' was a '/', so it is the end
           element */
        *pbStartTag = FALSE;
        field->prf_pstrData ++;
        field->prf_sData --;

        /* If we look for the '>' we can find the end of this element */
        u32Ret = parseString(
            ppElem, field->prf_pstrData, 0, field->prf_sData, ">", 1);
    }
    else
    {
        /* The first character after the '<' was not a '/', so this is a start
           element */
        *pbStartTag = TRUE;

        /* If we look for the '>' we can find the end of this element */
        u32Ret = parseString(
            ppElem, field->prf_pstrData, 0, field->prf_sData, ">", 1);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if ((*ppElem)->pr_pprfFirst->
                prf_pstrData[(*ppElem)->pr_pprfFirst->prf_sData - 1] == '/')
            {
                /* If this element ended with a '/' this is an EmptyElement */
                *pbEmptyTag = TRUE;
            }
        }
    }

    return u32Ret;
}

static u32 _parseXmlAttribute(
    parse_result_t * pElem, parse_result_t ** ppAttr)
{
    u32 u32Ret = OLERR_NO_ERROR;

    /* Parsing on the ' ', isolate the element name from the attributes.
       The first token is the element name */
    u32Ret = parseString(
        ppAttr, pElem->pr_pprfFirst->prf_pstrData, 0,
        pElem->pr_pprfFirst->prf_sData, " ", 1);

    return u32Ret;
}

static u32 _parseXmlElementName(
    parse_result_t * pAttr, olchar_t ** ppNs, olsize_t * psNs,
    olchar_t ** ppName, olsize_t * psName)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * pTag = NULL;

    /* Now that we have the token that contains the element name,
       we need to parse on the ":" because we need to figure out
       what the namespace qualifiers are */
    u32Ret = parseString(
        &pTag, pAttr->pr_pprfFirst->prf_pstrData, 0,
        pAttr->pr_pprfFirst->prf_sData, ":", 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pTag->pr_u32NumOfResult == 1)
        {
            /* If there is only one token, there was no namespace prefix. 
               The whole token is the attribute name */
            *ppNs = NULL;
            *psNs = 0;
            *ppName = pTag->pr_pprfFirst->prf_pstrData;
            *psName = pTag->pr_pprfFirst->prf_sData;
        }
        else
        {
            /* The first token is the namespace prefix, the second is
               the attribute name */
            *ppNs = pTag->pr_pprfFirst->prf_pstrData;
            *psNs = pTag->pr_pprfFirst->prf_sData;
            *ppName = pTag->pr_pprfFirst->prf_pprfNext->prf_pstrData;
            *psName = pTag->pr_pprfFirst->prf_pprfNext->prf_sData;
        }

        destroyParseResult(&pTag);
    }

    return u32Ret;
}

/** Element: <name>content</name>
 *  Empty element: <name/>
 *  Declaration: <?xml version="1.0"?>
 *  Comment: <!-- comment -->
 */
static u32 _parseXML(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, xml_node_t ** ppNode)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * xml = NULL;
    parse_result_field_t * field;
    parse_result_t * pAttr = NULL;
    parse_result_t * pElem = NULL;
    olchar_t * tagName;
    olsize_t sTagName;
    boolean_t bStartTag;
    boolean_t bEmptyTag;
    xml_node_t * retval = NULL;
    xml_node_t * current = NULL;
    olchar_t * nsTag;
    olsize_t sNsTag;

    /* All XML elements start with a '<' character */
    u32Ret = parseString(&xml, pstrBuf, sOffset, sBuf, "<", 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _parseXmlDeclaration(xml, &field);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (field == NULL)
            u32Ret = OLERR_CORRUPTED_XML_FILE;
    }

    while ((field != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        u32Ret = _parseXmlElement(field, &bEmptyTag, &bStartTag, &pElem);

        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = _parseXmlAttribute(pElem, &pAttr);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = _parseXmlElementName(
                pAttr, &nsTag, &sNsTag, &tagName, &sTagName);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            if (sTagName != 0)
            {
                u32Ret = _newXMLNode(
                    &retval, &current, tagName, sTagName, bStartTag,
                    bEmptyTag, nsTag, sNsTag, field, pElem);
            }
        }

        if (pElem != NULL)
            destroyParseResult(&pElem);

        if (pAttr != NULL)
            destroyParseResult(&pAttr);

        field = field->prf_pprfNext;
    }

    if (xml != NULL)
        destroyParseResult(&xml);

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppNode = retval;
    }
    else if (retval != NULL)
    {
        _destroyXMLNodeList(&retval);
    }

    return u32Ret;
}

/** Checks XML for validity, while at the same time populate helper properties
 *  on each node This method call will populate various helper properties such
 *  as Parent, Peer, etc, to aid in XML parsing.
 *
 *  @param pxd [in] the XML Tree to process
 *
 *  @return the error code
 */
static u32 _processXMLNodeList(xml_doc_t * pxd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t * current = pxd->xd_pxnRoot;
    xml_node_t * temp, * parent;
    void * TagStack = NULL;

    initStack(&TagStack);
    /* Iterate through the node list, and setup all the pointers
       such that all StartElements have pointers to EndElements,
       And all StartElements have pointers to siblings and parents. */
    while (current != NULL)
    {
        if (current->xn_bStartTag)
        {
            /* Start Tag */
            parent = peekStack(&TagStack);
            current->xn_pxnParent = parent;
            if ((parent != NULL) && (parent->xn_pxnChildren == NULL))
                parent->xn_pxnChildren = current;
            pushStack(&TagStack, current);
        }
        else
        {
            /* Close Tag */
            /* Check to see if there is supposed to be an EndElement */
            temp = (xml_node_t *)popStack(&TagStack);
            if (temp != NULL)
            {
                /* Checking to see if this end element is correct in scope */
                if ((temp->xn_sName == current->xn_sName) &&
                    (ol_memcmp(
                        temp->xn_pstrName, current->xn_pstrName,
                        current->xn_sName) == 0))
                {
                    /* The end element is correct, set the peer pointers of the
                       previous sibling */
                    if (current->xn_pxnNext != NULL)
                    {
                        if (current->xn_pxnNext->xn_bStartTag)
                        {
                            temp->xn_pxnSibling = current->xn_pxnNext;
                        }
                    }
                    temp->xn_pxnClosingTag = current;
                    current->xn_pxnStartingTag = temp;
                }
                else
                {
                    /* Illegal Close Tag Order */
                    pxd->xd_pxnError = temp;
                    u32Ret = OLERR_UNMATCHED_CLOSE_TAG;
                    break;
                }
            }
            else
            {
                /* Illegal Close Tag */
                pxd->xd_pxnError = current;
                u32Ret = OLERR_ILLEGAL_CLOSE_TAG;
                break;
            }
        }
        current = current->xn_pxnNext;
    }

    /* If there are still elements in the stack, that means not all the
       start elements have associated end elements, which means this XML
       is not valid XML */
    if (TagStack != NULL)
    {
        /* Incomplete XML */
        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = OLERR_INCOMPLETE_XML;
        clearStack(&TagStack);
    }

    return u32Ret;
}

static u32 _getXmlAttribute(
    parse_result_field_t * field, xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_attribute_t * retval = NULL;
    xml_attribute_t * current = NULL;
    parse_result_t * pAttr;
    parse_result_t * pValue;

    /* Iterate through all the other tokens, as these are all attributes */
    while ((field != NULL) && (u32Ret == OLERR_NO_ERROR))
    {
        pAttr = pValue = NULL;
        if (retval == NULL)
        {
            /* If we haven't already created an attribute node, create it now */
            u32Ret = xcalloc((void **)&retval, sizeof(xml_attribute_t));
        }
        else
        {
            /* We already created an attribute node, so simply create a new one,
               and attach it on the beginning of the old one. */
            u32Ret = xcalloc((void **)&current, sizeof(xml_attribute_t));
            if (u32Ret == OLERR_NO_ERROR)
            {
                current->xa_pxaNext = retval;
                retval = current;
            }
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            /* Parse each token by the ':'. If this results is more than one
               token, we can figure that the first token is the namespace
               prefix */
            u32Ret = parseStringAdv(
                &pAttr, field->prf_pstrData, 0, field->prf_sData, ":", 1);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            if (pAttr->pr_u32NumOfResult == 1)
            {
                /* This attribute has no prefix, so just parse on the '='. The
                   first token is the attribute name, the other is the value */
                retval->xa_pstrPrefix = NULL;
                retval->xa_sPrefix = 0;
                u32Ret = parseStringAdv(
                    &pValue, field->prf_pstrData, 0, field->prf_sData, "=", 1);
            }
            else
            {
                /* Since there is a namespace prefix, seperate that out, and
                   parse the remainder on the '=' to figure out what the
                   attribute name and value are */
                retval->xa_pstrPrefix = pAttr->pr_pprfFirst->prf_pstrData;
                retval->xa_sPrefix = pAttr->pr_pprfFirst->prf_sData;
                u32Ret = parseStringAdv(
                    &pValue, field->prf_pstrData, retval->xa_sPrefix + 1,
                    field->prf_sData - retval->xa_sPrefix - 1, "=", 1);
            }
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            retval->xa_pstrName = pValue->pr_pprfFirst->prf_pstrData;
            retval->xa_sName = pValue->pr_pprfFirst->prf_sData;
            retval->xa_pstrValue = pValue->pr_pprfLast->prf_pstrData;
            retval->xa_sValue = pValue->pr_pprfLast->prf_sData;
        }

        if (pAttr != NULL)
            destroyParseResult(&pAttr);

        if (pValue != NULL)
            destroyParseResult(&pValue);

        field = field->prf_pprfNext;
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppAttribute = retval;
    else if (retval != NULL)
        destroyXMLAttributeList(&retval);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 destroyXMLAttributeList(xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_attribute_t * temp;
    xml_attribute_t * attribute = *ppAttribute;

    while (attribute != NULL)
    {
        temp = attribute->xa_pxaNext;
        xfree((void **)&attribute);
        attribute = temp;
    }

    return u32Ret;
}

/** Resolves a namespace prefix from the scope of the given node
 *
 *  @param node [in] the node used to start the resolve
 *  @param prefix [in] the namespace prefix to resolve
 *  @param sPrefix [in] the lenght of the prefix
 *  @param ppstr [out] the resolved namespace, NULL if unable to resolve
 *
 *  @return the error code
 */
u32 lookupXMLNamespace(
    xml_node_t * node, olchar_t * prefix, olsize_t sPrefix, olchar_t ** ppstr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t * temp = node;

    /*If the specified prefix is zero length, we interpret that to mean
      they want to lookup the default namespace*/
    if (sPrefix == 0)
    {
        /*This is the default namespace prefix*/
        prefix = "xmlns";
        sPrefix = 5;
    }

    /*From the current node, keep traversing up the parents, until we find
      a match. Each step we go up, is a step wider in scope.*/
    do
    {
        if (hasHashtreeEntry(&temp->xn_hNameSpace, prefix, sPrefix))
        {
            /*As soon as we find the namespace declaration, stop
              iterating the tree, as it would be a waste of time*/
            getHashtreeEntry(
                &temp->xn_hNameSpace, prefix, sPrefix, (void **)ppstr);
            break;
        }

        temp = temp->xn_pxnParent;
    } while ((temp != NULL) && (u32Ret == OLERR_NO_ERROR));

    return u32Ret;
}

/** Builds the lookup table used by lookupXMLNamespace
 *
 *  @param node [in] This node will be the highest scoped
 *
 *  @return the error code
 */
u32 buildXMLNamespaceLookupTable(xml_node_t * node)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_attribute_t *attr, *currentAttr;
    xml_node_t *current = node;

    /*Iterate through all the start elements, and build a table of the
      declared namespaces*/
    while (current != NULL)
    {
        if (! current->xn_bStartTag)
        {
            current = current->xn_pxnNext;
            continue;
        }

        /*xn_hNameSpace is the hash table containing the fully qualified
          namespace keyed by the namespace prefix*/
        initHashtree(&current->xn_hNameSpace);

        u32Ret = getXMLAttributes(current, &attr);
        if (u32Ret == OLERR_NO_ERROR)
        {
            currentAttr = attr;
            /*Iterate through all the attributes to find namespace
              declarations*/
            while (currentAttr != NULL)
            {
                if (currentAttr->xa_sName == 5 &&
                    ol_memcmp(currentAttr->xa_pstrName, "xmlns", 5) == 0)
                {
                    /*default namespace declaration*/
                    currentAttr->xa_pstrValue[currentAttr->xa_sValue] = 0;
                    addHashtreeEntry(
                        &current->xn_hNameSpace, "xmlns", 5,
                        currentAttr->xa_pstrValue);
                }
                else if (currentAttr->xa_sPrefix == 5 &&
                         ol_memcmp(currentAttr->xa_pstrPrefix, "xmlns", 5) == 0)
                {
                    /*Other Namespace Declaration*/
                    currentAttr->xa_pstrValue[currentAttr->xa_sValue] = 0;
                    addHashtreeEntry(
                        &current->xn_hNameSpace, currentAttr->xa_pstrName,
                        currentAttr->xa_sName, currentAttr->xa_pstrValue);
                }
                currentAttr = currentAttr->xa_pxaNext;
            }
            destroyXMLAttributeList(&attr);
        }

        current = current->xn_pxnNext;
    }

    return u32Ret;
}

u32 readInnerXML(xml_node_t * node, olchar_t ** ppstrData, olsize_t * psData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_node_t *x = node;
    olsize_t sBuf = 0;
    basic_stack_t *tagStack;

    /*Starting with the current start element, we use this stack to find
      the matching end element, so we can figure out what we need to return*/
    initStack(&tagStack);
    do
    {
        if (x->xn_bStartTag)
        {
            pushStack(&tagStack, x);
        }
        x = x->xn_pxnNext;

        if (x->xn_sName != node->xn_sName)
            continue;
        
        if (ol_memcmp(x->xn_pstrName, node->xn_pstrName, node->xn_sName) != 0)
            continue;
    } while (! ((! x->xn_bStartTag) && (popStack(&tagStack) == node)));

    clearStack(&tagStack);

    /*The xn_pReserved fields of the start element and end element are used as
      pointers representing the data segment of the XML*/
    sBuf = (olsize_t)
        ((olchar_t *)x->xn_pReserved - (olchar_t *)node->xn_pReserved - 1);
    if (sBuf < 0)
    {
        sBuf = 0;
    }
    *ppstrData = (olchar_t *) node->xn_pReserved;
    *psData = sBuf;

    return u32Ret;
}

u32 getXMLAttributes(xml_node_t * node, xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t *c;
    olint_t nEndReserved = (! node->xn_bEmptyTag) ? 1 : 2;
    parse_result_t * xml;
    parse_result_field_t * field;

    /* The reserved field is used to show where the data segments start and
       stop. We can also use them to figure out where the attributes start
       and stop */
    c = (olchar_t *) node->xn_pReserved - 1;
    while (*c != '<')
    {
        /* The xn_pReserved field of the start element points to the first
           character after the '>' of the start element. Just work our way
           backwards to find the start of the start element */
        c = c - 1;
    }
    c = c + 1;

    /* Now that we isolated the string between the '<' and the '>', we can parse
       the string as delimited by ' '. Use parseStringAdv because these
       attributes can be within quotation marks */
    u32Ret = parseStringAdv(
        &xml, c, 0, ((olchar_t *) node->xn_pReserved - c - nEndReserved),
        " ", 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        field = xml->pr_pprfFirst;

        /* We skip the first token, because the first token, is the Element name */
        if (field != NULL)
        {
            field = field->prf_pprfNext;
        }

        u32Ret = _getXmlAttribute(field, ppAttribute);
    }

    return u32Ret;
}

u32 parseXML(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, xml_doc_t * pxd)
{
    u32 u32Ret = OLERR_NO_ERROR;

    ol_bzero(pxd, sizeof(xml_doc_t));

    u32Ret = _parseXML(pstrBuf, sOffset, sBuf, &(pxd->xd_pxnRoot));
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _processXMLNodeList(pxd);
    }

    return u32Ret;
}

u32 destroyXMLNodeList(xml_node_t ** ppNode)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _destroyXMLNodeList(ppNode);

    return u32Ret;
}

u32 parseXMLFile(const olchar_t * pstrFilename, xml_file_t ** ppFile)
{
    u32 u32Ret = OLERR_NO_ERROR;
    FILE * fp = NULL;
    olsize_t u32Size;
    file_stat_t filestat;
    xml_file_t * pxf;

    assert((pstrFilename != NULL) && (ppFile != NULL));

    u32Ret = xmalloc((void **)&pxf, sizeof(xml_file_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(pxf, 0, sizeof(xml_file_t));
        u32Ret = getFileStat(pstrFilename, &filestat);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Size = (olsize_t)filestat.fs_u64Size;
        u32Ret = xmalloc((void **)&(pxf->xf_pstrBuf), u32Size);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpOpenFile(pstrFilename, "r", &fp);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpReadn(fp, pxf->xf_pstrBuf, &u32Size);
        if (u32Size != (u32)filestat.fs_u64Size)
            u32Ret = OLERR_CORRUPTED_XML_FILE;
    }

    if (fp != NULL)
        fpCloseFile(&fp);

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = parseXML(pxf->xf_pstrBuf, 0, u32Size, &(pxf->xf_xdDoc));
    }

    if (pxf != NULL)
        *ppFile = pxf;

    return u32Ret;
}

u32 destroyXMLFile(xml_file_t ** ppFile)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xml_file_t * pxf;

    assert((ppFile != NULL) && (*ppFile != NULL));

    pxf = *ppFile;

    if (pxf->xf_xdDoc.xd_pxnRoot != NULL)
        _destroyXMLNodeList(&(pxf->xf_xdDoc.xd_pxnRoot));

    if (pxf->xf_pstrBuf != NULL)
        xfree((void **)&(pxf->xf_pstrBuf));

    return u32Ret;
}

void printXMLNodeList(xml_node_t * pxn, u8 u8Indent)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t tagname[50];
    olchar_t value[50];
    olchar_t * pstrValue;
    olsize_t sBuf;
    xml_node_t * temp;
    u8 u8Index;

    temp = pxn;
    while (temp != NULL)
    {
        memcpy(tagname, temp->xn_pstrName, temp->xn_sName);
        tagname[temp->xn_sName] = '\0';

        for (u8Index = 0; u8Index < u8Indent; u8Index ++)
            ol_printf(" ");

        ol_printf("%s", tagname);

        if (temp->xn_pxnChildren != NULL)
        {
            ol_printf("\n");
            printXMLNodeList(temp->xn_pxnChildren, u8Indent + 2);
        }
        else
        {
            u32Ret = readInnerXML(temp, &pstrValue, &sBuf);
            if (u32Ret == OLERR_NO_ERROR)
            {
                memcpy(value, pstrValue, sBuf);
                value[sBuf] = '\0';
                ol_printf("  %s\n", value);
            }
        }

        temp = temp->xn_pxnSibling;
    }
}

/*---------------------------------------------------------------------------*/

