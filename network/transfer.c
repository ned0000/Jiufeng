/**
 *  @file network/transfer.c
 *
 *  @brief Implementation file for routines to transfer data.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_network.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _sendDataToServer(
    jf_network_socket_t * pSocket, jf_network_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sData = 0;

    JF_LOGGER_DEBUG("size: %d", transfer->jntdp_sSendBuf);

    /*Send the request message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sData = transfer->jntdp_sSendBuf;

        u32Ret = jf_network_sendnWithTimeout(
            pSocket, transfer->jntdp_pSendBuf, &sData, transfer->jntdp_u32Timeout);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sData != transfer->jntdp_sSendBuf)
            u32Ret = JF_ERR_FAIL_SEND_DATA;
    }

    return u32Ret;
}

static u32 _recvOtherDataFromServer(
    jf_network_socket_t * pSocket, jf_network_transfer_data_param_t * transfer, olsize_t sFull)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sData = 0;

    if (sFull > transfer->jntdp_sRecvBuf)
        u32Ret = JF_ERR_BUFFER_TOO_SMALL;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sData = sFull - transfer->jntdp_sHeader;

        u32Ret = jf_network_recvnWithTimeout(
            pSocket, transfer->jntdp_pRecvBuf + transfer->jntdp_sRecvData, &sData,
            transfer->jntdp_u32Timeout);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sData != sFull - transfer->jntdp_sHeader)
            u32Ret = JF_ERR_FAIL_RECV_DATA;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        transfer->jntdp_sRecvData += sData;
    }

    return u32Ret;
}

static u32 _recvDataFromServer(
    jf_network_socket_t * pSocket, jf_network_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sData = 0, sFull = 0;

    JF_LOGGER_DEBUG("header: %d", transfer->jntdp_sHeader);

    transfer->jntdp_sRecvData = 0;
    sData = transfer->jntdp_sHeader;

    /*Receive the header.*/
    u32Ret = jf_network_recvnWithTimeout(
        pSocket, transfer->jntdp_pRecvBuf, &sData, transfer->jntdp_u32Timeout);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (sData != transfer->jntdp_sHeader)
            u32Ret = JF_ERR_FAIL_RECV_DATA;
    }

    /*Receive the full data.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        transfer->jntdp_sRecvData = transfer->jntdp_sHeader;
        sFull = transfer->jntdp_fnGetFullDataSize(transfer->jntdp_pRecvBuf, transfer->jntdp_sHeader);
        JF_LOGGER_DEBUG("full data size: %d", sFull);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sFull > transfer->jntdp_sHeader))
        u32Ret = _recvOtherDataFromServer(pSocket, transfer, sFull);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_transferData(jf_network_transfer_data_param_t * transfer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * pSocket = NULL;

    assert(transfer->jntdp_u32Timeout > 0);
    assert((transfer->jntdp_pSendBuf != NULL) && (transfer->jntdp_sSendBuf > 0));
    if (transfer->jntdp_bReply)
        assert((transfer->jntdp_pRecvBuf != NULL) && (transfer->jntdp_sRecvBuf > 0) &&
               (transfer->jntdp_fnGetFullDataSize != NULL) && (transfer->jntdp_sHeader > 0));

    /*Create the socket.*/
    u32Ret = jf_network_createTypeStreamSocket(transfer->jntdp_pjiServer->ji_u8AddrType, &pSocket);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Connect to the remote server.*/
        u32Ret = jf_network_connectWithTimeout(
            pSocket, transfer->jntdp_pjiServer, transfer->jntdp_u16Port, transfer->jntdp_u32Timeout);
    }

    /*Send the data.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _sendDataToServer(pSocket, transfer);

    /*Receive the data.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && transfer->jntdp_bReply)
        u32Ret = _recvDataFromServer(pSocket, transfer);

    /*Destroy the socket.*/
    if (pSocket != NULL)
        jf_network_destroySocket(&pSocket);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
