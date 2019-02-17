/**
 *  @file prng.h
 *
 *  @brief Header file for pseudo random number generator
 *
 *  @author Min Zhang
 *
 *  @note The prng library is thread safe
 *  @note Link with olfiles library
 *  @note The library is written based on openssl rand function
 */

#ifndef JIUFENG_PRNG_H
#define JIUFENG_PRNG_H

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

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

PRNGAPI u32 PRNGCALL initPrng(void);

PRNGAPI u32 PRNGCALL finiPrng(void);

PRNGAPI u32 PRNGCALL getPrngData(u8 * pu8Data, u32 u32Len);

/** Pseudo-random bytes that are guaranteed to be unique but not unpredictable
 */
PRNGAPI u32 PRNGCALL getPrngPseudoData(u8 * pu8Data, u32 u32Len);

#endif /*JIUFENG_PRNG_H*/

/*---------------------------------------------------------------------------*/

