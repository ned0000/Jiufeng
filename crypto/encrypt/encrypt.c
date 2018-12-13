/**
 *  @file encrypt.c
 *
 *  @brief Encryption header file, provide function to en/decrypt file and
 *   string
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "encrypt.h"
#include "xmalloc.h"
#include "stringparse.h"
#include "ollimit.h"
#include "files.h"

/* --- constant definitions ------------------------------------------------ */

/* --- private data/data structure section --------------------------------- */

/* --- private routine section ------------------------------------------------ */

static void _setEncryptIv(u8 * piv)
{
    ol_memset(piv, 'E', AES_BLOCK_SIZE);
}

static u32 _setEncryptKey(olchar_t * pKey, AES_KEY * pAesKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    int len = ol_strlen(pKey);
    int ret;

    if (len == 16)
    {
        ret = AES_set_encrypt_key((u8 *)pKey, len * BITS_PER_U8, pAesKey);
        if (ret != 0)
            u32Ret = OLERR_INVALID_ENCRYPT_KEY;
    }
    else
    {
        u32Ret = OLERR_INVALID_ENCRYPT_KEY;
    }

    return u32Ret;
}

static u32 _setDecryptKey(olchar_t * pKey, AES_KEY * pAesKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    int len = ol_strlen(pKey);
    int ret;

    if (len == 16)
    {
        ret = AES_set_decrypt_key((u8 *)pKey, len * BITS_PER_U8, pAesKey);
        if (ret != 0)
            u32Ret = OLERR_INVALID_DECRYPT_KEY;
    }
    else
    {
        u32Ret = OLERR_INVALID_DECRYPT_KEY;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 encryptFile(olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    AES_KEY aeskey;
    FILE * fpSrc = NULL;
    FILE * fpDest = NULL;
    u8 iv[AES_BLOCK_SIZE];
    olchar_t * pBuf = NULL;
    file_stat_t filestat;
    olsize_t sread;

    u32Ret = _setEncryptKey(pKey, &aeskey);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xmalloc((void **)&pBuf, MAX_DATA_TRANSFER_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getFileStat(pSrcFile, &filestat);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpOpenFile(pSrcFile, "r", &fpSrc);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpOpenFile(pDestFile, "w", &fpDest);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpWriten(fpDest, (void *)&filestat.fs_u64Size, sizeof(u64));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _setEncryptIv(iv);
        do
        {
            sread = MAX_DATA_TRANSFER_SIZE;
            u32Ret = fpReadn(fpSrc, pBuf, &sread);
            if (u32Ret == OLERR_NO_ERROR)
            {
                sread = ALIGN(sread, 16);
                AES_cbc_encrypt(
                    (u8 *)pBuf, (u8 *)pBuf, sread, &aeskey, iv, AES_ENCRYPT);

                u32Ret = fpWriten(fpDest, pBuf, sread);
            }
            
        } while (u32Ret == OLERR_NO_ERROR);
    }

    if (u32Ret == OLERR_END_OF_FILE)
        u32Ret = OLERR_NO_ERROR;

    if (pBuf != NULL)
        xfree((void **)&pBuf);
    if (fpSrc != NULL)
        fpCloseFile(&fpSrc);
    if (fpDest != NULL)
        fpCloseFile(&fpDest);

    return u32Ret; 
}

u32 decryptFile(olchar_t * pSrcFile, olchar_t * pDestFile, olchar_t * pKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    AES_KEY aeskey;
    FILE * fpSrc = NULL;
    FILE * fpDest = NULL;
    u8 iv[AES_BLOCK_SIZE];
    olchar_t * pBuf = NULL;
    olsize_t sread, sleft;
    u64 u64Size = 0;

    u32Ret = _setDecryptKey(pKey, &aeskey);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xmalloc((void **)&pBuf, MAX_DATA_TRANSFER_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpOpenFile(pSrcFile, "r", &fpSrc);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = fpOpenFile(pDestFile, "w", &fpDest);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        sread = sizeof(u64Size);
        u32Ret = fpReadn(fpSrc, (void *)&u64Size, &sread);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _setEncryptIv(iv);
        sleft = (olsize_t)u64Size;
        do
        {
            sread = MAX_DATA_TRANSFER_SIZE;
            u32Ret = fpReadn(fpSrc, pBuf, &sread);
            if (u32Ret == OLERR_NO_ERROR)
            {
                AES_cbc_encrypt(
                    (u8 *)pBuf, (u8 *)pBuf, sread, &aeskey, iv, AES_DECRYPT);

                if (sread > sleft)
                    sread = sleft;
                u32Ret = fpWriten(fpDest, pBuf, sread);
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                if (sleft < sread)
                    break;
                sleft -= sread;
            }            
        } while (u32Ret == OLERR_NO_ERROR);
    }

    if (u32Ret == OLERR_END_OF_FILE)
        u32Ret = OLERR_NO_ERROR;

    if (pBuf != NULL)
        xfree((void **)&pBuf);
    if (fpSrc != NULL)
        fpCloseFile(&fpSrc);
    if (fpDest != NULL)
        fpCloseFile(&fpDest);

    return u32Ret;   
}

u32 encryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    AES_KEY aeskey;
    u8 * pstr = NULL;
    olchar_t * pDestStr = NULL;
    olsize_t len = ol_strlen(pSrcStr);
    olsize_t outlen = ALIGN(len, AES_BLOCK_SIZE);
    u8 iv[AES_BLOCK_SIZE];

    u32Ret = _setEncryptKey(pKey, &aeskey);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xmalloc((void **)&pstr, outlen);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xmalloc((void **)&pDestStr, 2 * outlen + 1);
    }
    
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_bzero(pstr, outlen);
        ol_memcpy(pstr, pSrcStr, len);
        _setEncryptIv(iv);
        AES_cbc_encrypt(pstr, pstr, outlen, &aeskey, iv, AES_ENCRYPT);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        getStringHex(pDestStr, 2 * outlen + 1, pstr, outlen);
    }

    if (pstr != NULL)
        xfree((void **)&pstr);

    if (u32Ret == OLERR_NO_ERROR)
        *ppDestStr = pDestStr;
    else if (pDestStr != NULL)
        xfree((void **)&pDestStr);

    return u32Ret;   
}

u32 decryptString(
    const olchar_t * pSrcStr, olchar_t ** ppDestStr, olchar_t * pKey)
{
    u32 u32Ret = OLERR_NO_ERROR;
    AES_KEY aeskey;
    olchar_t * pDestStr = NULL;
    olsize_t outlen = ol_strlen(pSrcStr) / 2;
    u8 iv[AES_BLOCK_SIZE];

    u32Ret = _setDecryptKey(pKey, &aeskey);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xmalloc((void **)&pDestStr, outlen + 1);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pDestStr[outlen] = '\0';
        getHexFromString(
            pSrcStr, ol_strlen(pSrcStr), (u8 *)pDestStr, outlen);

        _setEncryptIv(iv);
        AES_cbc_encrypt(
            (u8 *)pDestStr, (u8 *)pDestStr, outlen, &aeskey, iv, AES_DECRYPT);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppDestStr = pDestStr;
    else if (pDestStr != NULL)
        xfree((void **)&pDestStr);
    
    return u32Ret; 
}

void freeEncryptString(olchar_t ** ppStr)
{
    xfree((void **)ppStr);
}

/*---------------------------------------------------------------------------*/



