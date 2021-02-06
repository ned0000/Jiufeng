/**
 *  @file httpparser/dataobject.c
 *
 *  @brief Httpparser data object implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_httpparser.h"
#include "jf_jiukun.h"

#include "chunkprocessor.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the internal http data object data type.
 */
typedef struct internal_httpparser_dataobject
{
    /**HTTP header is received if it's TRUE.*/
    boolean_t ihd_bFinHeader;
    /**The data in HTTP body is chunked.*/
    boolean_t ihd_bChunked;
    /**Full packet is received.*/
    boolean_t ihd_bFullPacket;
    u8 ihd_u8Reserved[5];

    /**If the buffer is not big enough to hold HTTP body, use this buffer for the data.*/
    u8 * ihd_pu8BodyBuf;
    /**Offset in body buffer for receiving data.*/
    u32 ihd_u32BodyOffset;

    /**Number of bytes left to be received, also the HTTP body length.*/
    olint_t ihd_nBytesLeft;
    olint_t ihd_nReserved;

    /**Size of caller's buffer.*/
    olsize_t ihd_sBuffer;

    /**Chunk processor.*/
    jf_httpparser_chunk_processor_t * ihd_pjhcpProcessor;

    /**HTTP header for response.*/
    jf_httpparser_packet_header_t * ihd_pjhphHeader;

} internal_httpparser_dataobject_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _parseHeaderContentInHpDo(internal_httpparser_dataobject_t * pihd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Encoding = JF_HTTPPARSER_TRANSFER_ENCODING_UNKNOWN;

    /*Parse header line, get the transfer encoding.*/
    u32Ret = jf_httpparser_parseHeaderTransferEncoding(pihd->ihd_pjhphHeader, &u8Encoding);
    if ((u32Ret == JF_ERR_NO_ERROR) && (u8Encoding == JF_HTTPPARSER_TRANSFER_ENCODING_CHUNKED))
    {
        /*This packet was chunk encoded.*/
        pihd->ihd_bChunked = TRUE;
        JF_LOGGER_DEBUG("encoding: chunked");

        /*Create chunk processor.*/
        u32Ret = jf_httpparser_createChunkProcessor(&pihd->ihd_pjhcpProcessor, pihd->ihd_sBuffer);
    }
    else
    {
        /*No chunk, parse content length.*/
        u32Ret = jf_httpparser_parseHeaderContentLength(
            pihd->ihd_pjhphHeader, &pihd->ihd_nBytesLeft);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("content-length: %d", pihd->ihd_nBytesLeft);

        if ((pihd->ihd_nBytesLeft == -1) && (! pihd->ihd_bChunked))
        {
            /*This request has no body.*/
            pihd->ihd_nBytesLeft = 0;
        }
    }

    return u32Ret;
}

static u32 _parseBodyInHpDo(
    internal_httpparser_dataobject_t * pihd, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, olsize_t sHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph = NULL;
    olsize_t zero = 0;

    /*There is still data to read. Lets see if any of the body arrived yet.*/
    if (! pihd->ihd_bChunked)
    {
        /*This isn't chunked, process normally.*/
        if (pihd->ihd_nBytesLeft != -1 &&
            (sEndPointer - (*psBeginPointer)) - (sHeader + 4) >= pihd->ihd_nBytesLeft)
        {
            /*Get the whole packet.*/
            JF_LOGGER_DEBUG("got entire packet");
            jf_httpparser_setBody(
                pihd->ihd_pjhphHeader, pu8Buffer + sHeader + 4, pihd->ihd_nBytesLeft, FALSE);
            /*Have the entire body, so we have the entire packet.*/
            pihd->ihd_bFullPacket = TRUE;

            *psBeginPointer = *psBeginPointer + sHeader + 4 + pihd->ihd_nBytesLeft;
        }
        else
        {
            /*Read some of the body, but not all of it yet.*/
            JF_LOGGER_DEBUG("got partial packet");
            *psBeginPointer = sHeader + 4;

            /*Clone the packet header, as the buffer will be used to receive body later.*/
            u32Ret = jf_httpparser_clonePacketHeader(&pjhph, pihd->ihd_pjhphHeader);

            /*Destroy the original packet header.*/
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_httpparser_destroyPacketHeader(&pihd->ihd_pjhphHeader);
                pihd->ihd_pjhphHeader = pjhph;
            }

            /*Allocate memory if the body size is larger than buffer size.*/
            if ((u32Ret == JF_ERR_NO_ERROR) && (pihd->ihd_nBytesLeft > pihd->ihd_sBuffer))
            {
                /*Buffer is not enough to hold the HTTP body.*/
                JF_LOGGER_DEBUG("Alloc memory for body");
                u32Ret = jf_jiukun_allocMemory(
                    (void **)&pihd->ihd_pu8BodyBuf, pihd->ihd_nBytesLeft);
            }
        }
    }
    else
    {
        /*This packet is chunk encoded, need to run it through our chunk processor.*/
        u32Ret = jf_httpparser_processChunk(
            pihd->ihd_pjhcpProcessor, pihd->ihd_pjhphHeader, pu8Buffer + sHeader + 4,
            &zero, (sEndPointer - (*psBeginPointer) - (sHeader + 4)));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            *psBeginPointer = sHeader + 4 + zero;

            /*Test the body pointer.*/
            if (pihd->ihd_pjhphHeader->jhph_pu8Body != NULL)
            {
                /*Full packet is received.*/
                pihd->ihd_bFullPacket = TRUE;
            }
            else
            {
                /*Header doesn't have body, it means the chunk is not completed processed.
                  Clone the packet header, as the buffer will be used to receive body later.*/
                u32Ret = jf_httpparser_clonePacketHeader(&pjhph, pihd->ihd_pjhphHeader);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    jf_httpparser_destroyPacketHeader(&pihd->ihd_pjhphHeader);
                    pihd->ihd_pjhphHeader = pjhph;
                }
            }
        }
    }

    return u32Ret;
}

static u32 _parseHeaderInHpDo(
    internal_httpparser_dataobject_t * pihd, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, olsize_t sHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("sHeader: %d", sHeader);

    /*Initalize the variable for parse.*/
    pihd->ihd_nBytesLeft = -1;
    pihd->ihd_bFinHeader = TRUE;
    pihd->ihd_bFullPacket = FALSE;
    pihd->ihd_bChunked = FALSE;

    /*Parse the buffer.*/
    u32Ret = jf_httpparser_parsePacketHeader(
        &pihd->ihd_pjhphHeader, (olchar_t *)pu8Buffer, *psBeginPointer,
        sEndPointer - (*psBeginPointer));

    /*Parse header content.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseHeaderContentInHpDo(pihd);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pihd->ihd_nBytesLeft == 0)
        {
            /*No body.*/
            pihd->ihd_bFullPacket = TRUE;

            *psBeginPointer = *psBeginPointer + sHeader + 4;
        }
        else
        {
            /*Parse the body.*/
            u32Ret = _parseBodyInHpDo(
                pihd, pu8Buffer, psBeginPointer, sEndPointer, sHeader);
        }
    }

    return u32Ret;
}

static u32 _processNonChunkedBodyInHpDo(
    internal_httpparser_dataobject_t * pihd, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t Fini = 0;

    if (pihd->ihd_pu8BodyBuf == NULL)
    {
        /*The buffer can hold the whole body.*/
        Fini = sEndPointer - *psBeginPointer - pihd->ihd_nBytesLeft;
        JF_LOGGER_DEBUG("Fini: %d", Fini);
        if (Fini >= 0)
        {
            /*Get full packet.*/
            jf_httpparser_setBody(
                pihd->ihd_pjhphHeader, pu8Buffer + *psBeginPointer, pihd->ihd_nBytesLeft, FALSE);

            pihd->ihd_bFullPacket = TRUE;
            *psBeginPointer = *psBeginPointer + pihd->ihd_nBytesLeft;
        }
    }
    else
    {
        /*The buffer cannot hold the whole body, copy the buffer to another buffer.*/
        Fini = pihd->ihd_u32BodyOffset + sEndPointer - *psBeginPointer - pihd->ihd_nBytesLeft;
        JF_LOGGER_DEBUG("Fini: %d, offset: %u", Fini, pihd->ihd_u32BodyOffset);
        if (Fini >= 0)
        {
            /*Get all data.*/
            Fini = pihd->ihd_nBytesLeft - pihd->ihd_u32BodyOffset;
            /*Copy date from caller's buffer to body buffer.*/
            ol_memcpy(
                pihd->ihd_pu8BodyBuf + pihd->ihd_u32BodyOffset, pu8Buffer + *psBeginPointer, Fini);
            jf_httpparser_setBody(
                pihd->ihd_pjhphHeader, pihd->ihd_pu8BodyBuf, pihd->ihd_nBytesLeft, FALSE);
            pihd->ihd_bFullPacket = TRUE;
            *psBeginPointer = *psBeginPointer + Fini;
        }
        else
        {
            /*Get partial data.*/
            Fini = sEndPointer - *psBeginPointer;
            ol_memcpy(
                pihd->ihd_pu8BodyBuf + pihd->ihd_u32BodyOffset, pu8Buffer + *psBeginPointer, Fini);
            pihd->ihd_u32BodyOffset += Fini;
            *psBeginPointer = sEndPointer;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_httpparser_processDataobject(
    jf_httpparser_dataobject_t * pDataobject, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_dataobject_t * pihd = (internal_httpparser_dataobject_t *) pDataobject;
    olsize_t sHeader = 0;

#ifdef DEBUG_HTTPPARSER
    JF_LOGGER_DEBUG("pointer: %d:%d", *psBeginPointer, sEndPointer);
#endif

/*
    JF_LOGGER_DATAA(
        pu8Buffer + *psBeginPointer, sEndPointer - *psBeginPointer, "HTTP data");
*/

    if (! pihd->ihd_bFinHeader)
    {
        /*Still Reading Headers.*/
        u32Ret = jf_httpparser_findHeader(pu8Buffer, *psBeginPointer, sEndPointer, &sHeader);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Parse the header.*/
            u32Ret = _parseHeaderInHpDo(
                pihd, pu8Buffer, psBeginPointer, sEndPointer, sHeader);
        }
    }
    else
    {
        /*We already processed the headers, so we are only expecting the body now.*/
        if (! pihd->ihd_bChunked)
        {
            /*This isn't chunk encoded.*/
            u32Ret = _processNonChunkedBodyInHpDo(
                pihd, pu8Buffer, psBeginPointer, sEndPointer);
        }
        else
        {
            /*This is chunk encoded, so run it through our chunk processor.*/
            u32Ret = jf_httpparser_processChunk(
                pihd->ihd_pjhcpProcessor, pihd->ihd_pjhphHeader, pu8Buffer, psBeginPointer,
                sEndPointer);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*It's full packet if the body is not NULL.*/
                if (pihd->ihd_pjhphHeader->jhph_pu8Body != NULL)
                {
                    pihd->ihd_bFullPacket = TRUE;
                }
            }
        }
    }

    return u32Ret;
}

u32 jf_httpparser_destroyDataobject(jf_httpparser_dataobject_t ** ppDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_dataobject_t * pihd = (internal_httpparser_dataobject_t *) *ppDataobject;

    JF_LOGGER_DEBUG("destroy");

    jf_httpparser_reinitDataobject(pihd);

    jf_jiukun_freeMemory(ppDataobject);

    return u32Ret;
}

u32 jf_httpparser_createtDataobject(jf_httpparser_dataobject_t ** ppDataobject, olsize_t sBuffer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_dataobject_t * pihd = NULL;

    JF_LOGGER_DEBUG("sBuffer: %d", sBuffer);

    u32Ret = jf_jiukun_allocMemory((void **)&pihd, sizeof(*pihd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pihd, sizeof(*pihd));

        pihd->ihd_sBuffer = sBuffer;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDataobject = pihd;
    else if (pihd != NULL)
        jf_httpparser_destroyDataobject((jf_httpparser_dataobject_t **)&pihd);

    return u32Ret;
}

u32 jf_httpparser_reinitDataobject(jf_httpparser_dataobject_t * pDataobject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_httpparser_dataobject_t * pihd = (internal_httpparser_dataobject_t *) pDataobject;

    JF_LOGGER_DEBUG("reinit");

    /*Reset the flags.*/
    pihd->ihd_bFinHeader = FALSE;
    pihd->ihd_bChunked = FALSE;
    pihd->ihd_bFullPacket = FALSE;

    /*Free body buffer.*/
    pihd->ihd_u32BodyOffset = 0;
    if (pihd->ihd_pu8BodyBuf != NULL)
        jf_jiukun_freeMemory((void **)&pihd->ihd_pu8BodyBuf);

    /*Destroy chunk processor.*/
    if (pihd->ihd_pjhcpProcessor != NULL)
        jf_httpparser_destroyChunkProcessor(&pihd->ihd_pjhcpProcessor);

    /*Destory packet header.*/
    if (pihd->ihd_pjhphHeader != NULL)
        jf_httpparser_destroyPacketHeader(&pihd->ihd_pjhphHeader);

    return u32Ret;
}

boolean_t jf_httpparser_getDataobjectFullPacket(
    jf_httpparser_dataobject_t * pDataobject, jf_httpparser_packet_header_t ** ppPacket)
{
    internal_httpparser_dataobject_t * pihd = (internal_httpparser_dataobject_t *) pDataobject;

    *ppPacket = pihd->ihd_pjhphHeader;

    return pihd->ihd_bFullPacket;
}

/*------------------------------------------------------------------------------------------------*/
