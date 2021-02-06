/**
 *  @file webclient/transfer.c
 *
 *  @brief Implementation file for routines to transfer data with HTTP server.
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

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _sendDataToHttpServer(
    jf_network_socket_t * pSocket, jf_webclient_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sData = 0;

    JF_LOGGER_DEBUG("size: %d", transfer->jwtdp_sSendBuf);

    /*Send the request message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sData = transfer->jwtdp_sSendBuf;

        u32Ret = jf_network_sendnWithTimeout(
            pSocket, transfer->jwtdp_pSendBuf, &sData, transfer->jwtdp_u32Timeout);
    }

    /*Compare the data size to make sure the data is actually sent.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sData != transfer->jwtdp_sSendBuf)
            u32Ret = JF_ERR_FAIL_SEND_DATA;
    }

    return u32Ret;
}

static u32 _recvDataFromHttpServer(
    jf_network_socket_t * pSocket, jf_webclient_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sData = 0, sOffset = 0, sStart = 0;
    jf_httpparser_dataobject_t * dataobject = NULL;
    jf_httpparser_packet_header_t * header = NULL;
    u8 * pu8Buffer = NULL;

    JF_LOGGER_DEBUG("expected data size: %d", transfer->jwtdp_sRecvData);

    /*Allocate buffer for receiving data, the size is given by caller.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buffer, transfer->jwtdp_sRecvData);

    /*Create data object for HTTP packet.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_httpparser_createtDataobject(&dataobject, transfer->jwtdp_sRecvData);

    /*Loop until full packet is received or error.*/
    while ((u32Ret == JF_ERR_NO_ERROR) &&
           (! jf_httpparser_getDataobjectFullPacket(dataobject, &header)))
    {
        /*Set the data size based on the offset.*/
        sData = transfer->jwtdp_sRecvData - sOffset;

        /*Receive the data with full size.*/
        u32Ret = jf_network_recvWithTimeout(
            pSocket, pu8Buffer + sOffset, &sData, transfer->jwtdp_u32Timeout);

        /*Process the data.*/
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            sOffset += sData;

            u32Ret = jf_httpparser_processDataobject(dataobject, pu8Buffer, &sStart, sOffset);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Buffer are consumed, clear the pointer.*/
            if (sStart == sOffset)
                sStart = sOffset = 0;
        }
    }

    /*Clone header, as the header in use is from data object, they will be destroyed later.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_httpparser_clonePacketHeader(&transfer->jwtdp_pjhphHeader, header);

    if (dataobject != NULL)
        jf_httpparser_destroyDataobject(&dataobject);

    if (pu8Buffer != NULL)
        jf_jiukun_freeMemory((void **)&pu8Buffer);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_webclient_transferData(jf_webclient_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * pSocket = NULL;

    assert(transfer->jwtdp_u32Timeout > 0);
    assert((transfer->jwtdp_pSendBuf != NULL) && (transfer->jwtdp_sSendBuf > 0));
    assert(transfer->jwtdp_sRecvData > 0);

    /*Create the socket.*/
    u32Ret = jf_network_createTypeStreamSocket(transfer->jwtdp_pjiServer->ji_u8AddrType, &pSocket);

    /*Connect to the remote server.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_connectWithTimeout(
            pSocket, transfer->jwtdp_pjiServer, transfer->jwtdp_u16Port, transfer->jwtdp_u32Timeout);

    /*Send the data.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _sendDataToHttpServer(pSocket, transfer);

    /*Receive the data.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _recvDataFromHttpServer(pSocket, transfer);

    /*Destroy the socket.*/
    if (pSocket != NULL)
        jf_network_destroySocket(&pSocket);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
