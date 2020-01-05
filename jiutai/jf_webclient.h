/**
 *  @file jf_webclient.h
 *
 *  @brief Header file defines the interface of web client library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_webclient library.
 */

#ifndef JIUFENG_WEBCLIENT_H
#define JIUFENG_WEBCLIENT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_httpparser.h"

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

/* --- constant definitions --------------------------------------------------------------------- */

typedef void  jf_webclient_t;

/** The possible value for nEvent in function jf_webclient_fnOnEvent_t
 */
typedef enum jf_webclient_event
{
    JF_WEBCLIENT_EVENT_UNKNOWN = 0,
    JF_WEBCLIENT_EVENT_INCOMING_DATA,
    JF_WEBCLIENT_EVENT_HTTP_REQ_DELETED,
} jf_webclient_event_t;

/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for creating webclient.
 */
typedef struct
{
    /**Number of connection in pool.*/
    u32 jwcp_u32PoolSize;
    /**Buffer size of the session.*/
    olsize_t jwcp_sBuffer;
} jf_webclient_create_param_t;

/** Callback function for webclient event.
 */
typedef u32 (* jf_webclient_fnOnEvent_t)(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t event,
    jf_httpparser_packet_header_t * header, void * pUser);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a new webclient.
 *
 *  @param pjnc [in] The chain to add this module to.
 *  @param ppWebclient [out] The created web client.
 *  @param pjwcp [in] The parameters for creating web client.
 *
 *  @return The error code.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient, jf_webclient_create_param_t * pjwcp);

/** Send a http message in packet format.
 *
 *  @param pWebclient [in] The webclient to queue the requests to.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *  @param pjhph [in] The header of the message.
 *  @param fnOnEvent [in] Data reception handler.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_sendHttpPacket(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_httpparser_packet_header_t * pjhph, jf_webclient_fnOnEvent_t fnOnEvent, void * pUser);

/** Send a http message with header and body
 *
 *  @note
 *  -# This method differs from jf_webclient_sendHttpPacket(), it directly specify the buffers,
 *   rather than a packet structure.
 *
 *  @param pWebclient [in] The webclient to queue the requests to.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *  @param pstrHeader [in] The buffer containing the headers.
 *  @param sHeader [in] The length of the headers.
 *  @param pstrBody [in] The buffer containing the HTTP body.
 *  @param sBody [in] The length of the buffer.
 *  @param fnOnEvent [in] Data reception handler.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_sendHttpHeaderAndBody(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port, olchar_t * pstrHeader,
    olsize_t sHeader, olchar_t * pstrBody, olsize_t sBody,
    jf_webclient_fnOnEvent_t fnOnEvent, void * pUser);

/** Deletes all pending requests to a specific IP/Port combination.
 *
 *  @param pWebclient [in] The web client.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *
 *  @return The error code.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_deleteRequest(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port);

/** Destory webclient object.
 *
 *  @param ppWebclient [in/out] The webclient to free.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_destroy(jf_webclient_t ** ppWebclient);

#endif /*JIUFENG_WEBCLIENT_H*/

/*------------------------------------------------------------------------------------------------*/


