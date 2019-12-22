/**
 *  @file jf_dlinklist.c
 *
 *  @brief The double linked list data structure
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
#include "jf_err.h"
#include "jf_dlinklist.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */



/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

/** Init the double linked list
 *
 */
void jf_dlinklist_init(jf_dlinklist_t * pList)
{
    assert(pList != NULL);

    pList->jd_pjdnHead = pList->jd_pjdnTail = NULL;
}

/** Create the double linked list
 *
 */
void jf_dlinklist_fini(jf_dlinklist_t * pList)
{
    jf_dlinklist_node_t * pjdn, * pNode;

    assert(pList != NULL);

    pjdn = pList->jd_pjdnHead;
    while (pjdn != NULL)
    {
        pNode = pjdn->jdn_pjdnNext;
        jf_jiukun_freeMemory((void **)&pjdn);
        pjdn = pNode;
    }

    jf_dlinklist_init(pList);
}

/** Create the double linked list with the function to free the data
 * 
 */
void jf_dlinklist_finiListAndData(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData)
{
    jf_dlinklist_node_t * pjdn, * pNode;

    assert(pList != NULL);

    pjdn = pList->jd_pjdnHead;
    while (pjdn != NULL)
    {
        pNode = pjdn->jdn_pjdnNext;

        fnFreeData(&(pjdn->jdn_pData));

        jf_jiukun_freeMemory((void **)&pjdn);
        pjdn = pNode;
    }

    jf_dlinklist_init(pList);
}

/** remove all notes from double linked list
 * 
 */
void jf_dlinklist_removeAllNodes(
    jf_dlinklist_t * pList, jf_dlinklist_fnFreeNodeData_t fnFreeData)
{
    jf_dlinklist_node_t * pjdn, * pNode;

    assert(pList != NULL);

    pjdn = pList->jd_pjdnHead;
    while (pjdn != NULL)
    {
        pNode = pjdn->jdn_pjdnNext;

        fnFreeData(&(pjdn->jdn_pData));

        jf_jiukun_freeMemory((void **)&pjdn);
        pjdn = pNode;
    }

    jf_dlinklist_init(pList);
}

/** find the data
 * 
 */
u32 jf_dlinklist_findFirstData(
    jf_dlinklist_t * pList, void ** ppData,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    jf_dlinklist_node_t * pjdn;

    assert((pList != NULL) && (ppData != NULL) &&
           (fnFindData != NULL));

    *ppData = NULL;

    pjdn = pList->jd_pjdnHead;
    while (pjdn != NULL)
    {
        if (fnFindData(pjdn->jdn_pData, pKey))
        {
            *ppData = pjdn->jdn_pData;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pjdn = pjdn->jdn_pjdnNext;
    }

    return u32Ret;
}

/** find the node
 * 
 */
u32 jf_dlinklist_findFirstNode(
    jf_dlinklist_t * pList, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    jf_dlinklist_node_t * pjdn;

    assert((pList != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;

    pjdn = pList->jd_pjdnHead;
    while (pjdn != NULL)
    {
        if (fnFindData(pjdn->jdn_pData, pKey))
        {
            *ppNode = pjdn;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pjdn = pjdn->jdn_pjdnNext;
    }

    return u32Ret;
}

/** Create the double linked list with the function to free the data
 * 
 */
u32 jf_dlinklist_findLastData(
    jf_dlinklist_t * pList, void ** ppData,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    jf_dlinklist_node_t * pjdn;

    assert((pList != NULL) && (ppData != NULL) &&
           (fnFindData != NULL));

    *ppData = NULL;

    pjdn = pList->jd_pjdnTail;
    while (pjdn != NULL)
    {
        if (fnFindData(pjdn->jdn_pData, pKey))
        {
            *ppData = pjdn->jdn_pData;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pjdn = pjdn->jdn_pjdnPrev;
    }

    return u32Ret;
}

/** find last node
 * 
 */
u32 jf_dlinklist_findLastNode(
    jf_dlinklist_t * pList, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    jf_dlinklist_node_t * pjdn;

    assert((pList != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;

    pjdn = pList->jd_pjdnTail;
    while (pjdn != NULL)
    {
        if (fnFindData(pjdn->jdn_pData, pKey))
        {
            *ppNode = pjdn;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pjdn = pjdn->jdn_pjdnPrev;
    }

    return u32Ret;
}

/** find the next node
 * 
 */
u32 jf_dlinklist_findNextNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;

    assert((pNode != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;
    pNode = pNode->jdn_pjdnNext;
    while (pNode != NULL)
    {
        if (fnFindData(pNode->jdn_pData, pKey))
        {
            *ppNode = pNode;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pNode = pNode->jdn_pjdnNext;
    }

    return u32Ret;
}

/** find the previous node
 * 
 */
u32 jf_dlinklist_findPrevNode(
    jf_dlinklist_node_t * pNode, jf_dlinklist_node_t ** ppNode,
    jf_dlinklist_fnFindNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;

    assert((pNode != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;
    pNode = pNode->jdn_pjdnPrev;
    while (pNode != NULL)
    {
        if (fnFindData(pNode->jdn_pData, pKey))
        {
            *ppNode = pNode;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pNode = pNode->jdn_pjdnPrev;
    }

    return u32Ret;
}

u32 jf_dlinklist_appendTo(jf_dlinklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dlinklist_node_t * pNode;

    assert(pList != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pNode, sizeof(*pNode));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pNode, sizeof(*pNode));
        pNode->jdn_pData = pData;

        if (pList->jd_pjdnHead == NULL)
        {
            pList->jd_pjdnHead = pNode;
            pList->jd_pjdnTail = pNode;
        }
        else
        {
            pNode->jdn_pjdnPrev = pList->jd_pjdnTail;
            pList->jd_pjdnTail->jdn_pjdnNext = pNode;
            pList->jd_pjdnTail = pNode;
        }
    }

    return u32Ret;
}

u32 jf_dlinklist_iterate(jf_dlinklist_t * pList, jf_dlinklist_fnOpNode_t fnOpNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dlinklist_node_t * pjdn = NULL, * pNode = NULL;

    assert(pList != NULL);

    pjdn = pList->jd_pjdnHead;
    while ((pjdn != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        pNode = pjdn->jdn_pjdnNext;

        u32Ret = fnOpNode(pjdn, pArg);
        if (u32Ret == JF_ERR_NO_ERROR)
            pjdn = pNode;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

