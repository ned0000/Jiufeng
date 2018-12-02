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

/** Function to do md5 digest
 */
CGHASHAPI u32 CGHASHCALL doMd5(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[MD5_DIGEST_LEN]);


#define SHA1_DIGEST_LEN   (20)

/** Function to do sha1 digest
 */
CGHASHAPI u32 CGHASHCALL doSha1(
    const u8 * pu8Input, u32 u32InputLen, u8 u8Digest[SHA1_DIGEST_LEN]);

#endif /*JIUFENG_CGHASH_H*/

/*---------------------------------------------------------------------------*/




