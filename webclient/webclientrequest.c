/**
 *  @file webclientrequest.c
 *
 *  @brief Webclient request implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_webclient.h"
#include "jf_jiukun.h"

#include "common.h"
#include "webclientrequest.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _internalDataobjectOnEvent(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t event,
    jf_httpparser_packet_header_t * header, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static u32 _createWebclientRequest(internal_webclient_request_t ** ppRequest, u8 u8OpCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;

    /*Allocate memory for the internal webclient request.*/
    u32Ret = jf_jiukun_allocMemory((void **)&piwr, sizeof(*piwr));

    /*Initialize the internal webclient request.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piwr, sizeof(*piwr));
        piwr->iwr_u8OpCode = u8OpCode;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        destroyWebclientRequest(&piwr);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyWebclientRequest(internal_webclient_request_t ** ppRequest)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;

    piwr = (internal_webclient_request_t *) *ppRequest;

    JF_LOGGER_DEBUG("opcode: %u", piwr->iwr_u8OpCode);

    /*Free send data.*/
    if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_SEND_DATA)
        jf_datavec_freeData(&piwr->iwr_jdDataVec);
    
    /*Free memory for the internal webclient request.*/
    jf_jiukun_freeMemory((void **)ppRequest);

    return u32Ret;
}

u32 createWebclientRequestSendData(
    internal_webclient_request_t ** ppRequest, u8 ** ppu8Data, olsize_t * psData, u16 u16Num,
    jf_ipaddr_t * pjiRemote, u16 u16Port, jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;

    /*Create webclient request.*/
    u32Ret = _createWebclientRequest(&piwr, WEBCLIENT_REQUEST_OPCODE_SEND_DATA);

    /*Initialize the webclient request object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piwr->iwr_fnOnEvent = (fnOnEvent == NULL) ? _internalDataobjectOnEvent : fnOnEvent;
        piwr->iwr_pUser = user;
        ol_memcpy(&piwr->iwr_jiRemote, pjiRemote, sizeof(*pjiRemote));
        piwr->iwr_u16RemotePort = u16Port;
    }

    /*Clone data to data vector.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_datavec_cloneData(&piwr->iwr_jdDataVec, ppu8Data, psData, u16Num);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        destroyWebclientRequest(&piwr);

    return u32Ret;
}

u32 createWebclientRequestDeleteRequest(
    internal_webclient_request_t ** ppRequest, jf_ipaddr_t * pjiRemote, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;

    /*Create webclient request.*/
    u32Ret = _createWebclientRequest(&piwr, WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST);

    /*Initialize the webclient request object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memcpy(&piwr->iwr_jiRemote, pjiRemote, sizeof(*pjiRemote));
        piwr->iwr_u16RemotePort = u16Port;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppRequest = piwr;
    else if (piwr != NULL)
        destroyWebclientRequest(&piwr);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
