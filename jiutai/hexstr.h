/**
 *  @file hexstr.h
 *
 *  @brief hex string header file
 *	 provide some functional routine to dump hex logs
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUTAI_HEXSTR_H
#define JIUTAI_HEXSTR_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

/** converte the data buffer into byte hex string, starting from the
 *  giving index.
 *
 *    [in] pu8Data, the data buffer
 *    [in] sData, the length of the data buffer in bytes
 *    [in] iStart, the start index of the data
 *    [out] pu8HexStr, the hex string will be returned here
 *    [in] sStr, the size of the hex string in characters
 *
 *  Return: the number of bytes converted to hex string
 */
olsize_t getByteHexString(u8 * pu8Data, olsize_t sData, olsize_t sOffset, 
    olchar_t * pstrHex, olsize_t sStr);

/** converte the data buffer into word hex string, starting from the giving index.
 *
 *    [in] pu16Data, the data buffer
 *    [in] sData, the length of the data buffer in words
 *    [in] iStart, the start index of the data
 *    [out] pstrHex, the hex string will be returned here
 *    [in] sStr, the size of the hex string in characters
 *
 *  Return: the number of words converted to hex string
 */
olsize_t getWordHexString(u16 * pu16Data, olsize_t sData, olsize_t sOffset,
    olchar_t * pstrHex, olsize_t sStr);

/** converte the data buffer into byte hex string, starting from the giving
 *  index with ASCSII appended to each line
 *
 *  Notes
 *   - the output string has folloing format
 *      
 */
olsize_t getByteHexStringWithAscii(u8 * pu8Data, olsize_t sData, olsize_t sOffset,
    olchar_t * pstrHex, olsize_t sStr);

/** dump data in byte hex format*/
void dumpDataInByteHex(u8 * pu8Data, olsize_t sLen);

#endif /*JIUTAI_HEXSTR_H*/

/*---------------------------------------------------------------------------*/


