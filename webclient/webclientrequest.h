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

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

enum webclient_request_opcode
{
    WEBCLIENT_REQUEST_OPCODE_SEND_DATA = 0,
    WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST,
};

typedef struct internal_webclient_request
{
    u8 iwr_u8OpCode;
    u8 iwr_u8Reserved[7];

    jf_datavec_t iwr_jdDataVec;

    jf_ipaddr_t iwr_jiRemote;
    u16 iwr_u16RemotePort;
    u16 iwr_u16Reserve[3];

    void * iwr_pUser;
    jf_webclient_fnOnEvent_t iwr_fnOnEvent;
} internal_webclient_request_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createWebclientRequestSendData(
    internal_webclient_request_t ** ppRequest, u8 ** ppu8Data, olsize_t * psData, u16 u16Num,
    jf_ipaddr_t * pjiRemote, u16 u16Port, jf_webclient_fnOnEvent_t fnOnEvent, void * user);

u32 createWebclientRequestDeleteRequest(
    internal_webclient_request_t ** ppRequest, jf_ipaddr_t * pjiRemote, u16 u16Port);

u32 destroyWebclientRequest(internal_webclient_request_t ** ppRequest);

#endif /*WEBCLIENT_REQUEST_H*/

/*------------------------------------------------------------------------------------------------*/


