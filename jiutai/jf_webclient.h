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
#include "jf_network.h"
#include "jf_ipaddr.h"

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

/** Define the web client data type.
 */
typedef void  jf_webclient_t;

/** The possible value for nEvent in function jf_webclient_fnOnEvent_t.
 */
typedef enum jf_webclient_event
{
    /**Unknown event.*/
    JF_WEBCLIENT_EVENT_UNKNOWN = 0,
    /**Incoming data.*/
    JF_WEBCLIENT_EVENT_INCOMING_DATA,
    /**HTTP request is deleted.*/
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
 *
 *  @param pAsocket [in] The async socket represents the connection.
 *  @param event [in] The event.
 *  @param header [in] The HTTP header.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_webclient_fnOnEvent_t)(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t event,
    jf_httpparser_packet_header_t * header, void * pUser);

/** Define the parameter for transfering data with HTTP server.
 */
typedef struct
{
/*in*/
    /**Server address.*/
    jf_ipaddr_t * jwtdp_pjiServer;
    /**Server port.*/
    u16 jwtdp_u16Port;
    u16 jwtdp_u16Reserved;
    /**Timeout for connecting to server, sending and receiving data.*/
    u32 jwtdp_u32Timeout;
    /**Buffer for sending data.*/
    void * jwtdp_pSendBuf;
    /**Size of sending data buffer.*/
    olsize_t jwtdp_sSendBuf;
    /**Size of expected received data, used for memory allocation during transfer.*/
    olsize_t jwtdp_sRecvData;
    u8 jwtdp_u8Reserved[8];
/*out*/
    /**HTTP packet of received response, it should be freed by caller with
       jf_httpparser_destroyPacketHeader().*/
    jf_httpparser_packet_header_t * jwtdp_pjhphHeader;
} jf_webclient_transfer_data_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a webclient object.
 *
 *  @param pjnc [in] The chain to add this module to.
 *  @param ppWebclient [out] The created web client.
 *  @param pjwcp [in] The parameters for creating web client.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_create(
    jf_network_chain_t * pjnc, jf_webclient_t ** ppWebclient, jf_webclient_create_param_t * pjwcp);

/** Send a http message in packet format.
 *
 *  @param pWebclient [in] The webclient to queue the requests to.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *  @param pjhph [in] The header of the message.
 *  @param fnOnEvent [in] Event handler.
 *  @param pUser [in] The user. It's the argument of the handler function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @param fnOnEvent [in] Event handler.
 *  @param pUser [in] The user. It's the argument of the handler function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_sendHttpHeaderAndBody(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port, olchar_t * pstrHeader,
    olsize_t sHeader, olchar_t * pstrBody, olsize_t sBody, jf_webclient_fnOnEvent_t fnOnEvent,
    void * pUser);

/** Deletes all pending requests to a specific IP/Port combination.
 *
 *  @param pWebclient [in] The web client.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_deleteRequest(
    jf_webclient_t * pWebclient, jf_ipaddr_t * pjiRemote, u16 u16Port);

/** Destory the webclient object.
 *
 *  @param ppWebclient [in/out] The webclient to destroy.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_destroy(jf_webclient_t ** ppWebclient);

/** Transfer data to HTTP server.
 *
 *  @note
 *  -# Web client data object is not required for running this routine.
 *  -# Socket is created and connected to the server address, then data is sent and received.
 *   The socket is closed after the session. There is no keep-alive.
 *  -# Reply from HTTP server is necessary.
 *  -# The routine is synchronous, it will not return until full HTTP response is received or error.
 *  -# For the HTTP response, it's parsed to HTTP packet header and put in the parameter.
 *  -# The header in parameter should be freed with jf_httpparser_destroyPacketHeader() by caller.
 *
 *  @param transfer [in] The data structure containing the detailed information of the transfer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
WEBCLIENTAPI u32 WEBCLIENTCALL jf_webclient_transferData(
    jf_webclient_transfer_data_param_t * transfer);

#endif /*JIUFENG_WEBCLIENT_H*/

/*------------------------------------------------------------------------------------------------*/
