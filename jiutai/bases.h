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
typedef struct stack_node
{
    void * sn_pData;
    struct stack_node * sn_psnNext;
} stack_node_t;

typedef void  basic_stack_t;

/*basic queue*/
typedef struct queue_node
{
    void * qn_pData;
    struct queue_node * qn_pqnNext;
} queue_node_t;

typedef struct basic_queue
{
    queue_node_t * bq_pqnHead;
    queue_node_t * bq_pqnTail;
} basic_queue_t;

/*linked list*/
typedef struct link_list_node
{
    struct link_list_node * lln_pllnNext;
    void * lln_pData;
} link_list_node_t;

typedef struct link_list
{
	link_list_node_t * ll_pllnHead;
} link_list_t;

/*double linked list*/
typedef struct dlink_list_node
{
    struct dlink_list_node * dln_pdlnNext;
    struct dlink_list_node * dln_pdlnPrev;
    void * dln_pData;
} dlink_list_node_t;

typedef struct dlink_list
{
    dlink_list_node_t * dl_pdlnHead;
    dlink_list_node_t * dl_pdlnTail;
} dlink_list_t;

typedef u32 (* fnFreeListNodeData_t)(void ** ppData);

typedef boolean_t (* fnFindListNodeData_t)(void * pData, void * pKey);

/*hash tree*/
typedef struct hash_node
{
    struct hash_node * hn_phnNext;
    struct hash_node * hn_phnPrev;
    olint_t hn_nKey;
    olchar_t * hn_pstrKeyValue;
    olsize_t hn_sKey;
    void * hn_pData;
} hash_node_t;

typedef struct hashtree_enumerator
{
    hash_node_t * he_phnNode;
} hashtree_enumerator_t;

typedef struct hashtree
{
    hash_node_t * h_phnRoot;
} hashtree_t;

/*list head*/
typedef u32 (* fnFreeHashtreeData_t)(void ** ppData);

typedef u32 (* fnFreeQueueData_t)(void ** ppData);

typedef struct list_head
{
    struct list_head * lh_plhNext, * lh_plhPrev;
} list_head_t;

/**
 *  Double linked lists with a single pointer list head.
 *  Mostly useful for hash tables where the two pointer list head is too
 *  wasteful.
 *  You lose the ability to access the tail in O(1).
 */
typedef struct hlist_node
{
    struct hlist_node * hn_phnNext, ** hn_pphnPrev;
} hlist_node_t;

typedef struct hlist_head
{
    hlist_node_t * hh_phnFirst;
} hlist_head_t;

/**
 *  Linked list using arrays of node
 */
typedef struct list_array
{
    u32 la_u32NumOfNode;
    u32 la_u32Head;
} list_array_t;

#define LIST_ARRAY_NODE(pla) \
    ((u32 *)(((list_array_t *)pla) + 1))

#define LIST_ARRAY_END   U32_MAX

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
void initStack(basic_stack_t ** ppStack);

/** Pushe an item onto the stack
 *
 *  @param ppStack [in/out] The stack to push to 
 *  @param pData [in] The data to push onto the stack 
 *
 *  @return the error code
 */
u32 pushStack(basic_stack_t ** ppStack, void * pData);

/** Pop an item from the stack
 *
 *  @param ppStack [in/out] The stack to pop from 
 *
 *  @return the item that was popped from the stack   
 *
 *  @note after peek, the item is removed from stack
 */
void * popStack(basic_stack_t ** ppStack);

/** Peeks at the item on the top of the stack
 *
 *  @param ppStack [in/out] The stack to peek from 
 *
 *  @return the item that is currently on the top of the stack   
 *
 *  @note after peek, the item is still in stack
 */
void * peekStack(basic_stack_t ** ppStack);

/** Clears all the items from the stack
 *
 *  @param ppStack [in/out] The stack to clear 
 *
 *  @return void
 */
void clearStack(basic_stack_t ** ppStack);

/**
 *  Basic Queue
 */

/** Initialize an empty Queue
 *
 *  @param pQueue [in] the basic queue to be initialized
 *
 *  @return void
 */
void initQueue(basic_queue_t * pQueue);

/** Free the resources associated with a queue
 *
 *  @param pQueue [in] The queue to finalize
 *  
 *  @return void
 */
void finiQueue(basic_queue_t * pQueue);

/** Fini the queue and data
 * 
 */
void finiQueueAndData(basic_queue_t * pQueue, fnFreeQueueData_t fnFreeData);

/** Check to see if a queue is empty
 *
 *  @param pQueue [in] The queue to check 
 *
 *  @return the queue empty state
 *  @retval TRUE the queue is empty
 *  @retval FALSE the queue is not empty
 */
boolean_t isQueueEmpty(basic_queue_t * pQueue);

/** Add an item to the queue
 *
 *  @param pQueue [in] The queue to add  
 *  @param data [in] The data to add to the queue
 *
 *  @return the error code
 */
u32 enqueue(basic_queue_t * pQueue, void * data);

/** Remove an item from the queue
 *
 *  @param pQueue [in] The queue to remove an item from 
 *
 *  @return the queue entry
 */
void * dequeue(basic_queue_t * pQueue);

/** Peek an item from the queue
 *
 *  @param pQueue [in] The queue to peek an item from
 *
 *  @return the queue entry
 */
void * peekQueue(basic_queue_t * pQueue);

/**
 *  linked list
 */
void initLinkList(link_list_t * pList);

void finiLinkList(link_list_t * pList);

void finiLinkListAndData(link_list_t * pList,
    fnFreeListNodeData_t fnFreeData);

/**
 *  Append to the tail of the linked list
 */
u32 appendToLinkList(link_list_t * pList, void * pData);

/**
 *  Intert to the head of the linked list
 */
u32 insertToLinkList(link_list_t * pList, void * pData);

static inline boolean_t isLinkListEmpty(link_list_t * pList)
{
    if (pList->ll_pllnHead == NULL)
        return TRUE;
    return FALSE;
}

/**
 *  Get the first node of linked list
 */
static inline link_list_node_t * getFirstNodeOfLinkList(link_list_t * pList)
{
	return pList->ll_pllnHead;
}

/**
 *  Get the next node of the specified node
 */
static inline link_list_node_t * getNextNodeOfLinkList(link_list_node_t * pNode)
{
	return pNode->lln_pllnNext;
}

/**
 *  Get data from the linked node
 */
static inline void * getDataFromLinkListNode(link_list_node_t * pNode)
{
	return pNode->lln_pData;
}

/**
 *  double linked list
 */
void initDlinkList(dlink_list_t * pList);

void finiDlinkList(dlink_list_t * pList);

void finiDlinkListAndData(dlink_list_t * pList,
    fnFreeListNodeData_t fnFreeData);

void removeAllNodesFromDlinkList(dlink_list_t * pList,
    fnFreeListNodeData_t fnFreeData);

u32 findFirstDataFromDlinkList(dlink_list_t * pList, void ** ppData,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 findLastDataFromDlinkList(dlink_list_t * pList, void ** ppData,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 findFirstNodeFromDlinkList(dlink_list_t * pList,
    dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 findLastNodeFromDlinkList(dlink_list_t * pList, dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 findNextNodeFromDlinkList(dlink_list_node_t * pNode,
    dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 findPrevNodeFromDlinkList(dlink_list_node_t * pNode,
    dlink_list_node_t ** ppNode,
    fnFindListNodeData_t fnFindData, void * pKey);

u32 appendToDlinkList(dlink_list_t * pList, void * pData);


/**
 *  Get data from the linked node
 */
static inline void * getDataFromDlinkListNode(dlink_list_node_t * pNode)
{
    return pNode->dln_pData;
}

/**
 *  Get the first node of double linked list
 */
static inline dlink_list_node_t * getFirstNodeOfDlinkList(dlink_list_t * pList)
{
    return pList->dl_pdlnHead;
}

/**
 *  Get the last node of the double linked list
 */
static inline dlink_list_node_t * getLastNodeOfDlinkList(dlink_list_t * pList)
{
    return pList->dl_pdlnTail;
}

/**
 *  Get the next node of the specified node
 */
static inline dlink_list_node_t * getNextNodeOfDlinkList(
    dlink_list_node_t * pNode)
{
    return pNode->dln_pdlnNext;
}

/**
 *  Get the previous node of the specified node
 */
static inline dlink_list_node_t * getPrevNodeOfDlinkList(
    dlink_list_node_t * pNode)
{
    return pNode->dln_pdlnPrev;
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
static inline void initHashtree(hashtree_t * pHashtree)
{
    pHashtree->h_phnRoot = NULL;
}

/** Free resources associated with a hashtree
 *
 *  @param pHashtree [in] the hashtree to free
 *
 *  @return void
 */
void finiHashtree(hashtree_t * pHashtree);

/** Free resources associated with a hashtree
 *
 *  @param pHashtree [in] the hashtree to free
 *  @param fnFreeData [in] the function to free data
 *
 *  @return void
 */
void finiHashtreeAndData(
    hashtree_t * pHashtree, fnFreeHashtreeData_t fnFreeData);

/** Determine if the hash tree is empty
 *
 *  @param pHashtree [in] the hashtree to free
 *
 *  @return the empty is of the hashtree
 *  @retval TRUE the hashtree is empty
 *  @retval FALSE the hash tree is not empty
 */
static inline boolean_t isHashtreeEmpty(hashtree_t * pHashtree)
{
    return ((pHashtree->h_phnRoot == NULL) ? TRUE : FALSE);
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
boolean_t hasHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

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
u32 addHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * pValue);

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
u32 getHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData);

/** Deletes a keyed item from the hashtree
 *
 *  @param pHashtree [in] the hashtree to operate on 
 *  @param pstrKey [in] the key 
 *  @param sKey [in] the length of the key 
 *
 *  @return the error code
 */
u32 deleteHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Return an Enumerator for a hash tree
 *
 *  @param pHashtree [in] the hashtree to get an enumerator for 
 *  @param pEnumerator [in/out] the enumerator
 *
 *  @return void
 */
static inline void initHashtreeEnumerator(
    hashtree_t * pHashtree, hashtree_enumerator_t * pEnumerator)
{
    /*the enumerator is basically a state machine that keeps track of 
      which node we are at in the tree. So initialize it to the root.*/
    pEnumerator->he_phnNode = pHashtree->h_phnRoot;
}

/** Free resources associated with an Enumerator created by 
 *  initHashtreeEnumerator
 *
 *  @param pEnumerator [in] the enumerator to free 
 *
 *  @return void
 */
static inline void finiHashtreeEnumerator(hashtree_enumerator_t * pEnumerator)
{
    memset(pEnumerator, 0, sizeof(hashtree_enumerator_t));
}

static inline boolean_t isHashtreeEnumeratorEmptyNode(
    hashtree_enumerator_t * pEnumerator)
{
    return ((pEnumerator->he_phnNode == NULL) ? TRUE : FALSE);
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
static inline u32 moveHashtreeNext(hashtree_enumerator_t * pEnumerator)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pEnumerator->he_phnNode != NULL)
    {
        /*Advance the enumerator to point to the next node. If there is a node
          return 0, else return 1*/
        pEnumerator->he_phnNode = pEnumerator->he_phnNode->hn_phnNext;
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
static inline void getHashtreeValue(
    hashtree_enumerator_t * pEnumerator,
    olchar_t ** ppstrKey, olsize_t * psKey, void ** ppData)
{
    /*All we do, is just assign the pointers.*/
    if (ppstrKey != NULL)
    {
        *ppstrKey = pEnumerator->he_phnNode->hn_pstrKeyValue;
    }
    if (psKey != NULL)
    {
        *psKey = pEnumerator->he_phnNode->hn_sKey;
    }
    if (ppData != NULL)
    {
        *ppData = pEnumerator->he_phnNode->hn_pData;
    }
}

/**
 *  list head
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    list_head_t name = LIST_HEAD_INIT(name)

static inline void listInit(list_head_t * list)
{
    list->lh_plhNext = list;
    list->lh_plhPrev = list;
}

/** Insert a new entry between two known consecutive entries.
 *  This is only for internal list manipulation where we know
 *  the prev/next entries already!
 */
static inline void _listAdd(list_head_t * new, list_head_t * prev,
    list_head_t * next)
{
    next->lh_plhPrev = new;
    new->lh_plhNext = next;
    new->lh_plhPrev = prev;
    prev->lh_plhNext = new;
}

/** Add a new entry, insert a new entry after the specified head.
 *
 *  @param head [in] list head to add it after
 *  @param new [in] new entry to be added
 *
 */
static inline void listAdd(list_head_t * head, list_head_t * new)
{
    _listAdd(new, head, head->lh_plhNext);
}

/** Add a new entry, insert a new entry before the specified head.
 *
 *  @param head [in] list head to add it before
 *  @param new [in] new entry to be added
 * 
 */
static inline void listAddTail(list_head_t * head, list_head_t * new)
{
    _listAdd(new, head->lh_plhPrev, head);
}

/** Delete a list entry by making the prev/next entries point to each other.
 *
 *  @param prev [in] list head to delete it before
 *  @param next [in] list head to delete it after
 *
 *  @note This is only for internal list manipulation where we know the
 *   prev/next entries already!
 */
static inline void _listDel(list_head_t * prev, list_head_t * next)
{
    next->lh_plhPrev = prev;
    prev->lh_plhNext = next;
}

/** Deletes entry from list.
 *
 *  @param entry [in] the element to delete from the list.
 */
static inline void listDel(list_head_t * entry)
{
    _listDel(entry->lh_plhPrev, entry->lh_plhNext);
    entry->lh_plhNext = NULL;
    entry->lh_plhPrev = NULL;
}

/** Replace old entry by new one. if 'old' is empty, it will be overwritten.
 *
 *  @param old [in] the element to be replaced
 *  @param new [in] the new element to insert
 *
 */
static inline void listReplace(list_head_t * old, list_head_t * new)
{
    new->lh_plhNext = old->lh_plhNext;
    new->lh_plhNext->lh_plhPrev = new;
    new->lh_plhPrev = old->lh_plhPrev;
    new->lh_plhPrev->lh_plhNext = new;
}

static inline void listReplaceInit(list_head_t * old, list_head_t * new)
{
    listReplace(old, new);
    listInit(old);
}

/** Deletes entry from list and reinitialize it.
 *
 *  @param entry [in] the element to delete from the list.
 */
static inline void listDelInit(list_head_t * entry)
{
    _listDel(entry->lh_plhPrev, entry->lh_plhNext);
    listInit(entry);
}

/** Delete from one list and add as another's head
 *
 *  @param head [in] the entry to move
 *  @param list [in] the head that will precede our entry
 */
static inline void listMove(list_head_t * head, list_head_t * list)
{
    _listDel(list->lh_plhPrev, list->lh_plhNext);
    listAdd(head, list);
}

/** Delete from one list and add as another's tail
 *
 *  @param head [in] the head that will follow our entry
 *  @param list [in] the entry to move
 */
static inline void listMoveTail(list_head_t * head, list_head_t * list)
{
    _listDel(list->lh_plhPrev, list->lh_plhNext);
    listAddTail(head, list);
}

/** Tests whether 'list' is the last entry in list 'head'
 *
 *  @param head [in] the head of the list
 *  @param list [in] the entry to test
 */
static inline boolean_t listIsLast(
    const list_head_t * head, const list_head_t * list)
{
    return list->lh_plhNext == head;
}

/** Tests whether 'list' is the first entry in list 'head'
 *
 *  @param head [in] the head of the list
 *  @param list [in] the entry to test
 */
static inline boolean_t listIsFirst(
    const list_head_t * head, const list_head_t * list)
{
    return list->lh_plhPrev == head;
}

/** Test whether a list is empty
 * 
 *  @param head [in] the list to test.
 */
static inline boolean_t listIsEmpty(const list_head_t * head)
{
    return head->lh_plhNext == head;
}

static inline void _listSplice(list_head_t * head, list_head_t * list)
{
    list_head_t * first = list->lh_plhNext;
    list_head_t * last = list->lh_plhPrev;
    list_head_t * at = head->lh_plhNext;

    first->lh_plhPrev = head;
    head->lh_plhNext = first;

    last->lh_plhNext = at;
    at->lh_plhPrev = last;
}

/** Join two lists
 * 
 *  @param head the place to add it in the first list.
 *  @param list the new list to add.
 */
static inline void listSplice(list_head_t * head, list_head_t * list)
{
    if (! listIsEmpty(list))
        _listSplice(head, list);
}

/** Join two lists and reinitialise the emptied list.
 * 
 *  @param head the place to add it in the first list.
 *  @param list the new list to add.
 *
 *  @note the list at 'list' is reinitialised
 */
static inline void listSpliceInit(list_head_t * head, list_head_t * list)
{
    if (! listIsEmpty(list))
    {
        _listSplice(head, list);
        listInit(list);
    }
}

/** Split the 'head' at the position 'list' and add the removed part
 *  (exclude 'list') to 'newhead'
 */
static inline void listSplit(list_head_t * head, list_head_t * list,
    list_head_t * newhead)
{
    list_head_t * first = head->lh_plhNext;
    list_head_t * last = list->lh_plhPrev;

    head->lh_plhNext = list;
    list->lh_plhPrev = head;

    newhead->lh_plhNext = first;
    first->lh_plhPrev = newhead;

    newhead->lh_plhPrev = last;
    last->lh_plhNext = newhead;
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
 *  @param ptr the list_head_t pointer.
 *  @param type the type of the struct this is embedded in.
 *  @param member the name of the list_struct within the struct.
 */
#define listEntry(ptr, type, member) \
    container_of(ptr, type, member)

/** Iterate over a list
 * 
 *  @param head the head for your list.
 *  @param pos the &list_head_t to use as a loop cursor.
 *
 */
#define listForEach(head, pos) \
    for (pos = (head)->lh_plhNext; pos != (head); pos = pos->lh_plhNext)

/** Iterate over a list safe against removal of list entry
 * 
 *  @param head the head for your list.
 *  @param pos the list_head_t to use as a loop cursor.
 *  @param n another list_head_t to use as temporary storage
 */
#define listForEachSafe(head, pos, n) \
    for (pos = (head)->lh_plhNext, n = pos->lh_plhNext; pos != (head); \
         pos = n, n = pos->lh_plhNext)

/** Iterate over a list backwards
 * 
 *  @param head the head for your list.
 *  @param pos the &list_head_t to use as a loop cursor.
 */
#define listForEachPrev(head, pos) \
    for (pos = (head)->lh_plhPrev; pos != (head); pos = pos->lh_plhPrev)

/** Iterate over a list backwards safe against removal of list entry
 *
 *  @param head the head for your list.
 *  @param pos the &list_head_t to use as a loop cursor.
 *  @param n another &struct list_head to use as temporary storage
 */
#define listForEachPrevSafe(head, pos, n)                         \
    for (pos = (head)->lh_plhPrev, n = pos->lh_plhPrev; pos != (head); \
         pos = n, n = pos->lh_plhPrev)

/**
 *  hlist
 */
#define HLIST_HEAD_INIT { .hh_phnFirst = NULL }
#define HLIST_HEAD(name) hlist_head_t name = {  .hh_phnFirst = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->hh_phnFirst = NULL)
#define INIT_HLIST_NODE(ptr) \
    ((ptr)->hn_phnNext = NULL, (ptr)->hn_pphnPrev = NULL)

#define hlistEntry(ptr, type, member) container_of(ptr, type, member)

#define hlistForEach(head, pos) \
    for (pos = (head)->hh_phnFirst; pos; pos = pos->hn_phnNext)

/** Iterate over list of given type
 *
 *  @param tpos the type * to use as a loop counter.
 *  @param pos the hlist_node_t to use as a loop counter.
 *  @param head the head for your list.
 *  @param member the name of the hlist_node_t within the struct.
 */
#define hlistForEachEntry(tpos, pos, head, member)            \
    for (pos = (head)->hh_phnFirst;                    \
         pos &&          \
         tpos = hlistEntry(pos, typeof(*tpos), member); \
         pos = pos->hn_phnNext)

static inline void _hlistDel(hlist_node_t * n)
{
    struct hlist_node *next = n->hn_phnNext;
    struct hlist_node **pprev = n->hn_pphnPrev;

    *pprev = next;
    if (next)
        next->hn_pphnPrev = pprev;
}

static inline void hlistDel(hlist_node_t * n)
{
    _hlistDel(n);
    n->hn_phnNext = NULL;
    n->hn_pphnPrev = NULL;
}

static inline void hlistDelInit(hlist_node_t * n)
{
    if (n->hn_pphnPrev)
    {
        _hlistDel(n);
        INIT_HLIST_NODE(n);
    }
}

static inline void hlistAddHead(hlist_head_t * h, hlist_node_t * n)
{
    hlist_node_t * first = h->hh_phnFirst;
    n->hn_phnNext = first;
    if (first)
        first->hn_pphnPrev = &n->hn_phnNext;
    h->hh_phnFirst = n;
    n->hn_pphnPrev = &h->hh_phnFirst;
}

/**
 *  list array
 */
static inline olsize_t sizeOfListArray(u32 u32NumOfNode)
{
    assert(u32NumOfNode > 0);

    return u32NumOfNode * BYTES_PER_U32 + sizeof(list_array_t);
}

static inline void initListArray(list_array_t * pla, u32 u32NumOfNode)
{
    u32 u32Index;

    memset(pla, 0, sizeOfListArray(u32NumOfNode));

    pla->la_u32NumOfNode = u32NumOfNode;
    pla->la_u32Head = 0;

    for (u32Index = 0; u32Index < u32NumOfNode; u32Index ++)
    {
        LIST_ARRAY_NODE(pla)[u32Index] = u32Index + 1;
    }
    LIST_ARRAY_NODE(pla)[u32Index - 1] = LIST_ARRAY_END;
}

static inline u32 getListArrayNode(list_array_t * pla)
{
    u32 u32Node = pla->la_u32Head;

    if (u32Node == LIST_ARRAY_END)
        return u32Node;

    pla->la_u32Head = LIST_ARRAY_NODE(pla)[pla->la_u32Head];

    return u32Node;
}

static inline void putListArrayNode(list_array_t * pla, u32 u32Node)
{
    assert(u32Node != LIST_ARRAY_END);

    LIST_ARRAY_NODE(pla)[u32Node] = pla->la_u32Head;

    pla->la_u32Head = u32Node;
}

static inline boolean_t isEndOfListArray(list_array_t * pla)
{
    return (pla->la_u32Head == LIST_ARRAY_END);
}

#endif /*JIUTAI_BASES_H*/

/*-----------------------------------------------------------------*/

