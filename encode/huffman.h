/**
 *  @file huffman.h
 *
 *  @brief huffman encoding and decoding header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef ENCODE_HUFFMAN_H
#define ENCODE_HUFFMAN_H

/* --- standard C lib header files ----------------------------------------- */
#include <limits.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */



/* --- functional routines ------------------------------------------------- */

/** By code length, frequency is ignored, code is generated
 *
 *  @param pjehc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
u32 jf_encode_genCanonicalHuffmanCodeByCodeLen(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode);


#endif /*ENCODE_HUFFMAN_H*/

/*---------------------------------------------------------------------------*/


