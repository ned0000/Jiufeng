/**
 *  @file jf_ptree.h
 *
 *  @brief Header file defines the interface for property tree.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_ptree object.
 *  -# The property tree provides a tree structure to store key/value pairs.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# Link with jf_string library for string parse.
 *  -# Link with jf_linklist object for attribute linked list.
 *  -# Link with jf_hashtree object for name space hash.
 *  -# Link with jf_stack object for stack operation.
 *  -# This object is not thread safe.
 *  -# All strings in this header file should be with length. 
 */

#ifndef JIUTAI_PTREE_H
#define JIUTAI_PTREE_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the property tree data type.
 */
typedef void  jf_ptree_t;

/** Define the property tree node data type.
 */
typedef void  jf_ptree_node_t;

/** Define the property tree node attibute data type.
 */
typedef void  jf_ptree_node_attribute_t;

/** Callback function to operate on property tree node when traversing the tree.
 *
 *  @note
 *  -# The iteration will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @return The error code.
 */
typedef u32 (* jf_ptree_fnOpNode_t)(jf_ptree_t * pPtree, jf_ptree_node_t * pNode, void * pArg);

/** Callback function to operate on attribute when iterating the attribute list.
 *
 *  @note
 *  -# The iteration will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @return The error code.
 */
typedef u32 (* jf_ptree_fnOpAttribute_t)(jf_ptree_node_attribute_t * pAttr, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Creates a property tree with root node.
 *  
 *  @param ppPtree [out] The property tree to create.
 *
 *  @return The error code.
 */
u32 jf_ptree_create(jf_ptree_t ** ppPtree);

/** Destroy property tree.
 *
 *  @param ppPtree [in/out] The property tree to destroy.
 *
 *  @return The error code.
 */
u32 jf_ptree_destroy(jf_ptree_t ** ppPtree);

/*--------------------------------------------------------------------------*/
/*Functions for node.*/
/*--------------------------------------------------------------------------*/

/** Find node of property tree with key.
 *
 *  @note
 *  -# If key is NULL, return the root node.
 *  -# The key uses "." as the separator by default.
 *  -# If multiple nodes are available, the first node is returned.
 *
 *  @param pPtree [in] The property tree to find node.
 *  @param pstrKey [in] The key in string.
 *  @param sKey [in] The length of the key.
 *  @param ppNode [out] The node found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR The node is found.
 *  @retval JF_ERR_PTREE_NODE_NOT_FOUND The node is not found.
 */
u32 jf_ptree_findNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, olsize_t sKey, jf_ptree_node_t ** ppNode);

/** Add node to property tree with key.
 *
 *  @note
 *  -# The key cannot be NULL.
 *  -# The key uses "." as the separator by default.
 *  -# If the node with same namespace and name is already existing in the tree, the old node
 *     is modified with the new value. Otherwise a new node is created.
 *
 *  @param pPtree [in] The property tree.
 *  @param pstrKey [in] The key in string.
 *  @param sKey [in] The length of the key.
 *  @param pstrValue [in] The value of the node.
 *  @param sValue [in] The size of the value.
 *  @param ppNode [out] The node to be added.
 *
 *  @return The error code.
 */
u32 jf_ptree_replaceNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, olsize_t sKey, const olchar_t * pstrValue,
    olsize_t sValue, jf_ptree_node_t ** ppNode);

/** Find all nodes of property tree with key.
 *
 *  @note
 *  -# If key is NULL, return the root node.
 *  -# The key uses "." as the separator by default.
 *  -# If multiple nodes are available, all nodes are returned unless maximum nodes are reached.
 *
 *  @param pPtree [in] The property tree to find node.
 *  @param pstrKey [in] The key in string.
 *  @param sKey [in] The length of the key.
 *  @param ppNode [out] The node array found.
 *  @param pu16NumOfNode [out] Number of node found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR The node is found.
 *  @retval JF_ERR_PTREE_NODE_NOT_FOUND The node is not found.
 */
u32 jf_ptree_findAllNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, olsize_t sKey, jf_ptree_node_t ** ppNode,
    u16 * pu16NumOfNode);

/** Find child node of the parent node by node name.
 *
 *  @note
 *  -# If multiple nodes are available, the first node is returned.
 *
 *  @param pPtree [in] The property tree to find child node.
 *  @param pNode [in] The property tree node.
 *  @param pstrNs [in] The name space of the node.
 *  @param sNs [in] The size of the name space.
 *  @param pstrName [in] The name of the node.
 *  @param sName [in] The size of the name.
 *  @param ppChild [out] The child node found.
 *
 *  @return The error code.
 *  @retval JF_ERR_PTREE_NODE_NOT_FOUND The node is not found.
 */
u32 jf_ptree_findChildNode(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, olchar_t * pstrNs, olsize_t sNs,
    olchar_t * pstrName, olsize_t sName, jf_ptree_node_t ** ppChild);

/** Iterate the children nodes of the specified node.
 *
 *  @note
 *  -# The iteration will stop if return code of callback function is not JF_ERR_NO_ERROR.
 *
 *  @param pPtree [in] The property tree to iterate.
 *  @param pNode [in] The property tree node.
 *  @param fnOpNode [in] The callback function for each child node.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 */
u32 jf_ptree_iterateNode(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/** Add child node to node.
 *
 *  @note
 *  -# If pNode is NULL, the node to be added is as root node. After add, the root node is returned
 *     by ppChildNode.
 *  -# If the node with same namespace, name and value is already existing in the tree, a new node
 *     is created and added to the tree.
 *
 *  @param pPtree [in] The property tree.
 *  @param pNode [in] The parent node to add child node.
 *  @param pstrNs [in] The name space of the node.
 *  @param sNs [in] The size of the name space.
 *  @param pstrName [in] The name of the node.
 *  @param sName [in] The size of the name.
 *  @param pstrValue [in] The value of the node.
 *  @param sValue [in] The size of the value.
 *  @param ppChildNode [out] The child node returned.
 *
 *  @return The error code.
 */
u32 jf_ptree_addChildNode(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, const olchar_t * pstrNs, olsize_t sNs,
    const olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue, olsize_t sValue,
    jf_ptree_node_t ** ppChildNode);

/** Change node of the property tree.
 *
 *  @param pNode [in] The property tree node.
 *  @param pstrValue [int] The node value to be set.
 *  @param sValue [in] The size of the value.
 *
 *  @return The error code.
 */
u32 jf_ptree_changeNodeValue(jf_ptree_node_t * pNode, const olchar_t * pstrValue, olsize_t sValue);

/** Get name space of the node.
 *
 *  @param pNode [in] The property tree node.
 *  @param ppstrNs [out] The node name space string.
 *  @param psNs [out] The size of the node name space, it can be NULL.
 *
 *  @return The error code.
 */
u32 jf_ptree_getNodeNs(jf_ptree_node_t * pNode, olchar_t ** ppstrNs, olsize_t * psNs);

/** Get full name of the node. The full name is from the top node to current node.
 *
 *  @param pPtree [in] The property tree.
 *  @param pNode [in] The property tree node.
 *  @param pstrName [out] The node name buffer.
 *  @param psName [in/out] The size of the node name buffer as in parameter; The size of node name
 *   as out parameter.
 *
 *  @return The error code.
 */
u32 jf_ptree_getNodeFullName(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, olchar_t * pstrName, olsize_t * psName);

/** Get name of the node.
 *
 *  @param pNode [in] The property tree node.
 *  @param ppstrName [out] The node name string.
 *  @param psName [out] The size of the node name, it can be NULL.
 *
 *  @return The error code.
 */
u32 jf_ptree_getNodeName(jf_ptree_node_t * pNode, olchar_t ** ppstrName, olsize_t * psName);

/** Get value of the node.
 *
 *  @param pNode [in] The property tree node.
 *  @param ppstrValue [out] The node value string.
 *  @param psValue [out] The size of the node value, it can be NULL.
 *
 *  @return The error code.
 */
u32 jf_ptree_getNodeValue(jf_ptree_node_t * pNode, olchar_t ** ppstrValue, olsize_t * psValue);

/** Check if the node is a leaf node.
 *
 *  @param pNode [in] The node to be checked.
 *
 *  @return If it's a leaf node.
 *  @retval TRUE It's a leaf node.
 *  @retval FALSE It's not a leaf node.
 */
boolean_t jf_ptree_isLeafNode(jf_ptree_node_t * pNode);

/** Delete node of property tree.
 *
 *  @note
 *  -# The node to be deleted cannot be the root node.
 *  -# Use jf_ptree_destroy() to destroy the tree including the root node.
 *
 *  @param ppNode [in/out] The node to be deleted.
 *
 *  @return The error code.
 */
u32 jf_ptree_deleteNode(jf_ptree_node_t ** ppNode);

/*--------------------------------------------------------------------------*/
/*Functions for travesing property tree by application itself.*/
/*--------------------------------------------------------------------------*/

/** Get the root node of property tree.
 *
 *  @param pPtree [in] The property tree.
 *
 *  @return The root node.
 */
jf_ptree_node_t * jf_ptree_getRootNode(jf_ptree_t * pPtree);

/** Get the child node of the property tree node.
 *
 *  @param pNode [in] The property tree node.
 *
 *  @return The child node.
 */
jf_ptree_node_t * jf_ptree_getChildNode(jf_ptree_node_t * pNode);

/** Get the sibling node of the property tree node.
 *
 *  @param pNode [in] The property tree node.
 *
 *  @return The sibling node.
 */
jf_ptree_node_t * jf_ptree_getSiblingNode(jf_ptree_node_t * pNode);

/*--------------------------------------------------------------------------*/
/*Functions for node attribute.*/
/*--------------------------------------------------------------------------*/

/** Find attribute of the node.
 *
 *  @note
 *  -# Prefix string may be NULL.
 *
 *  @param pNode [in] The property tree node.
 *  @param pstrPrefix [in] The attribute prefix string.
 *  @param sPrefix [in] The size of the attribute prefix string.
 *  @param pstrName [in] The attribute name string.
 *  @param sName [in] The size of the name string.
 *  @param ppAttr [out] The attribute found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR The node attribute is found.
 *  @retval JF_ERR_PTREE_NODE_ATTR_NOT_FOUND The node attribute is not found.
 */
u32 jf_ptree_findNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName, jf_ptree_node_attribute_t ** ppAttr);

/** Get prefix of the node attribute.
 *
 *  @param pAttr [in] The node attribute.
 *  @param ppstrPrefix [out] The attribute prefix string.
 *  @param psPrefix [out] The size of the node attribute prefix, it can be NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_ptree_getNodeAttributePrefix(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrPrefix, olsize_t * psPrefix);

/** Get name of the node attribute.
 *
 *  @param pAttr [in] The node attribute.
 *  @param ppstrName [out] The attribute name string.
 *  @param psName [out] The size of the node attribute name, it can be NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_ptree_getNodeAttributeName(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrName, olsize_t * psName);

/** Get value of the node attribute.
 *
 *  @param pAttr [in] The node attribute.
 *  @param ppstrValue [out] The attribute value string.
 *  @param psValue [out] The size of the node attribute value, it can be NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_ptree_getNodeAttributeValue(
    jf_ptree_node_attribute_t * pAttr, olchar_t ** ppstrValue, olsize_t * psValue);

/** Iterate the attribute of the node.
 *
 *  @note
 *  -# The iteration will stop if return code of callback function is not JF_ERR_NO_ERROR.
 *
 *  @param pNode [in] The property tree node.
 *  @param fnOpAttribute [in] The callback function for each attribute.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 */
u32 jf_ptree_iterateNodeAttribute(
    jf_ptree_node_t * pNode, jf_ptree_fnOpAttribute_t fnOpAttribute, void * pArg);

/** Delete node attribute.
 *
 *  @param pNode [in] The property tree node.
 *  @param pstrPrefix [in] The attribute prefix.
 *  @param sPrefix [in] The size of the attribute prefix string.
 *  @param pstrName [in] The attribute name.
 *  @param sName [in] The size of the attribute name string.
 *
 *  @return The error code.
 */
u32 jf_ptree_deleteNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName);

/** Add node attribute.
 *
 *  @param pNode [in] The node to add attribute.
 *  @param pstrPrefix [in] The attribute prefix.
 *  @param sPrefix [in] The size of the attribute prefix string.
 *  @param pstrName [in] The name of the attribute.
 *  @param sName [in] The size of the attribute name string.
 *  @param pstrValue [in] The value of the attribute.
 *  @param sValue [in] The size of the attribute value string.
 *
 *  @return The error code.
 */
u32 jf_ptree_addNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue, olsize_t sValue);

/** Change node attribute.
 *
 *  @param pNode [in] The node to change attribute.
 *  @param pstrPrefix [in] The attribute prefix.
 *  @param sPrefix [in] The size of the attribute prefix string.
 *  @param pstrName [in] The attribute name string.
 *  @param sName [in] The size of the attribute name string.
 *  @param pstrValue [in] The value of the attribute.
 *  @param sValue [in] The size of the attribute value string.
 *
 *  @return The error code.
 */
u32 jf_ptree_changeNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue, olsize_t sValue);

/*--------------------------------------------------------------------------*/
/*Functions for traversing property tree.*/
/*--------------------------------------------------------------------------*/

/** Traverse property tree.
 *
 *  @note
 *  -# The callback function is called from parent to children.
 *
 *  @param pPtree [in] The property tree.
 *  @param fnOpNode [in] The callback function for each node.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 */
u32 jf_ptree_traverse(jf_ptree_t * pPtree, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/*--------------------------------------------------------------------------*/
/*Functions for debug property tree.*/
/*--------------------------------------------------------------------------*/

/** Dump property tree.
 *
 *  @param pPtree [in] The property tree.
 *
 *  @return The error code.
 */
u32 jf_ptree_dump(jf_ptree_t * pPtree);

/*--------------------------------------------------------------------------*/
/*Functions for name space of node.*/
/*--------------------------------------------------------------------------*/

/** Resolves a namespace prefix from the scope of the given node.
 *
 *  @param pNode [in] The node used to start the resolve.
 *  @param pstrPrefix [in] The namespace prefix to resolve.
 *  @param sPrefix [in] The lenght of the prefix.
 *  @param ppstr [out] The resolved namespace, NULL if unable to resolve.
 *
 *  @return The error code.
 */
u32 jf_ptree_lookupNamespace(
    jf_ptree_node_t * pNode, olchar_t * pstrPrefix, olsize_t sPrefix, olchar_t ** ppstr);

/** Build XML name space table.
 *
 *  @param pPtree [in] The property tree.
 *
 *  @return The error code.
 */
u32 jf_ptree_buildNamespaceTable(jf_ptree_t * pPtree);

/*--------------------------------------------------------------------------*/
/*Functions for declaration of property tree.*/
/*--------------------------------------------------------------------------*/

/** Add attribute to the declaration of the property tree.
 *
 *  @param pPtree [in] The property tree.
 *  @param pstrPrefix [in] The attribute prefix.
 *  @param sPrefix [in] The size of the prefix string.
 *  @param pstrName [in] The name of the attribute.
 *  @param sName [in] The size of the name string.
 *  @param pstrValue [in] The value of the attribute.
 *  @param sValue [in] The size of the value string.
 *
 *  @return The error code.
 */
u32 jf_ptree_addDeclarationAttribute(
    jf_ptree_t * pPtree, const olchar_t * pstrPrefix, olsize_t sPrefix,
    const olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue, olsize_t sValue);

/** Iterate the attibute of the property tree declaration.
 *
 *  @param pPtree [in] The property tree.
 *  @param fnOpAttribute [in] The callback function for each attribute.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 */
u32 jf_ptree_iterateDeclarationAttribute(
    jf_ptree_t * pPtree, jf_ptree_fnOpAttribute_t fnOpAttribute, void * pArg);

/*--------------------------------------------------------------------------*/
/*Functions for application to store the private data in node.*/
/*--------------------------------------------------------------------------*/

/** Attach private data to property tree node.
 */
u32 jf_ptree_attachPrivate(jf_ptree_node_t * pNode, void * pPrivate);

/** Detach private data from property tree node.
 */
u32 jf_ptree_detachPrivate(jf_ptree_node_t * pNode);

/** Get private data of property tree node.
 */
void * jf_ptree_getPrivate(jf_ptree_node_t * pNode);

#endif /*JIUTAI_PTREE_H*/

/*------------------------------------------------------------------------------------------------*/
