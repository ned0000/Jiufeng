/**
 *  @file jf_prng.h
 *
 *  @brief Header file for pseudo random number generator
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_prng library
 *  @note The prng library is thread safe
 *  @note Link with jf_files library
 *  @note The library is written based on openssl rand function
 */

#ifndef JIUFENG_PRNG_H
#define JIUFENG_PRNG_H

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

#undef PRNGAPI
#undef PRNGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_PRNG_DLL)
        #define PRNGAPI  __declspec(dllexport)
        #define PRNGCALL
    #else
        #define PRNGAPI
        #define PRNGCALL __cdecl
    #endif
#else
    #define PRNGAPI
    #define PRNGCALL
#endif

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

PRNGAPI u32 PRNGCALL jf_prng_init(void);

PRNGAPI u32 PRNGCALL jf_prng_fini(void);

PRNGAPI u32 PRNGCALL jf_prng_getData(u8 * pu8Data, u32 u32Len);

/** Pseudo-random bytes that are guaranteed to be unique but not unpredictable
 */
PRNGAPI u32 PRNGCALL jf_prng_getPseudoData(u8 * pu8Data, u32 u32Len);

#endif /*JIUFENG_PRNG_H*/

/*---------------------------------------------------------------------------*/


