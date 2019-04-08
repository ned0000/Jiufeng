/**
 *  @file jf_hashtree.h
 *
 *  @brief The basic hash tree data structure
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
#include "jf_mem.h"
#include "jf_err.h"
#include "jf_hashtree.h"

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

