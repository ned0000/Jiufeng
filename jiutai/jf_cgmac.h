/**
 *  @file jf_cgmac.h
 *
 *  @brief Header file of keyed-hash message authentication code(HMAC or KHMAC),
 *   HMAC is a type of message authentication code (MAC) calculated using a
 *   specific algorithm involving a cryptographic hash function in combination
 *   with a secret key.
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_cgmac library
 *  
 */

#ifndef JIUFENG_CGMAC_H
#define JIUFENG_CGMAC_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_cghash.h"

#undef CGMACAPI
#undef CGMACCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_CGMAC_DLL)
        #define CGMACAPI  __declspec(dllexport)
        #define CGMACCALL
    #else
        #define CGMACAPI
        #define CGMACCALL __cdecl
    #endif
#else
    #define CGMACAPI
    #define CGMACCALL
#endif

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

CGMACAPI u32 CGMACCALL jf_cgmac_doHmacMd5(
    const u8 * pu8Key, olsize_t sKey, const u8 * pu8Input, olsize_t sInput,
    u8 u8Digest[JF_CGHASH_MD5_DIGEST_LEN]);

CGMACAPI u32 CGMACCALL jf_cgmac_doHmacSha1(
    const u8 * pu8Key, olsize_t sKey, const u8 * pu8Input, olsize_t sInput,
    u8 u8Digest[JF_CGHASH_SHA1_DIGEST_LEN]);

#endif /*JIUFENG_CGMAC_H*/

/*------------------------------------------------------------------------------------------------*/




