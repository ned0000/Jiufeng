/**
 *  @file jf_listhead.h
 *
 *  @brief The header file defines the list head data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# This object is not thread safe.
 *  
 */

#ifndef JIUTAI_LISTHEAD_H
#define JIUTAI_LISTHEAD_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the list head data type.
 */
typedef struct jf_listhead
{
    /**Next list head.*/
    struct jf_listhead * jl_pjlNext;
    /**Previous list head.*/
    struct jf_listhead * jl_pjlPrev;
} jf_listhead_t;

/** Macro definition for initializing the list head.
 */
#define JF_LISTHEAD_INIT(name) { &(name), &(name) }

/** Macro definition for declaring the list head variable and initilizing it.
 */
#define JF_LISTHEAD(name) \
    jf_listhead_t name = JF_LISTHEAD_INIT(name)

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize a list head.
 *
 *  @param list [in] The list head to initialize.
 *
 *  @return Void.
 */
static inline void jf_listhead_init(jf_listhead_t * list)
{
    list->jl_pjlNext = list;
    list->jl_pjlPrev = list;
}

/** Insert a new entry between two known consecutive entries.
 *
 *  @note
 *  -# This is only for internal list manipulation where we know the prev/next entries already.
 *
 *  @param new [in] New entry to be added.
 *  @param prev [in] Previous entry to add it after.
 *  @param next [in] Next entry to add it before.
 *
 *  @return Void.
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
 *  @param head [in] List head to add it after.
 *  @param new [in] New entry to be added.
 *
 *  @return Void.
 */
static inline void jf_listhead_add(jf_listhead_t * head, jf_listhead_t * new)
{
    _listAdd(new, head, head->jl_pjlNext);
}

/** Add a new entry to the end of list.
 *
 *  @param head [in] List head to add it before.
 *  @param new [in] New entry to be added.
 * 
 *  @return Void.
 */
static inline void jf_listhead_addTail(jf_listhead_t * head, jf_listhead_t * new)
{
    _listAdd(new, head->jl_pjlPrev, head);
}

/** Delete a list entry by making the prev/next entries point to each other.
 *
 *  @note
 *  -# This is only for internal list manipulation where we know the prev/next entries already.
 *
 *  @param prev [in] List head to delete it before.
 *  @param next [in] List head to delete it after.
 *
 *  @return Void.
 */
static inline void _listDel(jf_listhead_t * prev, jf_listhead_t * next)
{
    next->jl_pjlPrev = prev;
    prev->jl_pjlNext = next;
}

/** Deletes entry from list.
 *
 *  @param entry [in] The element to delete from the list.
 *
 *  @return Void.
 */
static inline void jf_listhead_del(jf_listhead_t * entry)
{
    _listDel(entry->jl_pjlPrev, entry->jl_pjlNext);
    entry->jl_pjlNext = NULL;
    entry->jl_pjlPrev = NULL;
}

/** Replace old entry by new one.
 *
 *  @param old [in] The element to be replaced.
 *  @param new [in] The new element to insert.
 *
 *  @return Void.
 */
static inline void jf_listhead_replace(jf_listhead_t * old, jf_listhead_t * new)
{
    new->jl_pjlNext = old->jl_pjlNext;
    new->jl_pjlNext->jl_pjlPrev = new;
    new->jl_pjlPrev = old->jl_pjlPrev;
    new->jl_pjlPrev->jl_pjlNext = new;
}

/** Replace old entry by new one.
 *
 *  @note
 *  -# After replace, initialize the old entry.
 *
 *  @param old [in] The element to be replaced.
 *  @param new [in] The new element to insert.
 *
 *  @return Void.
 */
static inline void jf_listhead_replaceInit(jf_listhead_t * old, jf_listhead_t * new)
{
    jf_listhead_replace(old, new);
    jf_listhead_init(old);
}

/** Delete entry from list and reinitialize it.
 *
 *  @param entry [in] The element to delete from the list.
 *
 *  @return Void.
 */
static inline void jf_listhead_delInit(jf_listhead_t * entry)
{
    _listDel(entry->jl_pjlPrev, entry->jl_pjlNext);
    jf_listhead_init(entry);
}

/** Delete entry from one list and add it to the head of another list.
 *
 *  @param head [in] The entry to move.
 *  @param list [in] The head that will precede our entry.
 *
 *  @return Void.
 */
static inline void jf_listhead_move(jf_listhead_t * head, jf_listhead_t * list)
{
    _listDel(list->jl_pjlPrev, list->jl_pjlNext);
    jf_listhead_add(head, list);
}

/** Delete entry from one list and add it to the tail of another list.
 *
 *  @param head [in] The head that will follow our entry.
 *  @param list [in] The entry to move.
 *
 *  @return Void.
 */
static inline void jf_listhead_moveTail(jf_listhead_t * head, jf_listhead_t * list)
{
    _listDel(list->jl_pjlPrev, list->jl_pjlNext);
    jf_listhead_addTail(head, list);
}

/** Test whether the entry is the last entry of the list.
 *
 *  @param head [in] The head of the list.
 *  @param entry [in] The entry to test.
 *
 *  @return The status of the entry.
 *  @retval TRUE The entry is the last entry. 
 *  @retval FALSE The entry is not the last entry. 
 */
static inline boolean_t jf_listhead_isLast(const jf_listhead_t * head, const jf_listhead_t * entry)
{
    return entry->jl_pjlNext == head;
}

/** Tests whether the entry is the first entry of the list.
 *
 *  @param head [in] The head of the list.
 *  @param entry [in] The entry to test.
 *
 *  @return The status of the entry.
 *  @retval TRUE The entry is the first entry. 
 *  @retval FALSE The entry is not the first entry. 
 */
static inline boolean_t jf_listhead_isFirst(const jf_listhead_t * head, const jf_listhead_t * entry)
{
    return entry->jl_pjlPrev == head;
}

/** Test whether a list is empty.
 * 
 *  @param head [in] The list to test.
 *
 *  @return The status of the list.
 *  @retval TRUE The list is the empty. 
 *  @retval FALSE The list is not empty.
 */
static inline boolean_t jf_listhead_isEmpty(const jf_listhead_t * head)
{
    return head->jl_pjlNext == head;
}

/** Join two lists, the list is joined at head.
 *
 *  @note
 *  -# This is only for internal list manipulation.
 *
 *  @param head [in] The place to add it in the first list.
 *  @param list [in] The new list to add.
 *
 *  @return Void.
 */
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

/** Join two lists, the list is joined at head.
 * 
 *  @param head [in] The place to add it in the first list.
 *  @param list [in] The new list to add.
 *
 *  @return Void.
 */
static inline void jf_listhead_splice(jf_listhead_t * head, jf_listhead_t * list)
{
    if (! jf_listhead_isEmpty(list))
    {
        _listSplice(head, list);
        jf_listhead_init(list);
    }
}

/** Join two lists, the list is joined at tail.
 *
 *  @note
 *  -# This is only for internal list manipulation.
 *
 *  @param head [in] The place to add it in the first list.
 *  @param list [in] The new list to add.
 *
 *  @return Void.
 */
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

/** Join two lists, the list is joined at tail.
 * 
 *  @param head [in] The place to add it in the first list.
 *  @param list [in] The new list to add.
 *
 *  @return Void.
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
 *  @note
 *  -# The list is reinitialised.
 *
 *  @param head [in] The place to add it in the first list.
 *  @param list [in] The new list to add.
 *
 *  @return Void.
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
 *  After split, 'list' is added to 'head'.
 *
 *  @param head [in] The list to split.
 *  @param list [in] The position to split.
 *  @param newhead [in] The new list.
 *
 *  @return Void.
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

    typedef olchar_t CHAR;
    typedef CHAR *PCHAR;

    #define container_of(address, type, field) ((type *)( \
       (PCHAR)(address) - offsetof(type, field)))

#endif

/** Get the struct for this entry.
 *
 *  @param ptr [in] The jf_listhead_t pointer.
 *  @param type [in] The type of the struct this is embedded in.
 *  @param member [in] The name of the list head within the struct.
 */
#define jf_listhead_getEntry(ptr, type, member) \
    container_of(ptr, type, member)

/** Iterate over a list.
 * 
 *  @param head [in] The head for your list.
 *  @param pos [in] The list head to use as a loop cursor.
 *
 */
#define jf_listhead_forEach(head, pos) \
    for (pos = (head)->jl_pjlNext; pos != (head); pos = pos->jl_pjlNext)

/** Iterate over a list safe against removal of list entry.
 * 
 *  @param head [in] The head for your list.
 *  @param pos [in] The list head to use as a loop cursor.
 *  @param n [in] Another list head to use as temporary storage.
 */
#define jf_listhead_forEachSafe(head, pos, n) \
    for (pos = (head)->jl_pjlNext, n = pos->jl_pjlNext; pos != (head); \
         pos = n, n = pos->jl_pjlNext)

/** Iterate over a list backwards.
 * 
 *  @param head [in] The head for your list.
 *  @param pos [in] The list head to use as a loop cursor.
 */
#define jf_listhead_forEachPrev(head, pos) \
    for (pos = (head)->jl_pjlPrev; pos != (head); pos = pos->jl_pjlPrev)

/** Iterate over a list backwards safe against removal of list entry.
 *
 *  @param head [in] The head for your list.
 *  @param pos [in] The list head to use as a loop cursor.
 *  @param n [in] Another list head to use as temporary storage.
 */
#define jf_listhead_forEachPrevSafe(head, pos, n)                      \
    for (pos = (head)->jl_pjlPrev, n = pos->jl_pjlPrev; pos != (head); \
         pos = n, n = pos->jl_pjlPrev)

#endif /*JIUTAI_LISTHEAD_H*/

/*------------------------------------------------------------------------------------------------*/

