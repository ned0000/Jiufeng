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
 *  @param phc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
u32 genCanonicalHuffmanCodeByCodeLen(huffman_code_t * phc, u16 u16NumOfCode);


#endif /*ENCODE_HUFFMAN_H*/

/*---------------------------------------------------------------------------*/


