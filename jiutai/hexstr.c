/**
 *  @file hexstr.c
 *
 *  @brief Hex String header file
 *     provide some functional routine to dump hex logs
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
#include "olbasic.h"
#include "hexstr.h"
#include "errcode.h"

/* --- private data/data structure section --------------------------------- */

/*xxxxxxxxh: xx xx xx xx ...*/
#define BYTE_HEXSTR_HEADER_LENGTH  13
#define MIN_BYTE_HEXSTR_LENGTH     (BYTE_HEXSTR_HEADER_LENGTH + 2)
#define MAX_BYTE_PER_HEXSTR_LINE   16

/*xxxxxxxxh: xx xx xx xx ...*/
#define WORD_HEXSTR_HEADER_LENGTH  13
#define MIN_WORD_HEXSTR_LENGTH     (WORD_HEXSTR_HEADER_LENGTH + 4)
#define MAX_WORD_PER_HEXSTR_LINE   12

/*xxxxxxxxh: xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx ; abcdefghijklmnop*/
#define MIN_BYTE_HEXSTR_WITH_ASCII_LENGTH  79

/* --- private routine section ------------------------------------------------ */

/* --- public routine section ---------------------------------------------- */

/** convert the data to hex string
 *
 *  - Notes:
 *    -# ONLY convert most MAX_BYTE_PER_HEXSTR_LINE data
 *
 *  @param pu8Data : u8 * <BR>
 *     @b [in] The Chain to add the link to
 *  @param sData : count <BR>
 *     @b [in] The link to add to the chain
 * 
 *  @return return the number of data stored in the pu8Hex
 */
olsize_t getByteHexString(
    u8 * pu8Data, olsize_t sData, olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr > MIN_BYTE_HEXSTR_LENGTH);

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

olsize_t getWordHexString(
    u16 * pu16Data, olsize_t sData, olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    u32 sLimited = 0, sLen;
    olchar_t strTemp[8];
    
    assert((pu16Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr < MIN_WORD_HEXSTR_LENGTH);

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

olsize_t getByteHexStringWithAscii(
    u8 * pu8Data, olsize_t sData, olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr > MIN_BYTE_HEXSTR_WITH_ASCII_LENGTH);

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

void dumpDataInByteHex(u8 * pu8Data, olsize_t sLen)
{
    olsize_t sIndex = 0, sDumped = 0xff;
    olchar_t strLine[80];

    while (sIndex < sLen)
    {
        sDumped = getByteHexStringWithAscii(pu8Data, sLen,
            sIndex, strLine, 80);
        if (sDumped > 0)
        {
            sIndex += sDumped;
            ol_printf("%s\n", strLine);
        }
    }

}

/*---------------------------------------------------------------------------*/


