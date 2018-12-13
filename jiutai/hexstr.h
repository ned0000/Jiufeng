/**
 *  @file hexstr.h
 *
 *  @brief hex string header file. Provide some functional routine to dump hex
 *   logs
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

/** Converte the data buffer into byte hex string, starting from the
 *  giving index.
 *
 *  @param pu8Data [in] the data buffer
 *  @param sData [in] the length of the data buffer in bytes
 *  @param sOffset [in] the start index of the data
 *  @param pstrHex [in/out] the hex string will be returned here
 *  @param sStr [in] the size of the hex string in characters
 *
 *  @return the number of bytes converted to hex string
 */
olsize_t getByteHexString(
    u8 * pu8Data, olsize_t sData, olsize_t sOffset,
    olchar_t * pstrHex, olsize_t sStr);

/** Converte the data buffer into word hex string, starting from the giving
 *  index.
 *
 *  @param pu16Data [in] the data buffer
 *  @param sData [in] the length of the data buffer in words
 *  @param sOffset [in] the start index of the data
 *  @param pstrHex [in/out] the hex string will be returned here
 *  @param sStr [in] the size of the hex string in characters
 *
 *  @return the number of words converted to hex string
 */
olsize_t getWordHexString(
    u16 * pu16Data, olsize_t sData, olsize_t sOffset,
    olchar_t * pstrHex, olsize_t sStr);

/** Converte the data buffer into byte hex string, starting from the giving
 *  index with ASCSII appended to each line
 *
 *  @param pu8Data [in] the data buffer
 *  @param sData [in] the length of the data buffer
 *  @param sOffset [in] the start index of the data
 *  @param pstrHex [in/out] the hex string will be returned here
 *  @param sStr [in] the size of the hex string in characters
 *
 *  @return the number of words converted to hex string and ASCII
 */
olsize_t getByteHexStringWithAscii(
    u8 * pu8Data, olsize_t sData, olsize_t sOffset,
    olchar_t * pstrHex, olsize_t sStr);

/** Dump data in byte hex format
 *
 *  @param pu8Data [in] the data buffer
 *  @param sLen [in] the length of the data buffer
 *
 *  @return void
 **/
void dumpDataInByteHex(u8 * pu8Data, olsize_t sLen);

#endif /*JIUTAI_HEXSTR_H*/

/*---------------------------------------------------------------------------*/


