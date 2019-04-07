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
#include "jf_basic.h"
#include "jf_limit.h"
#include "errcode.h"
#include "cghash.h"
#include "cgmac.h"
#include "md5.h"
#include "sha1.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

u32 jf_cgmac_doHmacMd5(
    const u8 * pu8Key, olsize_t sKey, const u8 * pu8Input, olsize_t sInput,
    u8 u8Digest[JF_CGHASH_MD5_DIGEST_LEN])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 isha[JF_CGHASH_MD5_DIGEST_LEN], osha[JF_CGHASH_MD5_DIGEST_LEN];
    u8 key[JF_CGHASH_MD5_DIGEST_LEN];
    u8 buf[MD5_BLOCK_SIZE];
    u32 u32Index;
    jf_cghash_md5_t md5;

    if (sKey > MD5_BLOCK_SIZE)
    {
        jf_cghash_doMd5(pu8Key, sKey, key);

        pu8Key = key;
        sKey = JF_CGHASH_MD5_DIGEST_LEN;
    }

    jf_cghash_initMd5(&md5);

    /* Pad the key for inner digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x36;

    for (u32Index = sKey; u32Index < MD5_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x36;

    jf_cghash_updateMd5(&md5, buf, MD5_BLOCK_SIZE);
    jf_cghash_updateMd5(&md5, pu8Input, sInput);

    jf_cghash_finalMd5(&md5, isha);

    /* Pad the key for outter digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x5C;
    for (u32Index = sKey; u32Index < MD5_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x5C;

    jf_cghash_updateMd5(&md5, buf, MD5_BLOCK_SIZE) ;
    jf_cghash_updateMd5(&md5, isha, JF_CGHASH_MD5_DIGEST_LEN) ;

    jf_cghash_finalMd5(&md5, osha);

    /* copy the results */
    ol_memcpy(u8Digest, osha, JF_CGHASH_MD5_DIGEST_LEN);

    return u32Ret;
}

u32 jf_cgmac_doHmacSha1(
    const u8 * pu8Key, olsize_t sKey, const u8 * pu8Input, olsize_t sInput,
    u8 u8Digest[JF_CGHASH_SHA1_DIGEST_LEN])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 isha[JF_CGHASH_SHA1_DIGEST_LEN], osha[JF_CGHASH_SHA1_DIGEST_LEN];
    u8 key[JF_CGHASH_SHA1_DIGEST_LEN];
    u8 buf[SHA1_BLOCK_SIZE];
    u32 u32Index;
    jf_cghash_sha1_t sha1;

    if (sKey > SHA1_BLOCK_SIZE)
    {
        jf_cghash_doSha1(pu8Key, sKey, key);

        pu8Key = key;
        sKey = JF_CGHASH_SHA1_DIGEST_LEN;
    }

    jf_cghash_initSha1(&sha1);

    /* Pad the key for inner digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x36;

    for (u32Index = sKey; u32Index < SHA1_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x36;

    jf_cghash_updateSha1(&sha1, buf, SHA1_BLOCK_SIZE);
    jf_cghash_updateSha1(&sha1, pu8Input, sInput);

    jf_cghash_finalSha1(&sha1, isha);

    /* Pad the key for outter digest */
    for (u32Index = 0; u32Index < sKey; ++ u32Index)
        buf[u32Index] = pu8Key[u32Index] ^ 0x5C;
    for (u32Index = sKey; u32Index < SHA1_BLOCK_SIZE; ++ u32Index)
        buf[u32Index] = 0x5C;

    jf_cghash_updateSha1(&sha1, buf, SHA1_BLOCK_SIZE) ;
    jf_cghash_updateSha1(&sha1, isha, JF_CGHASH_SHA1_DIGEST_LEN) ;

    jf_cghash_finalSha1(&sha1, osha);

    /* copy the results */
    ol_memcpy(u8Digest, osha, JF_CGHASH_SHA1_DIGEST_LEN);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


