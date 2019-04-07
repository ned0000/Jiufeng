/**
 *  @file bases.c
 *
 *  @brief provide common data structure, stack, queue, hashtree.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <unistd.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "xmalloc.h"
#include "errcode.h"
#include "bases.h"

/* --- private data/data structure section --------------------------------- */



/* --- private routine section---------------------------------------------- */
/** Calculate a numeric Hash from a given string
 *
 *  @param pKey [in] The string to hash 
 *  @param u32KeyLen [in] The length of the string to hash 
 
 *  @return the hash value   
 */
static olint_t _getHashValue(void * pKey, u32 u32KeyLen)
{
    olint_t value = 0;
    olchar_t temp[4];

    if (u32KeyLen <= 4)
    {
        // If the key length is <= 4, the hash is just the key 
        // expressed as an integer
        memset(temp, 0, 4);
        memcpy(temp, pKey, u32KeyLen);
        value = *((olint_t *) temp);
    }
    else
    {
        // If the key length is >4, the hash is the first 4 bytes 
        // XOR with the last 4
        memcpy(temp, pKey, 4);
        value = *((olint_t *) temp);
        memcpy(temp, (olchar_t *) pKey + (u32KeyLen - 4), 4);
        value = value ^ (*((olint_t *) temp));

        // If the key length is >= 10, the hash is also XOR with the 
        // middle 4 bytes
        if (u32KeyLen >= 10)
        {
            memcpy(temp, (olchar_t *) pKey + (u32KeyLen / 2), 4);
            value = value ^ (*((olint_t *) temp));
        }
    }

    return value;
}

static u32 _newHashtreeEntry(
    jf_hashtree_t * pHashtree, void * pKey, olsize_t sKey, olint_t value,
    jf_hashtree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtree_node_t * node = NULL;

    u32Ret = jf_mem_calloc((void **)&node, sizeof(jf_hashtree_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mem_duplicate((void **)&node->jhn_pstrKeyValue, pKey, sKey);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            node->jhn_nKey = value;
            node->jhn_sKey = sKey;

            node->jhn_pjhnNext = pHashtree->jh_pjhnRoot;
            pHashtree->jh_pjhnRoot = node;
            if (node->jhn_pjhnNext != NULL)
                node->jhn_pjhnNext->jhn_pjhnPrev = node;

            *ppNode = node;
        }
        else
            jf_mem_free((void **)&node);
    }

    return u32Ret;
}

/** Determine if a key entry exists in a hash tree, and creates it 
 *  if requested
 *
 *  @param pHashtree [in] the hashtree to operate on 
 *  @param pKey [in] the key 
 *  @param sKey [in] the length of the key 
 *  @param bCreate [in] if true, create the entry
 *  @param ppNode [in/out] the hashtree node
 * 
 *  @return the error code
 */
static u32 _findHashtreeEntry(
    jf_hashtree_t * pHashtree, void * pKey,
    olsize_t sKey, boolean_t bCreate, jf_hashtree_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_HASHTREE_ENTRY_NOT_FOUND;
    jf_hashtree_node_t * current = pHashtree->jh_pjhnRoot;
    olint_t value = _getHashValue(pKey, sKey);

    if (sKey == 0)
        return u32Ret;

    *ppNode = NULL;
    /*Iterate through our tree to see if we can find this key entry*/
    while (current != NULL)
    {
        /*Integer compares are very fast, this will weed out most non-matches*/
        if (current->jhn_nKey == value)
        {
            /*Verify this is really a match*/
            if (current->jhn_sKey == sKey &&
                memcmp(current->jhn_pstrKeyValue, pKey, sKey) == 0)
            {
                *ppNode = current;
                u32Ret = JF_ERR_NO_ERROR;
                break;
            }
        }
        current = current->jhn_pjhnNext;
    }

    if (*ppNode == NULL)
    {
        if (bCreate)
            return _newHashtreeEntry(pHashtree, pKey, sKey, value, ppNode);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

void jf_queue_init(jf_queue_t * pQueue)
{
    pQueue->jq_pjqnHead = pQueue->jq_pjqnTail = NULL;
}

void jf_queue_fini(jf_queue_t * pQueue)
{
    jf_queue_node_t * pjqn, * temp;

	assert(pQueue != NULL);

	temp = pQueue->jq_pjqnHead;
    while (temp != NULL)
    {
        pjqn = temp->jqn_pjqnNext;
        jf_mem_free((void **)&temp);
        temp = pjqn;
    }
}

void jf_queue_finiQueueAndData(
    jf_queue_t * pQueue, jf_queue_fnFreeData_t fnFreeData)
{
    jf_queue_node_t * pjqn, * temp;

	assert(pQueue != NULL);

	temp = pQueue->jq_pjqnHead;
    while (temp != NULL)
    {
        pjqn = temp->jqn_pjqnNext;

        fnFreeData(&(temp->jqn_pData));

        jf_mem_free((void **)&temp);
        temp = pjqn;
    }
}

boolean_t jf_queue_isEmpty(jf_queue_t * pQueue)
{
    return (pQueue->jq_pjqnHead == NULL ? TRUE : FALSE);
}

u32 jf_queue_enqueue(jf_queue_t * pQueue, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_queue_node_t * pjqn;

    u32Ret = jf_mem_calloc((void **)&pjqn, sizeof(jf_queue_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjqn->jqn_pData = data;

        if (pQueue->jq_pjqnHead == NULL)
        {
            /*If there is no head, this new entry is the head*/
            pQueue->jq_pjqnHead = pjqn;
            pQueue->jq_pjqnTail = pjqn;
        }
        else
        {
            /*Since there is already a head, just attach this entry 
              to the tail, andcall this the new tail*/
            pQueue->jq_pjqnTail->jqn_pjqnNext = pjqn;
            pQueue->jq_pjqnTail = pjqn;
        }
    }

    return u32Ret;
}

void * jf_queue_dequeue(jf_queue_t * pQueue)
{
    jf_queue_node_t * temp;
    void * retval = NULL;

    assert(pQueue != NULL);

    if (pQueue->jq_pjqnHead == NULL)
        return NULL;

    temp = pQueue->jq_pjqnHead;
    retval = temp->jqn_pData;
    pQueue->jq_pjqnHead = pQueue->jq_pjqnHead->jqn_pjqnNext;
    if (pQueue->jq_pjqnHead == NULL)
    {
        pQueue->jq_pjqnTail = NULL;
    }
    jf_mem_free((void **)&temp);

    return retval;
}

void * jf_queue_peek(jf_queue_t * pQueue)
{
    if (pQueue->jq_pjqnHead == NULL)
        return NULL;
    else
        return pQueue->jq_pjqnHead->jqn_pData;
}

/*
 * Stack Methods
 */

void jf_stack_init(jf_stack_t ** ppStack)
{
    *ppStack = NULL;
}

u32 jf_stack_push(jf_stack_t ** ppStack, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_stack_node_t * retval;

    u32Ret = jf_mem_alloc((void **)&retval, sizeof(jf_stack_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->jsn_pData = data;
        retval->jsn_pjsnNext = * ppStack;

        *ppStack = retval;
    }

    return u32Ret;
}

void * jf_stack_pop(jf_stack_t ** ppStack)
{
    void * retval = NULL;
    void * temp;

    if (*ppStack != NULL)
    {
        retval = ((jf_stack_node_t *) *ppStack)->jsn_pData;
        temp = *ppStack;
        *ppStack = ((jf_stack_node_t *) *ppStack)->jsn_pjsnNext;
        jf_mem_free((void **)&temp);
    }

    return retval;
}

void * jf_stack_peek(jf_stack_t ** ppStack)
{
    void * retval = NULL;

    if (*ppStack != NULL)
        retval = ((jf_stack_node_t *) *ppStack)->jsn_pData;

    return retval;
}

void jf_stack_clear(jf_stack_t ** ppStack)
{
    void * temp = *ppStack;

    do
    {
        jf_stack_pop(&temp);
    }
    while (temp != NULL);

    *ppStack = NULL;
}

/*
 * linked list
 */
void jf_linklist_init(jf_linklist_t * pList)
{
    assert(pList != NULL);

    pList->jl_pjlnHead = NULL;
}

/** Finalize the linked list
 *
 */
void jf_linklist_fini(jf_linklist_t * pList)
{
    jf_linklist_node_t * pjln, * pNode;

    assert(pList != NULL);

	pjln = pList->jl_pjlnHead;
    while (pjln != NULL)
    {
		pNode = pjln->jln_pjlnNext;
        jf_mem_free((void **)&pjln);
        pjln = pNode;
    }

    jf_linklist_init(pList);
}

/** Finalize the linked list with the function to free the data
 * 
 */
void jf_linklist_finiListAndData(
    jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData)
{
    jf_linklist_node_t * pjln, * pNode;

    assert((pList != NULL) && (fnFreeData != NULL));

	pjln = pList->jl_pjlnHead;
    while (pjln != NULL)
    {
		pNode = pjln->jln_pjlnNext;

		fnFreeData(&(pjln->jln_pData));

        jf_mem_free((void **)&pjln);
        pjln = pNode;
    }

    jf_linklist_init(pList);
}

/** Append the data to the linked list
 *
 */
u32 jf_linklist_appendTo(jf_linklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode, * pjln;

    assert(pList != NULL);

    u32Ret = jf_mem_calloc((void **)&pNode, sizeof(jf_linklist_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode->jln_pData = pData;

		if (pList->jl_pjlnHead == NULL)
        {
            pList->jl_pjlnHead = pNode;
        }
        else
        {
			pjln = pList->jl_pjlnHead;
			while (pjln->jln_pjlnNext != NULL)
				pjln = pjln->jln_pjlnNext;

			pjln->jln_pjlnNext = pNode;
        }
    }

    return u32Ret;
}

/** Insert the data to the head of the linked list
 *
 */
u32 jf_linklist_insertTo(jf_linklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_linklist_node_t * pNode;

    assert((pList != NULL) && (pData != NULL));

    u32Ret = jf_mem_calloc((void **)&pNode, sizeof(jf_linklist_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode->jln_pData = pData;

		pNode->jln_pjlnNext = pList->jl_pjlnHead;
		pList->jl_pjlnHead = pNode;
    }

    return u32Ret;
}

/*
 * double linked list
 */

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
        jf_mem_free((void **)&pjdn);
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

        jf_mem_free((void **)&pjdn);
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

        jf_mem_free((void **)&pjdn);
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

/** Append the data to the double linked list
 *
 */
u32 jf_dlinklist_appendTo(jf_dlinklist_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dlinklist_node_t * pNode;

    assert(pList != NULL);

    u32Ret = jf_mem_calloc((void **)&pNode, sizeof(jf_dlinklist_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
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

/*
 * Hashtree Methods
 */

void jf_hashtree_fini(jf_hashtree_t * pHashtree)
{
    jf_hashtree_node_t * pjhn;
    jf_hashtree_node_t * pNode;

    assert(pHashtree != NULL);

    pjhn = pHashtree->jh_pjhnRoot;
    while (pjhn != NULL)
    {
        // Iterate through each node, and free all the resources
        pNode = pjhn->jhn_pjhnNext;
        if (pjhn->jhn_pstrKeyValue != NULL)
        {
            jf_mem_free((void **)&(pjhn->jhn_pstrKeyValue));
        }
        jf_mem_free((void **)&pjhn);
        pjhn = pNode;
    }
}

void jf_hashtree_finiHashtreeAndData(
    jf_hashtree_t * pHashtree, jf_hashtree_fnFreeData_t fnFreeData)
{
    jf_hashtree_node_t * pjhn;
    jf_hashtree_node_t * pNode;

    assert((pHashtree != NULL) && (fnFreeData != NULL));

    pjhn = pHashtree->jh_pjhnRoot;
    while (pjhn != NULL)
    {
        // Iterate through each node, and free all the resources
        pNode = pjhn->jhn_pjhnNext;

        if (pjhn->jhn_pData != NULL)
            fnFreeData(&pjhn->jhn_pData);

        if (pjhn->jhn_pstrKeyValue != NULL)
        {
            jf_mem_free((void **)&pjhn->jhn_pstrKeyValue);
        }
        jf_mem_free((void **)&pjhn);

        pjhn = pNode;
    }
}

boolean_t jf_hashtree_hasEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey)
{
    boolean_t bRet = FALSE;
    jf_hashtree_node_t * pjhn;
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*This can be duplicated by calling Find entry, 
      but setting the create flag to false*/
    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &pjhn);
    if (u32Ret == JF_ERR_NO_ERROR)
        bRet = TRUE;

    return bRet;
}

u32 jf_hashtree_addEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*This can be duplicated by calling FindEntry, and setting create to true*/
    jf_hashtree_node_t * pjhn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, TRUE, &pjhn);
    if (u32Ret == JF_ERR_NO_ERROR)
        pjhn->jhn_pData = value;

    return u32Ret;
}

u32 jf_hashtree_getEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*This can be duplicated by calling FindEntry and setting create to false.
      If a match is found, just return the data*/
    jf_hashtree_node_t * pjhn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &pjhn);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppData = pjhn->jhn_pData;
    }

    return u32Ret;
}

u32 jf_hashtree_deleteEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtree_node_t * pjhn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &pjhn);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Then remove it from the tree*/
        if (pjhn == pHashtree->jh_pjhnRoot)
        {
            pHashtree->jh_pjhnRoot = pjhn->jhn_pjhnNext;
            if (pjhn->jhn_pjhnNext != NULL)
                pjhn->jhn_pjhnNext->jhn_pjhnPrev = NULL;
        }
        else
        {
            pjhn->jhn_pjhnPrev->jhn_pjhnNext = pjhn->jhn_pjhnNext;
            if (pjhn->jhn_pjhnNext != NULL)
                pjhn->jhn_pjhnNext->jhn_pjhnPrev = pjhn->jhn_pjhnPrev;
        }
        jf_mem_free((void **)&pjhn->jhn_pstrKeyValue);
        jf_mem_free((void **)&pjhn);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

