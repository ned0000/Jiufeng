/**
 *  @file jf_hashtree.h
 *
 *  @brief Header file which define the basic hash tree object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_hashtree object.
 *  -# Link with jiukun library for memory allocation.
 *  -# The simple hash tree, suitable for items less than 50. For large amount of items, uses
 *   hash table in jf_hashtable.h.
 *  -# This object is not thread safe.
 *  -# The hash tree will hash string key to integer key when adding application data. When
 *   seaching, the integer key is compared firstly and then the string key, this will speed up the
 *   finding of data.
 */

#ifndef JIUTAI_HASHTREE_H
#define JIUTAI_HASHTREE_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the hash tree node data type.
 */
typedef struct jf_hashtree_node
{
    /**The next node.*/
    struct jf_hashtree_node * jhn_pjhnNext;
    /**The previous node.*/
    struct jf_hashtree_node * jhn_pjhnPrev;
    /**The key after hash.*/
    olint_t jhn_nKey;
    /**The key string.*/
    olchar_t * jhn_pstrKey;
    /**The length of the key string.*/
    olsize_t jhn_sKey;
    /**The application data.*/
    void * jhn_pData;
} jf_hashtree_node_t;

/** Define the hash tree enumerator data type.
 */
typedef struct jf_hashtree_enumerator
{
    /**The hash tree node.*/
    jf_hashtree_node_t * jhe_pjhnNode;
} jf_hashtree_enumerator_t;

/** Define the hash tree data type.
 */
typedef struct jf_hashtree
{
    /**The root node of the hash tree.*/
    jf_hashtree_node_t * jh_pjhnRoot;
} jf_hashtree_t;

/** Callback function for freeing data in hash tree node.
 */
typedef u32 (* jf_hashtree_fnFreeData_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Creates an empty hash tree.
 *  
 *  @param pHashtree [in] The hash tree to free.
 *
 *  @return Void.
 */
static inline void jf_hashtree_init(jf_hashtree_t * pHashtree)
{
    pHashtree->jh_pjhnRoot = NULL;
}

/** Free resources associated with a hash tree.
 *
 *  @param pHashtree [in] The hash tree to free.
 *
 *  @return Void.
 */
void jf_hashtree_fini(jf_hashtree_t * pHashtree);

/** Free resources associated with a hash tree.
 *
 *  @param pHashtree [in] The hash tree to free.
 *  @param fnFreeData [in] The function to free data.
 *
 *  @return Void.
 */
void jf_hashtree_finiHashtreeAndData(
    jf_hashtree_t * pHashtree, jf_hashtree_fnFreeData_t fnFreeData);

/** Determine if the hash tree is empty.
 *
 *  @param pHashtree [in] The hash tree to free.
 *
 *  @return Void.
 */
static inline boolean_t jf_hashtree_isEmpty(jf_hashtree_t * pHashtree)
{
    return ((pHashtree->jh_pjhnRoot == NULL) ? TRUE : FALSE);
}

/** Determines if a key entry exists in a hash tree.
 *
 *  @param pHashtree [in] The hash tree to operate on.
 *  @param pstrKey [in] The key.
 *  @param sKey [in] The length of the key.
 * 
 *  @return the existing state of the entry.
 *  @retval TRUE the entry is existing.
 *  @retval FALSE the entry is not existing.
 */
boolean_t jf_hashtree_hasEntry(jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Adds an item to the hash tree.
 * 
 *  @param pHashtree [in] The hash tree to operate on.
 *  @param pstrKey [in] The key.
 *  @param sKey [in] The length of the key.
 *  @param pValue [in] The data to add into the hash tree.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memory.
 *
 */
u32 jf_hashtree_addEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * pValue);

/** Gets an item from a hash tree.
 *
 *  @param pHashtree [in] The hash tree to operate on.
 *  @param pstrKey [in] The key.
 *  @param sKey [in] The length of the key.
 *  @param ppData [in/out] The pointer to the entry returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_HASHTREE_ENTRY_NOT_FOUND Entry not found.
 */
u32 jf_hashtree_getEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData);

/** Deletes a keyed item from the hash tree.
 *
 *  @param pHashtree [in] The hash tree to operate on.
 *  @param pstrKey [in] The key.
 *  @param sKey [in] The length of the key.
 *
 *  @return The error code.
 */
u32 jf_hashtree_deleteEntry(
    jf_hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Return an Enumerator for a hash tree.
 *
 *  @param pHashtree [in] The hash tree to get an enumerator for.
 *  @param pEnumerator [in/out] The enumerator.
 *
 *  @return Void.
 */
static inline void jf_hashtree_initEnumerator(
    jf_hashtree_t * pHashtree, jf_hashtree_enumerator_t * pEnumerator)
{
    /*The enumerator is basically a state machine that keeps track of which node we are at in the
      tree. So initialize it to the root.*/
    pEnumerator->jhe_pjhnNode = pHashtree->jh_pjhnRoot;
}

/** Free resources associated with an Enumerator created by jf_hashtree_initEnumerator().
 *
 *  @param pEnumerator [in] The enumerator to free.
 *
 *  @return Void.
 */
static inline void jf_hashtree_finiEnumerator(jf_hashtree_enumerator_t * pEnumerator)
{
    ol_memset(pEnumerator, 0, sizeof(jf_hashtree_enumerator_t));
}

/** Check if it's the end of the enumerator.
 *
 *  @param pEnumerator [in] The enumerator to check.
 *
 *  @return The status.
 *  @retval TRUE The enumerator reach the end.
 *  @retval FALSE The enumerator is not the end.
 */
static inline boolean_t jf_hashtree_isEndOfEnumerator(jf_hashtree_enumerator_t * pEnumerator)
{
    return ((pEnumerator->jhe_pjhnNode == NULL) ? TRUE : FALSE);
}

/** Advance an enumerator to the next item.
 
 *  @param pEnumerator [in] The enumerator to advance.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_END_OF_HASHTREE End of hash tree.
 */
static inline u32 jf_hashtree_moveEnumerator(jf_hashtree_enumerator_t * pEnumerator)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pEnumerator->jhe_pjhnNode != NULL)
    {
        /*Advance the enumerator to point to the next node. If there is a node return 0,
          else return 1.*/
        pEnumerator->jhe_pjhnNode = pEnumerator->jhe_pjhnNode->jhn_pjhnNext;
    }
    else
    {
        /*There are no more nodes.*/
        u32Ret = JF_ERR_END_OF_HASHTREE;
    }

    return u32Ret;
}

/** Get data from current node of an enumerator.
 *
 *  @param pEnumerator [in] The enumerator to read from.
 *  @param ppstrKey [out] The key of the current item. 
 *  @param psKey [out] The length of the key of the current item. 
 *  @param ppData [out] The data of the current item.
 *
 *  @return Void.
 */
static inline void jf_hashtree_getEnumeratorNodeData(
    jf_hashtree_enumerator_t * pEnumerator, olchar_t ** ppstrKey, olsize_t * psKey, void ** ppData)
{
    /*All we do, is just assign the pointers.*/
    if (ppstrKey != NULL)
    {
        *ppstrKey = pEnumerator->jhe_pjhnNode->jhn_pstrKey;
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

