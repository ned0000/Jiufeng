/**
 *  @file webclient.h
 *
 *  @brief web client header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_WEBCLIENT_H
#define JIUFENG_WEBCLIENT_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "httpparser.h"

#undef WEBCLIENTAPI
#undef WEBCLIENTCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_WEBCLIENT_DLL)
        #define WEBCLIENTAPI  __declspec(dllexport)
        #define WEBCLIENTCALL
    #else
        #define WEBCLIENTAPI
        #define WEBCLIENTCALL __cdecl
    #endif
#else
    #define WEBCLIENTAPI
    #define WEBCLIENTCALL
#endif

/* --- constant definitions ------------------------------------------------ */
typedef void  webclient_t;

/**The possible value for nInterruptFlag in function fnWebclientOnResponse_t
 */
enum webclient_interrupt_flag
{
    WIF_UNKNOWN = 0,
    WIF_WEB_DATAOBJECT_DESTROYED,
    WIF_WEB_REQUEST_DELETED,
};

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olint_t wp_nPoolSize;
    olsize_t wp_sBuffer;
} webclient_param_t;

typedef u32 (* fnWebclientOnResponse_t)(
    asocket_t * pAsocket, olint_t nInterruptFlag,
    packet_header_t * header, void * user, boolean_t * pbPause);

/* --- functional routines ------------------------------------------------- */

/** Create a new webclient
 *
 *  @param pbc [in] the chain to add this module to
 *  @param ppWebclient [out] the created web client
 *  @param pwd [in] the parameters for creating web client

 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL createWebclient(
    basic_chain_t * pbc, webclient_t ** ppWebclient, webclient_param_t * pwp);

/** Queues a new web request
 *
 *  This method differs from pipelineWebRequest, in that this method
 *  allows you to directly specify the buffers, rather than a packet structure
 *
 *  @param pWebclient [in] the webclient to queue the requests to
 *  @param piaRemote [in] the address of remote server
 *  @param u16Port [in] the port of remote server
 *  @param pph [in] the header of the message
 *  @param fnOnResponse [in] data reception handler
 *  @param user [in] the user
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL pipelineWebRequest(
    webclient_t * pWebclient, ip_addr_t * piaRemote, u16 u16Port,
    packet_header_t * pph, fnWebclientOnResponse_t fnOnResponse, void * user);

/** Queues a new web request
 *
 *  This method differs from pipelineWebRequest, in that this method
 *  allows you to directly specify the buffers, rather than a packet structure
 *
 *  @param pWebclient [in] the webclient to queue the requests to
 *  @param piaRemote [in] the address of remote server
 *  @param u16Port [in] the port of remote server
 *  @param pstrHeader [in] the buffer containing the headers
 *  @param sHeader [in] the length of the headers
 *  @param bStaticHeader [in] flag indicating memory ownership of buffer, if
 *   true, the header is static
 *  @param pstrBody [in] the buffer containing the HTTP body
 *  @param sBody [in] the length of the buffer
 *  @param bStaticBody [in] flag indicating memory ownership of buffer, if true,
 *   the body is static
 *  @param fnOnResponse [in] data reception handler
 *  @param user [in] the user
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL pipelineWebRequestEx(
    webclient_t * pWebclient, ip_addr_t * piaRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, boolean_t bStaticHeader,
    olchar_t * pstrBody, olsize_t sBody, boolean_t bStaticBody,
    fnWebclientOnResponse_t fnOnResponse, void * user);

/** Deletes all pending requests to a specific IP/Port combination
 *
 *  @param pWebclient [in] the web client
 *  @param piaRemote [in] the address of remote server
 *  @param u16Port [in] the port of remote server
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL deleteWebRequests(
    webclient_t * pWebclient, ip_addr_t * piaRemote, u16 u16Port);

/** Destory webclient object
 *
 *  @param ppWebclient [in/out] the webclient to free
 */
WEBCLIENTAPI u32 WEBCLIENTCALL destroyWebclient(webclient_t ** ppWebclient);

#endif /*JIUFENG_WEBCLIENT_H*/

/*---------------------------------------------------------------------------*/


