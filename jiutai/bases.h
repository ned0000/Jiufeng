/**
 *  @file bases.h
 *
 *  @brief contain the base data structure
 *     basic stack, basic queue, link list, double link list list head,
 *     hash list head,
 *
 *  @author Min Zhang
 *
 *  @note
 *   - link with xmalloc object file
 *   - here is the simple hash tree, suitable for items less than 50
 *     for large amount of items, uses hash table in hash.h
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

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
typedef struct hlist_node
{
    struct hlist_node * hn_phnNext, ** hn_pphnPrev;
} hlist_node_t;

typedef struct hlist_head
{
    hlist_node_t * hh_phnFirst;
} hlist_head_t;

/*linked list using arrays of node*/
typedef struct list_array
{
    u32 la_u32NumOfNode;
    u32 la_u32Head;
} list_array_t;

#define LIST_ARRAY_NODE(pla) \
    ((u32 *)(((list_array_t *)pla) + 1))

#define LIST_ARRAY_END   U32_MAX

/* --- functional routines ------------------------------------------------- */

/*
 * Basic Stack
 */

/*init an empty Stack*/
void initStack(basic_stack_t ** ppStack);

/*Pushes an item onto the stack*/
u32 pushStack(basic_stack_t ** ppStack, void * pData);

/*Pops an item from the stack*/
void * popStack(basic_stack_t ** ppStack);

/*Peeks at the item on the top of the stack*/
void * peekStack(basic_stack_t ** ppStack);

/*Clears all the items from the stack*/
void clearStack(basic_stack_t ** ppStack);

/*
 * Basic Queue
 */

/*init an empty Queue*/
void initQueue(basic_queue_t * pQueue);

/*fini the queue*/
void finiQueue(basic_queue_t * pQueue);

/*fini the queue and data*/
void finiQueueAndData(basic_queue_t * pQueue, fnFreeQueueData_t fnFreeData);

/*Check to see if a queue is empty*/
boolean_t isQueueEmpty(basic_queue_t * q);

/*Adds an item to the queue*/
u32 enqueue(basic_queue_t * q, void * data);

/*Remove an item from the queue*/
void * dequeue(basic_queue_t * q);

/*Peek at an item from the queue*/
void * peekQueue(basic_queue_t *q);

/*
 * linked list
 */
void initLinkList(link_list_t * pList);

void finiLinkList(link_list_t * pList);

void finiLinkListAndData(link_list_t * pList,
    fnFreeListNodeData_t fnFreeData);

/*append to the tail of the linked list*/
u32 appendToLinkList(link_list_t * pList, void * pData);

/*intert to the head of the linked list*/
u32 insertToLinkList(link_list_t * pList, void * pData);

/** get the first node of linked list
 *
 */
static inline link_list_node_t * getFirstNodeOfLinkList(link_list_t * pList)
{
	return pList->ll_pllnHead;
}

/** get the next node of the specified node
 *
 */
static inline link_list_node_t * getNextNodeOfLinkList(link_list_node_t * pNode)
{
	return pNode->lln_pllnNext;
}

/** get data from the linked node
 *
 */
static inline void * getDataFromLinkListNode(link_list_node_t * pNode)
{
	return pNode->lln_pData;
}

/*
 * double linked list
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


/** get data from the linked node
 *
 */
static inline void * getDataFromDlinkListNode(dlink_list_node_t * pNode)
{
    return pNode->dln_pData;
}

/** get the first node of double linked list
 *
 */
static inline dlink_list_node_t * getFirstNodeOfDlinkList(dlink_list_t * pList)
{
    return pList->dl_pdlnHead;
}

/** get the last node of the double linked list
 *
 */
static inline dlink_list_node_t * getLastNodeOfDlinkList(dlink_list_t * pList)
{
    return pList->dl_pdlnTail;
}

/** get the next node of the specified node
 *
 */
static inline dlink_list_node_t * getNextNodeOfDlinkList(
    dlink_list_node_t * pNode)
{
    return pNode->dln_pdlnNext;
}

/** get the previous node of the specified node
 *
 */
static inline dlink_list_node_t * getPrevNodeOfDlinkList(
    dlink_list_node_t * pNode)
{
    return pNode->dln_pdlnPrev;
}

/*
 * HashTree Methods
 */

/*Creates an empty hash tree*/
static inline void initHashtree(hashtree_t * pHashtree)
{
    pHashtree->h_phnRoot = NULL;
}

/*Destroy a hash tree*/
void finiHashtree(hashtree_t * pHashtree);

void finiHashtreeAndData(
    hashtree_t * pHashtree, fnFreeHashtreeData_t fnFreeData);

/*determine if the hash tree is empty*/
static inline boolean_t isHashtreeEmpty(hashtree_t * pHashtree)
{
    return ((pHashtree->h_phnRoot == NULL) ? TRUE : FALSE);
}

/*Determines if a key entry exists in a hash tree*/
boolean_t hasHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/*Adds an item to the hash tree*/
u32 addHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void * pValue);

/*Gets an item from a hash tree*/
u32 getHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey, void ** ppData);

/*Deletes a keyed item from the hash tree*/
u32 deleteHashtreeEntry(
    hashtree_t * pHashtree, olchar_t * pstrKey, olsize_t sKey);

/** Return an Enumerator for a hash tree
 *
 *  @param pHashtree : hashtree_t * <BR> 
 *     @b [in] The HashTree to get an enumerator for 
 *
 *  @return return An enumerator   
 */
static inline void initHashtreeEnumerator(
    hashtree_t * pHashtree, hashtree_enumerator_t * pEnumerator)
{
    /*The enumerator is basically a state machine that keeps track of 
      which node we are at in the tree. So initialize it to the root.*/
    pEnumerator->he_phnNode = pHashtree->h_phnRoot;
}

/** Free resources associated with an Enumerator created by 
 *  initHashtreeEnumerator
 *
 *  @param pEnumerator : hashtree_enumerator_t * <BR> 
 *     @b [in] The enumerator to free 
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
 *  - Notes:  
 *    -# Functionally identicle to a DictionaryEnumerator in .NET
 *
 *  @param tree_enumerator : hashtree_enumerator_t * <BR> 
 *     @b [in] The enumerator to advance 
 *
 *  @return return A zero value if successful;
 *          return nonzero if no more items   
 */
static inline u32 moveHashtreeNext(hashtree_enumerator_t * pEnumerator)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pEnumerator->he_phnNode != NULL)
    {
        /*Advance the enumerator to point to the next node. If there is a node
          return 0, else return 1*/
        pEnumerator->he_phnNode = pEnumerator->he_phnNode->hn_phnNext;
    }
    else
    {
        /*There are no more nodes*/
        u32Ret = OLERR_END_OF_HASHTREE;
    }

    return u32Ret;
}

/** Read from the current item of an enumerator
 *
 *  @param pEnumerator : hashtree_enumerator_t * <BR> 
 *     @b [in] The enumerator to read from 
 *  @param ppu8Key : u8 ** <BR> 
 *     @b [out] The key of the current item 
 *  @param pu32KeyLen : u32  * <BR> 
 *     @b [out] The length of the key of the current item 
 *  @param ppData : void ** <BR> 
 *     @b [out] The data of the current item 
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

/*
 * list head
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

/** add a new entry, insert a new entry after the specified head.
 *
 *  @param head: list_head_t <BR>
 *     @b [in] list head to add it after
 *  @param new: list_head_t <BR>
 *     @b [in] new entry to be added
 */
static inline void listAdd(list_head_t * head, list_head_t * new)
{
    _listAdd(new, head, head->lh_plhNext);
}

/** add a new entry, insert a new entry before the specified head.
 *
 *  @param head: list_head_t <BR>
 *     @b [in] list head to add it before
 *  @param new: list_head_t <BR>
 *     @b [in] new entry to be added
 * 
 */
static inline void listAddTail(list_head_t * head, list_head_t * new)
{
    _listAdd(new, head->lh_plhPrev, head);
}

/** Delete a list entry by making the prev/next entries
 *  point to each other.
 *  This is only for internal list manipulation where we know
 *  the prev/next entries already!
 */
static inline void _listDel(list_head_t * prev, list_head_t * next)
{
    next->lh_plhPrev = prev;
    prev->lh_plhNext = next;
}

/** deletes entry from list.
 *
 *  - Notes
 *   -# list_empty on entry does not return true after this, the entry is
 *      in an undefined state.
 *
 *  @param entry: list_head_t <BR>
 *     @b [in] the element to delete from the list.
 */
static inline void listDel(list_head_t * entry)
{
    _listDel(entry->lh_plhPrev, entry->lh_plhNext);
    entry->lh_plhNext = NULL;
    entry->lh_plhPrev = NULL;
}

/** replace old entry by new one. if 'old' is empty, it will be overwritten.
 *
 *  @param old : list_head_t <BR>
 *     @b [in] the element to be replaced
 *  @param new : list_head_t <BR>
 *     @b [in] the new element to insert
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

/** deletes entry from list and reinitialize it.
 *
 *  @param entry : list_head_t <BR>
 *     @b [in] the element to delete from the list.
 */
static inline void listDelInit(list_head_t * entry)
{
    _listDel(entry->lh_plhPrev, entry->lh_plhNext);
    listInit(entry);
}

/** delete from one list and add as another's head
 *
 *  @param list: list_head_t <BR>
 *     @b [in] the entry to move
 *  @param head: list_head_t <BR>
 *     @b [in] the head that will precede our entry
 */
static inline void listMove(list_head_t * head, list_head_t * list)
{
    _listDel(list->lh_plhPrev, list->lh_plhNext);
    listAdd(head, list);
}

/** delete from one list and add as another's tail
 *
 *  @param list: list_head_t <BR>
 *     @b [in] the entry to move
 *  @param head: list_head_t <BR>
 *     @b [in] the head that will follow our entry
 */
static inline void listMoveTail(list_head_t * head, list_head_t * list)
{
    _listDel(list->lh_plhPrev, list->lh_plhNext);
    listAddTail(head, list);
}

/** tests whether @list is the last entry in list @head
 *
 *  @param head: list_head_t <BR>
 *     @b [in] the head of the list
 *  @param list: list_head_t <BR>
 *     @b [in] the entry to test
 */
static inline boolean_t listIsLast(const list_head_t * head,
    const list_head_t * list)
{
    return list->lh_plhNext == head;
}

/** tests whether @list is the first entry in list @head
 *
 *  @param head: list_head_t <BR>
 *     @b [in] the head of the list
 *  @param list: list_head_t <BR>
 *     @b [in] the entry to test
 */
static inline boolean_t listIsFirst(const list_head_t * head,
    const list_head_t * list)
{
    return list->lh_plhPrev == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
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

/** join two lists
 * 
 *  @head: the place to add it in the first list.
 *  @list: the new list to add.
 */
static inline void listSplice(list_head_t * head, list_head_t * list)
{
    if (! listIsEmpty(list))
        _listSplice(head, list);
}

/** join two lists and reinitialise the emptied list.
 * 
 *  @list: the new list to add.
 *  @head: the place to add it in the first list.
 *
 *  The list at @list is reinitialised
 */
static inline void listSpliceInit(list_head_t * head, list_head_t * list)
{
    if (! listIsEmpty(list))
    {
        _listSplice(head, list);
        listInit(list);
    }
}

/** split the 'head' at the position 'list' and add the removed part
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

/** get the struct for this entry
 *
 * @ptr:    the &list_head_t pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define listEntry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * listForEach  -   iterate over a list
 * @head:   the head for your list.
 * @pos:    the &list_head_t to use as a loop cursor.
 *
 */
#define listForEach(head, pos) \
    for (pos = (head)->lh_plhNext; pos != (head); pos = pos->lh_plhNext)

/**
 * listForEachSafe - iterate over a list safe against removal of list entry
 * @head:   the head for your list.
 * @pos:    the &struct list_head to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 */
#define listForEachSafe(head, pos, n) \
    for (pos = (head)->lh_plhNext, n = pos->lh_plhNext; pos != (head); \
         pos = n, n = pos->lh_plhNext)

/**
 * listForEachPrev   -   iterate over a list backwards
 * @pos:    the &list_head_t to use as a loop cursor.
 * @head:   the head for your list.
 */
#define listForEachPrev(head, pos) \
    for (pos = (head)->lh_plhPrev; pos != (head); pos = pos->lh_plhPrev)

/**
 * listForEachPrevSafe   -   iterate over a list backwards safe against
 *    removal of list entry
 * @head:   the head for your list.
 * @pos:    the &list_head_t to use as a loop cursor.
 * @n:      another &struct list_head to use as temporary storage
 */
#define listForEachPrevSafe(head, pos, n)                         \
    for (pos = (head)->lh_plhPrev, n = pos->lh_plhPrev; pos != (head); \
         pos = n, n = pos->lh_plhPrev)

/*
 * hlist
 */
#define HLIST_HEAD_INIT { .hh_phnFirst = NULL }
#define HLIST_HEAD(name) hlist_head_t name = {  .hh_phnFirst = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->hh_phnFirst = NULL)
#define INIT_HLIST_NODE(ptr) \
    ((ptr)->hn_phnNext = NULL, (ptr)->hn_pphnPrev = NULL)

#define hlistEntry(ptr, type, member) container_of(ptr, type, member)

#define hlistForEach(head, pos) \
    for (pos = (head)->hh_phnFirst; pos; pos = pos->hn_phnNext)

/**
 * hlistForEachEntry - iterate over list of given type
 * @tpos:   the type * to use as a loop counter.
 * @pos:    the &struct hlist_node to use as a loop counter.
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
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

/*list array*/
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

