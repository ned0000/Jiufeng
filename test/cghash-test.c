/**
 *  @file cghash-test.c
 *
 *  @brief test file for testing cghash library
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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "cghash.h"
#include "hexstr.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bMd5 = FALSE;
static boolean_t ls_bSha1 = FALSE;
static olchar_t * ls_pstrSource = NULL;
static boolean_t ls_bVerbose = FALSE;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: cghash-test [-m] [-s] [-a string] [-h] \n\
         -a hash string by sha1\n\
         -m test MD5\n\
         -s test sha1\n\
         -h show this usage\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "va:msh?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'v':
            ls_bVerbose = TRUE;
            break;
        case 'a':
            ls_pstrSource = optarg;
            break;
        case 'h':
            _printUsage();
            break;
        case 'm':
            ls_bMd5 = TRUE;
            break;
        case 's':
            ls_bSha1 = TRUE;
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testMd5(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 u8Digest[MD5_DIGEST_LEN];
    olchar_t str[MD5_DIGEST_LEN * 2 + 1];

    ol_printf("Testing MD5\n");
    ol_printf("test case 1\n");

    doMd5((u8 *)"Jefe", 4, u8Digest);
    getStringHex(str, sizeof(str), u8Digest, MD5_DIGEST_LEN);

    str[MD5_DIGEST_LEN * 2] = '\0';

    if (strncmp(
            str, "0f71e3c00b7b8162c6d7ef6edc738aa0", MD5_DIGEST_LEN * 2) == 0)
        ol_printf("MD5 succeeds\n");
    else
        ol_printf("MD5 fails\n");

    ol_printf("test case 2\n");

    doMd5((u8 *)"Doing a market survey.", 22, u8Digest);
    getStringHex(str, sizeof(str), u8Digest, MD5_DIGEST_LEN);

    str[MD5_DIGEST_LEN * 2] = '\0';

    if (strncmp(str, "fefef3a5df1fc52ff2548b5586f4daf7", MD5_DIGEST_LEN * 2) == 0)
        ol_printf("MD5 succeeds\n");
    else
        ol_printf("MD5 fails\n");

    return u32Ret;
}

static u32 _testSha1(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 u8Digest[SHA1_DIGEST_LEN];
    olchar_t str[SHA1_DIGEST_LEN * 2 + 1];

    ol_printf("Testing SHA1\n");

    ol_printf("test case 1\n");

    u32Ret = doSha1((u8 *)"Jefe", 4, u8Digest);
    if (u32Ret == OLERR_NO_ERROR)
    {
        getStringHex(str, sizeof(str), u8Digest, SHA1_DIGEST_LEN);

        str[SHA1_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "cb5551f403fac5fd3d6d1b6329993c3848c468ce",
                SHA1_DIGEST_LEN * 2) == 0)
            ol_printf("SHA1 succeeds\n");
        else
            ol_printf("SHA1 fails\n");
    }

    ol_printf("test case 2\n");

    u32Ret = doSha1((u8 *)"Doing a market survey.", 22, u8Digest);
    if (u32Ret == OLERR_NO_ERROR)
    {
        getStringHex(str, sizeof(str), u8Digest, SHA1_DIGEST_LEN);

        str[SHA1_DIGEST_LEN * 2] = '\0';

        if (strncmp(str, "b8b264042b227b11618aa65dd1dbe1bf807f3059",
                SHA1_DIGEST_LEN * 2) == 0)
            ol_printf("SHA1 succeeds\n");
        else
            ol_printf("SHA1 fails\n");
    }

    return u32Ret;
}

static u32 _sha1Str(olchar_t * source)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 u8Digest[SHA1_DIGEST_LEN];
    olchar_t str[SHA1_DIGEST_LEN * 2 + 1];

    ol_printf("source string: %s\n", source);

    u32Ret = doSha1((u8 *)source, ol_strlen(source), u8Digest);
    if (u32Ret == OLERR_NO_ERROR)
    {
        getStringHex(str, sizeof(str), u8Digest, SHA1_DIGEST_LEN);

        str[SHA1_DIGEST_LEN * 2] = '\0';

        ol_printf("SHA: %s\n", str);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_pstrSource != NULL)
            u32Ret = _sha1Str(ls_pstrSource);
        else if (ls_bMd5)
            u32Ret = _testMd5();
        else if (ls_bSha1)
            u32Ret = _testSha1();
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUsage();
        }
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/
