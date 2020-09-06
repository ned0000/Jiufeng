/**
 *  @file jf_linklist.c
 *
 *  @brief Implementation file for linked list.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */

static void _freeLinklist(jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData)
{
    jf_linklist_node_t * pjln = NULL, * pNode = NULL;

    assert(pList != NULL);

	pjln = pList->jl_pjlnHead;
    while (pjln != NULL)
    {
		pNode = pjln->jln_pjlnNext;

        /*Free the node data.*/
        if (fnFreeData != NULL)
            fnFreeData(&pjln->jln_pData);

        /*Free the node.*/
        jf_jiukun_freeMemory((void **)&pjln);

        pjln = pNode;
    }

    jf_linklist_init(pList);
}

/* --- public routine section ------------------------------------------------------------------- */

void jf_linklist_init(jf_linklist_t * pList)
{
    assert(pList != NULL);

    pList->jl_pjlnHead = NULL;
}

void jf_linklist_fini(jf_linklist_t * pList)
{
    assert(pList != NULL);

    _freeLinklist(pList, NULL);
}

void jf_linklist_finiListAndData(
    jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData)
{
    assert((pList != NULL) && (fnFreeData != NULL));

    _freeLinklist(pList, fnFreeData);
}

u32 jf_linklist_appendTo(jf_linklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode = NULL, * pjln = NULL;

    assert(pList != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pNode, sizeof(*pNode));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pNode, sizeof(*pNode));
		pNode->jln_pData = pData;

        /*Check if the head is empty.*/
		if (pList->jl_pjlnHead == NULL)
        {
            pList->jl_pjlnHead = pNode;
        }
        else
        {
            /*Add the new node to the end of linked list.*/
			pjln = pList->jl_pjlnHead;
			while (pjln->jln_pjlnNext != NULL)
				pjln = pjln->jln_pjlnNext;

			pjln->jln_pjlnNext = pNode;
        }
    }

    return u32Ret;
}

u32 jf_linklist_remove(jf_linklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNext = NULL, * pjln = NULL;

    assert(pList != NULL);

    /*Empty linked list, return.*/
    if (pList->jl_pjlnHead == NULL)
        return u32Ret;

    /*The data to be removed is the head of the linked list.*/
    if (pList->jl_pjlnHead->jln_pData == pData)
    {
        pjln = pList->jl_pjlnHead;
        pList->jl_pjlnHead = pjln->jln_pjlnNext;
        jf_jiukun_freeMemory((void **)&pjln);
        return u32Ret;
    }

    /*The data is not the head of the linked list, continue to find.*/
	pjln = pList->jl_pjlnHead;
    pNext = pjln->jln_pjlnNext;
    while (pNext != NULL)
    {
        if (pNext->jln_pData == pData)
        {
            pjln->jln_pjlnNext = pNext->jln_pjlnNext;
            jf_jiukun_freeMemory((void **)&pNext);
            break;
        }

        pjln = pjln->jln_pjlnNext;
        pNext = pjln->jln_pjlnNext;
    }

    return u32Ret;
}

u32 jf_linklist_insertTo(jf_linklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode;

    assert((pList != NULL) && (pData != NULL));

    u32Ret = jf_jiukun_allocMemory((void **)&pNode, sizeof(*pNode));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode->jln_pData = pData;

		pNode->jln_pjlnNext = pList->jl_pjlnHead;
		pList->jl_pjlnHead = pNode;
    }

    return u32Ret;
}

u32 jf_linklist_iterate(jf_linklist_t * pList, jf_linklist_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pjln = NULL, * pNode = NULL;

    assert(pList != NULL);

	pjln = pList->jl_pjlnHead;
    while ((pjln != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
		pNode = pjln->jln_pjlnNext;

        u32Ret = fnOpNode(pjln, pArg);
        if (u32Ret == JF_ERR_NO_ERROR)
            pjln = pNode;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
