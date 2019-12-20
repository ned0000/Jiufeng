/**
 *  @file jf_ptree.c
 *
 *  @brief The implementation file for property tree.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <unistd.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_jiukun.h"
#include "jf_err.h"
#include "jf_ptree.h"
#include "jf_linklist.h"
#include "jf_hashtree.h"
#include "jf_string.h"
#include "jf_linklist.h"
#include "jf_stack.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define property tree node attribute data type.
 */
typedef struct internal_ptree_node_attribute
{
    /**The prefix string of name space.*/
    olchar_t * ipna_pstrPrefix;
    /**Size of name space string.*/
    olsize_t ipna_sPrefix;

    /**The string of attribute name.*/
    olchar_t * ipna_pstrName;
    /**Size of name string.*/
    olsize_t ipna_sName;

    /**The string of attribute value.*/
    olchar_t * ipna_pstrValue;
    /**Size of value string.*/
    olsize_t ipna_sValue;
} internal_ptree_node_attribute_t;

/** Define property tree node data type.
 */
typedef struct internal_ptree_node
{
    /**The name space string.*/
    olchar_t * ipn_pstrNs;
    /**Size of name space string.*/
    olsize_t ipn_sNs;

    /**The name string.*/
    olchar_t * ipn_pstrName;
    /**Size of name string.*/
    olsize_t ipn_sName;

    /**The value string.*/
    olchar_t * ipn_pstrValue;
    /**Size of value string.*/
    olsize_t ipn_sValue;

    /**Hash tree for name space.*/
    jf_hashtree_t ipn_jhNameSpace;
    /**Linked list for attribute.*/
    jf_linklist_t ipn_jlAttribute;

    /**The parent ptree node.*/
    struct internal_ptree_node * ipn_pipnParent;
    /**The children ptree node.*/
    struct internal_ptree_node * ipn_pipnChildren;
    /**The sibling ptree node.*/
    struct internal_ptree_node * ipn_pipnSibling;

} internal_ptree_node_t;

/** Define the internal property tree data type.
 */
typedef struct
{
    internal_ptree_node_t * ip_pipnRoot;

    olchar_t * ip_pstrSeparator;
    olsize_t ip_sSeparator;
} internal_ptree_t;

/** Parameter for getting property node.
 */
typedef struct
{
    jf_string_parse_result_t * gpnp_pjsprKey;
    jf_ptree_node_t ** gpnp_ppNode;
    u16 gpnp_u16MaxNode;
    u16 gpnp_u16NumOfNode;
} get_ptree_node_param_t;

/* --- private routine section ------------------------------------------------------------------ */

static void _printPtreeNodeAttributeList(jf_linklist_t * pLinklist)
{
    olchar_t value[64];
    jf_linklist_node_t * pNode = NULL;
    internal_ptree_node_attribute_t * pAttr = NULL;
    ol_printf("(");

    pNode = jf_linklist_getFirstNode(pLinklist);
    while (pNode != NULL)
    {
        pAttr = jf_linklist_getDataFromNode(pNode);

        if (pAttr->ipna_sPrefix > 0)
        {
            ol_memcpy(value, pAttr->ipna_pstrPrefix, pAttr->ipna_sPrefix);
            value[pAttr->ipna_sPrefix] = '\0';
            ol_printf("%s:", value);
        }

        ol_memcpy(value, pAttr->ipna_pstrName, pAttr->ipna_sName);
        value[pAttr->ipna_sName] = '\0';
        ol_printf("%s=", value);

        ol_memcpy(value, pAttr->ipna_pstrValue, pAttr->ipna_sValue);
        value[pAttr->ipna_sValue] = '\0';
        ol_printf("%s ", value);

        pNode = jf_linklist_getNextNode(pNode);
    }
    ol_printf(")");
}

static u32 _destroyPtreeNodeAttribute(internal_ptree_node_attribute_t ** ppAttribute)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_attribute_t * pipna = *ppAttribute;

    if (pipna->ipna_pstrPrefix != NULL)
        jf_jiukun_freeMemory((void **)&pipna->ipna_pstrPrefix);

    if (pipna->ipna_pstrName != NULL)
        jf_jiukun_freeMemory((void **)&pipna->ipna_pstrName);

    if (pipna->ipna_pstrValue != NULL)
        jf_jiukun_freeMemory((void **)&pipna->ipna_pstrValue);

    jf_jiukun_freeMemory((void **)ppAttribute);

    return u32Ret;
}

static u32 _createPtreeNodeAttribute(
    internal_ptree_node_attribute_t ** ppAttribute, const olchar_t * pstrPrefix,
    const olsize_t sPrefix, const olchar_t * pstrName, const olsize_t sName,
    const olchar_t * pstrValue, const olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_attribute_t * retval = NULL;

    *ppAttribute = NULL;
    
    u32Ret = jf_jiukun_allocMemory((void **)&retval, sizeof(*retval));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(retval, sizeof(*retval));
        retval->ipna_sPrefix = sPrefix;
        retval->ipna_sName = sName;
        retval->ipna_sValue = sValue;

        if (sPrefix > 0)
            u32Ret = jf_string_duplicateWithLen(&retval->ipna_pstrPrefix, pstrPrefix, sPrefix);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sName > 0))
        u32Ret = jf_string_duplicateWithLen(&retval->ipna_pstrName, pstrName, sName);

    if ((u32Ret == JF_ERR_NO_ERROR) && (sValue > 0))
        u32Ret = jf_string_duplicateWithLen(&retval->ipna_pstrValue, pstrValue, sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAttribute = retval;
    else if (retval != NULL)
        _destroyPtreeNodeAttribute(&retval);

    return u32Ret;
}

static u32 _fnFreePtreeNodeAttribute(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _destroyPtreeNodeAttribute((internal_ptree_node_attribute_t **) ppData);

    return u32Ret;
}

static void _printPtreeNodeIndentSpace(u16 u16Indent)
{
    u16 u16Index = 0;

    for (u16Index = 0; u16Index < u16Indent; u16Index ++)
        ol_printf(" ");
}

static void _printPtreeNodeList(internal_ptree_node_t * pixxn, u16 u16Indent)
{
    olchar_t value[128];
    internal_ptree_node_t * temp = NULL;

    temp = pixxn;
    while (temp != NULL)
    {
        _printPtreeNodeIndentSpace(u16Indent);

        ol_printf("%s", temp->ipn_pstrName);
        _printPtreeNodeAttributeList(&temp->ipn_jlAttribute);
        ol_printf(": ");

        if (temp->ipn_pipnChildren != NULL)
        {
            ol_printf("\n");
            _printPtreeNodeList(temp->ipn_pipnChildren, u16Indent + 4);
        }
        else if (temp->ipn_pstrValue != NULL)
        {
            ol_memcpy(value, temp->ipn_pstrValue, temp->ipn_sValue);
            value[temp->ipn_sValue] = '\0';
            ol_printf("%s\n", value);
        }
        else
        {
            ol_printf("\n");
        }

        temp = temp->ipn_pipnSibling;
    }
}

static u32 _destroyPtreeNode(internal_ptree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = *ppNode;
    
    /*If there was a namespace table, delete it*/
    jf_hashtree_fini(&pipn->ipn_jhNameSpace);
    jf_linklist_finiListAndData(&pipn->ipn_jlAttribute, _fnFreePtreeNodeAttribute);

    if (pipn->ipn_pstrNs != NULL)
        jf_string_free(&pipn->ipn_pstrNs);

    if (pipn->ipn_pstrName != NULL)
        jf_string_free(&pipn->ipn_pstrName);

    if (pipn->ipn_pstrValue != NULL)
        jf_string_free(&pipn->ipn_pstrValue);

    jf_jiukun_freeMemory((void **)ppNode);
    
    return u32Ret;
}

static u32 _createPtreeNode(
    internal_ptree_node_t ** ppNode, const olchar_t * pstrNs, const olsize_t sNs,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = NULL;
    
    *ppNode = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pipn, sizeof(*pipn));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pipn, sizeof(*pipn));

        /*Init the namespace hash table.*/
        jf_hashtree_init(&pipn->ipn_jhNameSpace);
        jf_linklist_init(&pipn->ipn_jlAttribute);
        pipn->ipn_sNs = sNs;
        pipn->ipn_sName = sName;
        pipn->ipn_sValue = sValue;

        if (sNs > 0)
            u32Ret = jf_string_duplicateWithLen(&pipn->ipn_pstrNs, pstrNs, sNs);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sName > 0))
        u32Ret = jf_string_duplicateWithLen(&pipn->ipn_pstrName, pstrName, sName);

    if ((u32Ret == JF_ERR_NO_ERROR) && (sValue > 0))
        u32Ret = jf_string_duplicateWithLen(&pipn->ipn_pstrValue, pstrValue, sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppNode = pipn;
    else if (pipn != NULL)
        _destroyPtreeNode(&pipn);

    return u32Ret;
}

/** Destroy the property tree node including all children nodes.
 *
 *  @param ppNode [in/out] The node to clean up.
 */
static u32 _destroyPtreeNodeList(internal_ptree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * node = *ppNode, * temp = NULL;

    *ppNode = NULL;

    while (node != NULL)
    {
        if (node->ipn_pipnChildren != NULL)
        {
            _destroyPtreeNodeList(&node->ipn_pipnChildren);
        }

        temp = node->ipn_pipnSibling;
        _destroyPtreeNode(&node);
        node = temp;
    }

    return u32Ret;
}

static u32 _changePtreeNode(
    internal_ptree_node_t * pipn, const olchar_t * pstrNs, const olchar_t * pstrName,
    const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrNs != NULL)
    {
        if (pipn->ipn_pstrNs != NULL)
            jf_string_free(&pipn->ipn_pstrNs);

        pipn->ipn_sNs = ol_strlen(pstrNs);
        u32Ret = jf_string_duplicate(&pipn->ipn_pstrNs, pstrNs);
    }

    if ((u32Ret = JF_ERR_NO_ERROR) && (pstrName != NULL))
    {
        if (pipn->ipn_pstrName != NULL)
            jf_string_free(&pipn->ipn_pstrName);

        pipn->ipn_sName = ol_strlen(pstrName);
        u32Ret = jf_string_duplicate(&pipn->ipn_pstrName, pstrName);
    }

    if ((u32Ret = JF_ERR_NO_ERROR) && (pstrValue != NULL))
    {
        if (pipn->ipn_pstrValue != NULL)
            jf_string_free(&pipn->ipn_pstrValue);

        pipn->ipn_sValue = ol_strlen(pstrValue);
        u32Ret = jf_string_duplicate(&pipn->ipn_pstrValue, pstrValue);
    }

    return u32Ret;
}

static boolean_t _traversePtree(
    internal_ptree_node_t * pipn, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    boolean_t bRet = TRUE;

    while (pipn != NULL)
    {
        if (pipn->ipn_pipnChildren != NULL)
        {
            bRet = _traversePtree(pipn->ipn_pipnChildren, fnOpNode, pArg);
            if (! bRet)
                break;
        }

        bRet = fnOpNode(pipn, pArg);
        if (! bRet)
            break;

        pipn = pipn->ipn_pipnSibling;
    }

    return bRet;
}

static u32 _addPtreeChildNode(internal_ptree_node_t * pNode, internal_ptree_node_t * pipn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * temp = NULL;

    pipn->ipn_pipnParent = pNode;
    if (pNode->ipn_pipnChildren == NULL)
    {
        /*If child node is NULL, set the child node to the new node*/
        pNode->ipn_pipnChildren = pipn;
    }
    else
    {
        /*If child node is not NULL, add the new node to the end of sibling node*/
        if (pNode->ipn_pipnChildren->ipn_pipnSibling == NULL)
        {
            pNode->ipn_pipnChildren->ipn_pipnSibling = pipn;
        }
        else
        {
            temp = pNode->ipn_pipnChildren->ipn_pipnSibling;
            while (temp->ipn_pipnSibling != NULL)
                temp = temp->ipn_pipnSibling;

            temp->ipn_pipnSibling = pipn;
        }
    }

    return u32Ret;
}

static u32 _deletePtreeNode(internal_ptree_node_t * pipn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pParent = NULL, * pPrev = NULL;

    pParent = pipn->ipn_pipnParent;

    if (pipn == pParent->ipn_pipnChildren)
    {
        /*The node to be deleted is the first child node.*/
        pParent->ipn_pipnChildren = pipn->ipn_pipnSibling;
    }
    else
    {
        pPrev = pParent->ipn_pipnChildren;
        while (pPrev->ipn_pipnSibling != pipn)
            pPrev = pPrev->ipn_pipnSibling;

        pPrev->ipn_pipnSibling = pipn->ipn_pipnSibling;
    }

    /*Clear the pointer to parent and sibling for the node to be deleted.*/
    pipn->ipn_pipnParent = NULL;
    pipn->ipn_pipnSibling = NULL;

    return u32Ret;
}

static u32 _getPtreeNodeAttribute(
    internal_ptree_node_t * pipn, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName, olchar_t ** pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;



    return u32Ret;
}

static u32 _getPtreeNodeLevel(internal_ptree_node_t * pipn)
{
    u32 u32Level = 0;

    while (pipn != NULL)
    {
        u32Level ++;

        pipn = pipn->ipn_pipnParent;
    }

    return u32Level;
}

/** Push all the nodes in the path to stack. The path is from leaf node to root node.
 */
static u32 _pushPtreeNodePathToStack(jf_stack_t ** ppjsNode, internal_ptree_node_t * pipn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    while ((pipn != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = jf_stack_push(ppjsNode, pipn);

        pipn = pipn->ipn_pipnParent;
    }

    return u32Ret;
}

/** Compare the key path and the nodes path.
 */
static u32 _comparePtreeNodePath(jf_stack_t ** ppjsNode, jf_string_parse_result_t * pjspr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_field_t * field = pjspr->jspr_pjsprfFirst;
    internal_ptree_node_t * pipn = NULL;

    while (field != NULL)
    {
        pipn = jf_stack_pop(ppjsNode);

        if ((pipn->ipn_sName != field->jsprf_sData) ||
            (ol_memcmp(pipn->ipn_pstrName, field->jsprf_pstrData, pipn->ipn_sName) != 0))
        {
            u32Ret = JF_ERR_NOT_MATCH;
            break;
        }

        field = field->jsprf_pjsprfNext;
    }

    return u32Ret;
}

static boolean_t _matchPtreeNode(jf_ptree_node_t * pNode, void * pArg)
{
    boolean_t bRet = TRUE;
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;
    get_ptree_node_param_t * param = (get_ptree_node_param_t *)pArg;
    jf_string_parse_result_t * pjspr = param->gpnp_pjsprKey;
    u32 u32NodeLevel = 0;
    jf_stack_t * pjsNode;

    u32NodeLevel = _getPtreeNodeLevel(pipn);
    if (u32NodeLevel == pjspr->jspr_u32NumOfResult)
    {
        /*Node level and key level are equal.*/
        jf_stack_init(&pjsNode);

        /*Compare the node path and key path.*/
        u32Ret = _pushPtreeNodePathToStack(&pjsNode, pipn);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _comparePtreeNodePath(&pjsNode, pjspr);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*The node path and key path are equal. The node is found.*/
            param->gpnp_ppNode[param->gpnp_u16NumOfNode] = pNode;
            param->gpnp_u16NumOfNode ++;
            if (param->gpnp_u16NumOfNode == param->gpnp_u16MaxNode)
            {
                /*Maximum node is reached, terminate the traverse.*/
                bRet = FALSE;
            }
        }

        jf_stack_clear(&pjsNode);
    }

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_ptree_destroy(jf_ptree_t ** ppPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = NULL;

    assert(ppPtree != NULL);

    pip = (internal_ptree_t *) *ppPtree;

    _destroyPtreeNodeList(&pip->ip_pipnRoot);

    jf_jiukun_freeMemory(ppPtree);

    return u32Ret;
}

u32 jf_ptree_create(
    jf_ptree_t ** ppPtree, const olchar_t * pstrNs, const olsize_t sNs, const olchar_t * pstrName,
    const olsize_t sName, const olchar_t * pstrValue, const olsize_t sValue,
    jf_ptree_node_t ** ppRootNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = NULL;

    assert(ppPtree != NULL);
    *ppRootNode = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pip, sizeof(*pip));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pip, sizeof(*pip));
        pip->ip_pstrSeparator = ".";
        pip->ip_sSeparator = 1;

        u32Ret = _createPtreeNode(
            &pip->ip_pipnRoot, pstrNs, sNs, pstrName, sName, pstrValue, sValue);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppRootNode = pip->ip_pipnRoot;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPtree = pip;
    else if (pip != NULL)
        jf_ptree_destroy((jf_ptree_t **)&pip);

    return u32Ret;    
}

u32 jf_ptree_finNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, jf_ptree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Num = 1;

    u32Ret = jf_ptree_findAllNode(pPtree, pstrKey, ppNode, &u16Num);

    return u32Ret;
}

u32 jf_ptree_findAllNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, jf_ptree_node_t ** ppNode, u16 * pu16NumOfNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;
    internal_ptree_node_t * pipn = pip->ip_pipnRoot;
    jf_string_parse_result_t * pjspr = NULL;
    jf_string_parse_result_field_t * field = NULL;
    get_ptree_node_param_t param;

    assert((ppNode != NULL) && (*pu16NumOfNode > 0));
    
    /*Return the root node is key is NULL.*/
    if (pstrKey == NULL)
    {
        ppNode[0] = pipn;
        *pu16NumOfNode = 1;
        return u32Ret;
    }

    /*Parse the key.*/
    u32Ret = jf_string_parse(
        &pjspr, pstrKey, 0, ol_strlen(pstrKey), pip->ip_pstrSeparator, pip->ip_sSeparator);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*If no separator is found, check the root node.*/
        if (pjspr->jspr_u32NumOfResult == 1)
        {
            field = pjspr->jspr_pjsprfFirst;
            if ((field->jsprf_sData == pipn->ipn_sName) &&
                (ol_memcmp(field->jsprf_pstrData, pipn->ipn_pstrName, field->jsprf_sData) == 0))
            {
                ppNode[0] = pipn;
                *pu16NumOfNode = 1;
            }
            else
            {
                u32Ret = JF_ERR_NOT_FOUND;
                *pu16NumOfNode = 0;
            }
        }
        else
        {
            ol_bzero(&param, sizeof(param));
            param.gpnp_pjsprKey = pjspr;
            param.gpnp_ppNode = ppNode;
            param.gpnp_u16MaxNode = *pu16NumOfNode;
            param.gpnp_u16NumOfNode = 0;

            _traversePtree(pip->ip_pipnRoot, _matchPtreeNode, &param);

            *pu16NumOfNode = param.gpnp_u16NumOfNode;
            if (*pu16NumOfNode == 0)
                u32Ret = JF_ERR_NOT_FOUND;
        }
    }

    if (pjspr != NULL)
        jf_string_destroyParseResult(&pjspr);

    return u32Ret;
}

u32 jf_ptree_iterateNode(jf_ptree_node_t * pNode, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    return u32Ret;
}

u32 jf_ptree_addChildNode(
    jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olsize_t sNs, const olchar_t * pstrName,
    const olsize_t sName, const olchar_t * pstrValue, const olsize_t sValue,
    jf_ptree_node_t ** ppChildNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = NULL;

    u32Ret = _createPtreeNode(&pipn, pstrNs, sNs, pstrName, sName, pstrValue, sValue);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _addPtreeChildNode(pNode, pipn);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppChildNode = pipn;
    else if (pipn != NULL)
        _destroyPtreeNode(&pipn);

    return u32Ret;
}

u32 jf_ptree_changeNode(
    jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olchar_t * pstrName,
    const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    _changePtreeNode(pipn, pstrNs, pstrName, pstrValue);

    return u32Ret;
}

u32 jf_ptree_deleteNode(jf_ptree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)*ppNode;

    /*If parent is NULL, the node is root node, reject the operation.*/
    if (pipn->ipn_pipnParent == NULL)
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _deletePtreeNode(pipn);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppNode = NULL;
        u32Ret = _destroyPtreeNodeList(&pipn);
    }

    return u32Ret;
}

u32 jf_ptree_getNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    olchar_t ** pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    u32Ret = _getPtreeNodeAttribute(
        pipn, pstrPrefix, ol_strlen(pstrPrefix), pstrName, ol_strlen(pstrName), pstrValue);

    return u32Ret;
}

u32 jf_ptree_deleteNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 jf_ptree_addNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;
    internal_ptree_node_attribute_t * pipna = NULL;
    olchar_t * pstr = NULL;

    u32Ret = _getPtreeNodeAttribute(pipn, pstrPrefix, sPrefix, pstrName, sName, &pstr);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createPtreeNodeAttribute(
            &pipna, pstrPrefix, sPrefix, pstrName, sName, pstrValue, sValue);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_linklist_appendTo(&pipn->ipn_jlAttribute, pipna);
    }

    return u32Ret;
}

u32 jf_ptree_changeNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    olchar_t ** pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 jf_ptree_traverse(jf_ptree_t * pTree, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pTree;

    _traversePtree(pip->ip_pipnRoot, fnOpNode, pArg);

    return u32Ret;
}

u32 jf_ptree_dump(jf_ptree_t * pTree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pTree;

    _printPtreeNodeList(pip->ip_pipnRoot, 0);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

