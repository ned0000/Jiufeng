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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_uuid.h"
#include "jf_prng.h"
#include "jf_jiukun.h"
#include "jf_logger.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */
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

/* --- private routine section ------------------------------------------------------------------ */

static void _printGenUuidUsage(void)
{
    ol_printf("\
Usage: genuuid [-v version] [-f output format] [-c count] [version 1 option] [version 3/5 option]\n\
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

static u32 _parseGenUuidOptv(olchar_t * pstrOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    u32Ret = jf_option_getU32FromString(optarg, &u32Value);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((u32Value != JF_UUID_VER_1) && (u32Value != JF_UUID_VER_3) &&
            (u32Value != JF_UUID_VER_4) && (u32Value != JF_UUID_VER_5))
            u32Ret = JF_ERR_INVALID_PARAM;
        else
            ls_juvVersion = (jf_uuid_ver_t)u32Value;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        ol_printf("Invalid version (1(default)|3|4|5) \n");

    return u32Ret;
}

static u32 _parseGenUuidOptf(olchar_t * pstrOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ol_strncmp(pstrOpt, "binary", 6) == 0)
    {
        ls_jufFormat = JF_UUID_FMT_BIN;
    }
    else if (ol_strncmp(pstrOpt, "string", 6) == 0)
    {
        ls_jufFormat = JF_UUID_FMT_STR;
    }
    else if (ol_strncmp(pstrOpt, "hexadecimal", 11) == 0)
    {
        ls_jufFormat = JF_UUID_FMT_HEX;
    }
    else if (ol_strncmp(pstrOpt, "siv", 3) == 0)
    {
        ls_jufFormat = JF_UUID_FMT_SIV;
    }
    else
    {
        ol_printf("Invalid format (string(default)|binary|hexadecimal|siv)\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    return u32Ret;
}

static u32 _parseGenUuidCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "n:c:v:f:m?h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printGenUuidUsage();
            exit(0);
        case 'n':
            ls_pstrName = (char *)optarg;
            break;
        case 'c':
            u32Ret = jf_option_getU32FromString(optarg, &ls_u32NumOfUuid);
            break;
        case 'v':
            u32Ret = _parseGenUuidOptv(optarg);
            break;
        case 'f':
            u32Ret = _parseGenUuidOptf(optarg);
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

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Uuid[50];
    jf_uuid_param_t jup;
    olchar_t strErrMsg[200];
    u32 u32Index;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "ARCHIVE";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseGenUuidCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_prng_init();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_bzero(&jup, sizeof(jf_uuid_param_t));
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

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 200);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

