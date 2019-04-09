/**
 *  @file base64.c
 *
 *  @brief base64 encode-decode library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_mem.h"
#include "jf_encode.h"

/* --- private data/data structure section --------------------------------- */

/** ASCII
 *   A ~ Z: 0x41 ~ 0x5A
 *   a ~ z: 0x61 ~ 0x7A
 *   0 ~ 9: 0x30 ~ 0x39
 *   +    : 0x2B
 *   /    : 0x2F
 */
static const olchar_t ls_cBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz0123456789+/";
static const olchar_t ls_cDecode[] = "|$$$}rstuvwxyz{$$$$$$$>?@"
    "ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ"
    "[\\]^_`abcdefghijklmnopq";

/* --- public routine section ---------------------------------------------- */

/** Encode 3 8-bit binary bytes as 4 '6-bit' characters 
 * 
 *  @param in [in] The input buffer
 *  @param len [in] Length of the input buffer
 *  @param out [out] The output buffer
 * 
 *  @return void
 */
static void _encodeBlock(const u8 in[3], olint_t len, u8 out[4])
{
    ol_memset(out, '=', 4);
    if (len == 1)
    {
        out[0] = (u8)ls_cBase64[in[0] >> 2];
        out[1] = (u8)ls_cBase64[(in[0] & 0x03) << 4];
    }
    else if (len == 2)
    {
        out[0] = (u8)ls_cBase64[in[0] >> 2];
        out[1] = (u8)ls_cBase64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
        out[2] = (u8)ls_cBase64[(in[1] & 0x0f) << 2];
    }
    else if (len == 3)
    {
        out[0] = (u8)ls_cBase64[in[0] >> 2];
        out[1] = (u8)ls_cBase64[((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)];
        out[2] = (u8)ls_cBase64[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)];
        out[3] = (u8)ls_cBase64[in[2] & 0x3f];
    }
}

/** Decode 4 '6-bit' characters into 3 8-bit binary bytes 
 * 
 *  @param in[ 4 ] : u8 <BR> 
 *     @b [in] The input buffer
 *  @param out[ 3 ] : u8 <BR> 
 *     @b [out] The output buffer
 * 
 *  @return void
 */
static void _decodeBlock(const u8 in[4], u8 out[3])
{
    out[0] = (u8) (in[0] << 2 | in[1] >> 4);
    out[1] = (u8) (in[1] << 4 | in[2] >> 2);
    out[2] = (u8) (((in[2] << 6) & 0xc0) | in[3]);
}

/* --- private routine section---------------------------------------------- */

u32 jf_encode_encodeBase64(
    const u8 * pu8Input, const olsize_t sInput, olchar_t ** ppstrOutput)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * out;
    const u8 * in;
    olsize_t slen;

    assert((pu8Input != NULL) && (sInput != 0));

    u32Ret = jf_mem_alloc((void **)ppstrOutput, ((sInput * 4) / 3) + 6);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        out = (u8 *)*ppstrOutput;
        in = pu8Input;
        slen = sInput;

        while (slen > 0)
        {
            if (slen > 3)
                _encodeBlock(in, 3, out);
            else
                _encodeBlock(in, slen, out);

            in += 3;
            out += 4;
            slen -= 3;
        }

        /* add trailing NULL character*/
        *out = 0;
    }

    return u32Ret;
}

u32 jf_encode_decodeBase64(
    const olchar_t * pstrInput, u8 ** ppu8Output, olsize_t * psOutput)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const olchar_t * input;
    u8 * out, v;
    u8 in[4];
    olint_t i, len;
    olsize_t sInput = ol_strlen(pstrInput);

    assert((pstrInput != NULL) && (sInput != 0));

    u32Ret = jf_mem_alloc((void **)ppu8Output, ((sInput * 3) / 4) + 6);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        out = *ppu8Output;
        input = pstrInput;

        while (input <= (pstrInput + sInput))
        {
            for (len = 0, i = 0;
                 i < 4 && input <= (pstrInput + sInput);
                 i ++)
            {
                v = 0;
                while (input <= (pstrInput + sInput) && v == 0)
                {
                    v = (u8) *input;
                    input++;
                    v = (u8) ((v < 0x2B || v > 0x7A) ? 0 : ls_cDecode[v - 0x2B]);
                    if (v)
                    {
                        v = (u8) ((v == '$') ? 0 : v - 0x3D);
                    }
                }
                if (input <= (pstrInput + sInput))
                {
                    len++;
                    if (v)
                    {
                        in[i] = (u8) (v - 1);
                    }
                }
                else
                {
                    in[i] = 0;
                }
            }
            if (len)
            {
                _decodeBlock(in, out);
                out += len - 1;
            }
        }
        *out = 0;

        *psOutput = (olsize_t) (out - *ppu8Output);
    }

    return u32Ret;
}

u32 jf_encode_freeBase64Buffer(u8 ** ppu8Output)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((ppu8Output != NULL) && (*ppu8Output != NULL));

    jf_mem_free((void **)ppu8Output);

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

