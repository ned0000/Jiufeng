/**
 *  @file jf_listhead.h
 *
 *  @brief The list head data structure
 *
 *  @author Min Zhang
 *
 *  @note This object is not thread safe
 *  
 */

#ifndef JIUTAI_LISTHEAD_H
#define JIUTAI_LISTHEAD_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/*list head*/
typedef struct jf_listhead
{
    struct jf_listhead * jl_pjlNext;
    struct jf_listhead * jl_pjlPrev;
} jf_listhead_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** list head
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
static inline void _listAdd(jf_listhead_t * new, jf_listhead_t * prev, jf_listhead_t * next)
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
static inline boolean_t jf_listhead_isLast(const jf_listhead_t * head, const jf_listhead_t * list)
{
    return list->jl_pjlNext == head;
}

/** Tests whether 'list' is the first entry in list 'head'
 *
 *  @param head [in] the head of the list
 *  @param list [in] the entry to test
 */
static inline boolean_t jf_listhead_isFirst(const jf_listhead_t * head, const jf_listhead_t * list)
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

/** Join two lists, the list is joined at head
 * 
 *  @param head [in] the place to add it in the first list.
 *  @param list [in] the new list to add.
 */
static inline void jf_listhead_splice(jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
    {
        _listSplice(head, list);
        jf_listhead_init(list);
    }
}

static inline void _listSpliceTail(jf_listhead_t * head, jf_listhead_t * list)
{
    jf_listhead_t * first = list->jl_pjlNext;
    jf_listhead_t * last = list->jl_pjlPrev;
    jf_listhead_t * at = head->jl_pjlPrev;

    first->jl_pjlPrev = at;
    at->jl_pjlNext = first;

    last->jl_pjlNext = head;
    head->jl_pjlPrev = last;
}

/** Join two lists, the list is joined at tail
 * 
 *  @param head [in] the place to add it in the first list.
 *  @param list [in] the new list to add.
 */
static inline void jf_listhead_spliceTail(jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
    {
        _listSpliceTail(head, list);
        jf_listhead_init(list);
    }
}

/** Join two lists and reinitialise the emptied list.
 * 
 *  @param head [in] the place to add it in the first list.
 *  @param list [in] the new list to add.
 *
 *  @note the list at 'list' is reinitialised
 */
static inline void jf_listhead_spliceInit(jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
    {
        _listSplice(head, list);
        jf_listhead_init(list);
    }
}

/** Split the 'head' at the position 'list' and add the removed part (exclude 'list') to 'newhead'
 *  After split, 'list' is added to 'head'
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

#endif /*JIUTAI_LISTHEAD_H*/

/*------------------------------------------------------------------------------------------------*/

