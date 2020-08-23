/**
 *  @file genuuid.c
 *
 *  @brief Application for generating UUID.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


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

/** Default UUID version.
 */
static jf_uuid_ver_t ls_juvVersion = JF_UUID_VER_1;

/** Default UUID format.
 */
static jf_uuid_fmt_t ls_jufFormat = JF_UUID_FMT_STR;

/** Multicast the MAC address if it's TRUE.
 */
static boolean_t ls_bMulticastMac = FALSE;

/** Number of UUID to be generated.
 */
static u32 ls_u32NumOfUuid = 1;

/** Name string.
 */
static olchar_t * ls_pstrName = NULL;

/** Name string is an X.500 DN (in DER or a text output format)
 */
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
Usage: jf_genuuid [general options] [version 1 options] [version 3/5 options] [logger options]\n\
general options: [-v <1|3|4|5>] [-f <string|binary|hexadecimal|siv>] [-c count] \n\
  -v: version, default is 1.\n\
  -f: output format, default is 'string'.\n\
  -c: number of UUID should be generated, default is 1.\n\
version 1 options: [-m] \n\
  -m: multicast MAC address.\n\
version 3/5 options: [-n] \n\
  -n: name.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n");

    ol_printf("\n");
}

static u32 _parseGenUuidOptv(olchar_t * pstrOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    u32Ret = jf_option_getU32FromString(jf_option_getArg(), &u32Value);
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

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "n:c:v:f:m?hT:F:S:O")) != -1))
    {
        switch (nOpt)
        {
        case ':':
        case '?':
        case 'h':
            _printGenUuidUsage();
            exit(0);
        case 'n':
            ls_pstrName = jf_option_getArg();
            break;
        case 'c':
            u32Ret = jf_option_getU32FromString(jf_option_getArg(), &ls_u32NumOfUuid);
            break;
        case 'v':
            u32Ret = _parseGenUuidOptv(jf_option_getArg());
            break;
        case 'f':
            u32Ret = _parseGenUuidOptf(jf_option_getArg());
            break;
        case 'm':
            ls_bMulticastMac = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(jf_option_getArg(), &pjlip->jlip_sLogFile);
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
    u32 u32Index = 0;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "GENUUID";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_INFO;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseGenUuidCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_uuid_init();
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

                jf_uuid_fini();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 200);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
