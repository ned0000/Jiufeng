/**
 *  @file genuuid.c
 *
 *  @brief Application for generating UUID
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
#include "errcode.h"
#include "stringparse.h"
#include "uuid.h"
#include "prng.h"

/* --- private data/data structure section --------------------------------- */
static jf_uuid_ver_t ls_juvVersion = JF_UUID_VER_1;
static jf_uuid_fmt_t ls_jufFormat = JF_UUID_FMT_STR;
static boolean_t ls_bMulticastMac = FALSE;
static u32 ls_u32NumOfUuid = 1;
static olchar_t * ls_pstrName = NULL;
/* Name string is an X.500 DN (in DER or a text output format) */
static u8 ls_u8NameSpace_X500[] = { /* 6ba7b814-9dad-11d1-80b4-00c04fd430c8 */
    0x6b, 0xa7, 0xb8, 0x14,
    0x9d, 0xad,
    0x11, 0xd1,
    0x80, 0xb4, 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
};

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: genuuid [-v version] [-f output format] [-c count] \n\
    [version 1 option] [version 3/5 option] \n\
general options: \n\
    -v <1|3|4|5> version, default is 1.\n\
    -f <string|binary|hexadecimal|siv> output format, default is 'string'.\n\
    -c number of UUID should be generated, default is 1.\n\
version 1 option: \n\
    -m multicast MAC address.\n\
version 3/5 option: \n\
    -n name.");
    ol_printf("\n");
}

static u32 _parseOptv(olchar_t * pstrOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    if (sscanf(optarg, "%d", &u32Value) == 1)
    {
        if ((u32Value != JF_UUID_VER_1) && (u32Value != JF_UUID_VER_3) &&
            (u32Value != JF_UUID_VER_4) && (u32Value != JF_UUID_VER_5))
            u32Ret = JF_ERR_INVALID_PARAM;
        else
            ls_juvVersion = (jf_uuid_ver_t)u32Value;
    }
    else
    {
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        ol_printf("Invalid version (1(default)|3|4|5) \n");

    return u32Ret;
}

static u32 _parseOptf(olchar_t * pstrOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (strncmp(pstrOpt, "binary", 6) == 0)
        ls_jufFormat = JF_UUID_FMT_BIN;
    else if (strncmp(pstrOpt, "string", 6) == 0)
        ls_jufFormat = JF_UUID_FMT_STR;
    else if (strncmp(pstrOpt, "hexadecimal", 11) == 0)
        ls_jufFormat = JF_UUID_FMT_HEX;
    else if (strncmp(pstrOpt, "siv", 3) == 0)
        ls_jufFormat = JF_UUID_FMT_SIV;
    else
    {
        ol_printf("Invalid format (string(default)|binary|hexadecimal|siv)\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    return u32Ret;
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value = 0;

    while (((nOpt = getopt(argc, argv,
        "n:c:v:f:m?h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
        case 'n':
            ls_pstrName = (char *)optarg;
            break;
        case 'c':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
                ls_u32NumOfUuid = (u8)u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'v':
            u32Ret = _parseOptv(optarg);
            break;
        case 'f':
            u32Ret = _parseOptf(optarg);
            break;
        case 'm':
            ls_bMulticastMac = TRUE;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

void _outputUuid(u8 * pu8Uuid, jf_uuid_fmt_t format)
{
    u8 u8Index;

    ol_printf("UUID: ");
    if (format == JF_UUID_FMT_BIN)
    {
        for (u8Index = 0; u8Index < JF_UUID_LEN_BIN; u8Index ++)
        {
            ol_printf("0x%X ", pu8Uuid[u8Index]);
        }
    }
    else
    {
        ol_printf("%s", pu8Uuid);
    }
    ol_printf("\n");
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Uuid[50];
    jf_uuid_param_t jup;
    olchar_t strErrMsg[200];
    u32 u32Index;

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_prng_init();
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(&jup, 0, sizeof(jf_uuid_param_t));
        jup.jup_ufFmt = ls_jufFormat;
        jup.jup_bMulticastMac = ls_bMulticastMac;
        jup.jup_pstrName = ls_pstrName;
        jup.jup_pu8NameSpace = ls_u8NameSpace_X500;
        for (u32Index = 0;
             (u32Index < ls_u32NumOfUuid) && (u32Ret == JF_ERR_NO_ERROR);
             u32Index ++)
        {
            u32Ret = jf_uuid_get(u8Uuid, 50, ls_juvVersion, &jup);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                _outputUuid(u8Uuid, ls_jufFormat);
            }
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 200);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

