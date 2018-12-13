/**
 *  @file encrypt-test.c
 *
 *  @brief test file for encrypt library
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
#include "encrypt.h"
#include "hexstr.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bTestEncryptString = FALSE;
static boolean_t ls_bTestEncryptFile = FALSE;
static olchar_t * ls_pstrSrcFile = NULL;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: encrypt-test [-s] [-f file] [-h] \n\
    -h: show this usage\n\
    -s: encrypt string\n\
    -f: encrypt file\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "sf:h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 's':
            ls_bTestEncryptString = TRUE;
            break;
        case 'f':
            ls_bTestEncryptFile = TRUE;
            ls_pstrSrcFile = (olchar_t *)optarg;
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

static u32 _testEncryptFile(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strEncryptFile[MAX_PATH_LEN];
    olchar_t strDecryptFile[MAX_PATH_LEN];
    olchar_t * pKey = "abcdefghijklmnop";

    ol_sprintf(strEncryptFile, "%s.encrypt", ls_pstrSrcFile);
    ol_sprintf(strDecryptFile, "%s.decrypt", ls_pstrSrcFile);

    u32Ret = encryptFile(ls_pstrSrcFile, strEncryptFile, pKey);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = decryptFile(strEncryptFile, strDecryptFile, pKey);
    }

    return u32Ret;
}

static u32 _testEncryptString(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pstr[] =
        {"hello world!",
         "1234567890123456",
         "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"};
    u32 u32NumOfStr = sizeof(pstr) / sizeof(olchar_t *);
    u32 u32Index;
    olchar_t * pEnStr = NULL;
    olchar_t * pDeStr = NULL;
    olchar_t * pKey = "abcdefghijklmnop";

    for (u32Index = 0; u32Index < u32NumOfStr; u32Index ++)
    {
        pEnStr = pDeStr = NULL;
        ol_printf("Source String: %s\n", pstr[u32Index]);
        u32Ret = encryptString(pstr[u32Index], &pEnStr, pKey);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("Encrypt String: %s\n", pEnStr);
            u32Ret = decryptString(pEnStr, &pDeStr, pKey);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("Decrypt String: %s\n", pDeStr);
            if (ol_strcmp(pstr[u32Index], pDeStr) == 0)
            {
                ol_printf("Test OK\n");
            }
            else
            {
                ol_printf("Test NOK\n");
            }
            ol_printf("\n");
        }

        if (pEnStr != NULL)
            freeEncryptString(&pEnStr);
        if (pDeStr != NULL)
            freeEncryptString(&pDeStr);
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
        if (ls_bTestEncryptString)
            u32Ret = _testEncryptString();
        else if (ls_bTestEncryptFile)
            u32Ret = _testEncryptFile();
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

