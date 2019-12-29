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

    /**The private data for application.*/
    void * ipn_pPrivate;
} internal_ptree_node_t;

/** Define the internal property tree data type.
 */
typedef struct
{
    /**The declaration of the tree.*/
    jf_linklist_t ip_jlDeclaration;
    /**The property tree node.*/
    internal_ptree_node_t * ip_pipnRoot;
    /**The separator of the key when finding nodes.*/
    olchar_t * ip_pstrSeparator;
    /**The length of the separator.*/
    olsize_t ip_sSeparator;
} internal_ptree_t;

/** Parameter for finding property node.
 */
typedef struct
{
    jf_string_parse_result_t * fpnp_pjsprKey;
    jf_ptree_node_t ** fpnp_ppNode;
    u16 fpnp_u16MaxNode;
    u16 fpnp_u16NumOfNode;
} find_ptree_node_param_t;

/** Parameter for finding property node attribute.
 */
typedef struct
{
    const olchar_t * fpnap_pstrPrefix;
    olsize_t fpnap_sPrefix;
    const olchar_t * fpnap_pstrName;
    olsize_t fpnap_sName;
    jf_ptree_node_attribute_t ** fpnap_ppjpnaAttr;
} find_ptree_node_attribute_param_t;

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

static u32 _changePtreeNodeValue(internal_ptree_node_t * pipn, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrValue != NULL)
    {
        if (pipn->ipn_pstrValue != NULL)
            jf_string_free(&pipn->ipn_pstrValue);

        pipn->ipn_sValue = ol_strlen(pstrValue);
        u32Ret = jf_string_duplicate(&pipn->ipn_pstrValue, pstrValue);
    }

    return u32Ret;
}

/** Traverse property tree.
 *
 *  @param pipn [in] The node of the property tree.
 *  @param fnOpNode [in] The callback function to operate on the node.
 *  @param pArg [in] The argument for the callback function.
 */
static u32 _traversePtree(
    internal_ptree_node_t * pipn, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    while ((pipn != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = fnOpNode(pipn, pArg);

        if (pipn->ipn_pipnChildren != NULL)
            u32Ret = _traversePtree(pipn->ipn_pipnChildren, fnOpNode, pArg);

        if (u32Ret == JF_ERR_NO_ERROR)
            pipn = pipn->ipn_pipnSibling;
    }

    return u32Ret;
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

static u32 _fnFindNodeAttribute(jf_linklist_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    find_ptree_node_attribute_param_t * pfpnap = (find_ptree_node_attribute_param_t *)pArg;
    internal_ptree_node_attribute_t * pipna = (internal_ptree_node_attribute_t *)pNode->jln_pData;

    if (pipna->ipna_sPrefix != pfpnap->fpnap_sPrefix)
        return u32Ret;

    if (pipna->ipna_sName != pfpnap->fpnap_sName)
        return u32Ret;

    if ((pipna->ipna_sPrefix > 0) &&
        (ol_memcmp(pipna->ipna_pstrPrefix, pfpnap->fpnap_pstrPrefix, pipna->ipna_sPrefix) != 0))
        return u32Ret;

    if (ol_memcmp(pipna->ipna_pstrName, pfpnap->fpnap_pstrName, pipna->ipna_sName) != 0)
        return u32Ret;

    *pfpnap->fpnap_ppjpnaAttr = pipna;

    return JF_ERR_TERMINATED;
}

static u32 _findPtreeNodeAttribute(
    jf_linklist_t * pjl, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, const olsize_t sName, jf_ptree_node_attribute_t ** ppAttr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    find_ptree_node_attribute_param_t fpnap;

    assert((sName > 0) && (pstrName != NULL));

    ol_bzero(&fpnap, sizeof(fpnap));
    fpnap.fpnap_pstrPrefix = pstrPrefix;
    fpnap.fpnap_sPrefix = sPrefix;
    fpnap.fpnap_pstrName = pstrName;
    fpnap.fpnap_sName = sName;
    fpnap.fpnap_ppjpnaAttr = ppAttr;

    u32Ret = jf_linklist_iterate(pjl, _fnFindNodeAttribute, &fpnap);
    if (u32Ret == JF_ERR_TERMINATED)
        u32Ret = JF_ERR_NO_ERROR;
    else
        u32Ret = JF_ERR_PTREE_NODE_ATTR_NOT_FOUND;

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

static u32 _matchPtreeNode(jf_ptree_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;
    find_ptree_node_param_t * param = (find_ptree_node_param_t *)pArg;
    jf_string_parse_result_t * pjspr = param->fpnp_pjsprKey;
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
            param->fpnp_ppNode[param->fpnp_u16NumOfNode] = pNode;
            param->fpnp_u16NumOfNode ++;
            if (param->fpnp_u16NumOfNode == param->fpnp_u16MaxNode)
            {
                /*Maximum node is reached, terminate the traverse.*/
                return JF_ERR_MAX_PTREE_NODE_FOUND;
            }
        }

        jf_stack_clear(&pjsNode);
    }

    return JF_ERR_NO_ERROR;
}

/** Builds the namespace hash tree.
 *
 *  @param pNode [in] This node to build hash tree.
 *  @param pArg [in] The argument.
 *
 *  @return The error code.
 */
static u32 _buildXmlNamespaceTable(jf_ptree_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * current = (internal_ptree_node_t *) pNode;
    internal_ptree_node_attribute_t * attr = NULL;
    jf_linklist_node_t * pjln = NULL;

    /*Iterate through all the attributes to find namespace declarations.*/
    pjln = jf_linklist_getFirstNode(&current->ipn_jlAttribute);
    while (pjln != NULL)
    {
        attr = jf_linklist_getDataFromNode(pjln);

        if ((attr->ipna_sName == 5) && (ol_strcmp(attr->ipna_pstrName, "xmlns") == 0))
        {
            /*Default namespace declaration. Eg. xmlns="http://a.b.c".*/
            u32Ret = jf_hashtree_addEntry(
                &current->ipn_jhNameSpace, attr->ipna_pstrName, attr->ipna_sName,
                attr->ipna_pstrValue);
        }
        else if ((attr->ipna_sPrefix == 5) && (ol_strcmp(attr->ipna_pstrPrefix, "xmlns") == 0))
        {
            /*Other namespace declaration. Eg. xmlns:a="http://a.b.c".*/
            u32Ret = jf_hashtree_addEntry(
                &current->ipn_jhNameSpace, attr->ipna_pstrName, attr->ipna_sName,
                attr->ipna_pstrValue);
        }

        pjln = jf_linklist_getNextNode(pjln);
    }

    return u32Ret;
}

static boolean_t _compareChildNode(
    internal_ptree_node_t * temp, olchar_t * pstrNs, olchar_t * pstrName)
{
    boolean_t bRet = FALSE;
    olsize_t sNs = 0, sName = 0;

    if (pstrNs == NULL)
        sNs = 0;
    else
        sNs = ol_strlen(pstrNs);

    if (sNs != temp->ipn_sNs)
        return bRet;

    sName = ol_strlen(pstrName);
    if (sName != temp->ipn_sName)
        return bRet;

    if ((sNs > 0) && (ol_memcmp(temp->ipn_pstrNs, pstrNs, temp->ipn_sNs) != 0))
        return bRet;

    if (ol_memcmp(temp->ipn_pstrName, pstrName, temp->ipn_sName) != 0)
        return bRet;

    return TRUE;
}

static u32 _iterateNodeAttribute(
    jf_linklist_t * pjl, jf_ptree_fnOpAttribute_t fnOpAttribute, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode = NULL;
    internal_ptree_node_attribute_t * pipna = NULL;

    pNode = jf_linklist_getFirstNode(pjl);
    while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pipna = jf_linklist_getDataFromNode(pNode);

        u32Ret = fnOpAttribute(pipna, pArg);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pNode = jf_linklist_getNextNode(pNode);
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_ptree_destroy(jf_ptree_t ** ppPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = NULL;

    assert(ppPtree != NULL);

    pip = (internal_ptree_t *) *ppPtree;

    _destroyPtreeNodeList(&pip->ip_pipnRoot);
    jf_linklist_finiListAndData(&pip->ip_jlDeclaration, _fnFreePtreeNodeAttribute);

    jf_jiukun_freeMemory(ppPtree);

    return u32Ret;
}

u32 jf_ptree_create(jf_ptree_t ** ppPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = NULL;

    assert(ppPtree != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pip, sizeof(*pip));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pip, sizeof(*pip));
        pip->ip_pstrSeparator = ".";
        pip->ip_sSeparator = 1;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPtree = pip;
    else if (pip != NULL)
        jf_ptree_destroy((jf_ptree_t **)&pip);

    return u32Ret;    
}

u32 jf_ptree_findNode(
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
    find_ptree_node_param_t param;

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
                u32Ret = JF_ERR_PTREE_NODE_NOT_FOUND;
                *pu16NumOfNode = 0;
            }
        }
        else
        {
            ol_bzero(&param, sizeof(param));
            param.fpnp_pjsprKey = pjspr;
            param.fpnp_ppNode = ppNode;
            param.fpnp_u16MaxNode = *pu16NumOfNode;
            param.fpnp_u16NumOfNode = 0;

            _traversePtree(pip->ip_pipnRoot, _matchPtreeNode, &param);

            *pu16NumOfNode = param.fpnp_u16NumOfNode;
            if (*pu16NumOfNode == 0)
                u32Ret = JF_ERR_PTREE_NODE_NOT_FOUND;
        }
    }

    if (pjspr != NULL)
        jf_string_destroyParseResult(&pjspr);

    return u32Ret;
}

u32 jf_ptree_iterateNode(jf_ptree_node_t * pNode, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    assert(pNode != NULL);

    pipn = pipn->ipn_pipnChildren;
    while ((pipn != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = fnOpNode(pipn, pArg);

        if (u32Ret == JF_ERR_NO_ERROR)
            pipn = pipn->ipn_pipnSibling;
    }

    return u32Ret;
}

u32 jf_ptree_addChildNode(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olsize_t sNs,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue, jf_ptree_node_t ** ppChildNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;
    internal_ptree_node_t * pipn = NULL;

    assert(pPtree != NULL);

    u32Ret = _createPtreeNode(&pipn, pstrNs, sNs, pstrName, sName, pstrValue, sValue);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pNode == NULL)
        {
            /*Parent node is NULL, set the created node as the root node.*/
            assert(pip->ip_pipnRoot == NULL);
            pip->ip_pipnRoot = pipn;
        }
        else
        {
            /*Add the created node as the child node.*/
            u32Ret = _addPtreeChildNode(pNode, pipn);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppChildNode = pipn;
    else if (pipn != NULL)
        _destroyPtreeNode(&pipn);

    return u32Ret;
}

u32 jf_ptree_changeNodeValue(jf_ptree_node_t * pNode, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    u32Ret = _changePtreeNodeValue(pipn, pstrValue);

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

u32 jf_ptree_findNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    jf_ptree_node_attribute_t ** ppAttr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;
    olsize_t sPrefix = 0;

    assert(pstrName != NULL);

    /*Prefix string may be NULL.*/
    if (pstrPrefix != NULL)
        sPrefix = ol_strlen(pstrPrefix);

    u32Ret = _findPtreeNodeAttribute(
        &pipn->ipn_jlAttribute, pstrPrefix, sPrefix, pstrName, ol_strlen(pstrName), ppAttr);

    return u32Ret;
}

u32 jf_ptree_getNodeAttributePrefix(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrPrefix, olsize_t * psPrefix)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_attribute_t * pipna = (internal_ptree_node_attribute_t *)pAttr;

    *ppstrPrefix = pipna->ipna_pstrPrefix;
    if (psPrefix != NULL)
        *psPrefix = pipna->ipna_sPrefix;

    return u32Ret;
}

u32 jf_ptree_getNodeAttributeName(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrName, olsize_t * psName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_attribute_t * pipna = (internal_ptree_node_attribute_t *)pAttr;

    *ppstrName = pipna->ipna_pstrName;
    if (psName != NULL)
        *psName = pipna->ipna_sName;

    return u32Ret;
}

u32 jf_ptree_getNodeAttributeValue(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrValue, olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_attribute_t * pipna = (internal_ptree_node_attribute_t *)pAttr;

    *ppstrValue = pipna->ipna_pstrValue;
    if (psValue != NULL)
        *psValue = pipna->ipna_sValue;

    return u32Ret;
}

u32 jf_ptree_iterateNodeAttribute(
    jf_ptree_node_t * pNode, jf_ptree_fnOpAttribute_t fnOpAttribute, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    u32Ret = _iterateNodeAttribute(&pipn->ipn_jlAttribute, fnOpAttribute, pArg);

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
    jf_ptree_node_attribute_t * pAttr = NULL;

    u32Ret = _findPtreeNodeAttribute(
        &pipn->ipn_jlAttribute, pstrPrefix, sPrefix, pstrName, sName, &pAttr);
    if (u32Ret == JF_ERR_PTREE_NODE_ATTR_NOT_FOUND)
    {
        u32Ret = _createPtreeNodeAttribute(
            &pipna, pstrPrefix, sPrefix, pstrName, sName, pstrValue, sValue);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_linklist_appendTo(&pipn->ipn_jlAttribute, pipna);
    }

    if ((u32Ret != JF_ERR_NO_ERROR) && (pipna != NULL))
        _destroyPtreeNodeAttribute(&pipna);

    return u32Ret;
}

u32 jf_ptree_changeNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    olchar_t ** pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 jf_ptree_traverse(jf_ptree_t * pPtree, jf_ptree_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;

    u32Ret = _traversePtree(pip->ip_pipnRoot, fnOpNode, pArg);

    return u32Ret;
}

u32 jf_ptree_dump(jf_ptree_t * pPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;

    ol_printf("-----------------------------------------------------------------------\n");

    _printPtreeNodeAttributeList(&pip->ip_jlDeclaration);
    ol_printf("\n");
    _printPtreeNodeList(pip->ip_pipnRoot, 0);

    return u32Ret;
}

u32 jf_ptree_lookupNamespace(
    jf_ptree_node_t * pNode, olchar_t * pstrPrefix, olsize_t sPrefix, olchar_t ** ppstr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * temp = (internal_ptree_node_t *)pNode;

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
        if (jf_hashtree_hasEntry(&temp->ipn_jhNameSpace, pstrPrefix, sPrefix))
        {
            /*As soon as we find the namespace declaration, stop iterating the tree, as it would be
              a waste of time.*/
            jf_hashtree_getEntry(&temp->ipn_jhNameSpace, pstrPrefix, sPrefix, (void **)ppstr);
            break;
        }

        temp = temp->ipn_pipnParent;
    } while ((temp != NULL) && (u32Ret == JF_ERR_NO_ERROR));

    return u32Ret;
}

u32 jf_ptree_buildNamespaceTable(jf_ptree_t * pPtree)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;

    u32Ret = _traversePtree(pip->ip_pipnRoot, _buildXmlNamespaceTable, NULL);

    return u32Ret;
}

u32 jf_ptree_addDeclarationAttribute(
    jf_ptree_t * pPtree, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;
    internal_ptree_node_attribute_t * pipna = NULL;
    jf_ptree_node_attribute_t * pAttr = NULL;

    u32Ret = _findPtreeNodeAttribute(
        &pip->ip_jlDeclaration, pstrPrefix, sPrefix, pstrName, sName, &pAttr);
    if (u32Ret == JF_ERR_PTREE_NODE_ATTR_NOT_FOUND)
    {
        u32Ret = _createPtreeNodeAttribute(
            &pipna, pstrPrefix, sPrefix, pstrName, sName, pstrValue, sValue);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_linklist_appendTo(&pip->ip_jlDeclaration, pipna);
    }

    if ((u32Ret != JF_ERR_NO_ERROR) && (pipna != NULL))
        _destroyPtreeNodeAttribute(&pipna);

    return u32Ret;
}

u32 jf_ptree_findChildNode(
    jf_ptree_node_t * pNode, olchar_t * pstrNs, olchar_t * pstrName, jf_ptree_node_t ** ppChild)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    internal_ptree_node_t * temp = (internal_ptree_node_t *)pNode;
    boolean_t bRet = FALSE;

    temp = temp->ipn_pipnChildren;
    while (temp != NULL)
    {
        bRet = _compareChildNode(temp, pstrNs, pstrName);
        if (bRet)
        {
            *ppChild = temp;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        temp = temp->ipn_pipnSibling;
    }

    return u32Ret;
}

u32 jf_ptree_getNodeNs(jf_ptree_node_t * pNode, olchar_t ** ppstrNs, olsize_t * psNs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * temp = (internal_ptree_node_t *)pNode;

    *ppstrNs = temp->ipn_pstrNs;
    if (psNs != NULL)
        *psNs = temp->ipn_sNs;

    return u32Ret;
}

u32 jf_ptree_getNodeName(jf_ptree_node_t * pNode, olchar_t ** ppstrName, olsize_t * psName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * temp = (internal_ptree_node_t *)pNode;

    *ppstrName = temp->ipn_pstrName;
    if (psName != NULL)
        *psName = temp->ipn_sName;

    return u32Ret;
}

u32 jf_ptree_getNodeValue(jf_ptree_node_t * pNode, olchar_t ** ppstrValue, olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * temp = (internal_ptree_node_t *)pNode;

    *ppstrValue = temp->ipn_pstrValue;
    if (psValue != NULL)
        *psValue = temp->ipn_sValue;

    return u32Ret;
}

boolean_t jf_ptree_isLeafNode(jf_ptree_node_t * pNode)
{
    boolean_t bRet = FALSE;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    if (pipn->ipn_pipnChildren == NULL)
        bRet = TRUE;

    return bRet;
}

jf_ptree_node_t * jf_ptree_getRootNode(jf_ptree_t * pPtree)
{
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;

    return pip->ip_pipnRoot;
}

jf_ptree_node_t * jf_ptree_getChildNode(jf_ptree_node_t * pNode)
{
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    return pipn->ipn_pipnChildren;
}

jf_ptree_node_t * jf_ptree_getSiblingNode(jf_ptree_node_t * pNode)
{
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    return pipn->ipn_pipnSibling;
}

u32 jf_ptree_iterateDeclarationAttribute(
    jf_ptree_t * pPtree, jf_ptree_fnOpAttribute_t fnOpAttribute, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_t * pip = (internal_ptree_t *)pPtree;

    u32Ret = _iterateNodeAttribute(&pip->ip_jlDeclaration, fnOpAttribute, pArg);

    return u32Ret;
}

u32 jf_ptree_attachPrivate(jf_ptree_node_t * pNode, void * pPrivate)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    pipn->ipn_pPrivate = pPrivate;

    return u32Ret;
}

u32 jf_ptree_detachPrivate(jf_ptree_node_t * pNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    pipn->ipn_pPrivate = NULL;

    return u32Ret;
}

void * jf_ptree_getPrivate(jf_ptree_node_t * pNode)
{
    internal_ptree_node_t * pipn = (internal_ptree_node_t *)pNode;

    return pipn->ipn_pPrivate;
}

/*------------------------------------------------------------------------------------------------*/

