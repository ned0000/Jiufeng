/**
 *  @file prngcommon.h
 *
 *  @brief Prng common header file, provide some common data structure and
 *   functional routine
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef CRYPTO_PRNG_COMMON_H
#define CRYPTO_PRNG_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
/** require 256 bits = 32 bytes of randomness
 */
#define ENTROPY_NEEDED   (32)


/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

PRNGAPI u32 PRNGCALL seedPrng(
    const u8 * pu8Data, olint_t u32Len, double dbEntropy);


#endif /*CRYPTO_PRNG_COMMON_H*/

/*---------------------------------------------------------------------------*/

