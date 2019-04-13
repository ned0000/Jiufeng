/**
 *  @file jf_httpparser.h
 *
 *  @brief http parser header file, provide some functional routine to parse
 *   http packet
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_httpparser library
 *
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

/* --- data structures -------------------------------------------------------------------------- */
typedef struct jf_httpparser_packet_header_field
{
    olchar_t * jhphf_pstrName;
    olsize_t jhphf_sName;
    olchar_t * jhphf_pstrData;
    olsize_t jhphf_sData;

    boolean_t jhphf_bAlloc;
    u8 jhphf_u8Reserved[7];

    struct jf_httpparser_packet_header_field * jhphf_pjhphfNext;
} jf_httpparser_packet_header_field_t;

typedef struct jf_httpparser_packet_header
{
    olchar_t * jhph_pstrDirective;
    olsize_t jhph_sDirective;
    olchar_t * jhph_pstrDirectiveObj;
    olsize_t jhph_sDirectiveObj;
    olint_t jhph_nStatusCode;
    olchar_t * jhph_pstrStatusData;
    olsize_t jhph_sStatusData;
    olchar_t * jhph_pstrVersion;
    olsize_t jhph_sVersion;
    u8 * jhph_pu8Body;
    olsize_t jhph_sBody;

    boolean_t jhph_bAllocDirective;
    boolean_t jhph_bAllocStatus;
    boolean_t jhph_bAllocVersion;
    boolean_t jhph_bAllocBody;
    u8 jhph_bReserved[4];

    jf_httpparser_packet_header_field_t * jhph_pjhphfFirst;
    jf_httpparser_packet_header_field_t * jhph_pjhphfLast;
} jf_httpparser_packet_header_t;


/* --- functional routines ---------------------------------------------------------------------- */

/** Frees resources associated with a packet that was created either by
 *  createEmptyPacketHeader or parsePacketheader
 *
 *  @param ppHeader [out] the packet header to free
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_destroyPacketHeader(
    jf_httpparser_packet_header_t ** ppHeader);

/** Parses the HTTP headers from a buffer, into a packetheader structure
 *
 *  @param ppHeader [out] the parsed packet header structure
 *  @param pstrBuf [in] the buffer to parse
 *  @param sOffset [in] the offset of the buffer to start parsing
 *  @param sBuf [in] the length of the buffer to parse
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parsePacketHeader(
    jf_httpparser_packet_header_t ** ppHeader, olchar_t * pstrBuf,
    olsize_t sOffset, olsize_t sBuf);

/** Clones a packet header. Because parsePacketHeader does not copy any data,
 *  the data will become invalid once the data is flushed. This method is used
 *  to preserve the data.
 *
 *  @param ppDest [out] a cloned packet structure
 *  @param pHeader [in] the packet to clone from
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_clonePacketHeader(
    jf_httpparser_packet_header_t ** ppDest,
    jf_httpparser_packet_header_t * pHeader);

/** Sets the version of a packet header structure. The Default version is 1.0
 *
 *  @param pHeader [in/out] the packet to modify
 *  @param pstrVersion [in] the version string to write. eg: 1.1
 *  @param sVersion [in] the length of the version string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setVersion(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrVersion,
    olsize_t sVersion);

/** Sets the status code of a packetheader structure. There is no default
 *
 *  @param pHeader [in] the packet to modify
 *  @param nStatusCode [in] the status code, eg: 200
 *  @param pstrStatus [in] the status string, eg: OK
 *  @param sStatus [in] the length of the status string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setStatusCode(
    jf_httpparser_packet_header_t * pHeader, olint_t nStatusCode,
    olchar_t * pstrStatus, olsize_t sStatus);

/** Sets the directive of a packet header structure. There is no default.
 *
 *  @param pHeader [in/out] the packet to modify
 *  @param pstrDirective [in] the directive to write, eg: GET
 *  @param sDirective [in] the length of the directive
 *  @param pstrDirectiveObj [in] the path component of the directive,
 *   eg: /index.html
 *  @param sDirectiveObj [in] the length of the path component
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setDirective(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrDirective,
    olsize_t sDirective, olchar_t * pstrDirectiveObj, olsize_t sDirectiveObj);

HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_setBody(
    jf_httpparser_packet_header_t * pHeader, u8 * pu8Body, olsize_t sBody,
    boolean_t bAlloc);

/** Add a header entry into a packet header structure
 *
 *  @param pHeader [in/out] the packet to modify
 *  @param pstrName [in] the header name, eg: CONTENT-TYPE
 *  @param sName [in] the length of the header name
 *  @param pstrData [in] the header value, eg: text/xml
 *  @param sData [in] the length of the header value
 *  @param bAlloc [in] if true, alloc memory for name and data string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_addHeaderLine(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrName, olsize_t sName,
    olchar_t * pstrData, olsize_t sData, boolean_t bAlloc);

/** Converts a packet header structure into a raw buffer
 *
 *  @note The returned buffer must be freed by user
 *
 *  @param pHeader [in] the packet header struture to convert
 *  @param ppstrBuf [out] The output buffer
 *  @param psBuf [out] the length of the output buffer
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_getRawPacket(
    jf_httpparser_packet_header_t * pHeader, olchar_t ** ppstrBuf,
    olsize_t * psBuf);

/** Creates an empty packetheader structure
 *  
 *  @param ppHeader [out] an empty packet
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_createEmptyPacketHeader(
    jf_httpparser_packet_header_t ** ppHeader);

/** Retrieves the header value for a specified header name.
 *
 *  @note the result must be freed
 *
 *  @param pHeader [in] The packet to introspect
 *  @param pstrName [in] the header name to lookup
 *  @param sName [in] the length of the header name
 *  @param ppField [out] the header field. NULL if not found
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_getHeaderLine(
    jf_httpparser_packet_header_t * pHeader, olchar_t * pstrName,
    olsize_t sName, jf_httpparser_packet_header_field_t ** ppField);

/** Parses a URI string into its IP address, port number and path components
 *
 *  @note the IP, and path components must be freed
 *
 *  @param pstrUri [in] The URI to parse 
 *  @param ppstrIp [out] The IP address component in dotted quad format 
 *  @param pu16Port [out] The port component. Default is 80 
 *  @param ppstrPath [out] The path component 
 *
 *  @return void
 */
HTTPPARSERAPI u32 HTTPPARSERCALL jf_httpparser_parseUri(
    olchar_t * pstrUri, olchar_t ** ppstrIp, u16 * pu16Port,
    olchar_t ** ppstrPath);

#endif /*JIUFENG_HTTPPARSER_H*/

/*------------------------------------------------------------------------------------------------*/


