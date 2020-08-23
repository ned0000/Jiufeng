/**
 *  @file jf_option.h
 *
 *  @brief Header file which provide some functional routine for option manipulation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_option common object.
 *  -# For option handling, use functions in jf_option object, not jf_string library.
 *  -# No memory allocation in jf_option object, so it's safe to use it before memory is initialized.
 *  
 *  @par Rules for Option
 *  -# Only short options are supported.
 *  -# Options are started with '-'.
 *  -# ':' after option is used to indicate that the option has an argument.
 *  -# Multiple options can follow one '-'.
 *  -# Space should be there between option and argument.
 *  @code
 *  command -x
 *  command -xy
 *  command -z value
 *  command -xyz value
 *  @endcode
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

/** Read an unsigned char from a string.
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

/** Read an unsigned short from a string.
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

/** Read a signed int from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param ps32Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getS32FromString(const olchar_t * pstrInteger, s32 * ps32Value);

/** Read an unsigned int from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu32Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getU32FromString(const olchar_t * pstrInteger, u32 * pu32Value);

/** Read a long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param numeric [out] the long value extracted from the string 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getLongFromString(const olchar_t * pstrInteger, slong * numeric);

/** Read an unsigned long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param numeric [out] The unsigned long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getUlongFromString(const olchar_t * pstrInteger, ulong * numeric);

/** Read an unsigned long long value from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param pu64Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getU64FromString(const olchar_t * pstrInteger, u64 * pu64Value);

/** Read a signed long long value from a string.
 *
 *  @param pstrInteger [in] The integer string with NULL terminated.
 *  @param ps64Value [out] The unsigned char read from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_INTEGER Invalid integer.
 */
u32 jf_option_getS64FromString(const olchar_t * pstrInteger, s64 * ps64Value);

/** Read a float from a string.
 *
 *  @param pstrFloat [in] The float string.
 *  @param pflValue [out] The size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_FLOAT Invalid float.
 */
u32 jf_option_getFloatFromString(const olchar_t * pstrFloat, olfloat_t * pflValue);

/** Read a double from a string.
 *
 *  @param pstrDouble [in] The float string.
 *  @param pdbValue [out] The size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Valid float.
 *  @retval JF_ERR_INVALID_FLOAT Invalid float.
 */
u32 jf_option_getDoubleFromString(const olchar_t * pstrDouble, oldouble_t * pdbValue);

/** Skip the space before string.
 *
 *  @note
 *  -# The string is not touched and the space is not removed. The new string is returned.
 *
 *  @param pstr [in] The original string.
 *
 *  @return The pointer to the string without space.
 */
olchar_t * jf_option_skipSpaceBeforeString(olchar_t * pstr);

/** Remove the space after string.
 *
 *  @note
 *  -# The space is replaced with '\0'.
 *
 *  @param pstr [in] The destination string.
 *
 *  @return Void.
 */
void jf_option_removeSpaceAfterString(olchar_t * pstr);

/* Functions for parsing argument array. */

/** Get an option from the argument array.
 *
 *  @note
 *  -# The option string contains all the supported options. If the option has argument followed,
 *   ':' is used.
 *  -# '?' and ':' is returned for error, the parse should not continue as all internal variables
 *   inside this function are reset.
 *
 *  @param argc [in] Number of elements in the argument array.
 *  @param argv [in] The argument array.
 *  @param stropt [in] The option string.
 *
 *  @return The option or the parse result.
 *  @retval option Option has been successfully found.
 *  @retval -1 All options have been parsed.
 *  @retval ? Unknown option which is not in option string.
 *  @retval ':' The option is with a missing argument.
 */
olint_t jf_option_get(olint_t argc, olchar_t ** const argv, const olchar_t * stropt);

/** Get argument of current option parsed.
 *
 *  @return The option argument.
 */
olchar_t * jf_option_getArg(void);

/** Reset the option parse.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_option_reset(void);

#endif /*JIUTAI_OPTION_H*/

/*------------------------------------------------------------------------------------------------*/
