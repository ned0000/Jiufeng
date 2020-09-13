/**
 *  @file chunkprocessor.c
 *
 *  @brief Implementation file for chunk processor which is used to process chunked data in HTTP
 *   body.
 *
 *  @author Min Zhang
 *
 *  @par HTTP packet with chuned data
 *   chunk-size\\r\\n        \n
 *   chunk-data\\r\\n        \n
 *   chunk-size\\r\\n        \n
 *   chunk-data\\r\\n        \n
 *   0\\r\\n                 \n
 *   \\r\\n                  \n
 *                           \n
 *   chunk-size is a hexadecimal string
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_httpparser.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the chunk processing flags.
 */
typedef enum httpparser_chunk_flag
{
    /**Start of chunk.*/
    HTTPPARSER_CHUNK_FLAG_START = 0,
    /**End of chunk by "\r\n".*/
    HTTPPARSER_CHUNK_FLAG_END,
    /**Data chunk.*/
    HTTPPARSER_CHUNK_FLAG_DATA,
    /**Footer of chunk by "0\r\n\r\n".*/
    HTTPPARSER_CHUNK_FLAG_FOOTER,
} httpparser_chunk_flag_t;

/** Define the internal HTTP parser chunk processor.
 */
typedef struct internal_httpparser_chunk_processor
{
    /**Data buffer.*/
    u8 * ihcp_pu8Buffer;
    /**Chunk flag.*/
    u8 ihcp_u8Flags;
    u8 ihcp_u8Reserved[7];
    /**Offset of valid data.*/
    u32 ihcp_u32Offset;
    /**Size of buffer allocated.*/
    u32 ihcp_u32MallocSize;
    /**Number of bytes expected.*/
    olint_t ihcp_nBytesLeft;
} internal_httpparser_chunk_processor_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _reallocHttpParserChunkMemory(
    internal_httpparser_chunk_processor_t * pihcp, u32 u32MallocSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buffer = NULL;

    assert(pihcp->ihcp_u32MallocSize < u32MallocSize);

    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buffer, u32MallocSize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memcpy(pu8Buffer, pihcp->ihcp_pu8Buffer, pihcp->ihcp_u32Offset);

        jf_jiukun_freeMemory((void **)&pihcp->ihcp_pu8Buffer);
        pihcp->ihcp_pu8Buffer = pu8Buffer;
        pihcp->ihcp_u32MallocSize = u32MallocSize;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_httpparser_destroyChunkProcessor(jf_httpparser_chunk_processor_t ** ppProcessor)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_chunk_processor_t * pihcp = NULL;

    pihcp = (internal_httpparser_chunk_processor_t *)*ppProcessor;

    if (pihcp->ihcp_pu8Buffer != NULL)
        jf_jiukun_freeMemory((void **)&pihcp->ihcp_pu8Buffer);

    jf_jiukun_freeMemory((void **)ppProcessor);

    return u32Ret;
}

u32 jf_httpparser_createChunkProcessor(
    jf_httpparser_chunk_processor_t ** ppProcessor, u32 u32MallocSize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_chunk_processor_t * pihcp = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)pihcp, sizeof(*pihcp));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pihcp, sizeof(*pihcp));
        pihcp->ihcp_u32MallocSize = u32MallocSize;
        pihcp->ihcp_u8Flags = HTTPPARSER_CHUNK_FLAG_START;

        u32Ret = jf_jiukun_allocMemory((void **)&pihcp->ihcp_pu8Buffer, u32MallocSize);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppProcessor = pihcp;
    else if (pihcp != NULL)
        jf_httpparser_destroyChunkProcessor((jf_httpparser_chunk_processor_t **)&pihcp);

    return u32Ret;
}

u32 jf_httpparser_processChunk(
    jf_httpparser_chunk_processor_t * pProcessor, jf_httpparser_packet_header_t * pjhph,
    u8 * buffer, olsize_t * psBeginPointer, olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;
    olsize_t sBeginPointer = *psBeginPointer;
    internal_httpparser_chunk_processor_t * pihcp = NULL;

    pihcp = (internal_httpparser_chunk_processor_t *)pProcessor;

    JF_LOGGER_DEBUG("chunk: %d:%d", *psBeginPointer, endPointer);

    while ((u32Ret == JF_ERR_NO_ERROR) && (*psBeginPointer < endPointer))
    {
        /*Based on the Chunk Flag, we can figure out how to parse this thing*/
        switch (pihcp->ihcp_u8Flags)
        {
        case HTTPPARSER_CHUNK_FLAG_START:
            JF_LOGGER_DEBUG("STARTCHUNK");
            /*Reading chunk header, it's at least 3 bytes.*/
            if (endPointer < 3)
            {
                return u32Ret;
            }
            for (index = 3; index < endPointer; ++ index)
            {
                if (buffer[index - 2] == '\r' && buffer[index - 1] == '\n')
                {
                    /*The chunk header is terminated with a CRLF. The part before the CRLF is the
                      hex number representing the length of the chunk.*/
                    u32Ret = jf_string_getS32FromHexString(
                        (olchar_t *)buffer, index - 2, &pihcp->ihcp_nBytesLeft);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        JF_LOGGER_DEBUG("chunk size: %d", pihcp->ihcp_nBytesLeft);
                        *psBeginPointer = index;
                        pihcp->ihcp_u8Flags = (pihcp->ihcp_nBytesLeft == 0) ?
                            HTTPPARSER_CHUNK_FLAG_FOOTER : HTTPPARSER_CHUNK_FLAG_DATA;
                    }
                    break;
                }
            }
            break;
        case HTTPPARSER_CHUNK_FLAG_END:
            JF_LOGGER_DEBUG("ENDCHUNK");
            if (endPointer >= 2)
            {
                /*There is more chunks to come.*/
                *psBeginPointer = 2;
                pihcp->ihcp_u8Flags = HTTPPARSER_CHUNK_FLAG_START;
            }
            break;
        case HTTPPARSER_CHUNK_FLAG_DATA:
            JF_LOGGER_DEBUG("DATACHUNK");
            if (endPointer >= pihcp->ihcp_nBytesLeft)
            {
                /*Only consume what we need.*/
                pihcp->ihcp_u8Flags = HTTPPARSER_CHUNK_FLAG_END;
                index = pihcp->ihcp_nBytesLeft;
            }
            else
            {
                /*Consume all of the data.*/
                index = endPointer;
            }

            if (pihcp->ihcp_u32Offset + endPointer > pihcp->ihcp_u32MallocSize)
            {
                JF_LOGGER_DEBUG("realloc memory");
                /*The buffer is too small, we need to make it bigger.
                  ToDo: Add code to enforce a max buffer size if specified */
                u32Ret = _reallocHttpParserChunkMemory(
                    pihcp, pihcp->ihcp_u32MallocSize + endPointer);
            }

            /*Write the decoded chunk blob into the buffer.*/
            ol_memcpy(pihcp->ihcp_pu8Buffer + pihcp->ihcp_u32Offset, buffer, index);
            assert(pihcp->ihcp_u32Offset + index <= pihcp->ihcp_u32MallocSize);

            /*Adjust our counters.*/
            pihcp->ihcp_nBytesLeft -= index;
            pihcp->ihcp_u32Offset += index;

            *psBeginPointer = index;
            break;
        case HTTPPARSER_CHUNK_FLAG_FOOTER:
            JF_LOGGER_DEBUG("FOOTERCHUNK");
            if (endPointer >= 2)
            {
                for (index = 2; index <= endPointer; ++ index)
                {
                    if (buffer[index - 2] == '\r' && buffer[index - 1] == '\n')
                    {
                        /*An empty line means the chunk is finished.*/
                        if (index == 2)
                        {
                            /*Finished.*/
                            jf_httpparser_setBody(
                                pjhph, pihcp->ihcp_pu8Buffer, pihcp->ihcp_u32Offset, FALSE);

                            *psBeginPointer = 2;
                            break;
                        }
                        else
                        {
                            u32Ret = JF_ERR_CORRUPTED_HTTP_CHUNK_DATA;
                        }
                    }
                }
            }
            break;
        }

        endPointer -= *psBeginPointer;
        buffer += *psBeginPointer;
        sBeginPointer += *psBeginPointer;
        *psBeginPointer = 0;
    }

    *psBeginPointer = sBeginPointer;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
