/**
 *  @file hmac.c
 *
 *  @brief Implementaion of keyed-hash message authentication code
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "cghash.h"
#include "cgmac.h"
#include "md5.h"
#include "sha1.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

u32 doHmacMd5(const u8 * pu8Key, olsize_t sKey,
    const u8 * pu8Input, olsize_t sInput, u8 u8Digest[MD5_DIGEST_LEN])
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 isha[MD5_DIGEST_LEN], osha[MD5_DIGEST_LEN];
    u8 key[MD5_DIGEST_LEN];
    u8 buf[MD5_BLOCK_SIZE];
    u32 u32Index;
    md5_t md5;

    if (sKey > MD5_BLOCK_SIZE)
    {
        doMd5(pu8Key, sKey, key);

        pu8Key = key ;
        sKey = MD5_DIGEST_LEN ;
    }

    initMd5(&md5);

    /* Pad the key for inner digest */
    for (u32Index = 0 ; u32Index < sKey ; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x36;

    for (u32Index =  sKey; u32Index < MD5_BLOCK_SIZE ; ++ u32Index)
        buf[u32Index] = 0x36;

    updateMd5(&md5, buf, MD5_BLOCK_SIZE);
    updateMd5(&md5, pu8Input, sInput);

    finalMd5(&md5, isha);

    /* Pad the key for outter digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x5C;
    for (u32Index = sKey; u32Index < MD5_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x5C;

    updateMd5(&md5, buf, MD5_BLOCK_SIZE) ;
    updateMd5(&md5, isha, MD5_DIGEST_LEN) ;

    finalMd5(&md5, osha);

    /* copy the results */
    memcpy(u8Digest, osha, MD5_DIGEST_LEN);

    return u32Ret;
}

u32 doHmacSha1(const u8 * pu8Key, olsize_t sKey,
    const u8 * pu8Input, olsize_t sInput, u8 u8Digest[SHA1_DIGEST_LEN])
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 isha[SHA1_DIGEST_LEN], osha[SHA1_DIGEST_LEN];
    u8 key[SHA1_DIGEST_LEN];
    u8 buf[SHA1_BLOCK_SIZE];
    u32 u32Index;
    sha1_t sha1;

    if (sKey > SHA1_BLOCK_SIZE)
    {
        doSha1(pu8Key, sKey, key);

        pu8Key = key ;
        sKey = SHA1_DIGEST_LEN ;
    }

    initSha1(&sha1);

    /* Pad the key for inner digest */
    for (u32Index = 0 ; u32Index < sKey ; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x36;

    for (u32Index =  sKey; u32Index < SHA1_BLOCK_SIZE ; ++ u32Index)
        buf[u32Index] = 0x36;

    updateSha1(&sha1, buf, SHA1_BLOCK_SIZE);
    updateSha1(&sha1, pu8Input, sInput);

    finalSha1(&sha1, isha);

    /* Pad the key for outter digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x5C;
    for (u32Index = sKey; u32Index < SHA1_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x5C;

    updateSha1(&sha1, buf, SHA1_BLOCK_SIZE) ;
    updateSha1(&sha1, isha, SHA1_DIGEST_LEN) ;

    finalSha1(&sha1, osha);

    /* copy the results */
    memcpy(u8Digest, osha, SHA1_DIGEST_LEN);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


