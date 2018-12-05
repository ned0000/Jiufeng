/**
 *  @file cghash.h
 *
 *  @brief cryptographic hash function
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_CGHASH_H
#define JIUFENG_CGHASH_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

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

/* --- data structures ----------------------------------------------------- */


/* --- functional routines ------------------------------------------------- */
#define MD5_DIGEST_LEN   (16)

typedef struct
{
    u8 m_u8Ctx[120];
} md5_t;

CGHASHAPI u32 CGHASHCALL initMd5(md5_t * pMd5);
CGHASHAPI u32 CGHASHCALL updateMd5(
    md5_t * pMd5, const u8 * pu8Buffer, u32 u32Len);
CGHASHAPI u32 CGHASHCALL finalMd5(md5_t * pMd5, u8 u8Digest[MD5_DIGEST_LEN]);

/** More convenient function to do md5 digest
 */
CGHASHAPI u32 CGHASHCALL doMd5(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[MD5_DIGEST_LEN]);


#define SHA1_DIGEST_LEN   (20)

typedef struct
{
    u8 s_u8Ctx[160];
} sha1_t;

CGHASHAPI u32 CGHASHCALL initSha1(sha1_t * pSha1);
CGHASHAPI u32 CGHASHCALL updateSha1(
    sha1_t * pSha1, const u8 * pu8Buffer, u32 u32Len);
CGHASHAPI u32 CGHASHCALL finalSha1(
    sha1_t * pSha1, u8 u8Digest[SHA1_DIGEST_LEN]);

/** More convenient function to do sha1 digest
 */
CGHASHAPI u32 CGHASHCALL doSha1(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[SHA1_DIGEST_LEN]);

#endif /*JIUFENG_CGHASH_H*/

/*---------------------------------------------------------------------------*/




