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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_mutex.h"
#include "jf_network.h"
#include "jf_httpparser.h"
#include "jf_webclient.h"
#include "jf_jiukun.h"
#include "jf_string.h"
#include "jf_hex.h"
#include "jf_hashtree.h"
#include "jf_queue.h"
#include "jf_datavec.h"

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

    u32Ret = jf_jiukun_allocMemory((void **)&piwr, sizeof(*piwr));
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

    jf_logger_logInfoMsg("destroy webclient req");

    piwr = (internal_webclient_request_t *) *ppRequest;

    if (piwr->iwr_u8OpCode == WEBCLIENT_REQUEST_OPCODE_SEND_DATA)
        jf_datavec_freeData(&piwr->iwr_jdDataVec);
    
    jf_jiukun_freeMemory((void **)ppRequest);

    return u32Ret;
}

u32 createWebclientRequestSendData(
    internal_webclient_request_t ** ppRequest, u8 ** ppu8Data, olsize_t * psData, u16 u16Num,
    jf_ipaddr_t * pjiRemote, u16 u16Port, jf_webclient_fnOnEvent_t fnOnEvent, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_webclient_request_t * piwr = NULL;

    jf_logger_logInfoMsg("new webclient req");

    u32Ret = _createWebclientRequest(&piwr, WEBCLIENT_REQUEST_OPCODE_SEND_DATA);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_datavec_cloneData(&piwr->iwr_jdDataVec, ppu8Data, psData, u16Num);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piwr->iwr_fnOnEvent = (fnOnEvent == NULL) ? _internalDataobjectOnEvent : fnOnEvent;
        piwr->iwr_pUser = user;
        ol_memcpy(&piwr->iwr_jiRemote, pjiRemote, sizeof(*pjiRemote));
        piwr->iwr_u16RemotePort = u16Port;
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

    jf_logger_logInfoMsg("new webclient req");

    u32Ret = _createWebclientRequest(&piwr, WEBCLIENT_REQUEST_OPCODE_DELETE_REQUEST);
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

