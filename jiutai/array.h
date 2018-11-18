/**
 *  @file array.h
 *
 *  @brief Basic array header file. The array element can be the pointer to any
 *   type of data
 *
 *  @author Min Zhang
 *
 *  @note It is NOT thread safe. The caller should provide synchronization for
 *   the array if necessary
 *  
 */

#ifndef JIUTAI_ARRAY_H
#define JIUTAI_ARRAY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef void basic_array_element_t;
typedef void basic_array_t;;

/* --- functional routines ------------------------------------------------- */

u32 createBasicArray(basic_array_t ** ppba);

u32 destroyBasicArray(basic_array_t ** ppba);

u32 getBasicArraySize(basic_array_t * pba);

u32 getAtBasicArray(basic_array_t * pba, u32 u32Index,
    basic_array_element_t ** ppbae);

u32 removeAtBasicArray(basic_array_t * pba, u32 u32Index);

u32 removeBasicArrayElement(basic_array_t * pba, basic_array_element_t * pbae);

u32 removeAllBasicArrayElements(basic_array_t * pba);

u32 insertAtBasicArray(
    basic_array_t * pba, u32 u32Index, basic_array_element_t * pbae);

u32 appendToBasicArray(basic_array_t * pba, basic_array_element_t * pbae);

typedef u32 (* fnDestroyBasicArrayElement_t)(basic_array_element_t ** ppbae);

u32 destroyAllBasicArrayElements(
    basic_array_t * pba, fnDestroyBasicArrayElement_t fnDestroyElement);

u32 destroyBasicArrayAndElements(
    basic_array_t ** ppba, fnDestroyBasicArrayElement_t fnDestroyElement);

typedef boolean_t (* fnFindBasicArrayElement_t)(
    basic_array_element_t * pbae, void * pKey);

u32 findBasicArrayElement(basic_array_t * pba,
    basic_array_element_t ** ppElement,
    fnFindBasicArrayElement_t fnFindElement, void * pKey);

typedef u32 (* fnOpOnBasicArrayElement_t)(basic_array_element_t * pbae,
    void * pData);

u32 traverseBasicArray(basic_array_t * pba,
    fnOpOnBasicArrayElement_t fnOpOnElement, void * pData);

#endif /*JIUTAI_ARRAY_H*/

/*---------------------------------------------------------------------------*/


