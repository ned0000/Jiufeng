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

#include "chunkprocessor.h"

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

    /*Allocate new buffer.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buffer, u32MallocSize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Copy the data to new buffer.*/
        ol_memcpy(pu8Buffer, pihcp->ihcp_pu8Buffer, pihcp->ihcp_u32Offset);

        /*Free the old buffer and save the new buffer.*/
        jf_jiukun_freeMemory((void **)&pihcp->ihcp_pu8Buffer);
        pihcp->ihcp_pu8Buffer = pu8Buffer;
        pihcp->ihcp_u32MallocSize = u32MallocSize;
    }

    return u32Ret;
}

static u32 _processStartChunkInHttpParser(
    internal_httpparser_chunk_processor_t * pihcp, u8 * buffer, olsize_t * psBeginPointer,
    olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;
    boolean_t bFound = FALSE;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG("endPointer: %u ", endPointer);
#endif

    /*The start chunk should be more than 3 bytes including the size and CRLF.*/
    for (index = 3; index < endPointer; ++ index)
    {
        if ((buffer[index - 2] == JF_STRING_CARRIAGE_RETURN_CHAR) &&
            (buffer[index - 1] == JF_STRING_LINE_FEED_CHAR))
        {
            /*The chunk header is terminated with a CRLF. The part before the CRLF is the
              hex number representing the length of the chunk.*/
            u32Ret = jf_string_getS32FromHexString(
                (olchar_t *)buffer, index - 2, &pihcp->ihcp_nBytesLeft);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
#ifdef DEBUG_HTTPPARSER
                JF_LOGGER_DEBUG("chunk size: %d", pihcp->ihcp_nBytesLeft);
#endif
                bFound = TRUE;
                *psBeginPointer = index;
                /*Move to the footer chunk if the size is 0, otherwise it's data chunk.*/
                pihcp->ihcp_u8Flags = (pihcp->ihcp_nBytesLeft == 0) ?
                    HTTPPARSER_CHUNK_FLAG_FOOTER : HTTPPARSER_CHUNK_FLAG_DATA;
            }
            break;
        }
    }

    /*CRLF are not found, data is incomplete.*/
    if (! bFound)
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    return u32Ret;
}

static u32 _processEndChunkInHttpParser(
    internal_httpparser_chunk_processor_t * pihcp, u8 * buffer, olsize_t * psBeginPointer,
    olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG("endPointer: %d ", endPointer);
#endif

    if (endPointer >= 2)
    {
        /*Skip the CRLF, continue parsing the start chunk.*/
        *psBeginPointer = 2;
        pihcp->ihcp_u8Flags = HTTPPARSER_CHUNK_FLAG_START;
    }
    else
    {
        /*Incomplete data if the size is 0 or 1.*/
        u32Ret = JF_ERR_INCOMPLETE_DATA;
    }

    return u32Ret;
}

static u32 _processDataChunkInHttpParser(
    internal_httpparser_chunk_processor_t * pihcp, u8 * buffer, olsize_t * psBeginPointer,
    olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t index = 0;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG(
        "endPointer: %d, nBytesLeft: %d", endPointer, pihcp->ihcp_nBytesLeft);
#endif

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
        /*The buffer is too small, need to make it bigger.
          ToDo: Add code to enforce a max buffer size if specified.*/
        u32Ret = _reallocHttpParserChunkMemory(
            pihcp, pihcp->ihcp_u32MallocSize + endPointer);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Write the decoded chunk blob into the buffer.*/
        ol_memcpy(pihcp->ihcp_pu8Buffer + pihcp->ihcp_u32Offset, buffer, index);
        assert(pihcp->ihcp_u32Offset + index <= pihcp->ihcp_u32MallocSize);

        /*Adjust the counters.*/
        pihcp->ihcp_nBytesLeft -= index;
        pihcp->ihcp_u32Offset += index;

        *psBeginPointer = index;
    }

    return u32Ret;
}

static u32 _processFooterChunkInHttpParser(
    internal_httpparser_chunk_processor_t * pihcp, jf_httpparser_packet_header_t * pjhph,
    u8 * buffer, olsize_t * psBeginPointer, olsize_t endPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG("endPointer: %u ", endPointer);
#endif

    /*Footer chunk is exactly 2 bytes with CRLF, incomplete data if end pointer is less than 2.*/
    if (endPointer < 2)
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((buffer[0] == JF_STRING_CARRIAGE_RETURN_CHAR) &&
            (buffer[1] == JF_STRING_LINE_FEED_CHAR))
        {
            /*Finished, set the body and body size.*/
            jf_httpparser_setBody(pjhph, pihcp->ihcp_pu8Buffer, pihcp->ihcp_u32Offset, FALSE);

            *psBeginPointer = 2;
        }
        else
        {
            /*The 2 bytes are not CRLF, corrupted chunk data.*/
            u32Ret = JF_ERR_CORRUPTED_HTTP_CHUNK_DATA;
#ifdef DEBUG_HTTPPARSER
            JF_LOGGER_DEBUG("corrupted data");
#endif
        }
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

    /*Allocate memory for chunk processor.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pihcp, sizeof(*pihcp));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pihcp, sizeof(*pihcp));
        pihcp->ihcp_u32MallocSize = u32MallocSize;
        pihcp->ihcp_u8Flags = HTTPPARSER_CHUNK_FLAG_START;

        /*Allocate buffer.*/
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
    olsize_t sBeginPointer = 0;
    internal_httpparser_chunk_processor_t * pihcp = NULL;

    pihcp = (internal_httpparser_chunk_processor_t *)pProcessor;

    /*Adjust the counters and pointers in case the begin pointer is not 0.*/
    endPointer -= *psBeginPointer;
    buffer += *psBeginPointer;
    sBeginPointer += *psBeginPointer;
    *psBeginPointer = 0;

    while ((u32Ret == JF_ERR_NO_ERROR) && (*psBeginPointer < endPointer))
    {
        /*Based on the chunk flag, figure out how to parse the data.*/
        switch (pihcp->ihcp_u8Flags)
        {
        case HTTPPARSER_CHUNK_FLAG_START:
            u32Ret = _processStartChunkInHttpParser(pihcp, buffer, psBeginPointer, endPointer);

            break;
        case HTTPPARSER_CHUNK_FLAG_END:
            u32Ret = _processEndChunkInHttpParser(pihcp, buffer, psBeginPointer, endPointer);

            break;
        case HTTPPARSER_CHUNK_FLAG_DATA:
            u32Ret = _processDataChunkInHttpParser(pihcp, buffer, psBeginPointer, endPointer);

            break;
        case HTTPPARSER_CHUNK_FLAG_FOOTER:
            u32Ret = _processFooterChunkInHttpParser(
                pihcp, pjhph, buffer, psBeginPointer, endPointer);

            break;
        }

        /*Adjust the counters and pointers.*/
        endPointer -= *psBeginPointer;
        buffer += *psBeginPointer;
        sBeginPointer += *psBeginPointer;
        *psBeginPointer = 0;
    }

    *psBeginPointer = sBeginPointer;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG("BeginPointer: %d", *psBeginPointer);
#endif

    /*Incomplete data is not error.*/
    if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
