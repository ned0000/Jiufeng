/**
 *  @file jf_encode.h
 *
 *  @brief  API for the encode-decode library
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_encode library
 *
 */

/*-----------------------------------------------------------------*/

#ifndef JIUFENG_ENCODE_H
#define JIUFENG_ENCODE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
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

#define JF_ENCODE_MAX_HUFFMAN_CODE_LEN    (8)

typedef struct jf_encode_huffman_code
{
    /** symbol, max 65536 symbols */
    u16 jehc_u16Symbol;
    /** code length */
    u16 jehc_u16CodeLen;
    /** freqwency of the symbol */
    u32 jehc_u32Freq;
    /** the code generated */
    jf_bitarray_t jehc_jbCode[JF_ENCODE_MAX_HUFFMAN_CODE_LEN];
} jf_encode_huffman_code_t;

/* --- functional routines ------------------------------------------------- */

/*base64 encode and decode*/

/** Base64 encode a stream, add padding and line breaks
 *
 *  @param pu8Input [in] the stream to encode 
 *  @param sInput [in] The length of the stream to encode 
 *  @param ppstrOutput [out] The encoded stream 

 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL jf_encode_encodeBase64(
    const u8 * pu8Input, const olsize_t sInput, olchar_t ** ppstrOutput);

/** Decode a base64 encoded stream discarding padding, line breaks and noise
 *
 *  @param pstrInput [in] The stream to decode 
 *  @param ppu8Output [out] The decoded stream 
 *  @param psOutput [out] The length of the decoded stream
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL jf_encode_decodeBase64(
    const olchar_t * pstrInput, u8 ** ppu8Output, olsize_t * psOutput);

/** Free the buffer allocated by the function base64Encode and base64Decode 
 *
 *  @param ppu8Buffer [in/out] the buffer to free
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL jf_encode_freeBase64Buffer(u8 ** ppu8Buffer);


/*huffman code*/

/** By frequency, code length and code are generated in one pass.
 *
 *  @param pjehc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL jf_encode_genHuffmanCode(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode);

/** By frequency, code length and code are generated. The code is canonical
 *  huffman code, and code with maximum length is assigned with all 0 bits. 
 *
 *  @param pjehc [in/out] the array of huffman code
 *  @param u16NumOfCode [in] number of code in the array
 *
 *  @return the error code
 */
ENCODEAPI u32 ENCODECALL jf_encode_genCanonicalHuffmanCode(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode);

#endif /*JIUFENG_ENCODE_H*/

/*-----------------------------------------------------------------*/


