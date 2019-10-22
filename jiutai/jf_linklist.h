/**
 *  @file jf_linklist.h
 *
 *  @brief The linked list data structure
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_linklist object
 *  @note Link with jiukun library for cache
 *  @note This object is not thread safe
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

/*linked list*/
typedef struct jf_linklist_node
{
    struct jf_linklist_node * jln_pjlnNext;
    void * jln_pData;
} jf_linklist_node_t;

typedef struct jf_linklist
{
	jf_linklist_node_t * jl_pjlnHead;
} jf_linklist_t;

typedef u32 (* jf_linklist_fnFreeNodeData_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize an empty linklist
 *
 *  @param pList [in] the linklist to be initialized
 *
 *  @return void
 */
void jf_linklist_init(jf_linklist_t * pList);

/** Finalize the linklist
 *
 *  @param pList [in] The linklist to finalize
 *  
 *  @return void
 */
void jf_linklist_fini(jf_linklist_t * pList);

/** Finalize the linklist and data
 *
 *  @param pList [in] The linklist to finalize
 *  @param fnFreeData [in] The call back function to free data
 *
 *  @return void
 */
void jf_linklist_finiListAndData(
    jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData);

/** Append to the tail of the linked list
 *
 *  @param pList [in] The linklist to append data
 *  @param pData [in] The data to be appended
 *
 *  @return the error code
 */
u32 jf_linklist_appendTo(jf_linklist_t * pList, void * pData);

/** Intert to the head of the linked list
 *
 *  @param pList [in] The linklist to insert data
 *  @param pData [in] The data to be inserted
 *
 *  @return the error code
 */
u32 jf_linklist_insertTo(jf_linklist_t * pList, void * pData);

/** Create cache for linklist node
 *
 *  @note Cache should be created before using other functions
 *
 *  @return the error code
 */
u32 jf_linklist_createCache(void);

/** Destroy cache for linklist node
 *
 *  @return the error code
 */
u32 jf_linklist_destroyCache(void);

static inline boolean_t jf_linklist_isEmpty(jf_linklist_t * pList)
{
    if (pList->jl_pjlnHead == NULL)
        return TRUE;
    return FALSE;
}

/**
 *  Get the first node of linked list
 */
static inline jf_linklist_node_t * jf_linklist_getFirstNode(
    jf_linklist_t * pList)
{
	return pList->jl_pjlnHead;
}

/**
 *  Get the next node of the specified node
 */
static inline jf_linklist_node_t * jf_linklist_getNextNode(
    jf_linklist_node_t * pNode)
{
	return pNode->jln_pjlnNext;
}

/**
 *  Get data from the linked node
 */
static inline void * jf_linklist_getDataFromNode(jf_linklist_node_t * pNode)
{
	return pNode->jln_pData;
}

#endif /*JIUTAI_LINKLIST_H*/

/*------------------------------------------------------------------------------------------------*/

