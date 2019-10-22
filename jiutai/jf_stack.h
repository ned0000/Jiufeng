/**
 *  @file jf_stack.h
 *
 *  @brief The basic stack data structure
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_stack object
 *  @note Link with jf_jiukun library for cache
 *  @note This object is not thread safe
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

/*basic stack*/
typedef struct jf_stack_node
{
    void * jsn_pData;
    struct jf_stack_node * jsn_pjsnNext;
} jf_stack_node_t;

typedef void  jf_stack_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Init an empty Stack
 *
 *  @param ppStack [in/out] the stack to be initialize
 *
 *  @return void
 *
 *  @note This module uses a void* that is preinitialized to NULL, eg:
 *   void *stack = NULL;
 *   jf_stack_init(&stack);
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

/** Create cache for stack node
 *
 *  @note Cache should be created before using other functions
 *
 *  @return the error code
 */
u32 jf_stack_createCache(void);

/** Destroy cache for stack node
 *
 *  @return the error code
 */
u32 jf_stack_destroyCache(void);

#endif /*JIUTAI_STACK_H*/

/*------------------------------------------------------------------------------------------------*/

