/**
 *  @file crc32c-test.c
 *
 *  @brief test file for testing crc32c common object
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
#include "crc32c.h"
#include "hexstr.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
static u32 _printHexDumpInByte(u8 * pu8Buffer, u32 u32Length)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0, u32Dumped = 0xff;
    olchar_t strLine[80];

    while (u32Index < u32Length)
    {
        u32Dumped = getByteHexStringWithAscii(pu8Buffer, u32Length,
            u32Index, strLine, 80);
        if (u32Dumped > 0)
        {
            u32Index += u32Dumped;
            ol_printf("%s\n", strLine);
        }
    }

    return u32Ret;
}

static void _fillData(u8 * pu8Data, u32 u32DataLen)
{
    memset(pu8Data, 0, u32DataLen);
#if 0
    pu8Data[0] = 0x4;
    pu8Data[1] = 0x80;
    pu8Data[7] = 0x10;
    pu8Data[19] = 0x1;
    pu8Data[20] = 0xff;
    pu8Data[21] = 0xff;
    pu8Data[22] = 0xff;
    pu8Data[23] = 0xff;
    pu8Data[27] = 0x1;
    pu8Data[31] = 0x2;
#else
    pu8Data[0] = 0x24;
    pu8Data[1] = 0x80;
    pu8Data[7] = 0x50;
    pu8Data[19] = 0x2;
    pu8Data[20] = 0xff;
    pu8Data[21] = 0xff;
    pu8Data[22] = 0xff;
    pu8Data[23] = 0xff;
    pu8Data[27] = 0x2;
    pu8Data[31] = 0x3;
    pu8Data[35] = 0x22;
#endif
}

static void _testCrc32c()
{
    u8 u8Data[100];
    u32 u32Result, u32Desire = 0x8218DDC7;
    crc32c_vec_t cv[4];

    _fillData(u8Data, 100);

    _printHexDumpInByte(u8Data, 48);

    ol_printf("--------------------------------------------------------\n");
    ol_printf("testing crc32c\n");

    crc32c(u8Data, 48, CRC32C_FLAG_INIT_RESULT, &u32Result);

    if (u32Result == u32Desire)
        ol_printf("pass, CRC: 0x%X\n", u32Result);
    else
        ol_printf("error, CRC: 0x%X\n", u32Result);

    ol_printf("--------------------------------------------------------\n");
    ol_printf("testing crc32c vec with 1 entry\n");

    memset(cv, 0, sizeof(cv));

    cv[0].cv_pu8Buffer = u8Data;
    cv[0].cv_u32Len = 48;

    crc32cVec(cv, 1, CRC32C_FLAG_INIT_RESULT, &u32Result);

    if (u32Result == u32Desire)
        ol_printf("pass, CRC: 0x%X\n", u32Result);
    else
        ol_printf("error, CRC: 0x%X\n", u32Result);

    ol_printf("--------------------------------------------------------\n");
    ol_printf("testing crc32c vec with 2 entry\n");

    memset(cv, 0, sizeof(cv));

    cv[0].cv_pu8Buffer = u8Data;
    cv[0].cv_u32Len = 20;
    cv[1].cv_pu8Buffer = u8Data + 20;
    cv[1].cv_u32Len = 28;

    crc32cVec(cv, 2, CRC32C_FLAG_INIT_RESULT, &u32Result);

    if (u32Result == u32Desire)
        ol_printf("pass, CRC: 0x%X\n", u32Result);
    else
        ol_printf("error, CRC: 0x%X\n", u32Result);

    ol_printf("--------------------------------------------------------\n");
    ol_printf("testing crc32c vec with 3 entry\n");

    memset(cv, 0, sizeof(cv));

    cv[0].cv_pu8Buffer = u8Data;
    cv[0].cv_u32Len = 12;
    cv[1].cv_pu8Buffer = u8Data + 12;
    cv[1].cv_u32Len = 24;
    cv[2].cv_pu8Buffer = u8Data + 36;
    cv[2].cv_u32Len = 12;

    crc32cVec(cv, 3, CRC32C_FLAG_INIT_RESULT, &u32Result);

    if (u32Result == u32Desire)
        ol_printf("pass, CRC: 0x%X\n", u32Result);
    else
        ol_printf("error, CRC: 0x%X\n", u32Result);

}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    _testCrc32c();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

