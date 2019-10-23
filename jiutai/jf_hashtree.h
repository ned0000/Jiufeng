/**
 *  @file jf_hashtree.h
 *
 *  @brief The basic hash tree data structure
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_hashtree object
 *  @note Link with jiukun library for memory allocation
 *  @note The simple hash tree, suitable for items less than 50. For large amount of items, uses
 *   hash table in jf_hashtable.h
 *  @note This object is not thread safe
 *  
 */

#ifndef JIUTAI_HASHTREE_H
#define JIUTAI_HASHTREE_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/*hash tree*/
typedef struct jf_hashtree_node
{
    struct jf_hashtree_node * jhn_pjhnNext;
    struct jf_hashtree_node * jhn_pjhnPrev;
    olint_t jhn_nKey;
    olchar_t * jhn_pstrKeyValue;
    olsize_t jhn_sKey;
    void * jhn_pData;
} jf_hashtree_node_t;

typedef struct jf_hashtree_enumerator
{
    jf_hashtree_node_t * jhe_pjhnNode;
} jf_hashtree_enumerator_t;

typedef struct jf_hashtree
{
    jf_hashtree_node_t * jh_pjhnRoot;
} jf_hashtree_t;

typedef u32 (* jf_hashtree_fnFreeData_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Creates an empty hash tree
 *  
 *  @param pHashtree [in] the hashtree to free
 *
 *  @return void
 */
static inline void jf_hashtree_init(jf_hashtree_t * pHashtree)
{
    pHashtree->jh_pjhnRoot = NULL;
}

/** Free resources associated with a hashtree
 *
 *  @param pHashtree [in] the hashtree to free
 *
 *  @return void
 */
void jf_hashtree_fini(jf_hashtree_t * pHashtree);

/** Free resources associated with a hashtree
 *
 *  @param pHashtree [in] the hashtree to free
 *  @param fnFreeData [in] the function to free data
 *
 *  @return void
 */
void jf_hashtree_finiHashtreeAndData(
    jf_hashtree_t * pHashtree, jf_hashtree_fnFreeData_t fnFreeData);

/** Determine if the hash tree is empty
 *
 *  @param pHashtree [in] the hashtree to free
 *
 *  @return the empty is of the hashtree
 *  @retval TRUE the hashtree is empty
 *  @retval FALSE the hash tree is not empty
 */
static inline boolean_t jf_hashtree_isEmpty(jf_hashtree_t * pHashtree)
{
    return ((pHashtree->jh_pjhnRoot == NULL) ? TRUE : FALSE);
}

/** Determines if a key entry exists in a hashtree
 *
 *  @param pHashtree [in] the hashtree to operate on 
 *  @param pstrKey [in] the key 
 *  @param sKey [in] the length of the key 
 * 
 *  @return the existing state of the entry
 *  @retval TRUE the entry is existing
 *  @retval FALSE the entry is not existing
 */
boolean_t jf_hashtree_hasEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Adds an item to the hashtree
 * 
 *  @param pHashtree [in] the hashtree to operate on
 *  @param pstrKey [in] the key
 *  @param sKey [in] the length of the key
 *  @param pValue [in] the data to add into the hashtree
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_OUT_OF_MEMORY out of memory
 *
 */
u32 jf_hashtree_addEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * pValue);

/** Gets an item from a hashtree
 *
 *  @param pHashtree [in] the hashtree to operate on 
 *  @param pstrKey [in] the key 
 *  @param sKey [in] the length of the key 
 *  @param ppData [in/out] the pointer to the entry returned 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_HASHTREE_ENTRY_NOT_FOUND entry not found
 */
u32 jf_hashtree_getEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData);

/** Deletes a keyed item from the hashtree
 *
 *  @param pHashtree [in] the hashtree to operate on 
 *  @param pstrKey [in] the key 
 *  @param sKey [in] the length of the key 
 *
 *  @return the error code
 */
u32 jf_hashtree_deleteEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Return an Enumerator for a hash tree
 *
 *  @param pHashtree [in] the hashtree to get an enumerator for 
 *  @param pEnumerator [in/out] the enumerator
 *
 *  @return void
 */
static inline void jf_hashtree_initEnumerator(
    jf_hashtree_t * pHashtree, jf_hashtree_enumerator_t * pEnumerator)
{
    /*the enumerator is basically a state machine that keeps track of 
      which node we are at in the tree. So initialize it to the root.*/
    pEnumerator->jhe_pjhnNode = pHashtree->jh_pjhnRoot;
}

/** Free resources associated with an Enumerator created by 
 *  initHashtreeEnumerator
 *
 *  @param pEnumerator [in] the enumerator to free 
 *
 *  @return void
 */
static inline void jf_hashtree_finiEnumerator(
    jf_hashtree_enumerator_t * pEnumerator)
{
    ol_memset(pEnumerator, 0, sizeof(jf_hashtree_enumerator_t));
}

static inline boolean_t jf_hashtree_isEnumeratorEmptyNode(
    jf_hashtree_enumerator_t * pEnumerator)
{
    return ((pEnumerator->jhe_pjhnNode == NULL) ? TRUE : FALSE);
}

/** Advance an enumerator to the next item
 *
 *  @param pEnumerator [in] the enumerator to advance 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_END_OF_HASHTREE end of hashtree
 *
 */
static inline u32 jf_hashtree_moveEnumeratorNext(
    jf_hashtree_enumerator_t * pEnumerator)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pEnumerator->jhe_pjhnNode != NULL)
    {
        /*Advance the enumerator to point to the next node. If there is a node
          return 0, else return 1*/
        pEnumerator->jhe_pjhnNode = pEnumerator->jhe_pjhnNode->jhn_pjhnNext;
    }
    else
    {
        /*There are no more nodes*/
        u32Ret = JF_ERR_END_OF_HASHTREE;
    }

    return u32Ret;
}

/** Read from the current item of an enumerator
 *
 *  @param pEnumerator [in] the enumerator to read from 
 *  @param ppstrKey [out] the key of the current item 
 *  @param psKey [out] the length of the key of the current item 
 *  @param ppData [out] the data of the current item 
 *
 *  @return void
 */
static inline void jf_hashtree_getEnumeratorValue(
    jf_hashtree_enumerator_t * pEnumerator,
    olchar_t ** ppstrKey, olsize_t * psKey, void ** ppData)
{
    /*All we do, is just assign the pointers.*/
    if (ppstrKey != NULL)
    {
        *ppstrKey = pEnumerator->jhe_pjhnNode->jhn_pstrKeyValue;
    }
    if (psKey != NULL)
    {
        *psKey = pEnumerator->jhe_pjhnNode->jhn_sKey;
    }
    if (ppData != NULL)
    {
        *ppData = pEnumerator->jhe_pjhnNode->jhn_pData;
    }
}

#endif /*JIUTAI_HASHTREE_H*/

/*------------------------------------------------------------------------------------------------*/

