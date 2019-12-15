/**
 *  @file xmlattr.c
 *
 *  @brief Implementation file for common routines for XML attribute.
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
#include "jf_jiukun.h"

#include "xmlcommon.h"
#include "xmlattr.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define XML attribute data type.
 */
typedef struct internal_xmlparser_xml_attribute
{
    /**The string of attribute name.*/
    olchar_t * ixxa_pstrName;
    /**Size of name string.*/
    olsize_t ixxa_sName;

    /**The prefix string of name space.*/
    olchar_t * ixxa_pstrPrefix;
    /**Size of name space string.*/
    olsize_t ixxa_sPrefix;

    /**The string of attribute value.*/
    olchar_t * ixxa_pstrValue;
    /**Size of value string.*/
    olsize_t ixxa_sValue;

} internal_xmlparser_xml_attribute_t;


/* --- private routine section ------------------------------------------------------------------ */

static u32 _destroyXmlAttribute(internal_xmlparser_xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_attribute_t * pixxa = *ppAttribute;

    if (pixxa->ixxa_pstrName != NULL)
        jf_jiukun_freeMemory((void **)&pixxa->ixxa_pstrName);

    if (pixxa->ixxa_pstrPrefix != NULL)
        jf_jiukun_freeMemory((void **)&pixxa->ixxa_pstrPrefix);

    if (pixxa->ixxa_pstrValue != NULL)
        jf_jiukun_freeMemory((void **)&pixxa->ixxa_pstrValue);

    jf_jiukun_freeMemory((void **)ppAttribute);

    return u32Ret;
}

static u32 _createXmlAttribute(internal_xmlparser_xml_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_attribute_t * retval = NULL;

    *ppAttribute = NULL;
    
    u32Ret = jf_jiukun_allocMemory((void **)&retval, sizeof(*retval));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(retval, sizeof(*retval));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAttribute = retval;
    else if (retval != NULL)
        _destroyXmlAttribute(&retval);

    return u32Ret;
}

static u32 _fnFreeXmlAttribute(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _destroyXmlAttribute((internal_xmlparser_xml_attribute_t **) ppData);

    return u32Ret;
}

static u32 _parseNameAndValueOfOneXmlAttribute(
    jf_string_parse_result_field_t * field, jf_string_parse_result_t * pAttr,
    internal_xmlparser_xml_attribute_t * retval)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pValue = NULL;

    if (pAttr->jspr_u32NumOfResult == 1)
    {
        /* This attribute has no prefix, so just parse on the '='. The first token is the
           attribute name, the other is the value */
        retval->ixxa_pstrPrefix = NULL;
        retval->ixxa_sPrefix = 0;
        u32Ret = jf_string_parseAdv(
            &pValue, field->jsprf_pstrData, 0, field->jsprf_sData, "=", 1);
    }
    else
    {
        /* Since there is a namespace prefix, seperate that out, and parse the remainder on
           the '=' to figure out what the attribute name and value are */
        u32Ret = jf_string_parseAdv(
            &pValue, field->jsprf_pstrData, retval->ixxa_sPrefix + 1,
            field->jsprf_sData - retval->ixxa_sPrefix - 1, "=", 1);
    }


    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->ixxa_sName = pValue->jspr_pjsprfFirst->jsprf_sData;

        u32Ret = jf_string_duplicateWithLen(
            &retval->ixxa_pstrName, pValue->jspr_pjsprfFirst->jsprf_pstrData,
            pValue->jspr_pjsprfFirst->jsprf_sData);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->ixxa_sValue = pValue->jspr_pjsprfLast->jsprf_sData;

        u32Ret = jf_string_duplicateWithLen(
            &retval->ixxa_pstrValue, pValue->jspr_pjsprfLast->jsprf_pstrData,
            pValue->jspr_pjsprfLast->jsprf_sData);
    }

    if (pValue != NULL)
        jf_string_destroyParseResult(&pValue);

    return u32Ret;
}

static u32 _parseOneXmlAttribute(jf_string_parse_result_field_t * field, jf_linklist_t * pLinklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_xmlparser_xml_attribute_t * retval = NULL;
    jf_string_parse_result_t * pAttr = NULL;

    u32Ret = _createXmlAttribute(&retval);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Parse each token by the ':'. If this results is more than one token, we can figure
          that the first token is the namespace prefix.*/
        u32Ret = jf_string_parseAdv(
            &pAttr, field->jsprf_pstrData, 0, field->jsprf_sData, ":", 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pAttr->jspr_u32NumOfResult > 1)
        {
            /*There is a namespace prefix, duplicate the string.*/
            retval->ixxa_sPrefix = pAttr->jspr_pjsprfFirst->jsprf_sData;
            u32Ret = jf_string_duplicateWithLen(
                &retval->ixxa_pstrPrefix, pAttr->jspr_pjsprfFirst->jsprf_pstrData,
                pAttr->jspr_pjsprfFirst->jsprf_sData);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseNameAndValueOfOneXmlAttribute(field, pAttr, retval);    
    }

    if (pAttr != NULL)
        jf_string_destroyParseResult(&pAttr);

    if (u32Ret == JF_ERR_NO_ERROR)
        /*Append to the end of the list.*/
        u32Ret = jf_linklist_appendTo(pLinklist, (void *)retval);

    if ((u32Ret != JF_ERR_NO_ERROR) && (retval != NULL))
        _destroyXmlAttribute(&retval);

    return u32Ret;
}

static u32 _parseXmlAttribute(jf_string_parse_result_field_t * field, jf_linklist_t * pLinklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Iterate through all the other tokens, as these are all attributes.*/
    while ((field != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if ((field->jsprf_sData == 0) ||
            isAllSpaceInXmlBuffer(field->jsprf_pstrData, field->jsprf_sData))
        {
            field = field->jsprf_pjsprfNext;
            continue;
        }

        u32Ret = _parseOneXmlAttribute(field, pLinklist);

        field = field->jsprf_pjsprfNext;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

/** Parse XML attributes from an XML node.
 *
 *  @note
 *  -# The first field in the parse result is the attribute string.
 *
 *  @param pElem [in] The element contain XML attribute.
 *  @param pLinklist [out] The linked list of attributes.
 *
 *  @return The error code.
 */
u32 parseXmlAttributeList(jf_string_parse_result_t * pElem, jf_linklist_t * pLinklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pjspr = NULL;
    jf_string_parse_result_field_t * field = NULL;

    /*Parse the attribute string as delimited by ' '. Use jf_string_parseAdv() because these
      attributes can be within quotation marks.*/
    u32Ret = jf_string_parseAdv(
        &pjspr, pElem->jspr_pjsprfFirst->jsprf_pstrData, 0, pElem->jspr_pjsprfFirst->jsprf_sData,
        " ", 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The first token is the element name, it cannot be NULL.*/
        field = pjspr->jspr_pjsprfFirst;
        if (field == NULL)
            u32Ret = JF_ERR_CORRUPTED_XML_DOCUMENT;
    }

    /*Parse XML attribute list for start element.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = field->jsprf_pjsprfNext;

        u32Ret = _parseXmlAttribute(field, pLinklist);
    }

    if (pjspr != NULL)
        jf_string_destroyParseResult(&pjspr);

    return u32Ret;
}

/** Frees resources from an attribute list.
 *
 *  @param pLinklist [in/out] The linked list for XML attributes.
 *
 *  @return The error code.
 */
u32 destroyXmlAttributeList(jf_linklist_t * pLinklist)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_linklist_finiListAndData(pLinklist, _fnFreeXmlAttribute);

    return u32Ret;
}

/** Print XML attributes list.
 *
 *  @param pLinklist [in] The linked list for XML attribute.
 *
 *  @return Void.
 */
void printXmlAttributeList(jf_linklist_t * pLinklist)
{
    olchar_t value[64];
    jf_linklist_node_t * pNode = NULL;
    internal_xmlparser_xml_attribute_t * pAttr = NULL;
    ol_printf("(");

    pNode = jf_linklist_getFirstNode(pLinklist);
    while (pNode != NULL)
    {
        pAttr = jf_linklist_getDataFromNode(pNode);

        if (pAttr->ixxa_sPrefix > 0)
        {
            ol_memcpy(value, pAttr->ixxa_pstrPrefix, pAttr->ixxa_sPrefix);
            value[pAttr->ixxa_sPrefix] = '\0';
            ol_printf("%s:", value);
        }

        ol_memcpy(value, pAttr->ixxa_pstrName, pAttr->ixxa_sName);
        value[pAttr->ixxa_sName] = '\0';
        ol_printf("%s=", value);

        ol_memcpy(value, pAttr->ixxa_pstrValue, pAttr->ixxa_sValue);
        value[pAttr->ixxa_sValue] = '\0';
        ol_printf("%s ", value);

        pNode = jf_linklist_getNextNode(pNode);
    }
    ol_printf(")");
}

/*------------------------------------------------------------------------------------------------*/

