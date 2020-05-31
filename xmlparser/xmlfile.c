/**
 *  @file xmlfile.c
 *
 *  @brief Implementation file for routines related to XML file.
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
#include "jf_xmlparser.h"
#include "jf_jiukun.h"
#include "jf_filestream.h"

#include "xmlcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _saveNewLineToXmlFile(FILE * fp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_filestream_writen(fp, "\n", 1);

    return u32Ret;
}

static u32 _saveIndentSpaceToXmlFile(u16 u16Indent, FILE * fp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    while ((u16Indent > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        --u16Indent;

        u32Ret = jf_filestream_writen(fp, " ", 1);
    }

    return u32Ret;
}

static u32 _fnSaveNodeAttributeToXmlFile(jf_ptree_node_attribute_t * pAttr, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrPrefix = NULL, * pstrName = NULL, * pstrValue = NULL;
    olsize_t sPrefix = 0, sName = 0, sValue = 0;
    FILE * fp = (FILE *)pArg;

    u32Ret = jf_filestream_writen(fp, " ", 1);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributePrefix(pAttr, &pstrPrefix, &sPrefix);

    if ((u32Ret == JF_ERR_NO_ERROR) && (pstrPrefix != NULL))
    {
        u32Ret = jf_filestream_writen(fp, pstrPrefix, sPrefix);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_filestream_writen(fp, ":", 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributeName(pAttr, &pstrName, &sName);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, pstrName, sName);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, "=\"", 2);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributeValue(pAttr, &pstrValue, &sValue);

    if ((u32Ret == JF_ERR_NO_ERROR) && (sValue > 0))
        u32Ret = jf_filestream_writen(fp, pstrValue, sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, "\"", 1);

    return u32Ret;
}

static u32 _saveDeclarationToXmlFile(FILE * fp, jf_ptree_t * pPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_filestream_writen(fp, "<?xml", 5);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_iterateDeclarationAttribute(pPtree, _fnSaveNodeAttributeToXmlFile, fp);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, " ?>", 3);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _saveNewLineToXmlFile(fp);
    
    return u32Ret;
}

static u32 _saveTagOfNodeToXmlFile(
    boolean_t bStartTag, u16 u16Indent, jf_ptree_node_t * pNode, FILE * fp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrNs = NULL, * pstrName = NULL;
    olsize_t sNs = 0, sName = 0;

    /*Do not write space for end tag of leaf node*/
    if (bStartTag || (! jf_ptree_isLeafNode(pNode)))
        u32Ret = _saveIndentSpaceToXmlFile(u16Indent, fp);

    /*Save the starting angle bracket, save "/" for end tag.*/
    if (bStartTag)
        u32Ret = jf_filestream_writen(fp, "<", 1);
    else
        u32Ret = jf_filestream_writen(fp, "</", 2);

    /*Save the name space.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeNs(pNode, &pstrNs, &sNs);

    if ((u32Ret == JF_ERR_NO_ERROR) && (pstrNs != NULL))
    {
        u32Ret = jf_filestream_writen(fp, pstrNs, sNs);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_filestream_writen(fp, ":", 1);
    }

    /*Save the node name.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeName(pNode, &pstrName, &sName);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, pstrName, sName);

    /*Save the node attribute, only for start tag.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && bStartTag)
    {
        u32Ret = jf_ptree_iterateNodeAttribute(pNode, _fnSaveNodeAttributeToXmlFile, fp);
    }

    /*Save the ending angle bracket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_filestream_writen(fp, ">", 1);

    return u32Ret;
}

static u32 _saveValueOfNodeToXmlFile(jf_ptree_node_t * pNode, FILE * fp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrValue = NULL;
    olsize_t sValue;

    /*Leaf node, save the node value*/
    u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);
    if ((u32Ret == JF_ERR_NO_ERROR) && (pstrValue != NULL))
        u32Ret = jf_filestream_writen(fp, pstrValue, sValue);

    return u32Ret;
}

static u32 _traverseXmlPtree(jf_ptree_node_t * pNode, FILE * fp, u16 u16Indent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _saveTagOfNodeToXmlFile(TRUE, u16Indent, pNode, fp);

        if (jf_ptree_isLeafNode(pNode))
        {
            /*Leaf node, save the value of the node.*/
            u32Ret = _saveValueOfNodeToXmlFile(pNode, fp);
        }
        else
        {
            /*Non leaf node, do not save the value of the node but save '\n'.*/
            u32Ret = _saveNewLineToXmlFile(fp);

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _traverseXmlPtree(jf_ptree_getChildNode(pNode), fp, u16Indent + 2);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _saveTagOfNodeToXmlFile(FALSE, u16Indent, pNode, fp);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _saveNewLineToXmlFile(fp);
        
        if (u32Ret == JF_ERR_NO_ERROR)
            pNode = jf_ptree_getSiblingNode(pNode);
    }

    return u32Ret;
}

static u32 _saveNodeToXmlFile(FILE * fp, jf_ptree_t * pPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pRoot = jf_ptree_getRootNode(pPtree);

    u32Ret = _traverseXmlPtree(pRoot, fp, 0);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_xmlparser_parseXmlFile(const olchar_t * pstrFilename, jf_ptree_t ** ppPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olsize_t sSize = 0;
    jf_file_stat_t filestat;
    olchar_t * pstrBuf = NULL;

    assert((pstrFilename != NULL) && (ppPtree != NULL));

    u32Ret = jf_file_getStat(pstrFilename, &filestat);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sSize = (olsize_t)filestat.jfs_u64Size;
        u32Ret = jf_jiukun_allocMemory((void **)&pstrBuf, sSize);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_open(pstrFilename, "r", &fp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_readn(fp, pstrBuf, &sSize);
        if (sSize != (u32)filestat.jfs_u64Size)
            u32Ret = JF_ERR_INVALID_XML_FILE;
    }

    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_xmlparser_parseXmlDoc(pstrBuf, 0, sSize, ppPtree);
    }

    if (pstrBuf != NULL)
        jf_jiukun_freeMemory((void **)&pstrBuf);

    if (u32Ret != JF_ERR_NO_ERROR)
        tryGenXmlErrMsg(u32Ret, pstrFilename, ol_strlen(pstrFilename));

    return u32Ret;
}

u32 jf_xmlparser_saveXmlFile(jf_ptree_t * pPtree, const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;

    assert((pstrFilename != NULL) && (pPtree != NULL));

    u32Ret = jf_filestream_open(pstrFilename, "w", &fp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _saveDeclarationToXmlFile(fp, pPtree);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _saveNodeToXmlFile(fp, pPtree);
    }

    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret != JF_ERR_NO_ERROR)
        tryGenXmlErrMsg(u32Ret, pstrFilename, ol_strlen(pstrFilename));

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

