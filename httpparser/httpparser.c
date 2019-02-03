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
    packet_header_t * retval, parse_result_field_t * field)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * startline = NULL;
    olint_t nret;
    parse_result_t * result;
    parse_result_field_t * pprf;

    /* The first token is where we can figure out the method, path, version,
       etc.*/
    u32Ret = parseString(
        &startline, field->prf_pstrData, 0, field->prf_sData, " ", 1);

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (startline->pr_u32NumOfResult < 3)
            u32Ret = OLERR_CORRUPTED_HTTP_MSG;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        nret = ol_memcmp(startline->pr_pprfFirst->prf_pstrData, "HTTP/", 5);
        if (nret == 0)
        {
            /*If the startline starts with HTTP/, then this is a response packet.
              We parse on the '/' character to determine the version, as it
              follows. 
              eg: HTTP/1.1 200 OK */
            u32Ret = parseString(
                &result, startline->pr_pprfFirst->prf_pstrData, 0,
                startline->pr_pprfFirst->prf_sData, "/", 1);
            if (u32Ret == OLERR_NO_ERROR)
            {
                retval->ph_pstrVersion = result->pr_pprfLast->prf_pstrData;
                retval->ph_sVersion = result->pr_pprfLast->prf_sData;

                destroyParseResult(&result);
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                /* The other tokens contain the status code and data */
                pprf = startline->pr_pprfFirst->prf_pprfNext;
                u32Ret = getS32FromString(
                    pprf->prf_pstrData, pprf->prf_sData,
                    &retval->ph_nStatusCode);
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                pprf = pprf->prf_pprfNext;
                retval->ph_pstrStatusData = pprf->prf_pstrData;
                retval->ph_sStatusData = pprf->prf_sData;

                pprf = pprf->prf_pprfNext;
                while (pprf != NULL)
                {
                    retval->ph_sStatusData += pprf->prf_sData + 1;
                    pprf = pprf->prf_pprfNext;
                }
            }
        }
        else
        {
            /* If the packet didn't start with HTTP/ then we know it's a request
               packet
               eg: GET /index.html HTTP/1.1
               The method (or directive), is the first token, and the Path
               (or ph_pstrDirectiveObj) is the second, and version in the 3rd. */
            pprf = startline->pr_pprfFirst;
            retval->ph_pstrDirective = pprf->prf_pstrData;
            retval->ph_sDirective = pprf->prf_sData;

            pprf = pprf->prf_pprfNext;
            retval->ph_pstrDirectiveObj = pprf->prf_pstrData;
            retval->ph_sDirectiveObj = pprf->prf_sData;
            retval->ph_nStatusCode = -1;

            /* We parse the last token on '/' to find the version */
            u32Ret = parseString(
                &result, startline->pr_pprfLast->prf_pstrData, 0,
                startline->pr_pprfLast->prf_sData, "/", 1);
            if (u32Ret == OLERR_NO_ERROR)
            {
                retval->ph_pstrVersion = result->pr_pprfLast->prf_pstrData;
                retval->ph_sVersion = result->pr_pprfLast->prf_sData;

                destroyParseResult(&result);
            }
        }
    }

    if (startline != NULL)
        destroyParseResult(&startline);

    return u32Ret;
}

static u32 _parseHttpHeaderLine(
    packet_header_t * retval, parse_result_field_t * field)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_field_t * headerline = field;
    packet_header_field_t * node;
    olint_t i = 0;
    olint_t FLNWS = -1;
    olint_t FTNWS = -1;

    /* Headerline starts with the second token. Then we iterate through
       the rest of the tokens*/
    while (headerline != NULL)
    {
        if (headerline->prf_sData == 0)
        {
            /* An empty line signals the end of the headers */
            break;
        }

        /* Instantiate a new header entry for each token */
        u32Ret = xcalloc((void **)&node, sizeof(packet_header_field_t));
        if (u32Ret == OLERR_NO_ERROR)
        {
            for (i = 0; i < headerline->prf_sData; ++i)
            {
                if (*((headerline->prf_pstrData) + i) == ':')
                {
                    node->phf_pstrName = headerline->prf_pstrData;
                    node->phf_sName = i;
                    node->phf_pstrData = headerline->prf_pstrData + i + 1;
                    node->phf_sData = (headerline->prf_sData) - i - 1;
                    break;
                }
            }
            if (node->phf_pstrName == NULL)
            {
                xfree((void **)&node);
                break;
            }

            /*We need to do white space processing, because we need to
              ignore them in the headers*/
            FLNWS = 0;
            FTNWS = node->phf_sData - 1;
            for (i = 0; i < node->phf_sData; ++i)
            {
                if (*((node->phf_pstrData) + i) != ' ')
                {
                    /* the first non-whitespace character */
                    FLNWS = i;
                    break;
                }
            }
            for (i = node->phf_sData - 1; i >= 0; --i)
            {
                if (*(node->phf_pstrData + i) != ' ')
                {
                    /* The last non-whitespace character */
                    FTNWS = i;
                    break;
                }
            }

            /* We are basically doing a 'trim' operation */
            node->phf_pstrData += FLNWS;
            node->phf_sData = (FTNWS - FLNWS) + 1;

            /*Since we are parsing an existing string, we set this flag to
              zero, so that it doesn't get freed*/
            node->phf_pphfNext = NULL;

            if (retval->ph_pphfFirst == NULL)
            {
                /* If there aren't any headers yet, this will be the first*/
                retval->ph_pphfFirst = node;
                retval->ph_pphfLast = node;
            }
            else
            {
                /* There are already headers, so link this in the tail*/
                retval->ph_pphfLast->phf_pphfNext = node;
            }
            retval->ph_pphfLast = node;

            headerline = headerline->prf_pprfNext;
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 destroyPacketHeader(packet_header_t ** ppHeader)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_t * packet = *ppHeader;
    packet_header_field_t *node = packet->ph_pphfFirst;
    packet_header_field_t *nextnode;

    /* Iterate through all the headers */
    while (node != NULL)
    {
        nextnode = node->phf_pphfNext;
        if (node->phf_bAlloc)
        {
            /* If the user allocated the string, then we need to free it.
               Otherwise these are just pointers into others strings, in which
               case we don't want to free them */
            xfree((void **)&node->phf_pstrName);
            xfree((void **)&node->phf_pstrData);
        }
        xfree((void **)&node);
        node = nextnode;
    }

    /* If this flag was set, it means the used createEmptyPacketHeader,
       and set these fields manually, which means the string was copied.
       In which case, we need to free the strings */
    if (packet->ph_bAllocStatus && packet->ph_pstrStatusData != NULL)
        xfree((void **)&packet->ph_pstrStatusData);

    if (packet->ph_bAllocDirective)
    {
        if (packet->ph_pstrDirective != NULL)
            xfree((void **)&packet->ph_pstrDirective);

        if (packet->ph_pstrDirectiveObj != NULL)
            xfree((void **)&packet->ph_pstrDirectiveObj);
    }

    if (packet->ph_bAllocVersion && packet->ph_pstrVersion != NULL)
        xfree((void **)&packet->ph_pstrVersion);

    if (packet->ph_bAllocBody && packet->ph_pu8Body != NULL)
        xfree((void **)&packet->ph_pu8Body);

    xfree((void **)ppHeader);

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
olint_t escapeHttpData(u8 * outdata, const u8 * data)
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
olint_t getHttpEscapeDataLen(const u8 *data)
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
olint_t unescapeHttpData(olchar_t * data)
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

u32 parsePacketHeader(
    packet_header_t ** ppHeader, olchar_t * pstrBuf,
    olsize_t sOffset, olsize_t sBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_t * retval = NULL;
    parse_result_t * pPacket = NULL;
    parse_result_field_t * headerline;
    parse_result_field_t * field;

    u32Ret = xcalloc((void **)&retval, sizeof(packet_header_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        /* All the headers are delineated with a CRLF, so we parse on that */
        u32Ret = parseString(&pPacket, pstrBuf, sOffset, sBuf, "\r\n", 2);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        field = pPacket->pr_pprfFirst;
        headerline = field->prf_pprfNext;

        u32Ret = _parseHttpStartLine(retval, field);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _parseHttpHeaderLine(retval, headerline);
    }

    if (pPacket != NULL)
        destroyParseResult(&pPacket);

    if (u32Ret == OLERR_NO_ERROR)
        *ppHeader = retval;
    else if (retval != NULL)
        destroyPacketHeader(&retval);

    return u32Ret;
}

u32 getRawPacket(packet_header_t * pph, olchar_t ** ppstrBuf, olsize_t * psBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t total;
    olsize_t sBuffer = 0;
    olchar_t *pBuffer;
    packet_header_field_t *node;

    *ppstrBuf = NULL;
    *psBuf = 0;

    if (pph->ph_nStatusCode != -1)
    {
        /* HTTP/1.1 200 OK\r\n
           12 is the total number of literal characters. Just add Version and
           StatusData */
        sBuffer = 12 + pph->ph_sVersion + pph->ph_sStatusData;
    }
    else
    {

        /* GET / HTTP/1.1\r\n 
           This is calculating the length for a request packet. It will work as
           long as the version is not > 9.9
           It should also add the length of the Version, but it's not critical.*/
        sBuffer = pph->ph_sDirective + pph->ph_sDirectiveObj + 12;
    }

    node = pph->ph_pphfFirst;
    while (node != NULL)
    {
        /* A conservative estimate adding the lengths of the header name and
           value, plus 4 characters for the ':' and CRLF */
        sBuffer += node->phf_sName + node->phf_sData + 4;
        node = node->phf_pphfNext;
    }

    /* Another conservative estimate adding in the packet body length plus a
       padding of 3 for the empty line */
    sBuffer += (3 + pph->ph_sBody);

    /* Allocate the buffer */
    u32Ret = xmalloc((void **)ppstrBuf, sBuffer);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    pBuffer = *ppstrBuf;
    if (pph->ph_nStatusCode != -1)
    {
        /* Write the response */
        memcpy(pBuffer, "HTTP/", 5);
        memcpy(pBuffer + 5, pph->ph_pstrVersion, pph->ph_sVersion);
        total = 5 + pph->ph_sVersion;

        total += ol_sprintf(pBuffer + total, " %d ", pph->ph_nStatusCode);
        memcpy(pBuffer + total, pph->ph_pstrStatusData, pph->ph_sStatusData);
        total += pph->ph_sStatusData;

        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        /* HTTP/1.1 200 OK\r\n */
    }
    else
    {
        /* Write the Request */
        memcpy(pBuffer, pph->ph_pstrDirective, pph->ph_sDirective);
        total = pph->ph_sDirective;
        memcpy(pBuffer + total, " ", 1);
        total += 1;
        memcpy(pBuffer + total, pph->ph_pstrDirectiveObj,
               pph->ph_sDirectiveObj);
        total += pph->ph_sDirectiveObj;
        memcpy(pBuffer + total, " HTTP/", 6);
        total += 6;
        memcpy(pBuffer + total, pph->ph_pstrVersion, pph->ph_sVersion);
        total += pph->ph_sVersion;
        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        /* GET / HTTP/1.1\r\n */
    }

    node = pph->ph_pphfFirst;
    while (node != NULL)
    {
        /* Write each header */
        memcpy(pBuffer + total, node->phf_pstrName, node->phf_sName);
        total += node->phf_sName;
        memcpy(pBuffer + total, ": ", 2);
        total += 2;
        memcpy(pBuffer + total, node->phf_pstrData, node->phf_sData);
        total += node->phf_sData;
        memcpy(pBuffer + total, "\r\n", 2);
        total += 2;
        sBuffer += node->phf_sName + node->phf_sData + 4;
        node = node->phf_pphfNext;
    }

    /* Write the empty line */
    memcpy(pBuffer + total, "\r\n", 2);
    total += 2;

    /* Write the body */
    memcpy(pBuffer + total, pph->ph_pu8Body, pph->ph_sBody);
    total += pph->ph_sBody;
    pBuffer[total] = '\0';

    *psBuf = total;

    return u32Ret;
}

u32 parseUri(
    olchar_t * pstrUri, olchar_t ** ppstrIp, u16 * pu16Port,
    olchar_t ** ppstrPath)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t *result = NULL, *result2 = NULL, *result3 = NULL;
    olchar_t * str1;
    olsize_t len1, len2;

    *ppstrIp = NULL;
    *ppstrPath = NULL;

    /* A scheme has the format xxx://yyy , so if we parse on ://, we can extract
       the path info */
    u32Ret = parseString(
        &result, pstrUri, 0, (olint_t) ol_strlen(pstrUri), "://", 3);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (result->pr_u32NumOfResult < 2)
            u32Ret = OLERR_INVALID_HTTP_URI;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        str1 = result->pr_pprfLast->prf_pstrData;
        len1 = result->pr_pprfLast->prf_sData;

        /* Parse path. The first '/' will occur after the IPAddress:port
           combination*/
        u32Ret = parseString(&result2, str1, 0, len1, "/", 1);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        len2 = len1 - result2->pr_pprfFirst->prf_sData;

        u32Ret = dupStringWithLen(
            ppstrPath, str1 + result2->pr_pprfFirst->prf_sData, len2);
        if (u32Ret == OLERR_NO_ERROR)
        {
            /* Parse port Number */
            u32Ret = parseString(
                &result3, result2->pr_pprfFirst->prf_pstrData,
                0, result2->pr_pprfFirst->prf_sData, ":", 1);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (result3->pr_u32NumOfResult == 1)
        {
            /* The default port is 80, if non is specified, because we are
               assuming an HTTP scheme*/
            *pu16Port = 80;
        }
        else
        {
            /* If a port was specified, use that */
            u32Ret = getU16FromString(
                result3->pr_pprfLast->prf_pstrData,
                result3->pr_pprfLast->prf_sData, pu16Port);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Parse IP Address */
        u32Ret = dupStringWithLen(
            ppstrIp, result3->pr_pprfFirst->prf_pstrData,
            result3->pr_pprfFirst->prf_sData);
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        if (*ppstrIp != NULL)
            xfree((void **)ppstrIp);

        if (*ppstrPath != NULL)
            xfree((void **)ppstrPath);
    }

    if (result3 != NULL)
        destroyParseResult(&result3);

    if (result2 != NULL)
        destroyParseResult(&result2);

    if (result != NULL)
        destroyParseResult(&result);

    return u32Ret;
}

u32 createEmptyPacketHeader(packet_header_t ** ppHeader)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_t * retval = NULL;

    u32Ret = xcalloc((void **)&retval, sizeof(packet_header_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        retval->ph_nStatusCode = -1;
        retval->ph_pstrVersion = "1.0";
        retval->ph_sVersion = 3;

        *ppHeader = retval;
    }
    
    return u32Ret;
}

u32 clonePacketHeader(packet_header_t ** dest, packet_header_t * pph)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_t *retval;
    packet_header_field_t *n;

    u32Ret = createEmptyPacketHeader(&retval);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;
    
    /*These three calls will result in the fields being copied*/
    setDirective(
        retval, pph->ph_pstrDirective, pph->ph_sDirective,
        pph->ph_pstrDirectiveObj, pph->ph_sDirectiveObj);

    setStatusCode(
        retval, pph->ph_nStatusCode, pph->ph_pstrStatusData,
        pph->ph_sStatusData);

    setVersion(retval, pph->ph_pstrVersion, pph->ph_sVersion);
    
    /*Iterate through each header, and copy them*/
    n = pph->ph_pphfFirst;
    while (n != NULL)
    {
        addHeaderLine(
            retval, n->phf_pstrName, n->phf_sName,
             n->phf_pstrData, n->phf_sData, TRUE);
        n = n->phf_pphfNext;
    }

    if (pph->ph_pu8Body != NULL)
        u32Ret = setBody(retval, pph->ph_pu8Body, pph->ph_sBody, TRUE);

    if (u32Ret == OLERR_NO_ERROR)
        *dest = retval;
    else if (retval != NULL)
        destroyPacketHeader(&retval);

    return u32Ret;
}

u32 setVersion(packet_header_t * pph, olchar_t * pstrVersion, olsize_t sVersion)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pstrVersion != NULL)
        u32Ret = dupStringWithLen(&(pph->ph_pstrVersion), pstrVersion, sVersion);

    if (u32Ret == OLERR_NO_ERROR)
    {
        pph->ph_sVersion = sVersion;
        pph->ph_bAllocVersion = TRUE;
    }

    return u32Ret;
}

u32 setStatusCode(
    packet_header_t * pph, olint_t nStatusCode,
    olchar_t * pstrStatusData, olsize_t sStatusData)
{
    u32 u32Ret = OLERR_NO_ERROR;

    pph->ph_nStatusCode = nStatusCode;

    if (pstrStatusData != NULL)
        u32Ret = dupStringWithLen(&(pph->ph_pstrStatusData),
            pstrStatusData, sStatusData);

    if (u32Ret == OLERR_NO_ERROR)
    {
        pph->ph_sStatusData = sStatusData;
        pph->ph_bAllocStatus = TRUE;
    }

    return u32Ret;
}

u32 setDirective(
    packet_header_t * pph, olchar_t * pstrDirective,
    olsize_t sDirective, olchar_t * pstrDirectiveObj, olsize_t sDirectiveObj)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pstrDirective != NULL)
        u32Ret = dupStringWithLen(&(pph->ph_pstrDirective), pstrDirective, sDirective);

    if (u32Ret == OLERR_NO_ERROR)
    {
        pph->ph_sDirective = sDirective;

        if (pstrDirectiveObj != NULL)
            u32Ret = dupStringWithLen(&(pph->ph_pstrDirectiveObj),
                pstrDirectiveObj, sDirectiveObj);
        if (u32Ret == OLERR_NO_ERROR)
        {
            pph->ph_sDirectiveObj = sDirectiveObj;
            pph->ph_bAllocDirective = TRUE;
        }
        else
        {
            freeString(&(pph->ph_pstrDirective));
        }
    }

    return u32Ret;
}

u32 setBody(packet_header_t * pph, u8 * pu8Body, olsize_t sBody, boolean_t bAlloc)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (bAlloc)
    {
        u32Ret = dupMemory((void **)&pph->ph_pu8Body, pu8Body, sBody);
        if (u32Ret == OLERR_NO_ERROR)
        {
            pph->ph_bAllocBody = bAlloc;
            pph->ph_sBody = sBody;
        }
    }
    else
    {
        pph->ph_bAllocBody = bAlloc;
        pph->ph_pu8Body = pu8Body;
        pph->ph_sBody = sBody;
    }

    return u32Ret;
}

u32 addHeaderLine(
    packet_header_t * pph, olchar_t * pstrName,
    olsize_t sName, olchar_t * pstrData, olsize_t sData, boolean_t bAlloc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    packet_header_field_t * node = NULL;
    
    /* Create the header node */
    u32Ret = xcalloc((void **)&node, sizeof(packet_header_field_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        node->phf_bAlloc = bAlloc;
        if (! bAlloc)
        {
            node->phf_pstrName = pstrName;
            node->phf_sName = sName;
            node->phf_pstrData = pstrData;
            node->phf_sData = sData;
        }
        else
        {
            u32Ret = dupStringWithLen(&(node->phf_pstrName), pstrName, sName);
            if (u32Ret == OLERR_NO_ERROR)
            {
                node->phf_sName = sName;

                u32Ret = dupStringWithLen(&(node->phf_pstrData), pstrData, sData);
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                node->phf_sData = sData;
            }
            else
            {
                if (node->phf_pstrName != NULL)
                    freeString(&(node->phf_pstrName));

                if (node->phf_pstrData != NULL)
                    freeString(&(node->phf_pstrData));

                xfree((void **)&node);
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* attach it to the linked list */
        if(pph->ph_pphfLast != NULL)
        {
            pph->ph_pphfLast->phf_pphfNext = node;
            pph->ph_pphfLast = node;
        }
        else
        {
            pph->ph_pphfLast = node;
            pph->ph_pphfFirst = node;
        }
    }

    return u32Ret;
}

u32 getHeaderLine(
    packet_header_t * pph, olchar_t * pstrName, olsize_t sName,
    packet_header_field_t ** ppHeader)
{
    packet_header_field_t * node = pph->ph_pphfFirst;

    *ppHeader = NULL;
    
    /*Iterate through the headers, until we find the one we're interested in*/
    while (node != NULL)
    {
        if (strncasecmp(pstrName, node->phf_pstrName, sName) == 0)
        {
            *ppHeader = node;

            return OLERR_NO_ERROR;
        }
        node = node->phf_pphfNext;
    }
    
    return OLERR_NOT_FOUND;
}

/*---------------------------------------------------------------------------*/

