/**
 *  @file webclientrequest.h
 *
 *  @brief Header file for webclient request.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef WEBCLIENT_REQUEST_H
#define WEBCLIENT_REQUEST_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_network.h"
#include "jf_datavec.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the webclient request operation code.
 */
enum webclient_request_opcode
{
    /**The operation is to send data.*/
    WEBCLIENT_REQUEST_OPCODE_SEND_DATA = 0,
    /**The operation is to delete request.*/
    WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST,
};

/** Define the interna webclient request data type.
 */
typedef struct internal_webclient_request
{
    /**Operation code.*/
    u8 iwr_u8OpCode;
    u8 iwr_u8Reserved[7];

    /**Data vector.*/
    jf_datavec_t iwr_jdDataVec;

    /**Address of remote server to send data.*/
    jf_ipaddr_t iwr_jiRemote;
    /**Port of remote server.*/
    u16 iwr_u16RemotePort;
    u16 iwr_u16Reserve[3];

    /**User object.*/
    void * iwr_pUser;
    /**The event handler.*/
    jf_webclient_fnOnEvent_t iwr_fnOnEvent;
} internal_webclient_request_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a webclient request to send data.
 *
 *  @param ppRequest [out] The webclient request to be created.
 *  @param ppu8Data [in] The data array.
 *  @param psData [in] The size array.
 *  @param u16Num [in] Number of entry in the 2 arrays.
 *  @param pjiRemote [in] Address of remote server.
 *  @param u16Port [in] Port of remote server.
 *  @param fnOnEvent [in] Event handler.
 *  @param user [in] The user. It's the argument of the handler function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createWebclientRequestSendData(
    internal_webclient_request_t ** ppRequest, u8 ** ppu8Data, olsize_t * psData, u16 u16Num,
    jf_ipaddr_t * pjiRemote, u16 u16Port, jf_webclient_fnOnEvent_t fnOnEvent, void * user);

/** Create a webclient request to delete previous request.
 *
 *  @param ppRequest [out] The webclient request to be created.
 *  @param pjiRemote [in] Address of remote server.
 *  @param u16Port [in] Port of remote server.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createWebclientRequestDeleteRequest(
    internal_webclient_request_t ** ppRequest, jf_ipaddr_t * pjiRemote, u16 u16Port);

/** Destroy the webclient request.
 *
 *  @param ppRequest [in/out] The webclient request to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroyWebclientRequest(internal_webclient_request_t ** ppRequest);

#endif /*WEBCLIENT_REQUEST_H*/

/*------------------------------------------------------------------------------------------------*/
