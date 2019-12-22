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

/** Callback function to Operate on property tree node when traversing the tree.
 *
 *  @note
 *  -# The iteration will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @return The error code.
 */
typedef u32 (* jf_ptree_fnOpNode_t)(jf_ptree_node_t * pNode, void * pArg);

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

/** Find node of property tree with key.
 *
 *  @note
 *  -# If key is NULL, return the root node.
 *  -# The key uses "." as the separator by default.
 *  -# If multiple nodes are available, the first node is returned.
 */
u32 jf_ptree_findNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, jf_ptree_node_t ** ppNode);

/** Find all nodes of property tree with key.
 *
 *  @note
 *  -# If key is NULL, return the root node.
 *  -# The key uses "." as the separator by default.
 *  -# If multiple nodes are available, all nodes are returned unless maximum nodes are reached.
 */
u32 jf_ptree_findAllNode(
    jf_ptree_t * pPtree, olchar_t * pstrKey, jf_ptree_node_t ** ppNode, u16 * pu16NumOfNode);

/** Find child node of the parent node by node name.
 *
 *  @note
 *  -# If multiple nodes are available, the first node is returned.
 */
u32 jf_ptree_findChildNode(
    jf_ptree_node_t * pNode, olchar_t * pstrNs, olchar_t * pstrName, jf_ptree_node_t ** ppChild);

/** Iterate the children nodes of the specified node.
 */
u32 jf_ptree_iterateNode(jf_ptree_node_t * pNode, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/** Add child node to node.
 *
 *  @note
 *  -# If pNode is NULL, the node to be added is as root node. After add, the root node is returned
 *     by ppChildNode.
 *
 *  @param pPtree [in] The property tree.
 *  @param pNode [in] The parent node to add child node.
 *  @param ppChildNode [out] The child node returned.
 */
u32 jf_ptree_addChildNode(
    jf_ptree_t * pPtree, jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olsize_t sNs,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue, jf_ptree_node_t ** ppChildNode);

/** Change node of the property tree.
 */
u32 jf_ptree_changeNode(
    jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olchar_t * pstrName,
    const olchar_t * pstrValue);

/** Get value of the node.
 */
u32 jf_ptree_getNodeValue(jf_ptree_node_t * pNode, olchar_t ** ppstrValue);

/** Delete node of property tree.
 *
 *  @note
 *  -# The node to be deleted cannot be the root node.
 *  -# Use jf_ptree_destroy() to destroy the tree including the root node.
 */
u32 jf_ptree_deleteNode(jf_ptree_node_t ** ppNode);

/** Find attribute of the node.
 *
 *  @note
 *  -# Prefix string may be NULL.
 */
u32 jf_ptree_findNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    olchar_t ** pstrValue);

/** Delete node attribute.
 */
u32 jf_ptree_deleteNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName);

/** Add node attribute.
 */
u32 jf_ptree_addNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue);

/** Change node attribute.
 */
u32 jf_ptree_changeNodeAttribute(
    jf_ptree_node_t * pNode, const olchar_t * pstrPrefix, const olchar_t * pstrName,
    olchar_t ** pstrValue);

/** Traverse property tree.
 *
 *  @param pPtree [in] The property tree.
 *  @param fnOpNode [in] The callback function for each node.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 */
u32 jf_ptree_traverse(jf_ptree_t * pPtree, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/** Dump property tree.
 *
 *  @param pPtree [in] The property tree.
 *
 *  @return The error code.
 */
u32 jf_ptree_dump(jf_ptree_t * pTree);

/** Resolves a namespace prefix from the scope of the given node.
 *
 *  @param pNode [in] The node used to start the resolve.
 *  @param pstrPrefix [in] The namespace prefix to resolve.
 *  @param sPrefix [in] The lenght of the prefix.
 *  @param ppstr [out] The resolved namespace, NULL if unable to resolve.
 *
 *  @return The error code.
 */
u32 jf_ptree_lookupXmlNamespace(
    jf_ptree_node_t * pNode, olchar_t * pstrPrefix, olsize_t sPrefix, olchar_t ** ppstr);

/** Build XML name space table.
 *
 *  @param pPtree [in] The property tree.
 *
 *  @return The error code.
 */
u32 jf_ptree_buildXmlNamespaceTable(jf_ptree_t * pPtree);

u32 jf_ptree_addDeclaration(
    jf_ptree_t * pPtree, const olchar_t * pstrPrefix, const olsize_t sPrefix,
    const olchar_t * pstrName, const olsize_t sName, const olchar_t * pstrValue,
    const olsize_t sValue);

#endif /*JIUTAI_PTREE_H*/

/*------------------------------------------------------------------------------------------------*/

