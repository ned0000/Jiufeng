/**
 *  @file cgmac-test.c
 *
 *  @brief test file for cgmac library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_cgmac.h"
#include "hexstr.h"
#include "jf_string.h"

/* --- private data/data structure section --------------------------------- */
boolean_t ls_bHmacSha1 = FALSE;
boolean_t ls_bHmacMd5 = FALSE;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: cgmac-test [-k] [-a] [-h] \n\
    -h show this usage\n\
    -k test HMAC-SHA1\n\
    -a test HMAC-MD5\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "kah?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'k':
            ls_bHmacSha1 = TRUE;
            break;
        case 'a':
            ls_bHmacMd5 = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testHmacSha1(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Key[100];
    u8 u8Data[100];
    u8 u8Digest[JF_CGHASH_SHA1_DIGEST_LEN];
    olchar_t str[JF_CGHASH_SHA1_DIGEST_LEN * 2 + 1];

    ol_printf("Testing HMAC-SHA1\n");

    ol_printf("test case 1\n");

    u32Ret = jf_cgmac_doHmacSha1(
        (u8 *)"Jefe", 4,
        (u8 *)"what do ya want for nothing?", 28, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_SHA1_DIGEST_LEN);

        str[JF_CGHASH_SHA1_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79",
                JF_CGHASH_SHA1_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-SHA1 succeeds\n");
        else
            ol_printf("HMAC-SHA1 fails\n");
    }

    ol_printf("test case 2\n");

    memset(u8Key, 0xaa, 80);

    u32Ret = jf_cgmac_doHmacSha1(
        u8Key, 80,
        (u8 *)"Test Using Larger Than Block-Size Key - Hash Key First",
        54, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_SHA1_DIGEST_LEN);

        str[JF_CGHASH_SHA1_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "aa4ae5e15272d00e95705637ce8a3b55ed402112",
                JF_CGHASH_SHA1_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-SHA1 succeeds\n");
        else
            ol_printf("HMAC-SHA1 fails\n");
    }

    ol_printf("test case 3\n");

    ol_memset(u8Key, 0xaa, 20);
    ol_memset(u8Data, 0xdd, 50);

    u32Ret = jf_cgmac_doHmacSha1(
        u8Key, 20, u8Data, 50, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_SHA1_DIGEST_LEN);

        str[JF_CGHASH_SHA1_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "125d7342b9ac11cd91a39af48aa17b4f63f175d3",
                JF_CGHASH_SHA1_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-SHA1 succeeds\n");
        else
            ol_printf("HMAC-SHA1 fails\n");
    }

    return u32Ret;
}

static u32 _testHmacMd5(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Key[100];
    u8 u8Data[100];
    u8 u8Digest[JF_CGHASH_MD5_DIGEST_LEN];
    olchar_t str[JF_CGHASH_MD5_DIGEST_LEN * 2 + 1];

    ol_printf("Testing HMAC-MD5\n");

    ol_printf("test case 1\n");

    u32Ret = jf_cgmac_doHmacMd5(
        (u8 *)"Jefe", 4,
        (u8 *) "what do ya want for nothing?", 28, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_MD5_DIGEST_LEN);

        str[JF_CGHASH_MD5_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "750c783e6ab0b503eaa86e310a5db738",
                JF_CGHASH_MD5_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-MD5 succeeds\n");
        else
            ol_printf("HMAC-MD5 fails\n");
    }

    ol_printf("test case 2\n");

    ol_memset(u8Key, 0xaa, 80);

    u32Ret = jf_cgmac_doHmacMd5(
        u8Key, 80,
        (u8 *)"Test Using Larger Than Block-Size Key - Hash Key First",
        54, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_MD5_DIGEST_LEN);

        str[JF_CGHASH_MD5_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "6b1ab7fe4bd7bf8f0b62e6ce61b9d0cd",
                JF_CGHASH_MD5_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-MD5 succeeds\n");
        else
            ol_printf("HMAC-MD5 fails\n");
    }

    ol_printf("test case 3\n");

    ol_memset(u8Key, 0xaa, 16);
    ol_memset(u8Data, 0xdd, 50);

    u32Ret = jf_cgmac_doHmacMd5(
        u8Key, 16, u8Data, 50, u8Digest);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_string_getStringHex(str, sizeof(str), u8Digest, JF_CGHASH_MD5_DIGEST_LEN);

        str[JF_CGHASH_MD5_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "56be34521d144c88dbb8c733f0e8b3f6",
                JF_CGHASH_MD5_DIGEST_LEN * 2) == 0)
            ol_printf("HMAC-MD5 succeeds\n");
        else
            ol_printf("HMAC-MD5 fails\n");
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bHmacSha1)
            u32Ret = _testHmacSha1();
        else if (ls_bHmacMd5)
            u32Ret = _testHmacMd5();
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUsage();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

