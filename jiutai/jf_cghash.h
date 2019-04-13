/**
 *  @file jf_cghash.h
 *
 *  @brief cryptographic hash function
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_cghash library
 *  
 */

#ifndef JIUFENG_CGHASH_H
#define JIUFENG_CGHASH_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#undef CGHASHAPI
#undef CGHASHCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_CGHASH_DLL)
        #define CGHASHAPI  __declspec(dllexport)
        #define CGHASHCALL
    #else
        #define CGHASHAPI
        #define CGHASHCALL __cdecl
    #endif
#else
    #define CGHASHAPI
    #define CGHASHCALL
#endif

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */
#define JF_CGHASH_MD5_DIGEST_LEN   (16)

typedef struct
{
    u8 jcm_u8Ctx[120];
} jf_cghash_md5_t;

CGHASHAPI void CGHASHCALL jf_cghash_initMd5(jf_cghash_md5_t * pMd5);
CGHASHAPI void CGHASHCALL jf_cghash_updateMd5(
    jf_cghash_md5_t * pMd5, const u8 * pu8Buffer, u32 u32Len);
CGHASHAPI void CGHASHCALL jf_cghash_finalMd5(
    jf_cghash_md5_t * pMd5, u8 u8Digest[JF_CGHASH_MD5_DIGEST_LEN]);

/** More convenient function to do md5 digest
 */
CGHASHAPI void CGHASHCALL jf_cghash_doMd5(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[JF_CGHASH_MD5_DIGEST_LEN]);


#define JF_CGHASH_SHA1_DIGEST_LEN   (20)

typedef struct
{
    u8 jcs_u8Ctx[160];
} jf_cghash_sha1_t;

CGHASHAPI void CGHASHCALL jf_cghash_initSha1(jf_cghash_sha1_t * pSha1);
CGHASHAPI u32 CGHASHCALL jf_cghash_updateSha1(
    jf_cghash_sha1_t * pSha1, const u8 * pu8Buffer, u32 u32Len);
CGHASHAPI u32 CGHASHCALL jf_cghash_finalSha1(
    jf_cghash_sha1_t * pSha1, u8 u8Digest[JF_CGHASH_SHA1_DIGEST_LEN]);

/** More convenient function to do sha1 digest
 */
CGHASHAPI u32 CGHASHCALL jf_cghash_doSha1(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[JF_CGHASH_SHA1_DIGEST_LEN]);

#endif /*JIUFENG_CGHASH_H*/

/*------------------------------------------------------------------------------------------------*/




