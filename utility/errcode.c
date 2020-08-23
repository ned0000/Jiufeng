/**
 *  @file utility/errcode.c
 *
 *  @brief The utility for errcode.
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
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bErrorCode = FALSE;

static u32 ls_u32ErrorCode;

/* --- private routine section ------------------------------------------------------------------ */

static void _printErrcodeUsage(void)
{
    ol_printf("\
Usage: jf_errcode [-c error code] [-h] \n\
    -c print error message for the error code.\n\
    -h print the usage.\n");

    ol_printf("\n");
}

static u32 _scanErrorCode(olchar_t * strcode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ol_strlen(strcode) > 2)
    {
        if (ol_strncasecmp(strcode, "0x", 2) == 0)
            ol_sscanf(strcode + 2, "%x", &ls_u32ErrorCode);
        else
            ol_sscanf(strcode, "%d", &ls_u32ErrorCode);
    }
    else
    {
        ol_sscanf(strcode, "%d", &ls_u32ErrorCode);
    }

    return u32Ret;
}

static u32 _parseErrcodeCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = 0;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "c:h")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printErrcodeUsage();
            exit(u32Ret);
            break;
        case 'c':
            ls_bErrorCode = TRUE;
            u32Ret = _scanErrorCode(jf_option_getArg());
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _printErrorCode(void)
{
    olchar_t msg[512];
    olint_t module = 0, code = 0;
    u32 u32System = 0;
    boolean_t bPos = FALSE;

    module = (ls_u32ErrorCode & JF_ERR_CODE_MODULE_MASK) >> JF_ERR_CODE_MODULE_SHIFT;
    code = ls_u32ErrorCode & JF_ERR_CODE_CODE_MASK;
    u32System = (ls_u32ErrorCode & JF_ERR_CODE_FLAG_SYSTEM);
    if (u32System == 0)
        bPos = FALSE;
    else
        bPos = TRUE;

    ol_printf("Module : 0x%x\n", module);
    ol_printf("Code   : 0x%x\n", code);
    ol_printf("System : %s\n", jf_string_getStringPositive(bPos));

    jf_err_readDescription(ls_u32ErrorCode, msg, sizeof(msg));

    ol_printf("%s\n", msg);
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _parseErrcodeCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bErrorCode)
        {
            _printErrorCode();
        }
        else
        {
            ol_printf("No operation is specified!\n\n");
            _printErrcodeUsage();
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


