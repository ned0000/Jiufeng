/**
 *  @file bitarray-test.c
 *
 *  @brief Test file for bit array object defined in jf_bitarray.h
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
#include "jf_err.h"
#include "jf_bitarray.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */
static boolean_t ls_bSizeof = FALSE;

/* --- private routine section ------------------------------------------------------------------ */
static void _printBitarrayTestUsage(void)
{
    ol_printf("\
Usage: bitarray-test [-h] [-t]\n\
    -h show this usage\n\
    -t test sizeof on local machine\n");
    ol_printf("\n");
}

static u32 _parseBitarrayTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "th?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printBitarrayTestUsage();
            exit(0);
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 't':
            ls_bSizeof = TRUE;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testBitArrayLogical(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_bitarray_t jb1[8], jb2[8], jb3[8], jb4[8];

    JF_BITARRAY_INIT(jb1);
    JF_BITARRAY_INIT(jb2);
    JF_BITARRAY_INIT(jb3);

    jf_bitarray_setBit(jb1, 2);
    jf_bitarray_setBit(jb1, 3);
    jf_bitarray_setBit(jb1, 5);
    jf_bitarray_setBit(jb1, 23);
    jf_bitarray_setBit(jb1, 31);
    jf_bitarray_setBit(jb1, 44);
    jf_bitarray_setBit(jb1, 53);
    jf_bitarray_setBit(jb1, 55);
    jf_bitarray_setBit(jb1, 58);
    jf_bitarray_setBit(jb1, 60);

    ol_printf("jb1:        ");
    JF_BITARRAY_DUMP(jb1);

    jf_bitarray_setBit(jb2, 0);
    jf_bitarray_setBit(jb2, 1);
    jf_bitarray_setBit(jb2, 5);
    jf_bitarray_setBit(jb2, 8);
    jf_bitarray_setBit(jb2, 13);
    jf_bitarray_setBit(jb2, 15);
    jf_bitarray_setBit(jb2, 20);
    jf_bitarray_setBit(jb2, 30);
    jf_bitarray_setBit(jb2, 40);
    jf_bitarray_setBit(jb2, 50);
    jf_bitarray_setBit(jb2, 55);

    ol_printf("jb2:        ");
    JF_BITARRAY_DUMP(jb2);

    jf_bitarray_setBit(jb3, 2);
    jf_bitarray_setBit(jb3, 4);
    jf_bitarray_setBit(jb3, 8);
    jf_bitarray_setBit(jb3, 16);
    jf_bitarray_setBit(jb3, 17);
    jf_bitarray_setBit(jb3, 18);
    jf_bitarray_setBit(jb3, 25);
    jf_bitarray_setBit(jb3, 34);
    jf_bitarray_setBit(jb3, 45);
    jf_bitarray_setBit(jb3, 50);

    ol_printf("jb3:        ");
    JF_BITARRAY_DUMP(jb3);

    ol_printf("jb1 & jb2:  ");
    JF_BITARRAY_AND(jb3, jb1, jb2);
    JF_BITARRAY_DUMP(jb3);

    ol_printf("jb1 | jb2:  ");
    JF_BITARRAY_OR(jb3, jb1, jb2);
    JF_BITARRAY_DUMP(jb3);

    ol_printf("jb1 ^ jb2:  ");
    JF_BITARRAY_XOR(jb3, jb1, jb2);
    JF_BITARRAY_DUMP(jb3);

    ol_printf("not jb1:    ");
    JF_BITARRAY_NOT(jb3, jb1);
    JF_BITARRAY_DUMP(jb3);

    ol_printf("copy jb1:   ");
    JF_BITARRAY_COPY(jb4, jb1);
    JF_BITARRAY_DUMP(jb4);

    return u32Ret;
}

static u32 _testBitArray(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_bitarray_t jb[8];
    u32 u32SetPos[] = {2, 3, 6, 8, 10, 13, 16, 18, 20, 23, 24, 27, 31, 32,
                       35, 37, 39, 40, 42, 43, 44, 47, 48, 50, 55, 57, 60};
    u32 u32NumOfSetPos = sizeof(u32SetPos) / sizeof(u32);
    u32 u32TestPos[] = {2, 3, 7, 8, 14, 20, 40};
    u32 u32NumOfTestPos = sizeof(u32TestPos) / sizeof(u32);
    u32 u32ClearPos[] = {2, 3, 8, 20, 48, 55, 57, 60};
    u32 u32NumOfClearPos = sizeof(u32ClearPos) / sizeof(u32);
    u32 u32LShift[] = {1, 4, 7, 8, 10, 30};
    u32 u32NumOfLShift = sizeof(u32LShift) / sizeof(u32);
    u32 u32RShift[] = {2, 3, 8, 20, 32};
    u32 u32NumOfRShift = sizeof(u32RShift) / sizeof(u32);
    u32 u32Index;
    boolean_t bRet;

    ol_printf("init bit array\n");
    JF_BITARRAY_INIT(jb);
    JF_BITARRAY_DUMP(jb);

    ol_printf("set bit array\n");
    JF_BITARRAY_SET(jb);
    JF_BITARRAY_DUMP(jb);

    ol_printf("clear bit array\n");
    JF_BITARRAY_INIT(jb);
    JF_BITARRAY_DUMP(jb);

    for (u32Index = 0; u32Index < u32NumOfSetPos; u32Index ++)
    {
        ol_printf("set position %u\n", u32SetPos[u32Index]);
        jf_bitarray_setBit(jb, u32SetPos[u32Index]);

        JF_BITARRAY_DUMP(jb);
    }

    for (u32Index = 0; u32Index < u32NumOfTestPos; u32Index ++)
    {
        bRet = jf_bitarray_testBit(jb, u32TestPos[u32Index]);
        ol_printf(
            "test position %u, %s\n", u32TestPos[u32Index],
            jf_string_getStringTrue(bRet));
    }

    for (u32Index = 0; u32Index < u32NumOfClearPos; u32Index ++)
    {
        ol_printf("clear position %u\n", u32ClearPos[u32Index]);
        jf_bitarray_clearBit(jb, u32ClearPos[u32Index]);

        JF_BITARRAY_DUMP(jb);
    }

    for (u32Index = 0; u32Index < u32NumOfLShift; u32Index ++)
    {
        ol_printf("set position %u\n", 14);
        jf_bitarray_setBit(jb, 14);

        JF_BITARRAY_DUMP(jb);

        ol_printf("left shift %u\n", u32LShift[u32Index]);
        JF_BITARRAY_LSHIFT(jb, u32LShift[u32Index]);
            
        JF_BITARRAY_DUMP(jb);
    }

    for (u32Index = 0; u32Index < u32NumOfRShift; u32Index ++)
    {
        ol_printf("set position %u\n", 4);
        jf_bitarray_setBit(jb, 4);

        JF_BITARRAY_DUMP(jb);

        ol_printf("right shift %u\n", u32RShift[u32Index]);
        JF_BITARRAY_RSHIFT(jb, u32RShift[u32Index]);
            
        JF_BITARRAY_DUMP(jb);
    }

    ol_printf("set position %u\n", 7);
    jf_bitarray_setBit(jb, 7);

    JF_BITARRAY_DUMP(jb);

    for (u32Index = 0; u32Index < 19; u32Index ++)
    {
        ol_printf("%u, increment bit array\n", u32Index + 1);
        JF_BITARRAY_INCREMENT(jb);

        JF_BITARRAY_DUMP(jb);
    }

    for (u32Index = 0; u32Index < 20; u32Index ++)
    {
        ol_printf("%u, decrement bit array\n", u32Index + 1);
        JF_BITARRAY_DECREMENT(jb);

        JF_BITARRAY_DUMP(jb);
    }

    return u32Ret;
}

static void _testSizeof(void)
{
    jf_bitarray_t a[20];
    jf_bitarray_t b[6][40];

    ol_printf("a[20]\n");
    ol_printf("sizeof(a) = %d\n", (s32)sizeof(a));
    ol_printf("sizeof(a[0]) = %d\n", (s32)sizeof(a[0]));

    ol_printf("b[6][40]\n");
    ol_printf("sizeof(b) = %d\n", (s32)sizeof(b));
    ol_printf("sizeof(b[0]) = %d\n", (s32)sizeof(b[0]));
    ol_printf("sizeof(b[1]) = %d\n", (s32)sizeof(b[1]));
    ol_printf("sizeof(b[1][1]) = %d\n", (s32)sizeof(b[1][1]));
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseBitarrayTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bSizeof)
        {
            _testSizeof();
        }
        else
        {
            _testBitArray();

            _testBitArrayLogical();
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

