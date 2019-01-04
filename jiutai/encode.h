/**
 *  @file encode.h
 *
 *  @brief  API for the encode-decode library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/*-----------------------------------------------------------------*/

#ifndef JIUFENG_ENCODE_H
#define JIUFENG_ENCODE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "bitarray.h"

#undef ENCODEAPI
#undef ENCODECALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_ENCODE_DLL)
        #define ENCODEAPI  __declspec(dllexport)
        #define ENCODECALL
    #else
        #define ENCODEAPI
        #define ENCODECALL __cdecl
    #endif
#else
    #define ENCODEAPI
    #define ENCODECALL
#endif

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
#define MAX_HUFFMAN_CODE_LEN    (8)

typedef struct huffman_code
{
    /** symbol, max 65536 symbols */
    u16 hc_u16Symbol;
    /** code length */
    u16 hc_u16CodeLen;
    /** freqwency of the symbol */
    u32 hc_u32Freq;
    /** the code generated */
    bit_array_t hc_baCode[MAX_HUFFMAN_CODE_LEN];
} huffman_code_t;

/* --- functional routines ------------------------------------------------- */

/*base64 encode and decode*/

/** Base64 encode a stream, add padding and line breaks
 *
 *  @param pu8Input [in] the stream to encode 
 *  @param sInput [in] The length of the stream to encode 
 *  @param ppstrOutput [out] The encoded stream 

 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL base64Encode(
    const u8 * pu8Input, const olsize_t sInput, olchar_t ** ppstrOutput);

/** Decode a base64 encoded stream discarding padding, line breaks and noise
 *
 *  @param pstrInput [in] The stream to decode 
 *  @param ppu8Output [out] The decoded stream 
 *  @param psOutput [out] The length of the decoded stream
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL base64Decode(
    const olchar_t * pstrInput, u8 ** ppu8Output, olsize_t * psOutput);

/** Free the buffer allocated by the function base64Encode and base64Decode 
 *
 *  @param ppu8Buffer [in/out] the buffer to free
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL freeBase64Buffer(u8 ** ppu8Buffer);


/*huffman code*/

/** By frequency, code length and code are generated in one pass.
 *
 *  @param phc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL genHuffmanCode(
    huffman_code_t * phc, u16 u16NumOfCode);

/** By frequency, code length and code are generated. The code is canonical
 *  huffman code, and code with maximum length is assigned with all 0 bits. 
 *
 *  @param phc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL genCanonicalHuffmanCode(
    huffman_code_t * phc, u16 u16NumOfCode);

#endif /*JIUFENG_ENCODE_H*/

/*-----------------------------------------------------------------*/


