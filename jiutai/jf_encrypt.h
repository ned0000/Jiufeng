/**
 *  @file jf_encrypt.h
 *
 *  @brief Encryption/decryption header file which provides fucntions to encrypt and decrypt file
 *   and string.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_encrypt library.
 *  -# On Linux platform, link with ssl and crypto library.
 *  -# Link with olstringparse and olfiles library
 *  -# Use AES as the encrypt algorithm, the length of the encrypt key should be 16 bytes long.
 *
 */

#ifndef JIUFENG_ENCRYPT_H
#define JIUFENG_ENCRYPT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

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

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Encrypt file.
 *
 *  @param pSrcFile [in] File to be encrypted.
 *  @param pDestFile [in] Generated encrypted file.
 *  @param pKey [in] Encryption key, the length of pKey array must >= 8. But only first 7 bytes are
 *   used.
 *
 *  @return The error code.
 */
u32 jf_encrypt_encryptFile(
    olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey);

/** Decrypt file.
 *
 *  @param pSrcFile [in] File to be decrypted.
 *  @param pDestFile [in] Generated decrypted file.
 *  @param pKey [in] Encryption key.
 *
 *  @return The error code.
 */
u32 jf_encrypt_decryptFile(olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey);

/** Encrypt a string, the encrypted string is convert to another string.
 *
 *  @param pSrcStr [in] String to be encrypted.
 *  @param ppDestStr [out] Generated encrypted string.
 *  @param pKey [in] Encryption key.
 *
 *  @return The error code.
 */
u32 jf_encrypt_encryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey);

/** Decrypt to generate original string.
 *
 *  @param pSrcStr [in] String to be decrypted.
 *  @param ppDestStr [out] Generated original string.
 *  @param pKey [in] Encryption key.
 *
 *  @return The error code.
 */
u32 jf_encrypt_decryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey);

/** Free string allocated by jf_encrypt_encryptString() and jf_encrypt_decryptString().
 *
 *  @param ppStr [in/out] String to free.
 *
 *  @return Void.
 */
void jf_encrypt_freeString(olchar_t ** ppStr);

#endif /*JIUFENG_ENCRYPT_H*/

/*------------------------------------------------------------------------------------------------*/




