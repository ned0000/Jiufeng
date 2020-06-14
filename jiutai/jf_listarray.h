/**
 *  @file jf_listarray.h
 *
 *  @brief Header file of list array data structure.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# This object is not thread safe.
 */

#ifndef JIUTAI_LISTARRAY_H
#define JIUTAI_LISTARRAY_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Linked list using arrays of node
 */
typedef struct jf_listarray
{
    /**Number of node in the array.*/
    u32 jl_u32NumOfNode;
    /**Head of list array.*/
    u32 jl_u32Head;
} jf_listarray_t;

/** Get list array node.
 */
#define JF_LISTARRAY_NODE(pla)             ((u32 *)(((jf_listarray_t *)pla) + 1))

/** The end of list array.
 */
#define JF_LISTARRAY_END                   U32_MAX

/* --- functional routines ---------------------------------------------------------------------- */

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

#endif /*JIUTAI_LISTARRAY_H*/

/*------------------------------------------------------------------------------------------------*/

