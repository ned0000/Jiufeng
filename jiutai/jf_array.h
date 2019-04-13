/**
 *  @file jf_array.h
 *
 *  @brief Basic array header file. The array element can be the pointer to any
 *   type of data
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_array object
 *  @note It is NOT thread safe. The caller should provide synchronization for
 *   the array if necessary
 *  
 */

#ifndef JIUTAI_ARRAY_H
#define JIUTAI_ARRAY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef void jf_array_element_t;
typedef void jf_array_t;;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_array_create(jf_array_t ** ppja);

u32 jf_array_destroy(jf_array_t ** ppja);

u32 jf_array_getSize(jf_array_t * pja);

u32 jf_array_getElementAt(
    jf_array_t * pja, u32 u32Index, jf_array_element_t ** ppjae);

u32 jf_array_removeElementAt(jf_array_t * pja, u32 u32Index);

u32 jf_array_removeElement(jf_array_t * pja, jf_array_element_t * pjae);

u32 jf_array_removeAllElements(jf_array_t * pja);

u32 jf_array_insertElementAt(
    jf_array_t * pja, u32 u32Index, jf_array_element_t * pjae);

u32 jf_array_appendElementTo(jf_array_t * pja, jf_array_element_t * pjae);

typedef u32 (* jf_array_fnDestroyElement_t)(jf_array_element_t ** ppjae);

u32 jf_array_destroyAllElements(
    jf_array_t * pja, jf_array_fnDestroyElement_t fnDestroyElement);

u32 jf_array_destroyArrayAndElements(
    jf_array_t ** ppja, jf_array_fnDestroyElement_t fnDestroyElement);

typedef boolean_t (* jf_array_fnFindElement_t)(
    jf_array_element_t * pjae, void * pKey);

u32 jf_array_findElement(
    jf_array_t * pja, jf_array_element_t ** ppElement,
    jf_array_fnFindElement_t fnFindElement, void * pKey);

typedef u32 (* jf_array_fnOpOnElement_t)(
    jf_array_element_t * pjae, void * pData);

u32 jf_array_traverse(
    jf_array_t * pja, jf_array_fnOpOnElement_t fnOpOnElement, void * pData);

#endif /*JIUTAI_ARRAY_H*/

/*------------------------------------------------------------------------------------------------*/


