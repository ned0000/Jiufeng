/**
 *  @file jf_dlinklist.h
 *
 *  @brief Header file define double linked list data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_dlinklist object.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# This object is not thread safe.
 *  
 */

#ifndef JIUTAI_DLINKLIST_H
#define JIUTAI_DLINKLIST_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the double linked list node data type.
 */
typedef struct jf_dlinklist_node
{
    /*Next node in the list.*/
    struct jf_dlinklist_node * jdn_pjdnNext;
    /*Previous node in the list.*/
    struct jf_dlinklist_node * jdn_pjdnPrev;
    /*Data of this node.*/
    void * jdn_pData;
} jf_dlinklist_node_t;

/** Define the double linked list data type.
 */
typedef struct jf_dlinklist
{
    /**Head node of the list.*/
    jf_dlinklist_node_t * jd_pjdnHead;
    /**Tail node of the list.*/
    jf_dlinklist_node_t * jd_pjdnTail;
} jf_dlinklist_t;

/** Callback function for freeing the node data.
 */
typedef u32 (* jf_dlinklist_fnFreeNodeData_t)(void ** ppData);

/** Callback function for finding the node.
 */
typedef boolean_t (* jf_dlinklist_fnFindNodeData_t)(void * pData, void * pKey);

/** Callback function to operation on the node when iterating the double linked list node.
 *
 *  @note
 *  -# The iteration will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @return The error code.
 */
typedef u32 (* jf_dlinklist_fnOpNode_t)(jf_dlinklist_node_t * pNode, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the double linked list.
 *
 *  @param pList [in] The linked list to initialize.
 *
 *  @return Void.
 */
void jf_dlinklist_init(jf_dlinklist_t * pList);

/** Finalize the double linked list.
 *
 *  @param pList [in] The linked list to finalize.
 *
 *  @return Void.
 */
void jf_dlinklist_fini(jf_dlinklist_t * pList);

/** Finalize the double linked list and free the node data.
 *
 *  @param pList [in] The linked list to finalize.
 *  @param fnFreeData [in] The callback function to free node data.
 *
 *  @return Void.
 */
void jf_dlinklist_finiListAndData(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData);

/** Remove all nodes from the double linked list and free the node data.
 *
 *  @param pList [in] The linked list.
 *  @param fnFreeData [in] The callback function to free node data.
 *
 *  @return Void.
 */
void jf_dlinklist_removeAllNodes(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData);

/** Find first data in double linked list.
 *
 *  @note
 *  -# The search is from head to tail.
 *
 *  @param pList [in] The linked list to find data.
 *  @param ppData [out] The data found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findFirstData(
    jf_dlinklist_t * pList, void ** ppData, jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Find last data in double linked list.
 *
 *  @note
 *  -# The search is from tail to head.
 *
 *  @param pList [in] The linked list to find data.
 *  @param ppData [out] The data found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findLastData(
    jf_dlinklist_t * pList, void ** ppData, jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Find first node in double linked list.
 *
 *  @note
 *  -# The search is from head to tail.
 *
 *  @param pList [in] The linked list to find data.
 *  @param ppNode [out] The node found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findFirstNode(
    jf_dlinklist_t * pList, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Find last node in double linked list.
 *
 *  @note
 *  -# The search is from tail to head.
 *
 *  @param pList [in] The linked list to find data.
 *  @param ppNode [out] The node found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findLastNode(
    jf_dlinklist_t * pList, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Find next node in double linked list.
 *
 *  @note
 *  -# The search is from current node to tail.
 *
 *  @param pNode [in] The current node in linked list.
 *  @param ppNode [out] The node found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findNextNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Find previous node in double linked list.
 *
 *  @note
 *  -# The search is from current node to head.
 *
 *  @param pNode [in] The current node in linked list.
 *  @param ppNode [out] The node found.
 *  @param fnFindData [in] The callback function to find node data.
 *  @param pArg [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Data is not found.
 */
u32 jf_dlinklist_findPrevNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pArg);

/** Append the data to the double linked list.
 *
 *  @param pList [in] The double linked list to append data to.
 *  @param pData [in] The data to be appended.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_dlinklist_appendTo(jf_dlinklist_t * pList, void * pData);

/** Iterate the double linked list and call the callback function.
 *
 *  @param pList [in] The linked list to iterate.
 *  @param fnOpNode [in] The callback function to operate on the data.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_dlinklist_iterate(jf_dlinklist_t * pList, jf_dlinklist_fnOpNode_t fnOpNode, void * pArg);

/** Get data from the linked node.
 *
 *  @param pNode [in] The list node.
 *
 *  @return The data of the node.
 */
static inline void * jf_dlinklist_getDataFromNode(jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pData;
}

/** Get the first node of double linked list.
 *
 *  @param pList [in] The linked list to iterate.
 *
 *  @return The first node of the list.
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getFirstNode(jf_dlinklist_t * pList)
{
    return pList->jd_pjdnHead;
}

/** Get the last node of the double linked list.
 *
 *  @param pList [in] The linked list to iterate.
 *
 *  @return The last node of the list.
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getLastNode(jf_dlinklist_t * pList)
{
    return pList->jd_pjdnTail;
}

/** Get the next node of the specified node.
 *
 *  @param pNode [in] The node of the list.
 *
 *  @return The next node.
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getNextNode(jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pjdnNext;
}

/** Get the previous node of the specified node.
 *
 *  @param pNode [in] The node of the list.
 *
 *  @return The previous node.
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getPrevNode(jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pjdnPrev;
}

#endif /*JIUTAI_DLINKLIST_H*/

/*------------------------------------------------------------------------------------------------*/

