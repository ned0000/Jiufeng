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
typedef void  jf_webclient_t;

/** The possible value for nEvent in function jf_webclient_fnOnResponse_t
 */
enum jf_webclient_event
{
    JF_WEBCLIENT_EVENT_DATA = 0,
    JF_WEBCLIENT_EVENT_DATAOBJECT_DESTROYED,
    JF_WEBCLIENT_EVENT_WEB_REQUEST_DELETED,
} jf_webclient_event_t;

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olint_t jwcp_nPoolSize;
    olsize_t jwcp_sBuffer;
} jf_webclient_create_param_t;

typedef u32 (* jf_webclient_fnOnResponse_t)(
    jf_network_asocket_t * pAsocket, olint_t nEvent,
    jf_httpparser_packet_header_t * header, void * pUser, boolean_t * pbPause);

/* --- functional routines ------------------------------------------------- */

/** Create a new webclient
 *
 *  @param pjnc [in] the chain to add this module to
 *  @param ppWebclient [out] the created web client
 *  @param pjwcp [in] the parameters for creating web client
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient,
    jf_webclient_create_param_t * pjwcp);

/** Queues a new web request
 *
 *  This method differs from pipelineWebRequest, in that this method
 *  allows you to directly specify the buffers, rather than a packet structure
 *
 *  @param pWebclient [in] the webclient to queue the requests to
 *  @param pjiRemote [in] the address of remote server
 *  @param u16Port [in] the port of remote server
 *  @param pjhph [in] the header of the message
 *  @param fnOnResponse [in] data reception handler
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_pipelineWebRequest(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_httpparser_packet_header_t * pjhph, jf_webclient_fnOnResponse_t fnOnResponse,
    void * pUser);

/** Queues a new web request
 *
 *  This method differs from pipelineWebRequest, in that this method
 *  allows you to directly specify the buffers, rather than a packet structure
 *
 *  @param pWebclient [in] the webclient to queue the requests to
 *  @param pjiRemote [in] the address of remote server
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
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_pipelineWebRequestEx(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    olchar_t * pstrHeader, olsize_t sHeader, boolean_t bStaticHeader,
    olchar_t * pstrBody, olsize_t sBody, boolean_t bStaticBody,
    jf_webclient_fnOnResponse_t fnOnResponse, void * pUser);

/** Deletes all pending requests to a specific IP/Port combination
 *
 *  @param pWebclient [in] the web client
 *  @param pjiRemote [in] the address of remote server
 *  @param u16Port [in] the port of remote server
 *
 *  @return the error code
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_deleteWebRequests(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port);

/** Destory webclient object
 *
 *  @param ppWebclient [in/out] the webclient to free
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_destroy(
    jf_webclient_t ** ppWebclient);

#endif /*JIUFENG_WEBCLIENT_H*/

/*---------------------------------------------------------------------------*/


