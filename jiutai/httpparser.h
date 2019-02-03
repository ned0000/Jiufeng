/**
 *  @file httpparser.h
 *
 *  @brief http parser header file, provide some functional routine to parse
 *   http packet
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_HTTPPARSER_H
#define JIUFENG_HTTPPARSER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

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

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct packet_header_field
{
    olchar_t * phf_pstrName;
    olsize_t phf_sName;
    olchar_t * phf_pstrData;
    olsize_t phf_sData;

    boolean_t phf_bAlloc;
    u8 phf_u8Reserved[7];

    struct packet_header_field * phf_pphfNext;
} packet_header_field_t;

typedef struct packet_header
{
    olchar_t * ph_pstrDirective;
    olsize_t ph_sDirective;
    olchar_t * ph_pstrDirectiveObj;
    olsize_t ph_sDirectiveObj;
    olint_t ph_nStatusCode;
    olchar_t * ph_pstrStatusData;
    olsize_t ph_sStatusData;
    olchar_t * ph_pstrVersion;
    olsize_t ph_sVersion;
    u8 * ph_pu8Body;
    olsize_t ph_sBody;

    boolean_t ph_bAllocDirective;
    boolean_t ph_bAllocStatus;
    boolean_t ph_bAllocVersion;
    boolean_t ph_bAllocBody;
    u8 ph_bReserved[4];

    packet_header_field_t * ph_pphfFirst;
    packet_header_field_t * ph_pphfLast;
} packet_header_t;


/* --- functional routines ------------------------------------------------- */

/** Frees resources associated with a packet that was created either by
 *  createEmptyPacketHeader or parsePacketheader
 *
 *  @param ppHeader [out] the packet header to free
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL destroyPacketHeader(
    packet_header_t ** ppHeader);

/** Parses the HTTP headers from a buffer, into a packetheader structure
 *
 *  @param ppHeader [out] the parsed packet header structure
 *  @param pstrBuf [in] the buffer to parse
 *  @param sOffset [in] the offset of the buffer to start parsing
 *  @param sBuf [in] the length of the buffer to parse
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL parsePacketHeader(
    packet_header_t ** ppHeader, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf);

/** Clones a packet header. Because parsePacketHeader does not copy any data,
 *  the data will become invalid once the data is flushed. This method is used
 *  to preserve the data.
 *
 *  @param dest [out] a cloned packet structure
 *  @param pph [in] the packet to clone from
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL clonePacketHeader(
    packet_header_t ** dest, packet_header_t * pph);

/** Sets the version of a packet header structure. The Default version is 1.0
 *
 *  @param pph [in/out] the packet to modify
 *  @param pstrVersion [in] the version string to write. eg: 1.1
 *  @param sVersion [in] the length of the version string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL setVersion(
    packet_header_t * pph, olchar_t * pstrVersion, olsize_t sVersion);

/** Sets the status code of a packetheader structure. There is no default
 *
 *  @param pph [in] the packet to modify
 *  @param nStatusCode [in] the status code, eg: 200
 *  @param pstrStatusData [in] the status string, eg: OK
 *  @param sStatusData [in] the length of the status string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL setStatusCode(
    packet_header_t * pph, olint_t nStatusCode, olchar_t * pstrStatus,
    olsize_t sStatus);

/** Sets the directive of a packet header structure. There is no default.
 *
 *  @param pph [in/out] the packet to modify
 *  @param pstrDirective [in] the directive to write, eg: GET
 *  @param sDirective [in] the length of the directive
 *  @param pstrDirectiveObj [in] the path component of the directive,
 *   eg: /index.html
 *  @param sDirectiveObj [in] the length of the path component
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL setDirective(
    packet_header_t * pph, olchar_t * pstrDirective, olsize_t sDirective,
    olchar_t * pstrDirectiveObj, olsize_t sDirectiveObj);

HTTPPARSERAPI u32 HTTPPARSERCALL setBody(
    packet_header_t * pph, u8 * pu8Body, olsize_t sBody, boolean_t bAlloc);

/** Add a header entry into a packet header structure
 *
 *  @param pph [in/out] the packet to modify
 *  @param pstrName [in] the header name, eg: CONTENT-TYPE
 *  @param sName [in] the length of the header name
 *  @param pstrData [in] the header value, eg: text/xml
 *  @param sData [in] the length of the header value
 *  @param bAlloc [in] if true, alloc memory for name and data string
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL addHeaderLine(
    packet_header_t * pph, olchar_t * pstrName, olsize_t sName,
    olchar_t * pstrData, olsize_t sData, boolean_t bAlloc);

/** Converts a packet header structure into a raw buffer
 *
 *  @note The returned buffer must be freed by user
 *
 *  @param pph [in] the packet header struture to convert
 *  @param ppstrBuf [out] The output buffer
 *  @return psBuf [out] the length of the output buffer
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL getRawPacket(
    packet_header_t * pph, olchar_t ** ppstrBuf, olsize_t * psBuf);

/** Creates an empty packetheader structure
 *  
 *  @param ppHeader [out] an empty packet
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL createEmptyPacketHeader(
    packet_header_t ** ppHeader);

/** Retrieves the header value for a specified header name.
 *
 *  @note the result must be freed
 *
 *  @param pph [in] The packet to introspect
 *  @param pstrName [in] the header name to lookup
 *  @param sName [in] the length of the header name
 *  @param ppHeader [out] the header value. NULL if not found
 *
 *  @return the error code
 */
HTTPPARSERAPI u32 HTTPPARSERCALL getHeaderLine(
    packet_header_t * pph,
    olchar_t * pstrName, olsize_t sName, packet_header_field_t ** ppHeader);

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
HTTPPARSERAPI u32 HTTPPARSERCALL parseUri(
    olchar_t * pstrUri, olchar_t ** ppstrIp, u16 * pu16Port,
    olchar_t ** ppstrPath);

#endif /*JIUFENG_HTTPPARSER_H*/

/*---------------------------------------------------------------------------*/


