/**
 *  @file httpparser.c
 *
 *  @brief The http parser
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
#include "olbasic.h"
#include "ollimit.h"
#include "httpparser.h"
#include "errcode.h"
#include "xmalloc.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

static u32 _parseHttpStartLine(
    jf_httpparser_packet_header_t * retval, jf_string_parse_result_field_t * field)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * startline = NULL;
    olint_t nret;
    jf_string_parse_result_t * result;
    jf_string_parse_result_field_t * pjsprf;

    /* The first token is where we can figure out the method, path, version,
       etc.*/
    u32Ret = jf_string_parse(
        &startline, field->jsprf_pstrData, 0, field->jsprf_sData, " ", 1);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (startline->jspr_u32NumOfResult < 3)
            u32Ret = JF_ERR_CORRUPTED_HTTP_MSG;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        nret = ol_memcmp(startline->jspr_pjsprfFirst->jsprf_pstrData, "HTTP/", 5);
        if (nret == 0)
        {
            /*If the startline starts with HTTP/, then this is a response packet.
              We parse on the '/' character to determine the version, as it
              follows. 
              eg: HTTP/1.1 200 OK */
            u32Ret = jf_string_parse(
                &result, startline->jspr_pjsprfFirst->jsprf_pstrData, 0,
                startline->jspr_pjsprfFirst->jsprf_sData, "/", 1);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                retval->jhph_pstrVersion = result->jspr_pjsprfLast->jsprf_pstrData;
                retval->jhph_sVersion = result->jspr_pjsprfLast->jsprf_sData;

                jf_string_destroyParseResult(&result);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /* The other tokens contain the status code and data */
                pjsprf = startline->jspr_pjsprfFirst->jsprf_pjsprfNext;
                u32Ret = jf_string_getS32FromString(
                    pjsprf->jsprf_pstrData, pjsprf->jsprf_sData,
                    &retval->jhph_nStatusCode);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pjsprf = pjsprf->jsprf_pjsprfNext;
                retval->jhph_pstrStatusData = pjsprf->jsprf_pstrData;
                retval->jhph_sStatusData = pjsprf->jsprf_sData;

                pjsprf = pjsprf->jsprf_pjsprfNext;
                while (pjsprf != NULL)
                {
                    retval->jhph_sStatusData += pjsprf->jsprf_sData + 1;
                    pjsprf = pjsprf->jsprf_pjsprfNext;
                }
            }
        }
        else
        {
            /* If the packet didn't start with HTTP/ then we know it's a request
               packet
               eg: GET /index.html HTTP/1.1
               The method (or directive), is the first token, and the Path
               (or jhph_pstrDirectiveObj) is the second, and version in the 3rd. */
            pjsprf = startline->jspr_pjsprfFirst;
            retval->jhph_pstrDirective = pjsprf->jsprf_pstrData;
            retval->jhph_sDirective = pjsprf->jsprf_sData;

            pjsprf = pjsprf->jsprf_pjsprfNext;
            retval->jhph_pstrDirectiveObj = pjsprf->jsprf_pstrData;
            retval->jhph_sDirectiveObj = pjsprf->jsprf_sData;
            retval->jhph_nStatusCode = -1;

            /* We parse the last token on '/' to find the version */
            u32Ret = jf_string_parse(
                &result, startline->jspr_pjsprfLast->jsprf_pstrData, 0,
                startline->jspr_pjsprfLast->jsprf_sData, "/", 1);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                retval->jhph_pstrVersion = result->jspr_pjsprfLast->jsprf_pstrData;
                retval->jhph_sVersion = result->jspr_pjsprfLast->jsprf_sData;

                jf_string_destroyParseResult(&result);
            }
        }
    }

    if (startline != NULL)
        jf_string_destroyParseResult(&startline);

    return u32Ret;
}

static u32 _parseHttpHeaderLine(
    jf_httpparser_packet_header_t * retval, jf_string_parse_result_field_t * field)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_field_t * headerline = field;
    jf_httpparser_packet_header_field_t * node;
    olint_t i = 0;
    olint_t FLNWS = -1;
    olint_t FTNWS = -1;

    /* Headerline starts with the second token. Then we iterate through
       the rest of the tokens*/
    while (headerline != NULL)
    {
        if (headerline->jsprf_sData == 0)
        {
            /* An empty line signals the end of the headers */
            break;
        }

        /* Instantiate a new header entry for each token */
        u32Ret = jf_mem_calloc((void **)&node, sizeof(jf_httpparser_packet_header_field_t));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (i = 0; i < headerline->jsprf_sData; ++i)
            {
                if (*((headerline->jsprf_pstrData) + i) == ':')
                {
                    node->jhphf_pstrName = headerline->jsprf_pstrData;
                    node->jhphf_sName = i;
                    node->jhphf_pstrData = headerline->jsprf_pstrData + i + 1;
                    node->jhphf_sData = (headerline->jsprf_sData) - i - 1;
                    break;
                }
            }
            if (node->jhphf_pstrName == NULL)
            {
                jf_mem_free((void **)&node);
                break;
            }

            /*We need to do white space processing, because we need to
              ignore them in the headers*/
            FLNWS = 0;
            FTNWS = node->jhphf_sData - 1;
            for (i = 0; i < node->jhphf_sData; ++i)
            {
                if (*((node->jhphf_pstrData) + i) != ' ')
                {
                    /* the first non-whitespace character */
                    FLNWS = i;
                    break;
                }
            }
            for (i = node->jhphf_sData - 1; i >= 0; --i)
            {
                if (*(node->jhphf_pstrData + i) != ' ')
                {
                    /* The last non-whitespace character */
                    FTNWS = i;
                    break;
                }
            }

            /* We are basically doing a 'trim' operation */
            node->jhphf_pstrData += FLNWS;
            node->jhphf_sData = (FTNWS - FLNWS) + 1;

            /*Since we are parsing an existing string, we set this flag to
              zero, so that it doesn't get freed*/
            node->jhphf_pjhphfNext = NULL;

            if (retval->jhph_pjhphfFirst == NULL)
            {
                /* If there aren't any headers yet, this will be the first*/
                retval->jhph_pjhphfFirst = node;
                retval->jhph_pjhphfLast = node;
            }
            else
            {
                /* There are already headers, so link this in the tail*/
                retval->jhph_pjhphfLast->jhphf_pjhphfNext = node;
            }
            retval->jhph_pjhphfLast = node;

            headerline = headerline->jsprf_pjsprfNext;
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_httpparser_destroyPacketHeader(jf_httpparser_packet_header_t ** ppHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * packet = *ppHeader;
    jf_httpparser_packet_header_field_t *node = packet->jhph_pjhphfFirst;
    jf_httpparser_packet_header_field_t *nextnode;

    /* Iterate through all the headers */
    while (node != NULL)
    {
        nextnode = node->jhphf_pjhphfNext;
        if (node->jhphf_bAlloc)
        {
            /* If the user allocated the string, then we need to free it.
               Otherwise these are just pointers into others strings, in which
               case we don't want to free them */
            jf_mem_free((void **)&node->jhphf_pstrName);
            jf_mem_free((void **)&node->jhphf_pstrData);
        }
        jf_mem_free((void **)&node);
        node = nextnode;
    }

    /* If this flag was set, it means the used createEmptyPacketHeader,
       and set these fields manually, which means the string was copied.
       In which case, we need to free the strings */
    if (packet->jhph_bAllocStatus && packet->jhph_pstrStatusData != NULL)
        jf_mem_free((void **)&packet->jhph_pstrStatusData);

    if (packet->jhph_bAllocDirective)
    {
        if (packet->jhph_pstrDirective != NULL)
            jf_mem_free((void **)&packet->jhph_pstrDirective);

        if (packet->jhph_pstrDirectiveObj != NULL)
            jf_mem_free((void **)&packet->jhph_pstrDirectiveObj);
    }

    if (packet->jhph_bAllocVersion && packet->jhph_pstrVersion != NULL)
        jf_mem_free((void **)&packet->jhph_pstrVersion);

    if (packet->jhph_bAllocBody && packet->jhph_pu8Body != NULL)
        jf_mem_free((void **)&packet->jhph_pu8Body);

    jf_mem_free((void **)ppHeader);

    return u32Ret;
}

/** Escapes a string according to HTTP Specifications.
 *
 *  @note The string to escape would typically be the string used in the path
 *   portion of an HTTP request. eg:
 *   GET foo/bar.txt HTTP/1.1
 *
 *  @note It should be noted that the output buffer needs to be allocated prior
 *   to calling this method. The required space can be determined by calling
 *   getHttpEscapeDataLen.
 *
 *  @param outdata [out] the escaped string
 *  @param data [in] the string to escape
 *
 *  @return the length of the escaped string
 */
olint_t jf_httpparser_escapeHttpData(u8 * outdata, const u8 * data)
{
    olint_t i = 0;
    olint_t x = 0;
    olchar_t hex[4];

    while (data[x] != 0)
    {
        if ((data[x] >= 63 && data[x] <= 90) ||
            (data[x] >= 97 && data[x] <= 122) ||
            (data[x] >= 47 && data[x] <= 57) ||
            data[x] == 59 || data[x] == 47 || data[x] == 63 || data[x] == 58 ||
            data[x] == 64 || data[x] == 61 || data[x] == 43 || data[x] == 36 ||
            data[x] == 45 || data[x] == 95 || data[x] == 46 || data[x] == 42)
        {
            /* These are all the allowed values for HTTP. If it's one of these
               characters, we're ok */
            outdata[i] = data[x];
            ++i;
        }
        else
        {
            /* If it wasn't one of these characters, then we need to escape it*/
            ol_sprintf(hex, "%02X", (u8) data[x]);
            outdata[i] = '%';
            outdata[i + 1] = hex[0];
            outdata[i + 2] = hex[1];
            i += 3;
        }
        ++x;
    }
    outdata[i] = 0;
    return (i + 1);
}

/** Determines the buffer space required to HTTP escape a particular string.
 *
 *  @param data [in] calculates the length requirements as if this string was
 *   escaped
 *
 *  @return the minimum required length
 */
olint_t jf_httpparser_getHttpEscapeDataLen(const u8 *data)
{
    olint_t i = 0;
    olint_t x = 0;

    while (data[x] != 0)
    {
        if ((data[x] >= 63 && data[x] <= 90) ||
            (data[x] >= 97 && data[x] <= 122) ||
            (data[x] >= 47 && data[x] <= 57) ||
            data[x] == 59 || data[x] == 47 || data[x] == 63 || data[x] == 58 ||
            data[x] == 64 || data[x] == 61 || data[x] == 43 || data[x] == 36 ||
            data[x] == 45 || data[x] == 95 || data[x] == 46 || data[x] == 42)
        {
            /*No need to escape*/
            ++i;
        }
        else
        {
            /*Need to escape*/
            i += 3;
        }
        ++x;
    }
    return (i + 1);
}

/** Unescaped a given string
 *
 *  The escaped representation of a string is always longer than the unescaped
 *  version so this method will overwrite the escaped string, with the unescaped
 *  result.
 *
 *  @param data [in] the buffer to unescape
 *
 *  @return the length of the unescaped string
 */
olint_t jf_httpparser_unescapeHttpData(olchar_t * data)
{
    olchar_t hex[3];
    u8 *stp;
    olint_t src_x = 0;
    olint_t dst_x = 0;

    olint_t length = (olint_t) ol_strlen(data);

    hex[2] = 0;

    while (src_x < length)
    {
        if (strncmp(data + src_x, "%", 1) == 0)
        {
            /* Since we encountered a '%' we know this is an escaped character*/
            hex[0] = data[src_x + 1];
            hex[1] = data[src_x + 2];
            data[dst_x] = (u8) strtol(hex, (olchar_t **)&stp, 16);
            dst_x += 1;
            src_x += 3;
        }
        else if (src_x != dst_x)
        {
            /* This doesn't need to be unescaped. If we didn't unescape anything
               previously there is no need to copy the string either */
            data[dst_x] = data[src_x];
            src_x += 1;
            dst_x += 1;
        }
        else
        {
            /* This doesn't need to be unescaped, however we need to copy the
               string */
            src_x += 1;
            dst_x += 1;
        }
    }
    return (dst_x);
}

u32 jf_httpparser_parsePacketHeader(
    jf_httpparser_packet_header_t ** ppHeader, olchar_t * pstrBuf,
    olsize_t sOffset, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * retval = NULL;
    jf_string_parse_result_t * pPacket = NULL;
    jf_string_parse_result_field_t * headerline;
    jf_string_parse_result_field_t * field;

    u32Ret = jf_mem_calloc((void **)&retval, sizeof(jf_httpparser_packet_header_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* All the headers are delineated with a CRLF, so we parse on that */
        u32Ret = jf_string_parse(&pPacket, pstrBuf, sOffset, sBuf, "\r\n", 2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        field = pPacket->jspr_pjsprfFirst;
        headerline = field->jsprf_pjsprfNext;

        u32Ret = _parseHttpStartLine(retval, field);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseHttpHeaderLine(retval, headerline);
    }

    if (pPacket != NULL)
        jf_string_destroyParseResult(&pPacket);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppHeader = retval;
    else if (retval != NULL)
        jf_httpparser_destroyPacketHeader(&retval);

    return u32Ret;
}

u32 jf_httpparser_getRawPacket(
    jf_httpparser_packet_header_t * pjhph, olchar_t ** ppstrBuf,
    olsize_t * psBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t total;
    olsize_t sBuffer = 0;
    olchar_t *pBuffer;
    jf_httpparser_packet_header_field_t *node;

    *ppstrBuf = NULL;
    *psBuf = 0;

    if (pjhph->jhph_nStatusCode != -1)
    {
        /* HTTP/1.1 200 OK\r\n
           12 is the total number of literal characters. Just add Version and
           StatusData */
        sBuffer = 12 + pjhph->jhph_sVersion + pjhph->jhph_sStatusData;
    }
    else
    {

        /* GET / HTTP/1.1\r\n 
           This is calculating the length for a request packet. It will work as
           long as the version is not > 9.9
           It should also add the length of the Version, but it's not critical.*/
        sBuffer = pjhph->jhph_sDirective + pjhph->jhph_sDirectiveObj + 12;
    }

    node = pjhph->jhph_pjhphfFirst;
    while (node != NULL)
    {
        /* A conservative estimate adding the lengths of the header name and
           value, plus 4 characters for the ':' and CRLF */
        sBuffer += node->jhphf_sName + node->jhphf_sData + 4;
        node = node->jhphf_pjhphfNext;
    }

    /* Another conservative estimate adding in the packet body length plus a
       padding of 3 for the empty line */
    sBuffer += (3 + pjhph->jhph_sBody);

    /* Allocate the buffer */
    u32Ret = jf_mem_alloc((void **)ppstrBuf, sBuffer);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    pBuffer = *ppstrBuf;
    if (pjhph->jhph_nStatusCode != -1)
    {
        /* Write the response */
        memcpy(pBuffer, "HTTP/", 5);
        memcpy(pBuffer + 5, pjhph->jhph_pstrVersion, pjhph->jhph_sVersion);
        total = 5 + pjhph->jhph_sVersion;

        total += ol_sprintf(pBuffer + total, " %d ", pjhph->jhph_nStatusCode);
        memcpy(pBuffer + total, pjhph->jhph_pstrStatusData, pjhph->jhph_sStatusData);
        total += pjhph->jhph_sStatusData;

        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        /* HTTP/1.1 200 OK\r\n */
    }
    else
    {
        /* Write the Request */
        memcpy(pBuffer, pjhph->jhph_pstrDirective, pjhph->jhph_sDirective);
        total = pjhph->jhph_sDirective;
        memcpy(pBuffer + total, " ", 1);
        total += 1;
        memcpy(pBuffer + total, pjhph->jhph_pstrDirectiveObj,
               pjhph->jhph_sDirectiveObj);
        total += pjhph->jhph_sDirectiveObj;
        memcpy(pBuffer + total, " HTTP/", 6);
        total += 6;
        memcpy(pBuffer + total, pjhph->jhph_pstrVersion, pjhph->jhph_sVersion);
        total += pjhph->jhph_sVersion;
        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        /* GET / HTTP/1.1\r\n */
    }

    node = pjhph->jhph_pjhphfFirst;
    while (node != NULL)
    {
        /* Write each header */
        memcpy(pBuffer + total, node->jhphf_pstrName, node->jhphf_sName);
        total += node->jhphf_sName;
        memcpy(pBuffer + total, ": ", 2);
        total += 2;
        memcpy(pBuffer + total, node->jhphf_pstrData, node->jhphf_sData);
        total += node->jhphf_sData;
        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        sBuffer += node->jhphf_sName + node->jhphf_sData + 4;
        node = node->jhphf_pjhphfNext;
    }

    /* Write the empty line */
    memcpy(pBuffer + total, "\r\n", 2);
    total += 2;

    /* Write the body */
    memcpy(pBuffer + total, pjhph->jhph_pu8Body, pjhph->jhph_sBody);
    total += pjhph->jhph_sBody;
    pBuffer[total] = '\0';

    *psBuf = total;

    return u32Ret;
}

u32 jf_httpparser_parseUri(
    olchar_t * pstrUri, olchar_t ** ppstrIp, u16 * pu16Port,
    olchar_t ** ppstrPath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t *result = NULL, *result2 = NULL, *result3 = NULL;
    olchar_t * str1;
    olsize_t len1, len2;

    *ppstrIp = NULL;
    *ppstrPath = NULL;

    /* A scheme has the format xxx://yyy , so if we parse on ://, we can extract
       the path info */
    u32Ret = jf_string_parse(
        &result, pstrUri, 0, (olint_t) ol_strlen(pstrUri), "://", 3);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (result->jspr_u32NumOfResult < 2)
            u32Ret = JF_ERR_INVALID_HTTP_URI;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        str1 = result->jspr_pjsprfLast->jsprf_pstrData;
        len1 = result->jspr_pjsprfLast->jsprf_sData;

        /* Parse path. The first '/' will occur after the IPAddress:port
           combination*/
        u32Ret = jf_string_parse(&result2, str1, 0, len1, "/", 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        len2 = len1 - result2->jspr_pjsprfFirst->jsprf_sData;

        u32Ret = jf_string_duplicateWithLen(
            ppstrPath, str1 + result2->jspr_pjsprfFirst->jsprf_sData, len2);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /* Parse port Number */
            u32Ret = jf_string_parse(
                &result3, result2->jspr_pjsprfFirst->jsprf_pstrData,
                0, result2->jspr_pjsprfFirst->jsprf_sData, ":", 1);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (result3->jspr_u32NumOfResult == 1)
        {
            /* The default port is 80, if non is specified, because we are
               assuming an HTTP scheme*/
            *pu16Port = 80;
        }
        else
        {
            /* If a port was specified, use that */
            u32Ret = jf_string_getU16FromString(
                result3->jspr_pjsprfLast->jsprf_pstrData,
                result3->jspr_pjsprfLast->jsprf_sData, pu16Port);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* Parse IP Address */
        u32Ret = jf_string_duplicateWithLen(
            ppstrIp, result3->jspr_pjsprfFirst->jsprf_pstrData,
            result3->jspr_pjsprfFirst->jsprf_sData);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        if (*ppstrIp != NULL)
            jf_mem_free((void **)ppstrIp);

        if (*ppstrPath != NULL)
            jf_mem_free((void **)ppstrPath);
    }

    if (result3 != NULL)
        jf_string_destroyParseResult(&result3);

    if (result2 != NULL)
        jf_string_destroyParseResult(&result2);

    if (result != NULL)
        jf_string_destroyParseResult(&result);

    return u32Ret;
}

u32 jf_httpparser_createEmptyPacketHeader(
    jf_httpparser_packet_header_t ** ppHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * retval = NULL;

    u32Ret = jf_mem_calloc((void **)&retval, sizeof(jf_httpparser_packet_header_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        retval->jhph_nStatusCode = -1;
        retval->jhph_pstrVersion = "1.0";
        retval->jhph_sVersion = 3;

        *ppHeader = retval;
    }
    
    return u32Ret;
}

u32 jf_httpparser_clonePacketHeader(
    jf_httpparser_packet_header_t ** dest, jf_httpparser_packet_header_t * pjhph)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t *retval;
    jf_httpparser_packet_header_field_t *n;

    u32Ret = jf_httpparser_createEmptyPacketHeader(&retval);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;
    
    /*These three calls will result in the fields being copied*/
    jf_httpparser_setDirective(
        retval, pjhph->jhph_pstrDirective, pjhph->jhph_sDirective,
        pjhph->jhph_pstrDirectiveObj, pjhph->jhph_sDirectiveObj);

    jf_httpparser_setStatusCode(
        retval, pjhph->jhph_nStatusCode, pjhph->jhph_pstrStatusData,
        pjhph->jhph_sStatusData);

    jf_httpparser_setVersion(
        retval, pjhph->jhph_pstrVersion, pjhph->jhph_sVersion);
    
    /*Iterate through each header, and copy them*/
    n = pjhph->jhph_pjhphfFirst;
    while (n != NULL)
    {
        jf_httpparser_addHeaderLine(
            retval, n->jhphf_pstrName, n->jhphf_sName,
            n->jhphf_pstrData, n->jhphf_sData, TRUE);
        n = n->jhphf_pjhphfNext;
    }

    if (pjhph->jhph_pu8Body != NULL)
        u32Ret = jf_httpparser_setBody(
            retval, pjhph->jhph_pu8Body, pjhph->jhph_sBody, TRUE);

    if (u32Ret == JF_ERR_NO_ERROR)
        *dest = retval;
    else if (retval != NULL)
        jf_httpparser_destroyPacketHeader(&retval);

    return u32Ret;
}

u32 jf_httpparser_setVersion(
    jf_httpparser_packet_header_t * pjhph, olchar_t * pstrVersion,
    olsize_t sVersion)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrVersion != NULL)
        u32Ret = jf_string_duplicateWithLen(
            &(pjhph->jhph_pstrVersion), pstrVersion, sVersion);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjhph->jhph_sVersion = sVersion;
        pjhph->jhph_bAllocVersion = TRUE;
    }

    return u32Ret;
}

u32 jf_httpparser_setStatusCode(
    jf_httpparser_packet_header_t * pjhph, olint_t nStatusCode,
    olchar_t * pstrStatusData, olsize_t sStatusData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pjhph->jhph_nStatusCode = nStatusCode;

    if (pstrStatusData != NULL)
        u32Ret = jf_string_duplicateWithLen(&(pjhph->jhph_pstrStatusData),
            pstrStatusData, sStatusData);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjhph->jhph_sStatusData = sStatusData;
        pjhph->jhph_bAllocStatus = TRUE;
    }

    return u32Ret;
}

u32 jf_httpparser_setDirective(
    jf_httpparser_packet_header_t * pjhph, olchar_t * pstrDirective,
    olsize_t sDirective, olchar_t * pstrDirectiveObj, olsize_t sDirectiveObj)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pstrDirective != NULL)
        u32Ret = jf_string_duplicateWithLen(&(pjhph->jhph_pstrDirective), pstrDirective, sDirective);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjhph->jhph_sDirective = sDirective;

        if (pstrDirectiveObj != NULL)
            u32Ret = jf_string_duplicateWithLen(&(pjhph->jhph_pstrDirectiveObj),
                pstrDirectiveObj, sDirectiveObj);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pjhph->jhph_sDirectiveObj = sDirectiveObj;
            pjhph->jhph_bAllocDirective = TRUE;
        }
        else
        {
            jf_string_free(&(pjhph->jhph_pstrDirective));
        }
    }

    return u32Ret;
}

u32 jf_httpparser_setBody(
    jf_httpparser_packet_header_t * pjhph, u8 * pu8Body, olsize_t sBody,
    boolean_t bAlloc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (bAlloc)
    {
        u32Ret = jf_mem_duplicate((void **)&pjhph->jhph_pu8Body, pu8Body, sBody);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pjhph->jhph_bAllocBody = bAlloc;
            pjhph->jhph_sBody = sBody;
        }
    }
    else
    {
        pjhph->jhph_bAllocBody = bAlloc;
        pjhph->jhph_pu8Body = pu8Body;
        pjhph->jhph_sBody = sBody;
    }

    return u32Ret;
}

u32 jf_httpparser_addHeaderLine(
    jf_httpparser_packet_header_t * pjhph, olchar_t * pstrName,
    olsize_t sName, olchar_t * pstrData, olsize_t sData, boolean_t bAlloc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_field_t * node = NULL;
    
    /* Create the header node */
    u32Ret = jf_mem_calloc((void **)&node, sizeof(jf_httpparser_packet_header_field_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        node->jhphf_bAlloc = bAlloc;
        if (! bAlloc)
        {
            node->jhphf_pstrName = pstrName;
            node->jhphf_sName = sName;
            node->jhphf_pstrData = pstrData;
            node->jhphf_sData = sData;
        }
        else
        {
            u32Ret = jf_string_duplicateWithLen(&(node->jhphf_pstrName), pstrName, sName);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                node->jhphf_sName = sName;

                u32Ret = jf_string_duplicateWithLen(&(node->jhphf_pstrData), pstrData, sData);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                node->jhphf_sData = sData;
            }
            else
            {
                if (node->jhphf_pstrName != NULL)
                    jf_string_free(&(node->jhphf_pstrName));

                if (node->jhphf_pstrData != NULL)
                    jf_string_free(&(node->jhphf_pstrData));

                jf_mem_free((void **)&node);
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* attach it to the linked list */
        if(pjhph->jhph_pjhphfLast != NULL)
        {
            pjhph->jhph_pjhphfLast->jhphf_pjhphfNext = node;
            pjhph->jhph_pjhphfLast = node;
        }
        else
        {
            pjhph->jhph_pjhphfLast = node;
            pjhph->jhph_pjhphfFirst = node;
        }
    }

    return u32Ret;
}

u32 jf_httpparser_getHeaderLine(
    jf_httpparser_packet_header_t * pjhph, olchar_t * pstrName, olsize_t sName,
    jf_httpparser_packet_header_field_t ** ppHeader)
{
    jf_httpparser_packet_header_field_t * node = pjhph->jhph_pjhphfFirst;

    *ppHeader = NULL;
    
    /*Iterate through the headers, until we find the one we're interested in*/
    while (node != NULL)
    {
        if (strncasecmp(pstrName, node->jhphf_pstrName, sName) == 0)
        {
            *ppHeader = node;

            return JF_ERR_NO_ERROR;
        }
        node = node->jhphf_pjhphfNext;
    }
    
    return JF_ERR_NOT_FOUND;
}

/*---------------------------------------------------------------------------*/

