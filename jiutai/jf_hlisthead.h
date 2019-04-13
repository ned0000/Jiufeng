/**
 *  @file jf_hlisthead.h
 *
 *  @brief The double linked list data structure
 *
 *  @author Min Zhang
 *
 *  @note This object is not thread safe
 *
 */

#ifndef JIUTAI_HLISTHEAD_H
#define JIUTAI_HLISTHEAD_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_listhead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Double linked lists with a single pointer list head.
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

/* --- functional routines ---------------------------------------------------------------------- */

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

#endif /*JIUTAI_HLISTHEAD_H*/

/*------------------------------------------------------------------------------------------------*/

