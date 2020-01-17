/**
 *  @file jf_dynlib.h
 *
 *  @brief Header file defines the interface for dynamic library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_dynlib object.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# Link with dl library on Linux platform.
 *  
 */

#ifndef JIUTAI_DYNLIB_H
#define JIUTAI_DYNLIB_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the dynlib data type.
 */
typedef void  jf_dynlib_t;

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

/** Load dynamic library.
 *
 *  @param pstrLibFile [in] The full path of the library file.
 *  @param ppLib [out] The dynamic library handle.
 *
 *  @return The error code.
 */
u32 jf_dynlib_load(const olchar_t * pstrLibFile, jf_dynlib_t ** ppLib);

/** Unload dynamic library.
 *
 *  @param ppLib [out] The dynamic library handle.
 *
 *  @return The error code.
 */
u32 jf_dynlib_unload(jf_dynlib_t ** ppLib);

/** Get address of a synmbol from dynamic library.
 *
 *  @param pLib [in] The dynamic library handle.
 *  @param pstrSymbol [in] The symbol string with NULL terminated.
 *  @param ppAddress [out] The address of the symbol.
 *
 *  @return The error code.
 */
u32 jf_dynlib_getSymbolAddress(jf_dynlib_t * pLib, const olchar_t * pstrSymbol, void ** ppAddress);

#endif /*JIUTAI_DYNLIB_H*/

/*------------------------------------------------------------------------------------------------*/


