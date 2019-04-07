/**
 *  @file hexstr.c
 *
 *  @brief Hex string header file, provide some functional routine to dump
 *   hex logs
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "hexstr.h"
#include "jf_err.h"

/* --- private data/data structure section --------------------------------- */

/*xxxxxxxxh: xx xx xx xx ...*/
#define BYTE_HEXSTR_HEADER_LENGTH  (11)
#define MIN_BYTE_HEXSTR_LENGTH     (BYTE_HEXSTR_HEADER_LENGTH + 3)
#define MAX_BYTE_PER_HEXSTR_LINE   (16)

/*xxxxxxxxh: xx xx xx xx ...*/
#define WORD_HEXSTR_HEADER_LENGTH  (11)
#define MIN_WORD_HEXSTR_LENGTH     (WORD_HEXSTR_HEADER_LENGTH + 5)
#define MAX_WORD_PER_HEXSTR_LINE   (12)

/*xxxxxxxxh: xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx ; abcdefghijklmnop*/
#define MIN_BYTE_HEXSTR_WITH_ASCII_LENGTH    (63)

/* --- private routine section ------------------------------------------------ */

/* --- public routine section ---------------------------------------------- */

olsize_t jf_hexstr_convertByteData(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= MIN_BYTE_HEXSTR_LENGTH);

    sLen = sData - sOffset;
    sLimited = (sStr - BYTE_HEXSTR_HEADER_LENGTH) / 3;
    if (sLimited > MAX_BYTE_PER_HEXSTR_LINE)
    {
        sLimited = MAX_BYTE_PER_HEXSTR_LINE;
    }
        
    if (sLimited > sLen)
    {
        sLimited = sLen;
    }
        
    ol_sprintf(pstrHex, "%08xh:", sOffset);

    sLen = 0;
    while (sLen < sLimited)
    {
        ol_sprintf(strTemp, " %02x", pu8Data[sOffset + sLen]);
        ol_strcat(pstrHex, strTemp);
        sLen ++;
    }

    return sLen;
}

olsize_t jf_hexstr_convertWordData(
    const u16 * pu16Data, const olsize_t sData, const olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    u32 sLimited = 0, sLen;
    olchar_t strTemp[8];
    
    assert((pu16Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= MIN_WORD_HEXSTR_LENGTH);

    sLen = sData - sOffset;
    sLimited = (sStr - WORD_HEXSTR_HEADER_LENGTH) / 5;
    if (sLimited > MAX_WORD_PER_HEXSTR_LINE)
    {
        sLimited = MAX_WORD_PER_HEXSTR_LINE;
    }
        
    if (sLimited > sLen)
    {
        sLimited = sLen;
    }
        
    ol_sprintf(pstrHex, "%08xh:", sOffset);

    sLen = 0;
    while (sLen < sLimited)
    {
        ol_sprintf(strTemp, " %04x", pu16Data[sOffset + sLen]);
        ol_strcat(pstrHex, strTemp);
        sLen++;
    }
    
    return sLen;
}

olsize_t jf_hexstr_convertByteDataWithAscii(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= MIN_BYTE_HEXSTR_WITH_ASCII_LENGTH);

    sLen = sData - sOffset;

    sLimited = MAX_BYTE_PER_HEXSTR_LINE;
    if (sLimited > sLen)
    {
        sLimited = sLen;
    }

    ol_sprintf(pstrHex, "%08xh:", sOffset);

    sLen = 0;
    while (sLen < sLimited)
    {
        ol_sprintf(strTemp, " %02x", pu8Data[sOffset + sLen]);
        ol_strcat(pstrHex, strTemp);
        sLen ++;
    }

    sLen = sLimited;
    while (sLen < MAX_BYTE_PER_HEXSTR_LINE)
    {
        ol_strcat(pstrHex, "   ");
        sLen ++;
    }

    ol_strcat(pstrHex, " ; ");

    sLen = 0;
    while (sLen < sLimited)
    {
        if (isprint(pu8Data[sOffset + sLen]))
            ol_sprintf(strTemp, "%c", pu8Data[sOffset + sLen]);
        else
            ol_sprintf(strTemp, "%c", '.');

        ol_strcat(pstrHex, strTemp);
        sLen ++;
    }

    return sLen;
}

void jf_hexstr_dumpByteDataBuffer(const u8 * pu8Data, const olsize_t sLen)
{
    olsize_t sIndex = 0, sDumped = 0xff;
    olchar_t strLine[80];

    while (sIndex < sLen)
    {
        sDumped = jf_hexstr_convertByteDataWithAscii(
            pu8Data, sLen, sIndex, strLine, 80);
        if (sDumped > 0)
        {
            sIndex += sDumped;
            ol_printf("%s\n", strLine);
        }
    }

}

/*---------------------------------------------------------------------------*/


