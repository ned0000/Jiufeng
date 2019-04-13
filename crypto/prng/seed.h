/**
 *  @file seed.h
 *
 *  @brief to get seed for OS
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef CRYPTO_PRNG_SEED_H
#define CRYPTO_PRNG_SEED_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */
u32 getSeed(void);

#if defined(LINUX)
u32 getSeedFromEgd(const u8 * pu8Path, u8 * pu8Buf, u32 * pu32Bytes);
#endif

#endif /*CRYPTO_PRNG_SEED_H*/

/*------------------------------------------------------------------------------------------------*/


