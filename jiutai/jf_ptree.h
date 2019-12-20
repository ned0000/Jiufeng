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
 *  @return The termination status.
 *  @retval TRUE Continue the traverse.
 *  @retval TRUE Terminate the traverse.
 */
typedef boolean_t (* jf_ptree_fnOpNode_t)(jf_ptree_node_t * pNode, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Creates a property tree with root node.
 *  
 *  @param ppPtree [out] The property tree to create.
 *
 *  @return The error code.
 */
u32 jf_ptree_create(
    jf_ptree_t ** ppPtree, const olchar_t * pstrNs, const olsize_t sNs, const olchar_t * pstrName,
    const olsize_t sName, const olchar_t * pstrValue, const olsize_t sValue,
    jf_ptree_node_t ** ppRootNode);

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

/** Iterate the children nodes of the specified node.
 */
u32 jf_ptree_iterateNode(jf_ptree_node_t * pNode, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/** Add child node to node. 
 */
u32 jf_ptree_addChildNode(
    jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olsize_t sNs, const olchar_t * pstrName,
    const olsize_t sName, const olchar_t * pstrValue, const olsize_t sValue,
    jf_ptree_node_t ** ppChildNode);

/** Change node of property tree.
 */
u32 jf_ptree_changeNode(
    jf_ptree_node_t * pNode, const olchar_t * pstrNs, const olchar_t * pstrName,
    const olchar_t * pstrValue);

/** Delete node of property tree.
 *
 *  @note
 *  -# The node to be deleted cannot be the root node.
 *  -# Use jf_ptree_destroy() to destroy the tree including the root node.
 */
u32 jf_ptree_deleteNode(jf_ptree_node_t ** ppNode);

/** Get node of property tree.
 *
 *  @note
 *  -# Prefix string may be NULL.
 */
u32 jf_ptree_getNodeAttribute(
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
 */
u32 jf_ptree_traverse(jf_ptree_t * pTree, jf_ptree_fnOpNode_t fnOpNode, void * pArg);

/** Dump property tree.
 */
u32 jf_ptree_dump(jf_ptree_t * pTree);

#endif /*JIUTAI_PTREE_H*/

/*------------------------------------------------------------------------------------------------*/

