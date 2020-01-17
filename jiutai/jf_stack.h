/**
 *  @file jf_stack.h
 *
 *  @brief The header file defines the stack data type.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_stack object.
 *  -# The stack is first in, last out.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# This object is not thread safe.
 *  
 */

#ifndef JIUTAI_STACK_H
#define JIUTAI_STACK_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stddef.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the stack node data type.
 */
typedef struct jf_stack_node
{
    /**The data.*/
    void * jsn_pData;
    /**The next node of the stack.*/
    struct jf_stack_node * jsn_pjsnNext;
} jf_stack_node_t;

/** Define the stack data type.
 */
typedef void  jf_stack_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize an empty Stack.
 *
 *  @par Example
 *  @code
 *     void *stack = NULL;
 *     jf_stack_init(&stack);
 *  @endcode
 *
 *  @param ppStack [in/out] The stack to be initialize.
 *
 *  @return Void.
 */
void jf_stack_init(jf_stack_t ** ppStack);

/** Push an item into the stack.
 *
 *  @param ppStack [in/out] The stack to push to. 
 *  @param pData [in] The data to push onto the stack. 
 *
 *  @return The error code.
 */
u32 jf_stack_push(jf_stack_t ** ppStack, void * pData);

/** Pop an item from the stack.
 *
 *  @note
 *  -# After peek, the item is removed from stack.
 *
 *  @param ppStack [in/out] The stack to pop from.
 *
 *  @return The item that was popped from the stack.
 */
void * jf_stack_pop(jf_stack_t ** ppStack);

/** Peek the item on the top of the stack.
 *
 *  @note
 *  -# After peek, the item is still in stack.
 *
 *  @param ppStack [in/out] The stack to peek from.
 *
 *  @return The item that is currently on the top of the stack.
 */
void * jf_stack_peek(jf_stack_t ** ppStack);

/** Clear all the items from the stack.
 *
 *  @param ppStack [in/out] The stack to clear.
 *
 *  @return Void.
 */
void jf_stack_clear(jf_stack_t ** ppStack);

#endif /*JIUTAI_STACK_H*/

/*------------------------------------------------------------------------------------------------*/

