/**
 *  @file bitarray-test.c
 *
 *  @brief test file for bit array object file
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
#include "hexstr.h"
#include "bitarray.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bSizeof = FALSE;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: bitarray-test [-h] [-t]\n\
         -h show this usage\n\
         -t test sizeof on local machine\n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "th?")) != -1) && (u32Ret == OLERR_NO_ERROR))
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
        case 't':
            ls_bSizeof = TRUE;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testBitArrayLogical(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    bit_array_t pba1[8], pba2[8], pba3[8], pba4[8];

    INIT_BIT_ARRAY(pba1);
    INIT_BIT_ARRAY(pba2);
    INIT_BIT_ARRAY(pba3);

    setBitArrayBit(pba1, 2);
    setBitArrayBit(pba1, 3);
    setBitArrayBit(pba1, 5);
    setBitArrayBit(pba1, 23);
    setBitArrayBit(pba1, 31);
    setBitArrayBit(pba1, 44);
    setBitArrayBit(pba1, 53);
    setBitArrayBit(pba1, 55);
    setBitArrayBit(pba1, 58);
    setBitArrayBit(pba1, 60);

    ol_printf("pba1:        ");
    DUMP_BIT_ARRAY(pba1);

    setBitArrayBit(pba2, 0);
    setBitArrayBit(pba2, 1);
    setBitArrayBit(pba2, 5);
    setBitArrayBit(pba2, 8);
    setBitArrayBit(pba2, 13);
    setBitArrayBit(pba2, 15);
    setBitArrayBit(pba2, 20);
    setBitArrayBit(pba2, 30);
    setBitArrayBit(pba2, 40);
    setBitArrayBit(pba2, 50);
    setBitArrayBit(pba2, 55);

    ol_printf("pba2:        ");
    DUMP_BIT_ARRAY(pba2);

    setBitArrayBit(pba3, 2);
    setBitArrayBit(pba3, 4);
    setBitArrayBit(pba3, 8);
    setBitArrayBit(pba3, 16);
    setBitArrayBit(pba3, 17);
    setBitArrayBit(pba3, 18);
    setBitArrayBit(pba3, 25);
    setBitArrayBit(pba3, 34);
    setBitArrayBit(pba3, 45);
    setBitArrayBit(pba3, 50);

    ol_printf("pba3:        ");
    DUMP_BIT_ARRAY(pba3);

    ol_printf("pba1 & pba2: ");
    AND_BIT_ARRAY(pba3, pba1, pba2);
    DUMP_BIT_ARRAY(pba3);

    ol_printf("pba1 | pba2: ");
    OR_BIT_ARRAY(pba3, pba1, pba2);
    DUMP_BIT_ARRAY(pba3);

    ol_printf("pba1 ^ pba2: ");
    XOR_BIT_ARRAY(pba3, pba1, pba2);
    DUMP_BIT_ARRAY(pba3);

    ol_printf("not pba1:    ");
    NOT_BIT_ARRAY(pba3, pba1);
    DUMP_BIT_ARRAY(pba3);

    ol_printf("copy pba1:   ");
    COPY_BIT_ARRAY(pba4, pba1);
    DUMP_BIT_ARRAY(pba4);

    return u32Ret;
}

static u32 _testBitArray(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    bit_array_t pba[8];
    u32 u32SetPos[] = {2, 3, 6, 8, 10, 13, 16, 18, 20, 23, 24, 27, 31, 32,
                       35, 37, 39, 40, 42, 43, 44, 47, 48, 50, 55, 57, 60, 64, 65, 66, 70};
    u32 u32NumOfSetPos = sizeof(u32SetPos) / sizeof(u32);
    u32 u32TestPos[] = {2, 3, 7, 8, 14, 20, 40, 65, 90};
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
    INIT_BIT_ARRAY(pba);
    DUMP_BIT_ARRAY(pba);

    ol_printf("set bit array\n");
    SET_BIT_ARRAY(pba);

    DUMP_BIT_ARRAY(pba);

    ol_printf("clear bit array\n");
    INIT_BIT_ARRAY(pba);

    DUMP_BIT_ARRAY(pba);

    for (u32Index = 0; u32Index < u32NumOfSetPos; u32Index ++)
    {
        ol_printf("set position %u\n", u32SetPos[u32Index]);
        setBitArrayBit(pba, u32SetPos[u32Index]);

        DUMP_BIT_ARRAY(pba);
    }

    for (u32Index = 0; u32Index < u32NumOfTestPos; u32Index ++)
    {
        bRet = testBitArrayBit(pba, u32TestPos[u32Index]);
        ol_printf("test position %u, %s\n", u32TestPos[u32Index],
               getStringTrue(bRet));
    }

    for (u32Index = 0; u32Index < u32NumOfClearPos; u32Index ++)
    {
        ol_printf("clear position %u\n", u32ClearPos[u32Index]);
        clearBitArrayBit(pba, u32ClearPos[u32Index]);

        DUMP_BIT_ARRAY(pba);
    }

    for (u32Index = 0; u32Index < u32NumOfLShift; u32Index ++)
    {
        ol_printf("set position %u\n", 14);
        setBitArrayBit(pba, 14);

        DUMP_BIT_ARRAY(pba);

        ol_printf("left shift %u\n", u32LShift[u32Index]);
        LSHIFT_BIT_ARRAY(pba, u32LShift[u32Index]);
            
        DUMP_BIT_ARRAY(pba);
    }

    for (u32Index = 0; u32Index < u32NumOfRShift; u32Index ++)
    {
        ol_printf("set position %u\n", 4);
        setBitArrayBit(pba, 4);

        DUMP_BIT_ARRAY(pba);

        ol_printf("right shift %u\n", u32RShift[u32Index]);
        RSHIFT_BIT_ARRAY(pba, u32RShift[u32Index]);
            
        DUMP_BIT_ARRAY(pba);
    }

    ol_printf("set position %u\n", 7);
    setBitArrayBit(pba, 7);

    DUMP_BIT_ARRAY(pba);

    for (u32Index = 0; u32Index < 19; u32Index ++)
    {
        ol_printf("increment bit array\n");
        INCREMENT_BIT_ARRAY(pba);

        DUMP_BIT_ARRAY(pba);
    }

    for (u32Index = 0; u32Index < 20; u32Index ++)
    {
        ol_printf("decrement bit array\n");
        DECREMENT_BIT_ARRAY(pba);

        DUMP_BIT_ARRAY(pba);
    }

    return u32Ret;
}

static void _testSizeof(void)
{
    bit_array_t a[20];
    bit_array_t b[6][40];

    ol_printf("a[20]\n");
    ol_printf("sizeof(a) = %d\n", (s32)sizeof(a));
    ol_printf("sizeof(a[0]) = %d\n", (s32)sizeof(a[0]));

    ol_printf("b[6][40]\n");
    ol_printf("sizeof(b) = %d\n", (s32)sizeof(b));
    ol_printf("sizeof(b[0]) = %d\n", (s32)sizeof(b[0]));
    ol_printf("sizeof(b[1]) = %d\n", (s32)sizeof(b[1]));
    ol_printf("sizeof(b[1][1]) = %d\n", (s32)sizeof(b[1][1]));
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
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

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

