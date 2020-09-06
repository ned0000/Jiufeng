/**
 *  @file jf_hex.c
 *
 *  @brief Implementation file for functional routines of hex data.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <ctype.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_hex.h"
#include "jf_err.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The length of byte hex string header. The header has format with "xxxxxxxxh: ". The first 8
 *  bytes are the offset of the hex data, the header includes a null-terminated character.
 */
#define JF_HEX_BYTE_HEXSTR_HEADER_LEN            (11)

/** Minimal byte hex string length. One byte hex data is converted to 3 bytes string.
 */
#define JF_HEX_MIN_BYTE_HEXSTR_LEN               (JF_HEX_BYTE_HEXSTR_HEADER_LEN + 3)

/** Minimal length of byte hex string with ASCII. The line will be like:
 *  xxxxxxxxh: 00                                              ; a
 */
#define JF_HEX_MIN_BYTE_HEXSTR_WITH_ASCII_LEN    (63)

/** Maximum length of byte hex string with ASCII. The line will be like:
 *  xxxxxxxxh: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff ; abcdefghijklmnop
 */
#define JF_HEX_MAX_BYTE_HEXSTR_WITH_ASCII_LEN    (78)

/** Maximum byte per hex string line. The line will be like
 *  "xxxxxxxxh: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff"
 */
#define JF_HEX_MAX_BYTE_PER_HEXSTR_LINE          (16)

/** The length of word hex string header. The header has format with "xxxxxxxxh: ". The first 8
 *  bytes are the offset of the hex data, the header includes a null-terminated character.
 */
#define JF_HEX_WORD_HEXSTR_HEADER_LEN            (11)

/** Minimal byte hex string length. One word hex data is converted to 5 bytes string.
 */
#define JF_HEX_MIN_WORD_HEXSTR_LEN               (JF_HEX_WORD_HEXSTR_HEADER_LEN + 5)

/** Maximum word per hex string line. The line will be like:
 *  "xxxxxxxxh: 0000 1111 2222 3333 4444 5555 6666 7777 8888 9999 aaaa bbbb cccc dddd eeee ffff"
 */
#define JF_HEX_MAX_WORD_PER_HEXSTR_LINE          (16)

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

olsize_t jf_hex_convertByteDataToString(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) && (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= JF_HEX_MIN_BYTE_HEXSTR_LEN);

    /*Calculate maximum data can be converted based on the string size.*/
    sLimited = (sStr - JF_HEX_BYTE_HEXSTR_HEADER_LEN) / 3;
    if (sLimited > JF_HEX_MAX_BYTE_PER_HEXSTR_LINE)
        sLimited = JF_HEX_MAX_BYTE_PER_HEXSTR_LINE;
        
    /*Calculate maximum data to be converted.*/
    sLen = sData - sOffset;
    if (sLimited > sLen)
        sLimited = sLen;
        
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

olsize_t jf_hex_convertWordDataToString(
    const u16 * pu16Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr)
{
    u32 sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu16Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= JF_HEX_MIN_WORD_HEXSTR_LEN);

    /*Calculate maximum data can be converted based on the string size.*/
    sLimited = (sStr - JF_HEX_WORD_HEXSTR_HEADER_LEN) / 5;
    if (sLimited > JF_HEX_MAX_WORD_PER_HEXSTR_LINE)
        sLimited = JF_HEX_MAX_WORD_PER_HEXSTR_LINE;

    /*Calculate maximum data to be converted.*/
    sLen = sData - sOffset;        
    if (sLimited > sLen)
        sLimited = sLen;
        
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

olsize_t jf_hex_convertByteDataToStringWithAscii(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr)
{
    olsize_t sLimited = 0, sLen = 0;
    olchar_t strTemp[8];
    
    assert((pu8Data != NULL) && (sData != 0) &&
           (sOffset < sData) && (pstrHex != NULL));
    assert(sStr >= JF_HEX_MIN_BYTE_HEXSTR_WITH_ASCII_LEN);

    sLen = sData - sOffset;

    sLimited = JF_HEX_MAX_BYTE_PER_HEXSTR_LINE;
    if (sLimited > sLen)
        sLimited = sLen;

    ol_sprintf(pstrHex, "%08xh:", sOffset);

    /*Convert the byte data to string.*/
    sLen = 0;
    while (sLen < sLimited)
    {
        ol_sprintf(strTemp, " %02x", pu8Data[sOffset + sLen]);
        ol_strcat(pstrHex, strTemp);
        sLen ++;
    }

    /*Append spaces to the string if the data length is less than 16.*/
    sLen = sLimited;
    while (sLen < JF_HEX_MAX_BYTE_PER_HEXSTR_LINE)
    {
        ol_strcat(pstrHex, "   ");
        sLen ++;
    }

    /*Append the delimiter to the string.*/
    ol_strcat(pstrHex, " ; ");

    /*Append the ASCII to the string.*/
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

void jf_hex_dumpByteDataBuffer(const u8 * pu8Data, const olsize_t sLen)
{
    olsize_t sIndex = 0, sDumped = 0xff;
    olchar_t strLine[JF_HEX_MAX_BYTE_HEXSTR_WITH_ASCII_LEN];

    while (sIndex < sLen)
    {
        sDumped = jf_hex_convertByteDataToStringWithAscii(
            pu8Data, sLen, sIndex, strLine, sizeof(strLine));
        if (sDumped > 0)
        {
            sIndex += sDumped;
            ol_printf("%s\n", strLine);
        }
    }

}

olsize_t jf_hex_convertStringToHex(
    const olchar_t * pstr, const olsize_t sStr, u8 * pu8Hex, olsize_t sHexLen)
{
    olsize_t sLen = 0, start = 0;
    u32 u32Hex = 0;
    olchar_t u8Temp[8];
    
    assert((pstr != NULL) && (sStr > 0) && (pu8Hex != NULL) && (sHexLen > 0));

    start = 0;
    ol_bzero(u8Temp, sizeof(u8Temp));
    while ((sLen < sHexLen) && (start + 2 <= sStr))
    {
        /*Copy 2 bytes to the temporary string buffer.*/
        ol_strncpy(u8Temp, &(pstr[start]), 2);

        /*Scan the string buffer and match an unsigned hexadecimal integer.*/
        ol_sscanf(u8Temp, "%x", &u32Hex);
        pu8Hex[sLen] = (u8)u32Hex;

        start += 2;

        sLen ++;
    }

    return sLen;
}

olsize_t jf_hex_convertHexToString(
    olchar_t * pstr, olsize_t sStr, const u8 * pu8Hex, const olsize_t sHex)
{
    olsize_t sLen = 0, sIndex = 0;

    sLen = 0;
    sIndex = 0;

    while ((sLen + 2 <= sStr) && (sIndex < sHex))
    {
        /*Convert 1 byte hex to 2 characters.*/
        ol_sprintf(pstr + sLen, "%02x", pu8Hex[sIndex]);

        sLen += 2;
        sIndex ++;
    }

    return sLen;
}

/*------------------------------------------------------------------------------------------------*/
