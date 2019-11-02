/**
 *  @file jf_option.h
 *
 *  @brief option header file, provide some functional routine for option manipulation
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_option common object.
 *  @note For option handling, use functions in jf_option object, not jf_string library.
 *  @note No memory allocation in jf_option object.
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

u32 jf_option_validateIntegerString(const olchar_t * pstrInteger, const olsize_t size);

u32 jf_option_validateFloatString(const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

u32 jf_option_getU8FromString(const olchar_t * pstrInteger, u8 * pu8Value);

u32 jf_option_getU16FromString(const olchar_t * pstrInteger, u16 * pu16Value);

u32 jf_option_getS32FromString(const olchar_t * pstrInteger, s32 * ps32Value);

u32 jf_option_getU32FromString(const olchar_t * pstrInteger, u32 * pu32Value);

/** Reads a long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param size [in] the length of the string 
 *  @param numeric [in] the long value extracted from the string 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_option_getLongFromString(const olchar_t * pstrInteger, slong * numeric);

/** Reads an unsigned long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param size [in] the length of the string 
 *  @param numeric [in] the unsigned long value extracted from the string 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_option_getUlongFromString(const olchar_t * pstrInteger, ulong * numeric);

u32 jf_option_getU64FromString(const olchar_t * pstrInteger, u64 * pu64Value);

u32 jf_option_getS64FromString(const olchar_t * pstrInteger, s64 * ps64Value);

u32 jf_option_getFloatFromString(const olchar_t * pstrFloat, olfloat_t * pflValue);

u32 jf_option_getDoubleFromString(const olchar_t * pstrDouble, oldouble_t * pdbValue);

#endif /*JIUTAI_OPTION_H*/

/*------------------------------------------------------------------------------------------------*/


