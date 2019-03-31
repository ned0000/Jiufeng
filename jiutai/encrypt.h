/**
 *  @file encrypt.h
 *
 *  @brief encryption/decryption header file. It provides fucntions to encrypt
 *   and decrypt file and string.
 *
 *  @author Min Zhang
 *
 *  @note On Linux platform, link with ssl and crypto library
 *  @note Link with olstringparse and olfiles library
 *  @note Use AES as the encrypt algorithm, the length of the encrypt key
 *   should be 16 bytes long
 *
 */

#ifndef JIUFENG_ENCRYPT_H
#define JIUFENG_ENCRYPT_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

#undef ENCRYPTAPI
#undef ENCRYPTCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_ENCRYPT_DLL)
        #define ENCRYPTAPI  __declspec(dllexport)
        #define ENCRYPTCALL
    #else
        #define ENCRYPTAPI
        #define ENCRYPTCALL __cdecl
    #endif
#else
    #define ENCRYPTAPI
    #define ENCRYPTCALL
#endif

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */


/* --- functional routines ------------------------------------------------- */

/** Encrypt file
 *
 *  @param pSrcFile [in] file to be encrypted
 *  @param pDestFile [in] generated encrypted file
 *  @param pKey [in] encryption key, the length of pKey array must >= 8. But
 *   only first 7 bytes are used
 *
 *  @return the error code
 */
u32 jf_encrypt_encryptFile(
    olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey);

/** Decrypt file
 *
 *  @param pSrcFile [in] file to be decrypted
 *  @param pDestFile [in] generated decrypted file
 *  @param pKey [in] encryption key
 *
 *  @return the error code
 */
u32 jf_encrypt_decryptFile(olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey);

/** Encrypt a string, the encrypted string is convert to another string
 *
 *  @param pSrcStr [in] string to be encrypted
 *  @param ppDestStr [out] generated encrypted string
 *  @param pKey [in] encryption key
 *
 *  @return the error code
 */
u32 jf_encrypt_encryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey);

/** Decrypt to generate original string
 *
 *  @param pSrcStr [in] string to be decrypted
 *  @param ppDestStr [out] generated original string
 *  @param pKey [in] encryption key
 *
 *  @return the error code
 */
u32 jf_encrypt_decryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey);

/** Free string allocated by encryptString() and decryptString()
 *  @param ppStr [in/out] string to free
 *
 *  @return void
 */
void jf_encrypt_freeString(olchar_t ** ppStr);

#endif /*JIUFENG_ENCRYPT_H*/

/*---------------------------------------------------------------------------*/




