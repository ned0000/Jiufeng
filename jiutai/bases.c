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
#include "olbasic.h"
#include "ollimit.h"
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
    hashtree_t * pHashtree, void * pKey, olsize_t sKey, olint_t value,
    hash_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_node_t * node = NULL;

    u32Ret = xcalloc((void **)&node, sizeof(hash_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = dupMemory((void **)&node->hn_pstrKeyValue, pKey, sKey);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            node->hn_nKey = value;
            node->hn_sKey = sKey;

            node->hn_phnNext = pHashtree->h_phnRoot;
            pHashtree->h_phnRoot = node;
            if (node->hn_phnNext != NULL)
                node->hn_phnNext->hn_phnPrev = node;

            *ppNode = node;
        }
        else
            xfree((void **)&node);
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
    hashtree_t * pHashtree, void * pKey,
    olsize_t sKey, boolean_t bCreate, hash_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_HASHTREE_ENTRY_NOT_FOUND;
    hash_node_t * current = pHashtree->h_phnRoot;
    olint_t value = _getHashValue(pKey, sKey);

    if (sKey == 0)
        return u32Ret;

    *ppNode = NULL;
    /*Iterate through our tree to see if we can find this key entry*/
    while (current != NULL)
    {
        /*Integer compares are very fast, this will weed out most non-matches*/
        if (current->hn_nKey == value)
        {
            /*Verify this is really a match*/
            if (current->hn_sKey == sKey &&
                memcmp(current->hn_pstrKeyValue, pKey, sKey) == 0)
            {
                *ppNode = current;
                u32Ret = JF_ERR_NO_ERROR;
                break;
            }
        }
        current = current->hn_phnNext;
    }

    if (*ppNode == NULL)
    {
        if (bCreate)
            return _newHashtreeEntry(pHashtree, pKey, sKey, value, ppNode);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

void initQueue(basic_queue_t * pQueue)
{
    pQueue->bq_pqnHead = pQueue->bq_pqnTail = NULL;
}

void finiQueue(basic_queue_t * pQueue)
{
    queue_node_t * pqn, * temp;

	assert(pQueue != NULL);

	temp = pQueue->bq_pqnHead;
    while (temp != NULL)
    {
        pqn = temp->qn_pqnNext;
        xfree((void **)&temp);
        temp = pqn;
    }
}

void finiQueueAndData(basic_queue_t * pQueue, fnFreeQueueData_t fnFreeData)
{
    queue_node_t * pqn, * temp;

	assert(pQueue != NULL);

	temp = pQueue->bq_pqnHead;
    while (temp != NULL)
    {
        pqn = temp->qn_pqnNext;

        fnFreeData(&(temp->qn_pData));

        xfree((void **)&temp);
        temp = pqn;
    }
}

boolean_t isQueueEmpty(basic_queue_t * pbq)
{
    return (pbq->bq_pqnHead == NULL ? TRUE : FALSE);
}

u32 enqueue(basic_queue_t * pbq, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    queue_node_t * pqn;

    u32Ret = xcalloc((void **)&pqn, sizeof(queue_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pqn->qn_pData = data;

        if (pbq->bq_pqnHead == NULL)
        {
            /*If there is no head, this new entry is the head*/
            pbq->bq_pqnHead = pqn;
            pbq->bq_pqnTail = pqn;
        }
        else
        {
            /*Since there is already a head, just attach this entry 
              to the tail, andcall this the new tail*/
            pbq->bq_pqnTail->qn_pqnNext = pqn;
            pbq->bq_pqnTail = pqn;
        }
    }

    return u32Ret;
}

void * dequeue(basic_queue_t * pbq)
{
    queue_node_t * temp;
    void * retval = NULL;

    assert(pbq != NULL);

    if (pbq->bq_pqnHead == NULL)
        return NULL;

    temp = pbq->bq_pqnHead;
    retval = temp->qn_pData;
    pbq->bq_pqnHead = pbq->bq_pqnHead->qn_pqnNext;
    if (pbq->bq_pqnHead == NULL)
    {
        pbq->bq_pqnTail = NULL;
    }
    xfree((void **)&temp);

    return retval;
}

void * peekQueue(basic_queue_t * pbq)
{
    if (pbq->bq_pqnHead == NULL)
        return NULL;
    else
        return pbq->bq_pqnHead->qn_pData;
}

/*
 * Stack Methods
 */

void initStack(basic_stack_t ** ppStack)
{
    *ppStack = NULL;
}

u32 pushStack(basic_stack_t ** ppStack, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    stack_node_t * retval;

    u32Ret = xmalloc((void **)&retval, sizeof(stack_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->sn_pData = data;
        retval->sn_psnNext = * ppStack;

        *ppStack = retval;
    }

    return u32Ret;
}

void * popStack(basic_stack_t ** ppStack)
{
    void * retval = NULL;
    void * temp;

    if (*ppStack != NULL)
    {
        retval = ((stack_node_t *) *ppStack)->sn_pData;
        temp = *ppStack;
        *ppStack = ((stack_node_t *) *ppStack)->sn_psnNext;
        xfree((void **)&temp);
    }

    return retval;
}

void * peekStack(basic_stack_t ** ppStack)
{
    void * retval = NULL;

    if (*ppStack != NULL)
        retval = ((stack_node_t *) *ppStack)->sn_pData;

    return retval;
}

void clearStack(basic_stack_t ** ppStack)
{
    void * temp = *ppStack;

    do
    {
        popStack(&temp);
    }
    while (temp != NULL);

    *ppStack = NULL;
}

/*
 * linked list
 */
void initLinkList(link_list_t * pList)
{
    assert(pList != NULL);

    pList->ll_pllnHead = NULL;
}

/** Finalize the linked list
 *
 */
void finiLinkList(link_list_t * pList)
{
    link_list_node_t * plln, * pNode;

    assert(pList != NULL);

	plln = pList->ll_pllnHead;
    while (plln != NULL)
    {
		pNode = plln->lln_pllnNext;
        xfree((void **)&plln);
        plln = pNode;
    }

    initLinkList(pList);
}

/** Finalize the linked list with the function to free the data
 * 
 */
void finiLinkListAndData(link_list_t * pList,
    fnFreeListNodeData_t fnFreeData)
{
    link_list_node_t * plln, * pNode;

    assert((pList != NULL) && (fnFreeData != NULL));

	plln = pList->ll_pllnHead;
    while (plln != NULL)
    {
		pNode = plln->lln_pllnNext;

		fnFreeData(&(plln->lln_pData));

        xfree((void **)&plln);
        plln = pNode;
    }

    initLinkList(pList);
}

/** Append the data to the linked list
 *
 */
u32 appendToLinkList(link_list_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    link_list_node_t * pNode, * plln;

    assert(pList != NULL);

    u32Ret = xcalloc((void **)&pNode, sizeof(link_list_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode->lln_pData = pData;

		if (pList->ll_pllnHead == NULL)
        {
            pList->ll_pllnHead = pNode;
        }
        else
        {
			plln = pList->ll_pllnHead;
			while (plln->lln_pllnNext != NULL)
				plln = plln->lln_pllnNext;

			plln->lln_pllnNext = pNode;
        }
    }

    return u32Ret;
}

/** Insert the data to the head of the linked list
 *
 */
u32 insertToLinkList(link_list_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    link_list_node_t * pNode;

    assert((pList != NULL) && (pData != NULL));

    u32Ret = xcalloc((void **)&pNode, sizeof(link_list_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode->lln_pData = pData;

		pNode->lln_pllnNext = pList->ll_pllnHead;
		pList->ll_pllnHead = pNode;
    }

    return u32Ret;
}

/*
 * double linked list
 */

/** Init the double linked list
 *
 */
void initDlinkList(dlink_list_t * pList)
{
    assert(pList != NULL);

    pList->dl_pdlnHead = pList->dl_pdlnTail = NULL;
}

/** Create the double linked list
 *
 */
void finiDlinkList(dlink_list_t * pList)
{
    dlink_list_node_t * pdln, * pNode;

    assert(pList != NULL);

    pdln = pList->dl_pdlnHead;
    while (pdln != NULL)
    {
        pNode = pdln->dln_pdlnNext;
        xfree((void **)&pdln);
        pdln = pNode;
    }

    initDlinkList(pList);
}

/** Create the double linked list with the function to free the data
 * 
 */
void finiDlinkListAndData(
    dlink_list_t * pList, fnFreeListNodeData_t fnFreeData)
{
    dlink_list_node_t * pdln, * pNode;

    assert(pList != NULL);

    pdln = pList->dl_pdlnHead;
    while (pdln != NULL)
    {
        pNode = pdln->dln_pdlnNext;

        fnFreeData(&(pdln->dln_pData));

        xfree((void **)&pdln);
        pdln = pNode;
    }

    initDlinkList(pList);
}

/** remove all notes from double linked list
 * 
 */
void removeAllNodesFromDlinkList(dlink_list_t * pList,
    fnFreeListNodeData_t fnFreeData)
{
    dlink_list_node_t * pdln, * pNode;

    assert(pList != NULL);

    pdln = pList->dl_pdlnHead;
    while (pdln != NULL)
    {
        pNode = pdln->dln_pdlnNext;

        fnFreeData(&(pdln->dln_pData));

        xfree((void **)&pdln);
        pdln = pNode;
    }

    initDlinkList(pList);
}

/** find the data
 * 
 */
u32 findFirstDataFromDlinkList(
    dlink_list_t * pList, void ** ppData,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    dlink_list_node_t * pdln;

    assert((pList != NULL) && (ppData != NULL) &&
           (fnFindData != NULL));

    *ppData = NULL;

    pdln = pList->dl_pdlnHead;
    while (pdln != NULL)
    {
        if (fnFindData(pdln->dln_pData, pKey))
        {
            *ppData = pdln->dln_pData;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pdln = pdln->dln_pdlnNext;
    }

    return u32Ret;
}

/** find the node
 * 
 */
u32 findFirstNodeFromDlinkList(
    dlink_list_t * pList, dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    dlink_list_node_t * pdln;

    assert((pList != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;

    pdln = pList->dl_pdlnHead;
    while (pdln != NULL)
    {
        if (fnFindData(pdln->dln_pData, pKey))
        {
            *ppNode = pdln;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pdln = pdln->dln_pdlnNext;
    }

    return u32Ret;
}

/** Create the double linked list with the function to free the data
 * 
 */
u32 findLastDataFromDlinkList(
    dlink_list_t * pList, void ** ppData,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    dlink_list_node_t * pdln;

    assert((pList != NULL) && (ppData != NULL) &&
           (fnFindData != NULL));

    *ppData = NULL;

    pdln = pList->dl_pdlnTail;
    while (pdln != NULL)
    {
        if (fnFindData(pdln->dln_pData, pKey))
        {
            *ppData = pdln->dln_pData;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pdln = pdln->dln_pdlnPrev;
    }

    return u32Ret;
}

/** find last node
 * 
 */
u32 findLastNodeFromDlinkList(dlink_list_t * pList, dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    dlink_list_node_t * pdln;

    assert((pList != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;

    pdln = pList->dl_pdlnTail;
    while (pdln != NULL)
    {
        if (fnFindData(pdln->dln_pData, pKey))
        {
            *ppNode = pdln;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pdln = pdln->dln_pdlnPrev;
    }

    return u32Ret;
}

/** find the next node
 * 
 */
u32 findNextNodeFromDlinkList(
    dlink_list_node_t * pNode, dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;

    assert((pNode != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;
    pNode = pNode->dln_pdlnNext;
    while (pNode != NULL)
    {
        if (fnFindData(pNode->dln_pData, pKey))
        {
            *ppNode = pNode;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pNode = pNode->dln_pdlnNext;
    }

    return u32Ret;
}

/** find the previous node
 * 
 */
u32 findPrevNodeFromDlinkList(
    dlink_list_node_t * pNode, dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;

    assert((pNode != NULL) && (ppNode != NULL) &&
           (fnFindData != NULL));

    *ppNode = NULL;
    pNode = pNode->dln_pdlnPrev;
    while (pNode != NULL)
    {
        if (fnFindData(pNode->dln_pData, pKey))
        {
            *ppNode = pNode;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pNode = pNode->dln_pdlnPrev;
    }

    return u32Ret;
}

/** Append the data to the double linked list
 *
 */
u32 appendToDlinkList(dlink_list_t * pList, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dlink_list_node_t * pNode;

    assert(pList != NULL);

    u32Ret = xcalloc((void **)&pNode, sizeof(dlink_list_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pNode->dln_pData = pData;

        if (pList->dl_pdlnHead == NULL)
        {
            pList->dl_pdlnHead = pNode;
            pList->dl_pdlnTail = pNode;
        }
        else
        {
            pNode->dln_pdlnPrev = pList->dl_pdlnTail;
            pList->dl_pdlnTail->dln_pdlnNext = pNode;
            pList->dl_pdlnTail = pNode;
        }
    }

    return u32Ret;
}

/*
 * Hashtree Methods
 */

void finiHashtree(hashtree_t * pHashtree)
{
    hash_node_t * phn;
    hash_node_t * pNode;

    assert(pHashtree != NULL);

    phn = pHashtree->h_phnRoot;
    while (phn != NULL)
    {
        // Iterate through each node, and free all the resources
        pNode = phn->hn_phnNext;
        if (phn->hn_pstrKeyValue != NULL)
        {
            xfree((void **)&(phn->hn_pstrKeyValue));
        }
        xfree((void **)&phn);
        phn = pNode;
    }
}

void finiHashtreeAndData(
    hashtree_t * pHashtree, fnFreeHashtreeData_t fnFreeData)
{
    hash_node_t * phn;
    hash_node_t * pNode;

    assert((pHashtree != NULL) && (fnFreeData != NULL));

    phn = pHashtree->h_phnRoot;
    while (phn != NULL)
    {
        // Iterate through each node, and free all the resources
        pNode = phn->hn_phnNext;

        if (phn->hn_pData != NULL)
            fnFreeData(&phn->hn_pData);

        if (phn->hn_pstrKeyValue != NULL)
        {
            xfree((void **)&phn->hn_pstrKeyValue);
        }
        xfree((void **)&phn);

        phn = pNode;
    }
}

boolean_t hasHashtreeEntry(hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey)
{
    boolean_t bRet = FALSE;
    hash_node_t * phn;
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*This can be duplicated by calling Find entry, 
      but setting the create flag to false*/
    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &phn);
    if (u32Ret == JF_ERR_NO_ERROR)
        bRet = TRUE;

    return bRet;
}

u32 addHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*This can be duplicated by calling FindEntry, and setting create to true*/
    hash_node_t * phn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, TRUE, &phn);
    if (u32Ret == JF_ERR_NO_ERROR)
        phn->hn_pData = value;

    return u32Ret;
}

u32 getHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*This can be duplicated by calling FindEntry and setting create to false.
      If a match is found, just return the data*/
    hash_node_t * phn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &phn);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppData = phn->hn_pData;
    }

    return u32Ret;
}

u32 deleteHashtreeEntry(hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_node_t * phn;

    u32Ret = _findHashtreeEntry(pHashtree, pstrKey, sKey, FALSE, &phn);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Then remove it from the tree*/
        if (phn == pHashtree->h_phnRoot)
        {
            pHashtree->h_phnRoot = phn->hn_phnNext;
            if (phn->hn_phnNext != NULL)
                phn->hn_phnNext->hn_phnPrev = NULL;
        }
        else
        {
            phn->hn_phnPrev->hn_phnNext = phn->hn_phnNext;
            if (phn->hn_phnNext != NULL)
                phn->hn_phnNext->hn_phnPrev = phn->hn_phnPrev;
        }
        xfree((void **)&phn->hn_pstrKeyValue);
        xfree((void **)&phn);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

