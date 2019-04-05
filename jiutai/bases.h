/**
 *  @file bases.h
 *
 *  @brief Contain the base data structure: basic stack, basic queue,
 *   link list, double link list list head, hash list head,
 *
 *  @author Min Zhang
 *
 *  @note Link with xmalloc object file
 *  @note The simple hash tree, suitable for items less than 50. For large
 *   amount of items, uses hash table in hash.h
 *  @note All base data structures here are not thread safe
 *  
 */

#ifndef JIUTAI_BASES_H
#define JIUTAI_BASES_H

/* --- standard C lib header files ----------------------------------------- */
#include <stddef.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/*basic stack*/
typedef struct jf_stack_node
{
    void * jsn_pData;
    struct jf_stack_node * jsn_pjsnNext;
} jf_stack_node_t;

typedef void  jf_stack_t;

/*basic queue*/
typedef struct jf_queue_node
{
    void * jqn_pData;
    struct jf_queue_node * jqn_pjqnNext;
} jf_queue_node_t;

typedef struct jf_queue
{
    jf_queue_node_t * jq_pjqnHead;
    jf_queue_node_t * jq_pjqnTail;
} jf_queue_t;

typedef u32 (* jf_queue_fnFreeData_t)(void ** ppData);

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

/*list head*/
typedef struct jf_listhead
{
    struct jf_listhead * jl_pjlNext;
    struct jf_listhead * jl_pjlPrev;
} jf_listhead_t;

/**
 *  Double linked lists with a single pointer list head.
 *  Mostly useful for hash tables where the two pointer list head is too
 *  wasteful.
 *  You lose the ability to access the tail in O(1).
 */
typedef struct jf_hlisthead_node
{
    struct jf_hlisthead_node * jhn_pjhnNext;
    struct jf_hlisthead_node ** jhn_ppjhnPrev;
} jf_hlisthead_node_t;

typedef struct jf_hlisthead
{
    jf_hlisthead_node_t * jh_pjhnFirst;
} jf_hlisthead_t;

/**
 *  Linked list using arrays of node
 */
typedef struct jf_listarray
{
    u32 jl_u32NumOfNode;
    u32 jl_u32Head;
} jf_listarray_t;

#define JF_LISTARRAY_NODE(pla) \
    ((u32 *)(((jf_listarray_t *)pla) + 1))

#define JF_LISTARRAY_END   U32_MAX

/* --- functional routines ------------------------------------------------- */

/**
 *  Basic Stack
 */

/** Init an empty Stack
 *
 *  @param ppStack [in/out] the stack to be initialize
 *
 *  @return void
 *
 *  @note This module uses a void* that is preinitialized to NULL, eg:
 *   void *stack = NULL;
 *   initStack(&stack);
 */
void jf_stack_init(jf_stack_t ** ppStack);

/** Pushe an item onto the stack
 *
 *  @param ppStack [in/out] The stack to push to 
 *  @param pData [in] The data to push onto the stack 
 *
 *  @return the error code
 */
u32 jf_stack_push(jf_stack_t ** ppStack, void * pData);

/** Pop an item from the stack
 *
 *  @param ppStack [in/out] The stack to pop from 
 *
 *  @return the item that was popped from the stack   
 *
 *  @note after peek, the item is removed from stack
 */
void * jf_stack_pop(jf_stack_t ** ppStack);

/** Peeks at the item on the top of the stack
 *
 *  @param ppStack [in/out] The stack to peek from 
 *
 *  @return the item that is currently on the top of the stack   
 *
 *  @note after peek, the item is still in stack
 */
void * jf_stack_peek(jf_stack_t ** ppStack);

/** Clears all the items from the stack
 *
 *  @param ppStack [in/out] The stack to clear 
 *
 *  @return void
 */
void jf_stack_clear(jf_stack_t ** ppStack);

/**
 *  Basic Queue
 */

/** Initialize an empty Queue
 *
 *  @param pQueue [in] the basic queue to be initialized
 *
 *  @return void
 */
void jf_queue_init(jf_queue_t * pQueue);

/** Free the resources associated with a queue
 *
 *  @param pQueue [in] The queue to finalize
 *  
 *  @return void
 */
void jf_queue_fini(jf_queue_t * pQueue);

/** Fini the queue and data
 * 
 */
void jf_queue_finiQueueAndData(
    jf_queue_t * pQueue, jf_queue_fnFreeData_t fnFreeData);

/** Check to see if a queue is empty
 *
 *  @param pQueue [in] The queue to check 
 *
 *  @return the queue empty state
 *  @retval TRUE the queue is empty
 *  @retval FALSE the queue is not empty
 */
boolean_t jf_queue_isEmpty(jf_queue_t * pQueue);

/** Add an item to the queue
 *
 *  @param pQueue [in] The queue to add  
 *  @param data [in] The data to add to the queue
 *
 *  @return the error code
 */
u32 jf_queue_enqueue(jf_queue_t * pQueue, void * data);

/** Remove an item from the queue
 *
 *  @param pQueue [in] The queue to remove an item from 
 *
 *  @return the queue entry
 */
void * jf_queue_dequeue(jf_queue_t * pQueue);

/** Peek an item from the queue
 *
 *  @param pQueue [in] The queue to peek an item from
 *
 *  @return the queue entry
 */
void * jf_queue_peek(jf_queue_t * pQueue);

/**
 *  linked list
 */
void jf_linklist_init(jf_linklist_t * pList);

void jf_linklist_fini(jf_linklist_t * pList);

void jf_linklist_finiListAndData(
    jf_linklist_t * pList, jf_linklist_fnFreeNodeData_t fnFreeData);

/**
 *  Append to the tail of the linked list
 */
u32 jf_linklist_appendTo(jf_linklist_t * pList, void * pData);

/**
 *  Intert to the head of the linked list
 */
u32 jf_linklist_insertTo(jf_linklist_t * pList, void * pData);

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

/**
 *  double linked list
 */
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

/**
 *  Hashtree Methods
 */

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

/**
 *  list head
 */
#define JF_LISTHEAD_INIT(name) { &(name), &(name) }

#define JF_LISTHEAD(name) \
    jf_listhead_t name = JF_LISTHEAD_INIT(name)

static inline void jf_listhead_init(jf_listhead_t * list)
{
    list->jl_pjlNext = list;
    list->jl_pjlPrev = list;
}

/** Insert a new entry between two known consecutive entries.
 *  This is only for internal list manipulation where we know
 *  the prev/next entries already!
 */
static inline void _listAdd(
    jf_listhead_t * new, jf_listhead_t * prev, jf_listhead_t * next)
{
    next->jl_pjlPrev = new;
    new->jl_pjlNext = next;
    new->jl_pjlPrev = prev;
    prev->jl_pjlNext = new;
}

/** Add a new entry, insert a new entry after the specified head.
 *
 *  @param head [in] list head to add it after
 *  @param new [in] new entry to be added
 *
 */
static inline void jf_listhead_add(jf_listhead_t * head, jf_listhead_t * new)
{
    _listAdd(new, head, head->jl_pjlNext);
}

/** Add a new entry, insert a new entry before the specified head.
 *
 *  @param head [in] list head to add it before
 *  @param new [in] new entry to be added
 * 
 */
static inline void jf_listhead_addTail(jf_listhead_t * head, jf_listhead_t * new)
{
    _listAdd(new, head->jl_pjlPrev, head);
}

/** Delete a list entry by making the prev/next entries point to each other.
 *
 *  @param prev [in] list head to delete it before
 *  @param next [in] list head to delete it after
 *
 *  @note This is only for internal list manipulation where we know the
 *   prev/next entries already!
 */
static inline void _listDel(jf_listhead_t * prev, jf_listhead_t * next)
{
    next->jl_pjlPrev = prev;
    prev->jl_pjlNext = next;
}

/** Deletes entry from list.
 *
 *  @param entry [in] the element to delete from the list.
 */
static inline void jf_listhead_del(jf_listhead_t * entry)
{
    _listDel(entry->jl_pjlPrev, entry->jl_pjlNext);
    entry->jl_pjlNext = NULL;
    entry->jl_pjlPrev = NULL;
}

/** Replace old entry by new one. if 'old' is empty, it will be overwritten.
 *
 *  @param old [in] the element to be replaced
 *  @param new [in] the new element to insert
 *
 */
static inline void jf_listhead_replace(jf_listhead_t * old, jf_listhead_t * new)
{
    new->jl_pjlNext = old->jl_pjlNext;
    new->jl_pjlNext->jl_pjlPrev = new;
    new->jl_pjlPrev = old->jl_pjlPrev;
    new->jl_pjlPrev->jl_pjlNext = new;
}

static inline void jf_listhead_replaceInit(jf_listhead_t * old, jf_listhead_t * new)
{
    jf_listhead_replace(old, new);
    jf_listhead_init(old);
}

/** Deletes entry from list and reinitialize it.
 *
 *  @param entry [in] the element to delete from the list.
 */
static inline void jf_listhead_delInit(jf_listhead_t * entry)
{
    _listDel(entry->jl_pjlPrev, entry->jl_pjlNext);
    jf_listhead_init(entry);
}

/** Delete from one list and add as another's head
 *
 *  @param head [in] the entry to move
 *  @param list [in] the head that will precede our entry
 */
static inline void jf_listhead_move(jf_listhead_t * head, jf_listhead_t * list)
{
    _listDel(list->jl_pjlPrev, list->jl_pjlNext);
    jf_listhead_add(head, list);
}

/** Delete from one list and add as another's tail
 *
 *  @param head [in] the head that will follow our entry
 *  @param list [in] the entry to move
 */
static inline void jf_listhead_moveTail(jf_listhead_t * head, jf_listhead_t * list)
{
    _listDel(list->jl_pjlPrev, list->jl_pjlNext);
    jf_listhead_addTail(head, list);
}

/** Tests whether 'list' is the last entry in list 'head'
 *
 *  @param head [in] the head of the list
 *  @param list [in] the entry to test
 */
static inline boolean_t jf_listhead_isLast(
    const jf_listhead_t * head, const jf_listhead_t * list)
{
    return list->jl_pjlNext == head;
}

/** Tests whether 'list' is the first entry in list 'head'
 *
 *  @param head [in] the head of the list
 *  @param list [in] the entry to test
 */
static inline boolean_t jf_listhead_isFirst(
    const jf_listhead_t * head, const jf_listhead_t * list)
{
    return list->jl_pjlPrev == head;
}

/** Test whether a list is empty
 * 
 *  @param head [in] the list to test.
 */
static inline boolean_t jf_listhead_isEmpty(const jf_listhead_t * head)
{
    return head->jl_pjlNext == head;
}

static inline void _listSplice(jf_listhead_t * head, jf_listhead_t * list)
{
    jf_listhead_t * first = list->jl_pjlNext;
    jf_listhead_t * last = list->jl_pjlPrev;
    jf_listhead_t * at = head->jl_pjlNext;

    first->jl_pjlPrev = head;
    head->jl_pjlNext = first;

    last->jl_pjlNext = at;
    at->jl_pjlPrev = last;
}

/** Join two lists
 * 
 *  @param head the place to add it in the first list.
 *  @param list the new list to add.
 */
static inline void jf_listhead_splice(
    jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
        _listSplice(head, list);
}

/** Join two lists and reinitialise the emptied list.
 * 
 *  @param head the place to add it in the first list.
 *  @param list the new list to add.
 *
 *  @note the list at 'list' is reinitialised
 */
static inline void jf_listhead_spliceInit(
    jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
    {
        _listSplice(head, list);
        jf_listhead_init(list);
    }
}

/** Split the 'head' at the position 'list' and add the removed part
 *  (exclude 'list') to 'newhead'
 */
static inline void jf_listhead_split(
    jf_listhead_t * head, jf_listhead_t * list, jf_listhead_t * newhead)
{
    jf_listhead_t * first = head->jl_pjlNext;
    jf_listhead_t * last = list->jl_pjlPrev;

    head->jl_pjlNext = list;
    list->jl_pjlPrev = head;

    newhead->jl_pjlNext = first;
    first->jl_pjlPrev = newhead;

    newhead->jl_pjlPrev = last;
    last->jl_pjlNext = newhead;
}

#if defined(LINUX)
    #ifndef offsetof
        #define offsetof(TYPE, MEMBER) ((olsize_t) &((TYPE *)0)->MEMBER)
    #endif
    #define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)((olchar_t *)__mptr - offsetof(type, member));})
#elif defined(WINDOWS)
    typedef long LONG;

    #if defined(WIN64)
        typedef s64 LONG_PTR; 
    #else
        typedef long LONG_PTR;
    #endif

    typedef olchar_t CHAR;
    typedef CHAR *PCHAR;

    #if defined(WIN64)
        typedef u64 ULONG_PTR;
    #else
        typedef unsigned long ULONG_PTR;
    #endif

//  #define offsetof(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))

    #define container_of(address, type, field) ((type *)( \
       (PCHAR)(address) - offsetof(type, field)))

#endif

/** Get the struct for this entry
 *
 *  @param ptr the jf_listhead_t pointer.
 *  @param type the type of the struct this is embedded in.
 *  @param member the name of the list_struct within the struct.
 */
#define jf_listhead_getEntry(ptr, type, member) \
    container_of(ptr, type, member)

/** Iterate over a list
 * 
 *  @param head the head for your list.
 *  @param pos the &jf_listhead_t to use as a loop cursor.
 *
 */
#define jf_listhead_forEach(head, pos) \
    for (pos = (head)->jl_pjlNext; pos != (head); pos = pos->jl_pjlNext)

/** Iterate over a list safe against removal of list entry
 * 
 *  @param head the head for your list.
 *  @param pos the jf_listhead_t to use as a loop cursor.
 *  @param n another jf_listhead_t to use as temporary storage
 */
#define jf_listhead_forEachSafe(head, pos, n) \
    for (pos = (head)->jl_pjlNext, n = pos->jl_pjlNext; pos != (head); \
         pos = n, n = pos->jl_pjlNext)

/** Iterate over a list backwards
 * 
 *  @param head the head for your list.
 *  @param pos the &jf_listhead_t to use as a loop cursor.
 */
#define jf_listhead_forEachPrev(head, pos) \
    for (pos = (head)->jl_pjlPrev; pos != (head); pos = pos->jl_pjlPrev)

/** Iterate over a list backwards safe against removal of list entry
 *
 *  @param head the head for your list.
 *  @param pos the &jf_listhead_t to use as a loop cursor.
 *  @param n another &struct list_head to use as temporary storage
 */
#define jf_listhead_forEachPrevSafe(head, pos, n)                      \
    for (pos = (head)->jl_pjlPrev, n = pos->jl_pjlPrev; pos != (head); \
         pos = n, n = pos->jl_pjlPrev)

/**
 *  hlist
 */
#define JF_HLISTHEAD(name) jf_hlisthead_t name = {  .jh_pjhnFirst = NULL }
#define JF_HLISTHEAD_INIT(ptr) ((ptr)->jh_pjhnFirst = NULL)
#define JF_HLISTHEAD_INIT_NODE(ptr) \
    ((ptr)->jhn_pjhnNext = NULL, (ptr)->jhn_ppjhnPrev = NULL)

#define jf_hlisthead_getEntry(ptr, type, member) container_of(ptr, type, member)

#define jf_hlisthead_forEach(head, pos) \
    for (pos = (head)->jh_pjhnFirst; pos; pos = pos->jhn_pjhnNext)

/** Iterate over list of given type
 *
 *  @param tpos [in] the type * to use as a loop counter.
 *  @param pos [in] the jf_hlisthead_node_t to use as a loop counter.
 *  @param head [in] the head for your list.
 *  @param member [in] the name of the jf_hlisthead_node_t within the struct.
 */
#define jf_hlisthead_forEachEntry(tpos, pos, head, member)            \
    for (pos = (head)->jh_pjhnFirst;                    \
         pos &&          \
         tpos = hlistEntry(pos, typeof(*tpos), member); \
         pos = pos->jhn_pjhnNext)

static inline void _hlistDel(jf_hlisthead_node_t * n)
{
    struct jf_hlisthead_node *next = n->jhn_pjhnNext;
    struct jf_hlisthead_node **pprev = n->jhn_ppjhnPrev;

    *pprev = next;
    if (next)
        next->jhn_ppjhnPrev = pprev;
}

static inline void jf_hlisthead_del(jf_hlisthead_node_t * n)
{
    _hlistDel(n);
    n->jhn_pjhnNext = NULL;
    n->jhn_ppjhnPrev = NULL;
}

static inline void jf_hlisthead_delInit(jf_hlisthead_node_t * n)
{
    if (n->jhn_ppjhnPrev)
    {
        _hlistDel(n);
        JF_HLISTHEAD_INIT_NODE(n);
    }
}

static inline void jf_hlisthead_addHead(jf_hlisthead_t * h, jf_hlisthead_node_t * n)
{
    jf_hlisthead_node_t * first = h->jh_pjhnFirst;
    n->jhn_pjhnNext = first;
    if (first)
        first->jhn_ppjhnPrev = &n->jhn_pjhnNext;
    h->jh_pjhnFirst = n;
    n->jhn_ppjhnPrev = &h->jh_pjhnFirst;
}

/**
 *  list array
 */
static inline olsize_t jf_listarray_getSize(u32 u32NumOfNode)
{
    assert(u32NumOfNode > 0);

    return u32NumOfNode * BYTES_PER_U32 + sizeof(jf_listarray_t);
}

static inline void jf_listarray_init(jf_listarray_t * pla, u32 u32NumOfNode)
{
    u32 u32Index;

    ol_memset(pla, 0, jf_listarray_getSize(u32NumOfNode));

    pla->jl_u32NumOfNode = u32NumOfNode;
    pla->jl_u32Head = 0;

    for (u32Index = 0; u32Index < u32NumOfNode; u32Index ++)
    {
        JF_LISTARRAY_NODE(pla)[u32Index] = u32Index + 1;
    }
    JF_LISTARRAY_NODE(pla)[u32Index - 1] = JF_LISTARRAY_END;
}

static inline u32 jf_listarray_getNode(jf_listarray_t * pla)
{
    u32 u32Node = pla->jl_u32Head;

    if (u32Node == JF_LISTARRAY_END)
        return u32Node;

    pla->jl_u32Head = JF_LISTARRAY_NODE(pla)[pla->jl_u32Head];

    return u32Node;
}

static inline void jf_listarray_putNode(jf_listarray_t * pla, u32 u32Node)
{
    assert(u32Node != JF_LISTARRAY_END);

    JF_LISTARRAY_NODE(pla)[u32Node] = pla->jl_u32Head;

    pla->jl_u32Head = u32Node;
}

static inline boolean_t jf_listarray_isEnd(jf_listarray_t * pla)
{
    return (pla->jl_u32Head == JF_LISTARRAY_END);
}

#endif /*JIUTAI_BASES_H*/

/*-----------------------------------------------------------------*/

