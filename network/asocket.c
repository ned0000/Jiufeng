/**
 *  @file asocket.c
 *
 *  @brief The async socket implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "network.h"
#include "syncmutex.h"
#include "jf_mem.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
typedef struct send_data
{
    u8 * sd_pu8Buffer;
    olsize_t sd_sBuf;
    olsize_t sd_sBytesSent;

    jf_network_mem_owner_t sd_jnmoOwner;
    struct send_data *sd_psdNext;
} send_data_t;

typedef struct
{
    jf_network_chain_object_header_t ia_jncohHeader;
    jf_network_chain_t * ia_pjncChain;

    olsize_t ia_sPendingBytesToSend;
    olsize_t ia_sTotalBytesSent;

    jf_ipaddr_t ia_jiRemote;
    u16 ia_u16Port;
    u16 ia_u16Reserved[2];

    u16 ia_u16LocalPort;
    jf_ipaddr_t ia_jiLocal;

    void * ia_pUser;

    jf_network_utimer_t * ia_pjnuUtimer;

    jf_network_socket_t * ia_pjnsSocket;

    /*if the asocket is free or not*/
    boolean_t ia_bFree;
    /*if the asocket is paused or not*/
    boolean_t ia_bPause;
    /*connection is established*/
    boolean_t ia_bFinConnect;
    /*if the asocket should read data or not*/
    boolean_t ia_bNoRead;
    u8 ia_u8Reserved2[4];

    send_data_t * ia_psdHead;
    send_data_t * ia_psdTail;

    jf_mutex_t * ia_pjmLock;

    olsize_t ia_sBeginPointer;
    olsize_t ia_sEndPointer;

    u8 * ia_pu8Buffer;
    olsize_t ia_sMalloc;
    u32 ia_u32Reserved2;

    u32 ia_u32Status;
    u32 ia_u32Reserved3;

    void * ia_pTag;

    /*if true, do not send notify to upper layer*/
    boolean_t ia_bSilent;
    u8 ia_u8Reserved10[7];

    jf_network_fnAsocketOnData_t ia_fnOnData;
    jf_network_fnAsocketOnConnect_t ia_fnOnConnect;
    jf_network_fnAsocketOnDisconnect_t ia_fnOnDisconnect;
    jf_network_fnAsocketOnSendOK_t ia_fnOnSendOK;
} internal_asocket_t;

/* --- private routine section---------------------------------------------- */

static u32 _asRecvn(jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_network_recv(pSocket, pBuffer, psRecv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (*psRecv == 0)
            u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
    }

    return u32Ret;
}

static void _destroySendData(send_data_t ** ppsd)
{
    send_data_t * psd = *ppsd;

    if (psd->sd_jnmoOwner == JF_NETWORK_MEM_OWNER_LIB)
    {
        jf_mem_free((void **)&psd->sd_pu8Buffer);
    }
    jf_mem_free((void **)ppsd);
}

static u32 _clearAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("clear as");

    pia->ia_sPendingBytesToSend = 0;
    pia->ia_sTotalBytesSent = 0;
    pia->ia_bPause = FALSE;
    pia->ia_bFinConnect = FALSE;
    pia->ia_bSilent = FALSE;

    pia->ia_pUser = NULL;
    /*Initialise the buffer pointers, since no data is in them yet*/
    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    pia->ia_u32Status = 0;

    memset(&(pia->ia_jiRemote), 0, sizeof(jf_ipaddr_t));
    pia->ia_u16Port = 0;

    memset(&(pia->ia_jiLocal), 0, sizeof(jf_ipaddr_t));

    return u32Ret;
}

/** Clears all the pending data to be sent for an AsyncSocket
 *
 *  @param pia [in] the asocket to clear
 */
static void _clearPendingSendOfAsocket(internal_asocket_t * pia)
{
    send_data_t *data, *temp;

    data = pia->ia_psdHead;
    pia->ia_psdTail = NULL;
    while (data != NULL)
    {
        temp = data->sd_psdNext;
        _destroySendData(&data);
        data = temp;
    }
    pia->ia_psdHead = NULL;
}

static u32 _freeAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("free as");

    /*Since the socket is closing, we need to clear the data that is pending to
      be sent*/
    _clearPendingSendOfAsocket(pia);

    if (pia->ia_pjnsSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsSocket));

    jf_network_removeUtimerItem(pia->ia_pjnuUtimer, pia);

    _clearAsocket(pia);

    pia->ia_bFree = TRUE;

    return u32Ret;
}

/** Disconnects an asocket
 *
 *  @param pAsocket [in] the asocket to disconnect
 */
static u32 _asDisconnect(jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    u32 u32Status = pia->ia_u32Status;
    void * pUser = pia->ia_pUser;

    jf_logger_logInfoMsg("as disconn");

    if ((! pia->ia_bSilent) && (pia->ia_fnOnDisconnect != NULL))
        /*Trigger the OnDissconnect event if necessary*/
        pia->ia_fnOnDisconnect(pia, u32Status, pUser);

    _freeAsocket(pia);

    return u32Ret;
}

static inline void _onDataNotify(internal_asocket_t * pia)
{
    /*Tell the user we have some data*/
    if (! pia->ia_bSilent)
        pia->ia_fnOnData(
            pia, pia->ia_pu8Buffer, &pia->ia_sBeginPointer,
            pia->ia_sEndPointer, pia->ia_pUser, &pia->ia_bPause);
}

/** Internal method called when data is ready to be processed on an asocket
 *
 *  @param pia [in] the asocket with pending data
 */
static u32 _processAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesReceived;

    if (pia->ia_bNoRead)
    {
        _onDataNotify(pia);
        return u32Ret;
    }

    jf_logger_logInfoMsg("as process");

    /* If the thing isn't paused, and the user set the pointers such that we
       still have data in our buffers, we need to call the user back with
       that data, before we attempt to read more data off the network */
    if ((pia->ia_sBeginPointer != pia->ia_sEndPointer) &&
        (pia->ia_sBeginPointer != 0))
    {
        memmove(
            pia->ia_pu8Buffer, pia->ia_pu8Buffer + pia->ia_sBeginPointer,
            pia->ia_sEndPointer - pia->ia_sBeginPointer);

        pia->ia_sEndPointer -= pia->ia_sBeginPointer;
        pia->ia_sBeginPointer = 0;

        _onDataNotify(pia);
    }

    /* clear pointer */
    if (pia->ia_sBeginPointer == pia->ia_sEndPointer)
    {
        pia->ia_sBeginPointer = 0;
        pia->ia_sEndPointer = 0;
    }

    bytesReceived = pia->ia_sMalloc - pia->ia_sEndPointer;
    if (bytesReceived == 0)
    {
        pia->ia_bPause = TRUE;
        u32Ret = JF_ERR_BUFFER_IS_FULL;
        jf_logger_logErrMsg(u32Ret, "buffer is full, pause the asocket");
        return u32Ret;
    }

    u32Ret = _asRecvn(
        pia->ia_pjnsSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer,
        &bytesReceived);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(u32Ret, "as process");
        /*This means the socket was gracefully closed by peer*/
        pia->ia_u32Status = u32Ret;
        _asDisconnect(pia);
    }
    else
    {
        /*Data was read, so increment our counters*/
        pia->ia_sEndPointer += bytesReceived;

        jf_logger_logInfoMsg("as process, end %d", pia->ia_sEndPointer);

        _onDataNotify(pia);

        /*If the user consumed all of the buffer, we can recycle it*/
        if (pia->ia_sBeginPointer == pia->ia_sEndPointer)
        {
            pia->ia_sBeginPointer = 0;
            pia->ia_sEndPointer = 0;
        }
    }

    return u32Ret;
}

/** Pre select handler for asocket
 *
 *  @param pAsocket [in] the async socket 
 *  @param readset [out] the read fd set
 *  @param writeset [out] the write fd set
 *  @param errorset [out] the error fd set
 *  @param pu32BlockTime [out] the block time in millisecond
 *
 *  @return the error code
 */
static u32 _preSelectAsocket(
    jf_network_chain_object_t * pAsocket, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if (pia->ia_pjmLock != NULL)
        jf_mutex_acquire(pia->ia_pjmLock);
    if (pia->ia_pjnsSocket != NULL)
    {
        if (! pia->ia_bFinConnect)
        {
            /* Not Connected Yet */
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);
        }
        else
        {
            jf_logger_logInfoMsg("pre sel asocket, pause %u", pia->ia_bPause);
            if (! pia->ia_bPause)
            {
                /* Already Connected, just needs reading */
                jf_network_setSocketToFdSet(pia->ia_pjnsSocket, readset);
                jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);
            }

            if (pia->ia_psdHead != NULL)
            {
                /* If there is pending data to be sent, then we need to check
                   when the socket is writable */
                jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
            }
        }
    }
    if (pia->ia_pjmLock != NULL)
        jf_mutex_release(pia->ia_pjmLock);

    return u32Ret;
}

static u32 _asPostSelectSendData(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent = 0;
    send_data_t * temp;

    /*Keep trying to send data, until we are told we can't*/
    while (u32Ret == JF_ERR_NO_ERROR)
    {
        bytesSent = pia->ia_psdHead->sd_sBuf - pia->ia_psdHead->sd_sBytesSent;
        u32Ret = jf_network_send(
            pia->ia_pjnsSocket, pia->ia_psdHead->sd_pu8Buffer +
            pia->ia_psdHead->sd_sBytesSent, &bytesSent);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pia->ia_sPendingBytesToSend -= bytesSent;
            pia->ia_sTotalBytesSent += bytesSent;
            pia->ia_psdHead->sd_sBytesSent += bytesSent;
            if (pia->ia_psdHead->sd_sBytesSent == pia->ia_psdHead->sd_sBuf)
            {
                /*Finished Sending this block*/
                if (pia->ia_psdHead == pia->ia_psdTail)
                    pia->ia_psdTail = NULL;

                temp = pia->ia_psdHead->sd_psdNext;
                _destroySendData(&pia->ia_psdHead);

                pia->ia_psdHead = temp;
                if (pia->ia_psdHead == NULL)
                    break;
            }
            else
            {
                /*We sent data, but not everything that needs to get
                  sent was sent, quit and try later*/
                break;
            }
        }
        else
        {
            /*There was an error sending*/
            jf_network_clearPendingSendOfAsocket(pia);
            u32Ret = JF_ERR_FAIL_SEND_DATA;
        }
    }

    return u32Ret;
}

/** Post select handler for chain
 *
 *  @param pAsocket [in] the async socket
 *  @param slct [in] number of ready socket 
 *  @param readset [in] the read fd set
 *  @param writeset [in] the write fd set
 *  @param errorset [in] the error fd set
 *
 *  @return the error code
 */
static u32 _postSelectAsocket(
    jf_network_chain_object_t * pAsocket, olint_t slct,
    fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bTriggerSendOK = FALSE;
    u8 u8Addr[100];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nLen;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if (pia->ia_pjmLock != NULL)
        jf_mutex_acquire(pia->ia_pjmLock);

    /*Write Handling*/
    if ((pia->ia_pjnsSocket != NULL) && pia->ia_bFinConnect &&
        jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent*/
        u32Ret = _asPostSelectSendData(pia);

        /*This triggers OnSendOK, if all the pending data has been sent.*/
        if ((u32Ret == JF_ERR_NO_ERROR) && (pia->ia_psdHead == NULL))
            bTriggerSendOK = TRUE;

        if (bTriggerSendOK && (pia->ia_fnOnSendOK != NULL) &&
            (! pia->ia_bSilent))
        {
            pia->ia_fnOnSendOK(pia, pia->ia_pUser);
        }
    }

    /*Connection Handling / Read Handling*/
    if (pia->ia_pjnsSocket != NULL)
    {
        /*close the connection if socket is in the errorset,
          maybe peer is closed*/
        if (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, errorset) != 0)
        {
            jf_logger_logInfoMsg("post as, in errorset");

            /*Connection Failed*/
            if (pia->ia_fnOnConnect != NULL && ! pia->ia_bSilent)
                pia->ia_fnOnConnect(pia, FALSE, pia->ia_pUser);

            _freeAsocket(pia);
        }
        else if ((! pia->ia_bFinConnect) &&
                 (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0))
        {
            /* Connected */
            jf_logger_logInfoMsg("post as, connected");

            jf_network_getSocketName(pia->ia_pjnsSocket, psa, &nLen);

            jf_ipaddr_convertSockAddrToIpAddr(
                psa, nLen, &(pia->ia_jiLocal), &(pia->ia_u16LocalPort));

            pia->ia_bFinConnect = TRUE;

            /*Connection Complete*/
            if (pia->ia_fnOnConnect != NULL && ! pia->ia_bSilent)
                pia->ia_fnOnConnect(pia, TRUE, pia->ia_pUser);
        }
        else if (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, readset) != 0)
        {
            /* Data Available */
            u32Ret = _processAsocket(pia);
        }
    }

    if (pia->ia_pjmLock != NULL)
        jf_mutex_release(pia->ia_pjmLock);

    return u32Ret;
}

static u32 _asConnectTo(
    internal_asocket_t * pia, jf_ipaddr_t * pjiRemote, u16 u16Port, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("as connect");

    /*If there isn't a socket already allocated, we need to allocate one*/
    if (pia->ia_pjnsSocket == NULL)
    {
        if (pjiRemote->ji_u8AddrType == JF_IPADDR_TYPE_V4)
            u32Ret = jf_network_createSocket(
                AF_INET, SOCK_STREAM, 0, &pia->ia_pjnsSocket);
        else
            u32Ret = jf_network_createSocket(
                AF_INET6, SOCK_STREAM, 0, &pia->ia_pjnsSocket);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_setSocketNonblock(pia->ia_pjnsSocket);

        /*Connect the socket, and force the chain to unblock, since the select
          statement doesn't have us in the fdset yet.*/
        u32Ret = jf_network_connect(pia->ia_pjnsSocket, pjiRemote, u16Port);
    }

    return u32Ret;
}

static u32 _asAddPendingData(
    internal_asocket_t * pia, send_data_t * pData, u8 * pu8Buffer,
    olsize_t sBuf, jf_network_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    send_data_t * psd = NULL;

    u32Ret = jf_mem_duplicate((void **)&psd, (u8 *)pData, sizeof(*pData));
    if ((u32Ret == JF_ERR_NO_ERROR) && (memowner == JF_NETWORK_MEM_OWNER_USER))
    {
        /*If we don't own this memory, we need to copy the buffer, because the
          user may free this memory before we have a chance to send it*/
        u32Ret = jf_mem_duplicate((void **)&psd->sd_pu8Buffer, pu8Buffer, sBuf);
        if (u32Ret == JF_ERR_NO_ERROR)
            psd->sd_jnmoOwner = JF_NETWORK_MEM_OWNER_LIB;
        else
            _destroySendData(&psd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*There are still bytes that are pending to be sent, so we need to queue
          this up */
        pia->ia_psdTail->sd_psdNext = psd;
        pia->ia_psdTail = psd;
        *pbUnblock = TRUE;
    }

    return u32Ret;
}

static u32 _asTrySendData(
    internal_asocket_t * pia, send_data_t * pData, u8 * pu8Buffer,
    olsize_t sBuf, jf_network_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent;
    send_data_t * psd = NULL;

    bytesSent = pData->sd_sBuf - pData->sd_sBytesSent;
    u32Ret = jf_network_send(
        pia->ia_pjnsSocket,
        pData->sd_pu8Buffer + pData->sd_sBytesSent, &bytesSent);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*We were able to send something, so lets increment the counters*/
        pData->sd_sBytesSent += bytesSent;

        pia->ia_sPendingBytesToSend -= bytesSent;
        pia->ia_sTotalBytesSent += bytesSent;

        if (pData->sd_sBytesSent != pData->sd_sBuf)
        {
            /*All of the data wasn't sent, so we need to copy the buffer if we
              don't own the memory, because the user may free the memory, before
              we have a chance to complete sending it.*/
            jf_logger_logInfoMsg("as send data, partially send");
            u32Ret = jf_mem_duplicate((void **)&psd, (u8 *)pData, sizeof(*pData));
            if ((u32Ret == JF_ERR_NO_ERROR) && (memowner == JF_NETWORK_MEM_OWNER_USER))
            {
                u32Ret = jf_mem_duplicate(
                    (void **)&psd->sd_pu8Buffer, pu8Buffer, sBuf);
                if (u32Ret == JF_ERR_NO_ERROR)
                    psd->sd_jnmoOwner = JF_NETWORK_MEM_OWNER_LIB;
                else
                    _destroySendData(&psd);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*There is data pending to be sent, so lets go ahead and try to
                  send it*/
                pia->ia_psdTail = psd;
                pia->ia_psdHead = psd;

                *pbUnblock = TRUE;
            }
        }
    }
    else
    {
        jf_logger_logInfoMsg(
            "as send data, failed, add to utimer for disconnect");
        /*Send returned an error, so lets figure out what it was, as it could be
          normal. Most likely the socket closed while we tried to send*/
        pia->ia_u32Status = u32Ret;
        /*calling disconnect function, asSendData() may be called in other
          thread, use utimer just in case*/
        u32Ret = jf_network_addUtimerItem(
            pia->ia_pjnuUtimer, pia, 0, _asDisconnect, NULL);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_network_destroyAsocket(jf_network_asocket_t ** ppAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia;

    assert((ppAsocket != NULL) && (*ppAsocket != NULL));

    pia = (internal_asocket_t *) *ppAsocket;
    jf_logger_logInfoMsg("destroy as");

    /*Close socket if necessary*/
    if (pia->ia_pjnsSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsSocket));

    /*Free the buffer if necessary*/
    if ((! pia->ia_bNoRead) && (pia->ia_pu8Buffer != NULL))
    {
        jf_mem_free((void **)&pia->ia_pu8Buffer);
        pia->ia_sMalloc = 0;
    }

    /*Clear all the data that is pending to be sent*/
    jf_network_clearPendingSendOfAsocket((jf_network_asocket_t *)pia);

    if (pia->ia_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&pia->ia_pjnuUtimer);

    jf_mem_free(ppAsocket);

    return u32Ret;
}

u32 jf_network_createAsocket(
    jf_network_chain_t * pChain, jf_network_asocket_t ** ppAsocket,
    jf_network_asocket_create_param_t * pjnacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = NULL;

    assert((pChain != NULL) && (pjnacp != NULL) && (ppAsocket != NULL));
    assert((pjnacp->jnacp_fnOnData != NULL));

    jf_logger_logInfoMsg("create as");

    u32Ret = jf_mem_calloc((void **)&pia, sizeof(internal_asocket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAsocket;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAsocket;
        pia->ia_pjncChain = pChain;
        pia->ia_bFree = TRUE;
        pia->ia_bNoRead = pjnacp->jnacp_bNoRead;
        pia->ia_pjnsSocket = NULL;
        pia->ia_sMalloc = pjnacp->jnacp_sInitialBuf;
        pia->ia_pjmLock = pjnacp->jnacp_pjmLock;

        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;
        pia->ia_fnOnConnect = pjnacp->jnacp_fnOnConnect;
        pia->ia_fnOnDisconnect = pjnacp->jnacp_fnOnDisconnect;
        pia->ia_fnOnSendOK = pjnacp->jnacp_fnOnSendOK;

        if (! pia->ia_bNoRead)
            u32Ret = jf_mem_alloc(
                (void **)&pia->ia_pu8Buffer, pjnacp->jnacp_sInitialBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(pChain, &(pia->ia_pjnuUtimer));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pChain, pia);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAsocket = pia;
    else if (pia != NULL)
        jf_network_destroyAsocket((jf_network_asocket_t **)&pia);

    return u32Ret;
}

u32 jf_network_disconnectAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    jf_logger_logInfoMsg("as disconnect");

    if (pia->ia_bFree)
    {
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;
        jf_logger_logErrMsg(u32Ret, "as disconnect, asocket is free");
        return u32Ret;
    }

    pia->ia_bSilent = TRUE;

    u32Ret = _freeAsocket(pia);

    return u32Ret;
}

void jf_network_clearPendingSendOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;

    _clearPendingSendOfAsocket(pia);
}

u32 jf_network_sendAsocketData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    jf_network_mem_owner_t memowner)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    send_data_t data;
    boolean_t bUnblock = FALSE;

    jf_logger_logInfoMsg(
        "as send data, %02x %02x %02x %02x, %d, %d",
        pu8Buffer[0], pu8Buffer[1], pu8Buffer[2], pu8Buffer[3], sBuf, memowner);

    ol_bzero(&data, sizeof(send_data_t));

    data.sd_pu8Buffer = pu8Buffer;
    data.sd_sBuf = sBuf;
    data.sd_jnmoOwner = memowner;

    if (pia->ia_pjnsSocket == NULL)
        /*the socket is closed*/
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pia->ia_sPendingBytesToSend += sBuf;

        if (pia->ia_psdTail != NULL)
        {
            jf_logger_logInfoMsg("as send data, pending data");

            u32Ret = _asAddPendingData(
                pia, &data, pu8Buffer, sBuf, memowner, &bUnblock);
        }
        else
        {
            u32Ret = _asTrySendData(
                pia, &data, pu8Buffer, sBuf, memowner, &bUnblock);
        }
    }

    if (bUnblock)
    {
        jf_network_wakeupChain(pia->ia_pjncChain);
    }

    return u32Ret;
}

u32 jf_network_sendnAsocket(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    jf_logger_logInfoMsg("as sendn");

    if ((! pia->ia_bFree) && (pia->ia_pjnsSocket != NULL))
        u32Ret = jf_network_sendn(pia->ia_pjnsSocket, pu8Buffer, psBuf);
    else
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 jf_network_connectAsocketTo(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiRemote, u16 u16Port,
    void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    olchar_t strServer[50];

    assert((pAsocket != NULL) && (pjiRemote != NULL));
    assert((pjiRemote->ji_u8AddrType == JF_IPADDR_TYPE_V4) ||
           (pjiRemote->ji_u8AddrType == JF_IPADDR_TYPE_V6));

    jf_ipaddr_getStringIpAddr(strServer, pjiRemote);
    jf_logger_logInfoMsg("as connect to %s:%u", strServer, u16Port);

    if (! pia->ia_bFree)
    {
        u32Ret = JF_ERR_ASOCKET_IN_USE;
    }
    else
    {
        pia->ia_bFree = FALSE;

        pia->ia_pUser = pUser;

        ol_bzero(&pia->ia_jiRemote, sizeof(jf_ipaddr_t));
        pia->ia_u16Port = u16Port;

        u32Ret = _asConnectTo(pia, pjiRemote, u16Port, pUser);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_network_wakeupChain(pia->ia_pjncChain);
        }
        else
        {
            jf_logger_logErrMsg(u32Ret, "as connect failed");

            _freeAsocket(pia);
        }
    }

    return u32Ret;
}

boolean_t jf_network_isAsocketFree(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_bFree;
}

olsize_t jf_network_getPendingBytesToSendOfAsocket(
    jf_network_asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t toSend;

    toSend = pia->ia_sPendingBytesToSend;

    return toSend;
}

olsize_t jf_network_getTotalBytesSentOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t total;

    total = pia->ia_sTotalBytesSent;

    return total;
}

void jf_network_resetTotalBytesSentOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    pia->ia_sTotalBytesSent = 0;
}

void jf_network_getBufferOfAsocket(
    jf_network_asocket_t * pAsocket, u8 ** ppBuffer,
    olsize_t * psBeginPointer, olsize_t * psEndPointer)
{
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    if (! pia->ia_bNoRead)
    {
        *ppBuffer = pia->ia_pu8Buffer;
        *psBeginPointer = pia->ia_sBeginPointer;
        *psEndPointer = pia->ia_sEndPointer;
    }
    else
    {
        *ppBuffer = NULL;
        *psBeginPointer = 0;
        *psEndPointer = 0;
    }
}

void jf_network_clearBufferOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;
}

u32 jf_network_useSocketForAsocket(
    jf_network_asocket_t * pAsocket, jf_network_socket_t * pSocket, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    jf_logger_logInfoMsg("use socket for as");

    if (! pia->ia_bFree)
    {
        u32Ret = JF_ERR_ASOCKET_IN_USE;
    }
    else
    {
        pia->ia_bFree = FALSE;

        pia->ia_pjnsSocket = pSocket;

        pia->ia_bFinConnect = TRUE;

        pia->ia_pUser = pUser;

        /*make sure the socket is non-blocking*/
        jf_network_setSocketNonblock(pia->ia_pjnsSocket);
    }

    return u32Ret;
}

void jf_network_setRemoteAddressOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    ol_memcpy(&(pia->ia_jiRemote), pjiAddr, sizeof(jf_ipaddr_t));
}

void jf_network_getRemoteInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    ol_memcpy(pjiAddr, &(pia->ia_jiRemote), sizeof(jf_ipaddr_t));
}

void jf_network_getLocalInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    ol_memcpy(pjiAddr, &(pia->ia_jiLocal), sizeof(jf_ipaddr_t));
}

u32 jf_network_resumeAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    jf_logger_logInfoMsg("resume as, pause %u", pia->ia_bPause);
    if (pia->ia_bPause)
    {
        pia->ia_bPause = FALSE;

        jf_logger_logInfoMsg("resume as");

        u32Ret = jf_network_wakeupChain(pia->ia_pjncChain);
    }

    return u32Ret;
}

u32 jf_network_recvAsocketData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if ((! pia->ia_bFree) && (pia->ia_pjnsSocket != NULL))
        u32Ret = _asRecvn(pia->ia_pjnsSocket, pu8Buffer, psRecv);
    else
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 jf_network_getAsocketOpt(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    assert(pAsocket != NULL);

    if ((! pia->ia_bFree) && (pia->ia_pjnsSocket != NULL))
        u32Ret = jf_network_getSocketOpt(
            pia->ia_pjnsSocket, level, optname, optval, optlen);
    else
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 jf_network_setAsocketOpt(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    assert(pAsocket != NULL);

    if ((! pia->ia_bFree) && (pia->ia_pjnsSocket != NULL))
        u32Ret = jf_network_setSocketOpt(
            pia->ia_pjnsSocket, level, optname, optval, optlen);
    else
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

void * jf_network_getTagOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_pTag;
}

void jf_network_setTagOfAsocket(jf_network_asocket_t * pAsocket, void * pTag)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    pia->ia_pTag = pTag;
}

/*---------------------------------------------------------------------------*/


