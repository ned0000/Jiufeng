/**
 *  @file encode-test.c
 *
 *  @brief test file for testing encode library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_encrypt.h"
#include "jf_hex.h"
#include "jf_string.h"
#include "jf_encode.h"
#include "jf_filestream.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */
static boolean_t ls_bBase64 = FALSE;
static boolean_t ls_bHuffman = FALSE;
static boolean_t ls_bCanonicalHuffman = FALSE;
static olchar_t * ls_pstrFile = NULL;

/* --- private routine section ------------------------------------------------------------------ */
static void _printUsage(void)
{
    ol_printf("\
Usage: encode-test [-b] [-m filename] [-c] \n\
  for base64 encode\n\
    -b test base64\n\
  for huffman code\n\
    -m test huffman coding\n\
    -c generate canonical huffman code");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "cm:bh?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 'm':
            ls_bHuffman = TRUE;
            ls_pstrFile = optarg;
            break;
        case 'b':
            ls_bBase64 = TRUE;
            break;
        case 'c':
            ls_bCanonicalHuffman = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

#define DATA_SIZE  500

static u32 _testBase64(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrOutput = NULL;
    u8 * pu8Decode = NULL;
    olsize_t sLen, sDecode;
    olchar_t * pstrData[] =
    {
        "1",
        "12",
        "123",
        "1234",
        "12345",
        "123456",
        "Man is distinguished, not only by his reason, but by this singular"
         " passion from other animals, which is a lust of the mind, that by a"
         " perseverance of delight in the continued and indefatigable"
         " generation of knowledge, exceeds the short vehemence of any"
         " carnal pleasure.",
    };
    u32 u32NumOfData = sizeof(pstrData) / sizeof(olchar_t *);
    u32 u32Index;

    ol_printf("Testing Base64 encoding\n");

    for (u32Index = 0; u32Index < u32NumOfData; u32Index ++)
    {
        sLen = ol_strlen(pstrData[u32Index]);
        ol_printf("Source data: %d, %s\n", sLen, pstrData[u32Index]);

        u32Ret = jf_encode_encodeBase64((u8 *)pstrData[u32Index], sLen, &pstrOutput);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            sLen = ol_strlen(pstrOutput);
            ol_printf("Encoded data: %d, %s\n", sLen, pstrOutput);

            u32Ret = jf_encode_decodeBase64(pstrOutput, &pu8Decode, &sDecode);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pu8Decode[sDecode] = '\0';
                ol_printf("Decoded data: %d, %s\n", sDecode, pu8Decode);

                if (ol_memcmp(
                        (olchar_t *)pu8Decode,
                        (olchar_t *)pstrData[u32Index], sDecode) == 0)
                    ol_printf("Base64 succeeds\n");
                else
                    ol_printf("Base64 fails\n");

                jf_encode_freeBase64Buffer(&pu8Decode);
            }

            jf_encode_freeBase64Buffer((u8 **)&pstrOutput);
        }
        ol_printf("\n");
    }

    return u32Ret;
}

static void _bit2String(u8 * pu8Bit, u16 u16BitLen, olchar_t * pstrBit)
{
    u16 u16Index;
    u16 u16Byte, u16ByteOffset;

    pstrBit[0] = '\0';

    for (u16Index = 0; u16Index < u16BitLen; u16Index ++)
    {
        u16Byte = u16Index / 8;
        u16ByteOffset = u16Index % 8;

        if (pu8Bit[u16Byte] & (1 << (8 - u16ByteOffset - 1)))
            ol_strcat(pstrBit, "1");
        else
            ol_strcat(pstrBit, "0");
    }
}

#define NUM_SYMBOL  256

static olint_t _encodeTestCompare(const void * item1, const void * item2)
{
    jf_encode_huffman_code_t * pjehc1 = *((jf_encode_huffman_code_t **)item1);
    jf_encode_huffman_code_t * pjehc2 = *((jf_encode_huffman_code_t **)item2);

    if (pjehc1->jehc_u16CodeLen > pjehc2->jehc_u16CodeLen)
    {
        /* item1 > item2 */
        return 1;
    }
    else if (pjehc1->jehc_u16CodeLen < pjehc2->jehc_u16CodeLen)
    {
        /* item1 < item2 */
        return -1;
    }
    else
    {
        /* both have equal code lengths break the tie using value */
        if (pjehc1->jehc_u16Symbol > pjehc2->jehc_u16Symbol)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;   /* we should never get here */
}

static void _printCanonicalHuffmanCode(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index;
    olchar_t strBit[100];
    jf_encode_huffman_code_t ** ppjehc = NULL;

    u32Ret = jf_mem_calloc(
        (void **)&ppjehc, sizeof(jf_encode_huffman_code_t *) * u16NumOfCode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16NumOfCode; u16Index ++)
        {
            ppjehc[u16Index] = &(pjehc[u16Index]);
        }

        /* sort by code length */
        qsort(
            ppjehc, u16NumOfCode, sizeof(jf_encode_huffman_code_t *), _encodeTestCompare);

        ol_printf("Char   Count     CodeLen   Encoding\n");
        ol_printf("-----  --------  --------  ----------------\n");
        for (u16Index = 0; u16Index < u16NumOfCode; u16Index ++)
        {
            if (ppjehc[u16Index]->jehc_u16CodeLen > 0)
            {
                _bit2String(
                    ppjehc[u16Index]->jehc_jbCode, ppjehc[u16Index]->jehc_u16CodeLen, strBit);
                ol_printf(
                    "0x%02X   %-8d  %-8d  %s\n",
                    ppjehc[u16Index]->jehc_u16Symbol, ppjehc[u16Index]->jehc_u32Freq,
                    ppjehc[u16Index]->jehc_u16CodeLen, strBit);
            }
        }
    }

    if (ppjehc != NULL)
        jf_mem_free((void **)&ppjehc);
}

static void _printHuffmanCode(jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u16Index;
    olchar_t strBit[100];

    ol_printf("Char   Count     CodeLen   Encoding\n");
    ol_printf("-----  --------  --------  ----------------\n");
    for (u16Index = 0; u16Index < u16NumOfCode; u16Index ++)
    {
        if (pjehc[u16Index].jehc_u16CodeLen > 0)
        {
            _bit2String(
                pjehc[u16Index].jehc_jbCode, pjehc[u16Index].jehc_u16CodeLen, strBit);
            ol_printf(
                "0x%02X   %-8d  %-8d  %s\n",
                pjehc[u16Index].jehc_u16Symbol, pjehc[u16Index].jehc_u32Freq,
                pjehc[u16Index].jehc_u16CodeLen, strBit);
        }
    }
}

static u32 _testGenHuffmanCode(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_encode_huffman_code_t pjehc[NUM_SYMBOL];
    u16 u16Index = 0;
    jf_filestream_t * fp;
    olint_t c;

    if (ls_bCanonicalHuffman)
        ol_printf("generate canonical huffman code\n");
    else
        ol_printf("generate huffman code\n");

    memset(pjehc, 0, sizeof(jf_encode_huffman_code_t) * NUM_SYMBOL);

    for (u16Index = 0; u16Index < NUM_SYMBOL; u16Index ++)
    {
        pjehc[u16Index].jehc_u16Symbol = u16Index;
    }

    u32Ret = jf_filestream_open(ls_pstrFile, "r", &fp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        while ((c = fgetc(fp)) != EOF)
        {
            if (pjehc[c].jehc_u32Freq < 0xFFFFFFFF)
            {
                /* increment count for character and include in tree */
                pjehc[c].jehc_u32Freq ++;
            }
        }

        jf_filestream_close(&fp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bCanonicalHuffman)
        {
            u32Ret = jf_encode_genCanonicalHuffmanCode(pjehc, NUM_SYMBOL);
            if (u32Ret == JF_ERR_NO_ERROR)
                _printCanonicalHuffmanCode(pjehc, NUM_SYMBOL);
        }
        else
        {
            u32Ret = jf_encode_genHuffmanCode(pjehc, NUM_SYMBOL);
            if (u32Ret == JF_ERR_NO_ERROR)
                _printHuffmanCode(pjehc, NUM_SYMBOL);
        }
    }

    return u32Ret;
}

static u32 _testHuffman(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _testGenHuffmanCode();

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bBase64)
            u32Ret = _testBase64();
        else if (ls_bHuffman)
            u32Ret = _testHuffman();
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUsage();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

