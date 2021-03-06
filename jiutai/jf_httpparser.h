/**
 *  @file jf_httpparser.h
 *
 *  @brief Http parser header file, provide some functional routine to parse http packet.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_httpparser library.
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# Link with jf_string library for string parse.
 *
 *  @par HTTP packet with chuned data
 *  -# Chunk size is a hexadecimal string.
 *  -# If the chunk size is 0, it's end of chunk.
 *  @code
 *   chunk-size\r\n
 *   chunk-data\r\n
 *   chunk-size\r\n
 *   chunk-data\r\n
 *   0\r\n
 *   \r\n
 *  @endcode
 */

#ifndef JIUFENG_HTTPPARSER_H
#define JIUFENG_HTTPPARSER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#undef HTTPPARSERAPI
#undef HTTPPARSERCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_HTTPPARSER_DLL)
        #define HTTPPARSERAPI  __declspec(dllexport)
        #define HTTPPARSERCALL
    #else
        #define HTTPPARSERAPI
        #define HTTPPARSERCALL __cdecl
    #endif
#else
    #define HTTPPARSERAPI
    #define HTTPPARSERCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the http transfer encoding type.
 */
typedef enum jf_httpparser_transfer_encoding
{
    /**Unknown transfer encoding type.*/
    JF_HTTPPARSER_TRANSFER_ENCODING_UNKNOWN = 0,
    /**Chunked transfer encoding type.*/
    JF_HTTPPARSER_TRANSFER_ENCODING_CHUNKED,
} jf_httpparser_transfer_encoding_t;

/** Define the http data object data type.
 */
typedef void  jf_httpparser_dataobject_t;

/* --- data structures -------------------------------------------------------------------------- */

/** Define the http packet header field data type.
 */
typedef struct jf_httpparser_packet_header_field
{
    /**The name of the field.*/
    olchar_t * jhphf_pstrName;
    /**The length of the name string.*/
    olsize_t jhphf_sName;
    /**The data of the field.*/
    olchar_t * jhphf_pstrData;
    /**The length of the data string.*/
    olsize_t jhphf_sData;
    /**The string is allocated if it's TRUE.*/
    boolean_t jhphf_bAlloc;
    u8 jhphf_u8Reserved[7];
    /**The next field.*/
    struct jf_httpparser_packet_header_field * jhphf_pjhphfNext;
} jf_httpparser_packet_header_field_t;

/** Define the http packet header data type.
 */
typedef struct jf_httpparser_packet_header
{
    /**The http directive for http request. Eg. GET, PUT, POST.*/
    olchar_t * jhph_pstrDirective;
    /**The size of the http directive string.*/
    olsize_t jhph_sDirective;
    /**The http directive object for http requesst. Eg. /index.html.*/
    olchar_t * jhph_pstrDirectiveObj;
    /**The size of the http directive object string.*/
    olsize_t jhph_sDirectiveObj;
    /**The status code for http response. It's -1 for http request.*/
    olint_t jhph_nStatusCode;
    /**The status data for http response, Eg. OK.*/
    olchar_t * jhph_pstrStatusData;
    /**The size of the http status data string.*/
    olsize_t jhph_sStatusData;
    /**The version string for both http request and response. Eg. 1.0, 1.1.*/
    olchar_t * jhph_pstrVersion;
    /**The size of the version string.*/
    olsize_t jhph_sVersion;
    /**The http body.*/
    u8 * jhph_pu8Body;
    /**The size of the http body.*/
    olsize_t jhph_sBody;

    /**The directive string is allocated if it's TRUE.*/
    boolean_t jhph_bAllocDirective;
    /**The status string is allocated if it's TRUE.*/
    boolean_t jhph_bAllocStatus;
    /**The version string is allocated if it's TRUE.*/
    boolean_t jhph_bAllocVersion;
    /**The body buffer is allocated if it's TRUE.*/
    boolean_t jhph_bAllocBody;
    u8 jhph_bReserved[4];

    /**The first field of the http packet header.*/
    jf_httpparser_packet_header_field_t * jhph_pjhphfFirst;
    /**The last field of the http packet header.*/
    jf_httpparser_packet_header_field_t * jhph_pjhphfLast;
} jf_httpparser_packet_header_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Frees resources associated with a http parser.
 *
 *  @note
 *  -# The resources are created by jf_httpparser_createEmptyPacketHeader() or
 *   jf_httpparser_clonePacketHeader() or jf_httpparser_parsePacketheader().
 *
 *  @param ppHeader [out] The packet header to free.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_destroyPacketHeader(
    jf_httpparser_packet_header_t ** ppHeader);

/** Parses the HTTP headers from a buffer, into a packetheader structure.
 *
 *  @note
 *  -# This routine doesn't copy any data, the pointers in packet header are pointing to the
 *   original buffer. The buffer should be there when using the packet header.
 *
 *  @param ppHeader [out] The parsed packet header structure.
 *  @param pstrBuf [in] The buffer to parse.
 *  @param sOffset [in] The offset of the buffer to start parsing.
 *  @param sBuf [in] The length of the buffer to parse.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_HTTP_HEADER_START_LINE Invalid HTTP header start line.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parsePacketHeader(
    jf_httpparser_packet_header_t ** ppHeader, olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf);

/** Clones a packet header.
 *
 *  @note
 *  -# This routine copies the data to the new packet header. The original header can be released
 *   after cloning.
 *
 *  @param ppDest [out] A cloned packet structure.
 *  @param pHeader [in] The packet to clone from.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_clonePacketHeader(
    jf_httpparser_packet_header_t ** ppDest, jf_httpparser_packet_header_t * pHeader);

/** Sets the version of a packet header structure. The Default version is 1.0.
 *
 *  @note
 *  -# The version string is duplicated.
 *
 *  @param pHeader [in/out] The packet to modify.
 *  @param pstrVersion [in] The version string to write. eg: 1.1.
 *  @param sVersion [in] The length of the version string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setVersion(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrVersion, olsize_t sVersion);

/** Sets the status code of a packetheader structure. There is no default.
 *
 *  @note
 *  -# The status data string is duplicated.
 *
 *  @param pHeader [in] The packet to modify.
 *  @param nStatusCode [in] The status code, eg: 200.
 *  @param pstrStatusData [in] The status data string, eg: OK.
 *  @param sStatusData [in] The length of the status data string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setStatusCode(
    jf_httpparser_packet_header_t * pHeader, olint_t nStatusCode, olchar_t * pstrStatusData,
    olsize_t sStatusData);

/** Sets the directive of a packet header structure. There is no default.
 *
 *  @note
 *  -# The directive and directive object are duplicated.
 *
 *  @param pHeader [in/out] The packet to modify.
 *  @param pstrDirective [in] The directive to write, eg: GET.
 *  @param sDirective [in] The length of the directive.
 *  @param pstrDirectiveObj [in] The path component of the directive, eg: /index.html.
 *  @param sDirectiveObj [in] The length of the path component.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setDirective(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrDirective, olsize_t sDirective,
    olchar_t * pstrDirectiveObj, olsize_t sDirectiveObj);

/** Set the http body for a http packet.
 *
 *  @param pHeader [in/out] The packet to modify.
 *  @param pu8Body [in] The body to set.
 *  @param sBody [in] Size of the body.
 *  @param bAlloc [in] Clone the body if it's TRUE.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setBody(
    jf_httpparser_packet_header_t * pHeader, u8 * pu8Body, olsize_t sBody, boolean_t bAlloc);

/** Add a header entry into a packet header structure.
 *
 *  @param pHeader [in/out] The packet to modify.
 *  @param pstrName [in] The header name, eg: CONTENT-TYPE.
 *  @param sName [in] The length of the header name.
 *  @param pstrData [in] The header value, eg: text/xml.
 *  @param sData [in] The length of the header value.
 *  @param bAlloc [in] Allocate memory for name and data string if it's TRUE.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_addHeaderLine(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrName, olsize_t sName,
    olchar_t * pstrData, olsize_t sData, boolean_t bAlloc);

/** Converts a packet header structure into a raw buffer.
 *
 *  @note
 *  -# The returned buffer must be freed by jf_jiukun_freeMemory().
 *
 *  @param pHeader [in] The packet header struture to convert.
 *  @param ppstrBuf [out] The output buffer.
 *  @param psBuf [out] The length of the output buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_getRawPacket(
    jf_httpparser_packet_header_t * pHeader, olchar_t ** ppstrBuf, olsize_t * psBuf);

/** Creates an empty packetheader structure.
 *  
 *  @param ppHeader [out] An empty packet.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_createEmptyPacketHeader(
    jf_httpparser_packet_header_t ** ppHeader);

/** Retrieves the header value for a specified header name.
 *
 *  @note
 *  -# When comparing the header name, ignore the case.
 *
 *  @param pHeader [in] The http packet.
 *  @param pstrName [in] The header name to lookup.
 *  @param sName [in] The length of the header name.
 *  @param ppField [out] The header field. NULL if not found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_FOUND Header is not found.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_getHeaderLine(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrName, olsize_t sName,
    jf_httpparser_packet_header_field_t ** ppField);

/** Parses a URI string into its IP address, port number and path components.
 *
 *  @note
 *  -# The port is set to 80 if no port is specified.
 *  -# Memory is allocated for the IP and path string, it must be freed by caller with
 *   jf_string_free().
 *
 *  @param pstrUri [in] The URI to parse.
 *  @param ppstrIp [out] The IP address component in dotted quad format.
 *  @param pu16Port [out] The port component. Default is 80.
 *  @param ppstrPath [out] The path component.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_INTEGER Invalid port number.
 *  @retval JF_ERR_INVALID_HTTP_URI Invalid HTTP URI.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parseUri(
    olchar_t * pstrUri, olchar_t ** ppstrIp, u16 * pu16Port, olchar_t ** ppstrPath);

/** Get the http transfer encoding stated in header field.
 *
 *  @note
 *  -# The header field name is "transfer-encoding".
 *
 *  @param pHeader [in] The http packet header.
 *  @param pu8Encoding [out] The transfer encoding.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parseHeaderTransferEncoding(
    jf_httpparser_packet_header_t * pHeader, u8 * pu8Encoding);

/** Get the http content length stated in header field.
 *
 *  @note
 *  -# The header field name is "content-length".
 *
 *  @param pHeader [in] The http packet header.
 *  @param pnLength [out] The content length.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parseHeaderContentLength(
    jf_httpparser_packet_header_t * pHeader, olint_t * pnLength);

/** Find the http header.
 *
 *  @param pu8Buffer [in] The buffer.
 *  @param sOffset [in] The offset of the buffer to start finding.
 *  @param sEnd [in] The end of the buffer.
 *  @param psHeader [out] Size of the header, start from offset.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_HTTP_HEADER_NOT_FOUND HTTP header is not found.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_findHeader(
    u8 * pu8Buffer, olsize_t sOffset, olsize_t sEnd, olsize_t * psHeader);

/** Create the http data object.
 *
 *  @note
 *  -# Data object is used to process incoming data, not receiving data.
 *  -# Caller should provide the buffer size which is also used by caller to receive data.
 *  -# It's ok if the buffer size is smaller than the actual received data size. A buffer is
 *   allocated by data object if actual received size is larger than buffer size.
 *
 *  @param ppDataobject [out] The data object to be created.
 *  @param sBuffer [in] The buffer size of the data object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_createtDataobject(
    jf_httpparser_dataobject_t ** ppDataobject, olsize_t sBuffer);

/** Destroy the http data object.
 *
 *  @param ppDataobject [out] The data object to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_destroyDataobject(
    jf_httpparser_dataobject_t ** ppDataobject);

/** Process data for data object.
 *
 *  @note
 *  -# The routine should be called to process incoming data, it's not reponsilbe for
 *   receiving data.
 *  -# The routine may be called for several times if the data is received more than once.
 *  -# Chunked data is processed in this routine.
 *  -# Caller should check the begin pointer and end pointer after return. If they are equal,
 *   total buffer are consumed.
 *
 *  @param pDataobject [in] The data object.
 *  @param pu8Buffer [in] The receive buffer.
 *  @param psBeginPointer [in/out] The start pointer pointing to the data in buffer.
 *  @param sEndPointer [in] The end pointer of the data.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_HTTP_HEADER_NOT_FOUND HTTP header is not found.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_processDataobject(
    jf_httpparser_dataobject_t * pDataobject, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer);

/** Reinitialize the data object so it can handle data again.
 *
 *  @param pDataobject [in] The httparser data object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_reinitDataobject(
    jf_httpparser_dataobject_t * pDataobject);

/** Get the http packet of the data object.
 *
 *  @note
 *  -# The packet may be partial as data is still being received. Application should check the
 *   return value to know the complete status of the packet.
 *  -# The HTTP packet header is available only when full packet is received.
 *
 *  @param pDataobject [in] The data object.
 *  @param ppPacket [out] The http packet parsed from received data.
 *
 *  @return The status if full packet is received.
 *  @retval TRUE Full packet is received.
 *  @retval FALSE Partial packet is received.
 */
HTTPPARSERAPI boolean_t HTTPPARSERCALL jf_httpparser_getDataobjectFullPacket(
    jf_httpparser_dataobject_t * pDataobject, jf_httpparser_packet_header_t ** ppPacket);

#endif /*JIUFENG_HTTPPARSER_H*/

/*------------------------------------------------------------------------------------------------*/
