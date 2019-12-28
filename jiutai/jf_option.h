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
 */
u32 jf_option_validateIntegerString(const olchar_t * pstrInteger, const olsize_t size);

/** Validate the float string.
 */
u32 jf_option_validateFloatString(const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

/** Reads an unsigned char from a string.
 */
u32 jf_option_getU8FromString(const olchar_t * pstrInteger, u8 * pu8Value);

/** Reads an unsigned short from a string.
 */
u32 jf_option_getU16FromString(const olchar_t * pstrInteger, u16 * pu16Value);

/** Reads a signed int from a string.
 */
u32 jf_option_getS32FromString(const olchar_t * pstrInteger, s32 * ps32Value);

/** Reads an unsigned int from a string.
 */
u32 jf_option_getU32FromString(const olchar_t * pstrInteger, u32 * pu32Value);

/** Reads a long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param numeric [in] the long value extracted from the string 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_option_getLongFromString(const olchar_t * pstrInteger, slong * numeric);

/** Reads an unsigned long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param numeric [in] The unsigned long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_option_getUlongFromString(const olchar_t * pstrInteger, ulong * numeric);

/** Reads an unsigned long long value from a string.
 */
u32 jf_option_getU64FromString(const olchar_t * pstrInteger, u64 * pu64Value);

/** Reads a signed long long value from a string.
 */
u32 jf_option_getS64FromString(const olchar_t * pstrInteger, s64 * ps64Value);

/** Reads a float from a string.
 */
u32 jf_option_getFloatFromString(const olchar_t * pstrFloat, olfloat_t * pflValue);

/** Reads a double from a string.
 */
u32 jf_option_getDoubleFromString(const olchar_t * pstrDouble, oldouble_t * pdbValue);

#endif /*JIUTAI_OPTION_H*/

/*------------------------------------------------------------------------------------------------*/


