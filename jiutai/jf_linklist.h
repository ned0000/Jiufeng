/**
 *  @file jf_linklist.h
 *
 *  @brief Header file defines the linked list data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_linklist object.
 *  -# Link with jiukun library for memory allocation.
 *  -# This object is not thread safe.
 *  
 */

#ifndef JIUTAI_LINKLIST_H
#define JIUTAI_LINKLIST_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** The linked list node.
 */
typedef struct jf_linklist_node
{
    struct jf_linklist_node * jln_pjlnNext;
    void * jln_pData;
} jf_linklist_node_t;

/** The linked list data type.
 */
typedef struct jf_linklist
{
	jf_linklist_node_t * jl_pjlnHead;
} jf_linklist_t;

typedef u32 (* jf_linklist_fnFreeNodeData_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize an empty linked list.
 *
 *  @param pList [in] The linklist to be initialized.
 *
 *  @return Void.
 */
void jf_linklist_init(jf_linklist_t * pList);

/** Finalize the linked list.
 *
 *  @param pList [in] The linklist to finalize.
 *  
 *  @return Void.
 */
void jf_linklist_fini(jf_linklist_t * pList);

/** Finalize the linked list and data
 *
 *  @param pList [in] The linked list to finalize
 *  @param fnFreeData [in] The call back function to free data
 *
 *  @return Void.
 */
void jf_linklist_finiListAndData(jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData);

/** Append to the tail of the linked list.
 *
 *  @param pList [in] The linked list to append data.
 *  @param pData [in] The data to be appended.
 *
 *  @return The error code.
 */
u32 jf_linklist_appendTo(jf_linklist_t * pList, void * pData);

/** Intert to the head of the linked list.
 *
 *  @param pList [in] The linked list to insert data.
 *  @param pData [in] The data to be inserted.
 *
 *  @return The error code.
 */
u32 jf_linklist_insertTo(jf_linklist_t * pList, void * pData);

static inline boolean_t jf_linklist_isEmpty(jf_linklist_t * pList)
{
    if (pList->jl_pjlnHead == NULL)
        return TRUE;
    return FALSE;
}

/** Get the first node of linked list.
 *
 */
static inline jf_linklist_node_t * jf_linklist_getFirstNode(jf_linklist_t * pList)
{
	return pList->jl_pjlnHead;
}

/** Get the next node of the specified node.
 *
 */
static inline jf_linklist_node_t * jf_linklist_getNextNode(jf_linklist_node_t * pNode)
{
	return pNode->jln_pjlnNext;
}

/** Get data from the linked node.
 *
 */
static inline void * jf_linklist_getDataFromNode(jf_linklist_node_t * pNode)
{
	return pNode->jln_pData;
}

#endif /*JIUTAI_LINKLIST_H*/

/*------------------------------------------------------------------------------------------------*/

