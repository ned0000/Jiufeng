/**
 *  @file jf_hex.h
 *
 *  @brief Header file which declares some functional routines for hex data.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_hex object.
 *
 */

#ifndef JIUTAI_HEX_H
#define JIUTAI_HEX_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

/** Convert the byte data into byte hex string, starting from the giving index.
 *
 *  @note
 *  -# The output string is "xxxxxxxxh: xx xx xx xx ...".
 *  -# Maximum 16 bytes are converted to string.
 *
 *  @param pu8Data [in] The data buffer.
 *  @param sData [in] The length of the data buffer in bytes.
 *  @param sOffset [in] The start index of the data.
 *  @param pstrHex [in/out] The hex string will be returned here.
 *  @param sStr [in] The size of the hex string in characters.
 *
 *  @return Number of bytes converted to hex string.
 */
olsize_t jf_hex_convertByteDataToString(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr);

/** Convert the word data into word hex string, starting from the giving index.
 *
 *  @note
 *  -# The output string is "xxxxxxxxh: xxxx xxxx xxxx xxxx ...".
 *  -# Maximum 12 words are converted to string.
 *
 *  @param pu16Data [in] The data buffer.
 *  @param sData [in] The length of the data buffer in words.
 *  @param sOffset [in] The start index of the data.
 *  @param pstrHex [in/out] The hex string will be returned here.
 *  @param sStr [in] The size of the hex string in characters.
 *
 *  @return Number of words converted to hex string.
 */
olsize_t jf_hex_convertWordDataToString(
    const u16 * pu16Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr);

/** Convert the byte data into byte hex string, starting from the giving index with ASCSII appended
 *  to each line.
 *
 *  @note
 *  -# The output string is "xxxxxxxxh: xx xx xx xx                   ; abcd".
 *  -# Maximum 16 bytes are converted to string.
 *
 *  @param pu8Data [in] The data buffer.
 *  @param sData [in] The length of the data buffer.
 *  @param sOffset [in] The start index of the data.
 *  @param pstrHex [in/out] The hex string will be returned here.
 *  @param sStr [in] The size of the hex string in characters.
 *
 *  @return Number of words converted to hex string and ASCII.
 */
olsize_t jf_hex_convertByteDataToStringWithAscii(
    const u8 * pu8Data, const olsize_t sData, const olsize_t sOffset, olchar_t * pstrHex,
    olsize_t sStr);

/** Dump byte data buffer, the output is byte hex string with ascii
 *
 *  @note
 *  -# All data are dumped.
 *
 *  @param pu8Data [in] The data buffer.
 *  @param sLen [in] The length of the data buffer.
 *
 *  @return void
 */
void jf_hex_dumpByteDataBuffer(const u8 * pu8Data, const olsize_t sLen);

/** Convert the string to byte hex.
 *
 *  @return Number of bytes hex converted from the string.
 */
olsize_t jf_hex_convertStringToHex(
    const olchar_t * pstr, const olsize_t sStr, u8 * pu8Hex, olsize_t sHex);

/** Convert the hex data to string.
 *
 *  @return Number of bytes hex converted to the string.
 */
olsize_t jf_hex_convertHexToString(
    olchar_t * pstr, olsize_t sStr, const u8 * pu8Hex, const olsize_t sHex);

#endif /*JIUTAI_HEX_H*/

/*------------------------------------------------------------------------------------------------*/


