/**
 *  @file jf_dlinklist.h
 *
 *  @brief The double linked list data structure
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_dlinklist object
 *  @note Link with xmalloc object file
 *  @note This object is not thread safe
 *  
 */

#ifndef JIUTAI_DLINKLIST_H
#define JIUTAI_DLINKLIST_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/*double linked list*/
typedef struct jf_dlinklist_node
{
    struct jf_dlinklist_node * jdn_pjdnNext;
    struct jf_dlinklist_node * jdn_pjdnPrev;
    void * jdn_pData;
} jf_dlinklist_node_t;

typedef struct jf_dlinklist
{
    jf_dlinklist_node_t * jd_pjdnHead;
    jf_dlinklist_node_t * jd_pjdnTail;
} jf_dlinklist_t;

typedef u32 (* jf_dlinklist_fnFreeNodeData_t)(void ** ppData);

typedef boolean_t (* jf_dlinklist_fnFindNodeData_t)(void * pData, void * pKey);

/* --- functional routines ---------------------------------------------------------------------- */

void jf_dlinklist_init(jf_dlinklist_t * pList);

void jf_dlinklist_fini(jf_dlinklist_t * pList);

void jf_dlinklist_finiListAndData(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData);

void jf_dlinklist_removeAllNodes(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData);

u32 jf_dlinklist_findFirstData(
    jf_dlinklist_t * pList, void ** ppData,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_findLastData(
    jf_dlinklist_t * pList, void ** ppData,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_findFirstNode(jf_dlinklist_t * pList,
    jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_findLastNode(
    jf_dlinklist_t * pList, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_findNextNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_findPrevNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey);

u32 jf_dlinklist_appendTo(jf_dlinklist_t * pList, void * pData);

/**
 *  Get data from the linked node
 */
static inline void * jf_dlinklist_getDataFromNode(jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pData;
}

/**
 *  Get the first node of double linked list
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getFirstNode(
    jf_dlinklist_t * pList)
{
    return pList->jd_pjdnHead;
}

/**
 *  Get the last node of the double linked list
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getLastNode(
    jf_dlinklist_t * pList)
{
    return pList->jd_pjdnTail;
}

/**
 *  Get the next node of the specified node
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getNextNode(
    jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pjdnNext;
}

/**
 *  Get the previous node of the specified node
 */
static inline jf_dlinklist_node_t * jf_dlinklist_getPrevNode(
    jf_dlinklist_node_t * pNode)
{
    return pNode->jdn_pjdnPrev;
}

#endif /*JIUTAI_DLINKLIST_H*/

/*------------------------------------------------------------------------------------------------*/

