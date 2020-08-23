/**
 *  @file prng-test.c
 *
 *  @brief Test file for pseudo random number generator defined in jf_prng library.
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
#include "jf_hex.h"
#include "jf_string.h"
#include "jf_prng.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

boolean_t ls_bMd5 = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printPrngTestUsage(void)
{
    ol_printf("\
Usage: prng-test [-h]\n\
  -h: show this usage.\n");

    ol_printf("\n");
}

static u32 _parsePrngTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "h?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printPrngTestUsage();
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

static u32 _testPrng(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Loop;
    boolean_t bDup = FALSE;
#define PRNG_DATA_COUNT   (100)
    u8 * u8Random[100];
    u32 u32Size = 64;

    ol_bzero(u8Random, sizeof(u8Random));

    u32Ret = jf_prng_init();
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0;
             (u32Index < PRNG_DATA_COUNT) && (u32Ret == JF_ERR_NO_ERROR);
             u32Index ++)
        {
            u32Ret = jf_jiukun_allocMemory((void **)&u8Random[u32Index], u32Size);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_printf("Get prng data: \n");
                u32Ret = jf_prng_getData(u8Random[u32Index], u32Size);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    jf_hex_dumpByteDataBuffer(u8Random[u32Index], u32Size);
                }
            }

            for (u32Loop = 0; u32Loop < u32Index; u32Loop ++)
            {
                if (ol_memcmp(u8Random[u32Index], u8Random[u32Loop], u32Size) == 0)
                {
                    bDup = TRUE;
                    break;
                }
            }

            if (bDup)
                break;
        }

        jf_prng_fini();
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (bDup)
        {
            ol_printf("Duplicate random data is found\n");
            ol_printf("Test NG\n");
        }
        else
        {
            ol_printf("No duplicate random data is found\n");
            ol_printf("Test OK\n");
        }
    }

    for (u32Index = 0; u32Index < PRNG_DATA_COUNT; u32Index ++)
    {
        if (u8Random[u32Index] != NULL)
            jf_jiukun_freeMemory((void **)&u8Random[u32Index]);
    }
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parsePrngTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testPrng();

            jf_jiukun_fini();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
