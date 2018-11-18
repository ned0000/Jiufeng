/**
 *  @file dynlib.h
 *
 *  @brief Dynamic library header file. Provide some functional routine for
 *   dynamic library
 *
 *  @author Min Zhang
 *
 *  @note Link with xmalloc object file
 *  @note Link with dl library on Linux platform
 *  
 */

#ifndef JIUTAI_DYNLIB_H
#define JIUTAI_DYNLIB_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */
typedef void  dyn_lib_t;

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
u32 loadDynLib(const olchar_t * pstrLibFile, dyn_lib_t ** ppLib);

u32 freeDynLib(dyn_lib_t ** ppLib);

u32 getSymbolAddress(
    dyn_lib_t * pLib, const olchar_t * pstrSymbol, void ** ppAddress);

#endif /*JIUTAI_DYNLIB_H*/

/*---------------------------------------------------------------------------*/


