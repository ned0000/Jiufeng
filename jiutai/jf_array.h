/**
 *  @file jf_array.h
 *
 *  @brief Array header file which declares routines for array operation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_array object.
 *  -# It is NOT thread safe. The caller should provide synchronization for the array if necessary.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# The array element is a pointer to any type of data.
 */

#ifndef JIUTAI_ARRAY_H
#define JIUTAI_ARRAY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the array element data type.
 */
typedef void jf_array_element_t;

/** Define the array data type.
 */
typedef void jf_array_t;

/** Define the function data type which is used to destroy element.
 *
 *  @param ppjae [in/out] The element to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_array_fnDestroyElement_t)(jf_array_element_t ** ppjae);

/** Define the function data type which is used to find element.
 *
 *  @param pjae [in] The element to be checked.
 *  @param pKey [in] The argument of the callback function.
 *
 *  @return The status of finding.
 *  @retval TRUE The element is found.
 *  @retval FALSE The element is not found.
 */
typedef boolean_t (* jf_array_fnFindElement_t)(jf_array_element_t * pjae, void * pKey);

/** Define the function data type which is used to do operation on element.
 *
 *  @note
 *  -# The traversal will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @param pjae [in] The element to operate.
 *  @param pData [in] The argument of the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_array_fnOpOnElement_t)(jf_array_element_t * pjae, void * pData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create array.
 *
 *  @param ppja [out] The pointer to the array.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of memory.
 */
u32 jf_array_create(jf_array_t ** ppja);

/** Destroy array.
 *
 *  @param ppja [in/out] The pointer to the array.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_destroy(jf_array_t ** ppja);

/** Get array size.
 *
 *  @param pja [in] The pointer to the array.
 *
 *  @return The array size
 */
u32 jf_array_getSize(jf_array_t * pja);

/** Get element of array at specified position.
 *
 *  @param pja [in] The pointer to the array.
 *  @param u32Index [in] The position of the element.
 *  @param ppjae [out] The pointer to the array element.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_RANGE The index is out of range.
 */
u32 jf_array_getElementAt(jf_array_t * pja, u32 u32Index, jf_array_element_t ** ppjae);

/** Remove element from array at specified position.
 *
 *  @param pja [in] The pointer to the array.
 *  @param u32Index [in] The position of the element.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_RANGE The index is out of range.
 */
u32 jf_array_removeElementAt(jf_array_t * pja, u32 u32Index);

/** Remove element from array with specified element.
 *
 *  @param pja [in] The pointer to the array.
 *  @param pjae [in] The element to be removed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND The element is not found.
 */
u32 jf_array_removeElement(jf_array_t * pja, jf_array_element_t * pjae);

/** Remove all elements from array.
 *
 *  @param pja [in] The pointer to the array.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_removeAllElements(jf_array_t * pja);

/** Insert element to array at specified position.
 *
 *  @note
 *  -# If the position is out of range, the element is appended to the end of array, no error is
 *   returned.
 *
 *  @param pja [in] The pointer to the array.
 *  @param u32Index [in] The position of the element.
 *  @param pjae [in] The element to be inserted.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of memory.
 */
u32 jf_array_insertElementAt(jf_array_t * pja, u32 u32Index, jf_array_element_t * pjae);

/** Append element to array.
 *
 *  @param pja [in] The pointer to the array.
 *  @param pjae [in] The element to be appended.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of memory.
 */
u32 jf_array_appendElementTo(jf_array_t * pja, jf_array_element_t * pjae);

/** Destroy all elements in array.
 *
 *  @param pja [in] The pointer to the array.
 *  @param fnDestroyElement [in] The callback function to destroy elements.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_destroyAllElements(jf_array_t * pja, jf_array_fnDestroyElement_t fnDestroyElement);

/** Destroy the array and all elements in array.
 *
 *  @param ppja [in/out] The pointer to the array.
 *  @param fnDestroyElement [in] The callback function to destroy elements.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_destroyArrayAndElements(
    jf_array_t ** ppja, jf_array_fnDestroyElement_t fnDestroyElement);

/** Find element in array.
 *
 *  @param pja [in] The pointer to the array.
 *  @param ppElement [out] The pointer to the element found.
 *  @param fnFindElement [in] The callback function to check elements.
 *  @param pKey [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_findElement(
    jf_array_t * pja, jf_array_element_t ** ppElement, jf_array_fnFindElement_t fnFindElement,
    void * pKey);

/** Traverse array and do operation to each elements.
 *
 *  @note
 *  -# The traversal will stop if return code of callback function is not JF_ERR_NO_ERROR.
 *
 *  @param pja [in] The pointer to the array.
 *  @param fnOpOnElement [out] The callback operation function.
 *  @param pData [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_array_traverse(jf_array_t * pja, jf_array_fnOpOnElement_t fnOpOnElement, void * pData);

#endif /*JIUTAI_ARRAY_H*/

/*------------------------------------------------------------------------------------------------*/
