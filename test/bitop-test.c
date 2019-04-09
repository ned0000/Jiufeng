/**
 *  @file bitop-test.c
 *
 *  @brief test file for bit op object file
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "hexstr.h"
#include "jf_bitop.h"
#include "jf_string.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: bitop-test [-h]\n\
         -h show this usage\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
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

static olchar_t * _printInBinary64(u64 u64Value)
{
    olint_t i;
    u64 u64Mask;
    static olchar_t str[200];
    olchar_t * pstr;

    pstr = str;

    for (i = BITS_PER_U64 - 1; i >= 0; i --)
    {
        u64Mask = (u64)1 << i;
        if (u64Value & u64Mask)
            *pstr = '1';
        else
            *pstr = '0';

        pstr ++;
    }

    *pstr = '\0';

    return str;
}

static u32 _testBitOp(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 word = 0x44443098ull, temp;
    u32 u32Index;

    u32 u32BitopMask[] = {31, 30, 24, 23, 20, 18, 16, 13, 10, 8, 6, 3, 2, 0};
    u32 u32NumOfBitopMask = sizeof(u32BitopMask) / sizeof(u32);

    u32 u32BitopMask64[] = {63, 60, 47, 45, 31, 30, 24, 23, 20, 18, 16, 13, 10, 8, 6, 3, 2, 0};
    u32 u32NumOfBitopMask64 = sizeof(u32BitopMask64) / sizeof(u32);

    u32 u32BitopSetLeft[] = {31, 24, 23, 20, 18, 13, 8, 3, 2};
    u32 u32BitopSetRight[] = {30, 23, 18, 16, 13, 10, 6, 2, 0};
    u32 u32BitopSetValue[] = {235, 23, 18, 16, 13, 10, 6, 19, 16};
    u32 u32BitopSet = sizeof(u32BitopSetLeft) / sizeof(u32);

    u32 u32BitopSetBitPos[] = {23, 20, 18, 13};
    u32 u32BitopSetBit = sizeof(u32BitopSetBitPos) / sizeof(u32);

    u32 u32BitopGetLeft[] = {31, 25, 23, 20, 18, 13, 8, 3, 2};
    u32 u32BitopGetRight[] = {28, 23, 23, 16, 14, 1, 6, 2, 0};
    u32 u32BitopGet = sizeof(u32BitopGetLeft) / sizeof(u32);

    u32 u32BitopGetBitPos[] = {23, 20, 18, 13};
    u32 u32BitopGetBit = sizeof(u32BitopGetBitPos) / sizeof(u32);

    u32 u32BitopClearLeft[] = {30, 28, 23, 22, 18, 20, 8, 3, 2};
    u32 u32BitopClearRight[] = {30, 23, 18, 15, 12, 10, 6, 2, 0};
    u32 u32BitopClear = sizeof(u32BitopClearLeft) / sizeof(u32);

    u32 u32BitopClearBitPos[] = {30, 28, 23, 21, 15, 13, 7, 3, 2};
    u32 u32BitopClearBit = sizeof(u32BitopClearBitPos) / sizeof(u32);

    for (u32Index = 0; u32Index < u32NumOfBitopMask; u32Index += 2)
    {
        ol_printf("JF_BITOP_MASK(64, %2u, %2u): %s\n", u32BitopMask[u32Index],
            u32BitopMask[u32Index + 1],
            _printInBinary64(JF_BITOP_MASK(word, u32BitopMask[u32Index],
                                      u32BitopMask[u32Index + 1])));
    }

    ol_printf("\n");
    for (u32Index = 0; u32Index < u32NumOfBitopMask64; u32Index += 2)
    {
        ol_printf("JF_BITOP_MASK(64, %2u, %2u): %s\n", u32BitopMask64[u32Index],
            u32BitopMask64[u32Index + 1],
            _printInBinary64(JF_BITOP_MASK(u64Word, u32BitopMask64[u32Index],
                                        u32BitopMask64[u32Index + 1])));
    }

    ol_printf("\nword: %101s\n", _printInBinary64(word));
    for (u32Index = 0; u32Index < u32BitopSet; u32Index ++)
    {
        temp = word;
        JF_BITOP_SET(temp, u32BitopSetLeft[u32Index],
                  u32BitopSetRight[u32Index], u32BitopSetValue[u32Index]);

        ol_printf("JF_BITOP_SET(%12llu, %2u, %2u, %8u): %s\n", word,
            u32BitopSetLeft[u32Index], u32BitopSetRight[u32Index],
            u32BitopSetValue[u32Index], _printInBinary64(temp));
    }

    ol_printf("\nword: %91s\n", _printInBinary64(word));
    for (u32Index = 0; u32Index < u32BitopGet; u32Index ++)
    {
        ol_printf("JF_BITOP_GET(%12llu, %2u, %2u): %42s\n", word,
            u32BitopGetLeft[u32Index], u32BitopGetRight[u32Index],
            _printInBinary64(JF_BITOP_GET(word, u32BitopGetLeft[u32Index],
                               u32BitopGetRight[u32Index])));
    }

    ol_printf("\nword: %91s\n", _printInBinary64(word));
    for (u32Index = 0; u32Index < u32BitopSetBit; u32Index ++)
    {
        temp = word;
        JF_BITOP_SET_BIT(temp, u32BitopSetBitPos[u32Index]);

        ol_printf("JF_BITOP_SET_BIT(%12llu, %2u): %42s\n", word,
            u32BitopSetBitPos[u32Index], _printInBinary64(temp));
    }

    for (u32Index = 0; u32Index < u32BitopGetBit; u32Index ++)
    {
        ol_printf("JF_BITOP_GET_BIT(%12llu, %2u): %u\n", word,
            u32BitopGetBitPos[u32Index],
            (u32)JF_BITOP_GET_BIT(word, u32BitopGetBitPos[u32Index]));
    }

    ol_printf("\nword: %93s\n", _printInBinary64(word));
    for (u32Index = 0; u32Index < u32BitopClear; u32Index ++)
    {
        temp = word;
        JF_BITOP_CLEAR(temp, u32BitopClearLeft[u32Index],
                    u32BitopClearRight[u32Index]);

        ol_printf("JF_BITOP_CLEAR(%12llu, %2u, %2u): %40s\n", word,
            u32BitopClearLeft[u32Index], u32BitopClearRight[u32Index],
            _printInBinary64(temp));
    }

    ol_printf("\nword: %93s\n", _printInBinary64(word));
    for (u32Index = 0; u32Index < u32BitopClearBit; u32Index ++)
    {
        temp = word;
        JF_BITOP_CLEAR_BIT(temp, u32BitopClearBitPos[u32Index]);

        ol_printf("JF_BITOP_CLEAR_BIT(%12llu, %2u): %40s\n", word,
            u32BitopClearBitPos[u32Index], _printInBinary64(temp));
    }

    return u32Ret;
}


/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testBitOp();
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

