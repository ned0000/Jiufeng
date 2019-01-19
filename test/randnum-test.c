/**
 *  @file randnum-test.c
 *
 *  @brief test file for random number common object
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
#include "randnum.h"
#include "stringparse.h"
#include "logger.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: randnum-test [-h] \n\
    -h print the usage\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
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

static u32 testRandomNumberInRange(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Lower = 10;
    u32 u32Upper = 110;
    u32 u32Value;
    olint_t i;

    for (i = 0; i < 10; i ++)
    {
        u32Value = getRandomU32InRange(u32Lower, u32Upper);
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

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = testRandomNumberInRange();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

