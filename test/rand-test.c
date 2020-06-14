/**
 *  @file rand-test.c
 *
 *  @brief Test file for rand operation defined in jf_rand common object.
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
#include "jf_rand.h"
#include "jf_string.h"
#include "jf_logger.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

static void _printRandTestUsage(void)
{
    ol_printf("\
Usage: randnum-test [-h] \n\
    -h print the usage\n");
    ol_printf("\n");
}

static u32 _parseRandTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printRandTestUsage();
            exit(0);
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

static u32 testRandomNumberInRange(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Lower = 10;
    u32 u32Upper = 110;
    u32 u32Value;
    olint_t i;

    for (i = 0; i < 10; i ++)
    {
        u32Value = jf_rand_getU32InRange(u32Lower, u32Upper);
        ol_printf("%u\n", u32Value);
        if ((u32Value < u32Lower) && (u32Value > u32Upper))
        {
            ol_printf("Invalid random number");
            break;
        }
    }
    ol_printf("\n");

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseRandTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = testRandomNumberInRange();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
