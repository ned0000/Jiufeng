/**
 *  @file jf_option.h
 *
 *  @brief Option header file which provide some functional routine for option manipulation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_option common object.
 *  -# For option handling, use functions in jf_option object, not jf_string library.
 *  -# No memory allocation in jf_option object.
 *  
 */

#ifndef JIUTAI_OPTION_H
#define JIUTAI_OPTION_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/*string validation*/

/** Validate the integer string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the integer string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid integer.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_validateIntegerString(const olchar_t * pstrInteger, const olsize_t size);

/** Validate the float string.
 *
 *  @param pstrFloat [in] The float string.
 *  @param size [in] The size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_FLOAT Invalid float.
 */
u32 jf_option_validateFloatString(const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

/** Reads an unsigned char from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu8Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 *  @retval JF_ERR_INTEGER_OUT_OF_RANGE The integer is out of range.
 */
u32 jf_option_getU8FromString(const olchar_t * pstrInteger, u8 * pu8Value);

/** Reads an unsigned short from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu16Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 *  @retval JF_ERR_INTEGER_OUT_OF_RANGE The integer is out of range.
 */
u32 jf_option_getU16FromString(const olchar_t * pstrInteger, u16 * pu16Value);

/** Reads a signed int from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param ps32Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getS32FromString(const olchar_t * pstrInteger, s32 * ps32Value);

/** Reads an unsigned int from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu32Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getU32FromString(const olchar_t * pstrInteger, u32 * pu32Value);

/** Reads a long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param numeric [out] the long value extracted from the string 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getLongFromString(const olchar_t * pstrInteger, slong * numeric);

/** Reads an unsigned long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param numeric [out] The unsigned long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getUlongFromString(const olchar_t * pstrInteger, ulong * numeric);

/** Reads an unsigned long long value from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu64Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getU64FromString(const olchar_t * pstrInteger, u64 * pu64Value);

/** Reads a signed long long value from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param ps64Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getS64FromString(const olchar_t * pstrInteger, s64 * ps64Value);

/** Reads a float from a string.
 *
 *  @param pstrFloat [in] The float string.
 *  @param pflValue [out] The size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_FLOAT Invalid float.
 */
u32 jf_option_getFloatFromString(const olchar_t * pstrFloat, olfloat_t * pflValue);

/** Reads a double from a string.
 *
 *  @param pstrDouble [in] The float string.
 *  @param pdbValue [out] The size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_FLOAT Invalid float.
 */
u32 jf_option_getDoubleFromString(const olchar_t * pstrDouble, oldouble_t * pdbValue);

#endif /*JIUTAI_OPTION_H*/

/*------------------------------------------------------------------------------------------------*/


