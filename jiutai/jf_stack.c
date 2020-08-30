/**
 *  @file jf_stack.c
 *
 *  @brief Implementation file for the stack data type.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <errno.h>
#include <stdio.h>
#include <time.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_stack.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

void jf_stack_init(jf_stack_t ** ppStack)
{
    *ppStack = NULL;
}

u32 jf_stack_push(jf_stack_t ** ppStack, void * data)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_stack_node_t * retval = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&retval, sizeof(*retval));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->jsn_pData = data;
        retval->jsn_pjsnNext = * ppStack;

        *ppStack = retval;
    }

    return u32Ret;
}

void * jf_stack_pop(jf_stack_t ** ppStack)
{
    void * retval = NULL;
    void * temp = NULL;

    if (*ppStack != NULL)
    {
        retval = ((jf_stack_node_t *) *ppStack)->jsn_pData;
        temp = *ppStack;
        /*Move the stack pointer to the next node.*/
        *ppStack = ((jf_stack_node_t *) *ppStack)->jsn_pjsnNext;
        /*Free the stack node.*/
        jf_jiukun_freeMemory((void **)&temp);
    }

    return retval;
}

void * jf_stack_peek(jf_stack_t ** ppStack)
{
    void * retval = NULL;

    if (*ppStack != NULL)
        retval = ((jf_stack_node_t *) *ppStack)->jsn_pData;

    return retval;
}

void jf_stack_clear(jf_stack_t ** ppStack)
{
    void * temp = *ppStack;

    do
    {
        jf_stack_pop(&temp);
    }
    while (temp != NULL);

    *ppStack = NULL;
}

/*------------------------------------------------------------------------------------------------*/
