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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "network.h"
#include "syncmutex.h"
#include "xmalloc.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
typedef struct send_data
{
    u8 * sd_pu8Buffer;
    olsize_t sd_sBuf;
    olsize_t sd_sBytesSent;

    as_mem_owner_t sd_amoOwner;
    struct send_data *sd_psdNext;
} send_data_t;

typedef struct
{
    basic_chain_object_header_t ia_bcohHeader;
    basic_chain_t * ia_pbcChain;

    olsize_t ia_sPendingBytesToSend;
    olsize_t ia_sTotalBytesSent;

    ip_addr_t ia_iaRemote;
    u16 ia_u16Port;
    u16 ia_u16Reserved[2];

    u16 ia_u16LocalPort;
    ip_addr_t ia_iaLocal;

    void * ia_pUser;

    utimer_t * ia_putUtimer;

    socket_t * ia_psSocket;

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

    sync_mutex_t * ia_psmLock;

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

    fnAsocketOnData_t ia_fnOnData;
    fnAsocketOnConnect_t ia_fnOnConnect;
    fnAsocketOnDisconnect_t ia_fnOnDisconnect;
    fnAsocketOnSendOK_t ia_fnOnSendOK;
} internal_asocket_t;

/* --- private routine section---------------------------------------------- */

static u32 _asRecvn(socket_t * pSocket, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = sRecv(pSocket, pBuffer, psRecv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (*psRecv == 0)
            u32Ret = OLERR_SOCKET_PEER_CLOSED;
    }

    return u32Ret;
}

static void _destroySendData(send_data_t ** ppsd)
{
    send_data_t * psd = *ppsd;

    if (psd->sd_amoOwner == AS_MEM_OWNER_ASOCKET)
    {
        xfree((void **)&psd->sd_pu8Buffer);
    }
    xfree((void **)ppsd);
}

static u32 _clearAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("clear as");

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

    memset(&(pia->ia_iaRemote), 0, sizeof(ip_addr_t));
    pia->ia_u16Port = 0;

    memset(&(pia->ia_iaLocal), 0, sizeof(ip_addr_t));

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
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("free as");

    /*Since the socket is closing, we need to clear the data that is pending to
      be sent*/
    _clearPendingSendOfAsocket(pia);

    if (pia->ia_psSocket != NULL)
        destroySocket(&(pia->ia_psSocket));

    removeUtimerItem(pia->ia_putUtimer, pia);

    _clearAsocket(pia);

    pia->ia_bFree = TRUE;

    return u32Ret;
}

/** Disconnects an asocket
 *
 *  @param pAsocket [in] the asocket to disconnect
 */
static u32 _asDisconnect(asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    u32 u32Status = pia->ia_u32Status;
    void * pUser = pia->ia_pUser;

    logInfoMsg("as disconn");

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
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t bytesReceived;

    if (pia->ia_bNoRead)
    {
        _onDataNotify(pia);
        return u32Ret;
    }

    logInfoMsg("as process");

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
        u32Ret = OLERR_BUFFER_IS_FULL;
        logErrMsg(u32Ret, "buffer is full, pause the asocket");
        return u32Ret;
    }

    u32Ret = _asRecvn(
        pia->ia_psSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer,
        &bytesReceived);
    if (u32Ret != OLERR_NO_ERROR)
    {
        logErrMsg(u32Ret, "as process");
        /*This means the socket was gracefully closed by peer*/
        pia->ia_u32Status = u32Ret;
        _asDisconnect(pia);
    }
    else
    {
        /*Data was read, so increment our counters*/
        pia->ia_sEndPointer += bytesReceived;

        logInfoMsg("as process, end %d", pia->ia_sEndPointer);

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
 *  @param readset: 
 *  @param writeset: 
 *  @param errorset: 
 *  @param blocktime: 
 */
static u32 _preSelectAsocket(
    basic_chain_object_t * pAsocket, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if (pia->ia_psmLock != NULL)
        acquireSyncMutex(pia->ia_psmLock);
    if (pia->ia_psSocket != NULL)
    {
        if (! pia->ia_bFinConnect)
        {
            /* Not Connected Yet */
            setSocketToFdSet(pia->ia_psSocket, writeset);
            setSocketToFdSet(pia->ia_psSocket, errorset);
        }
        else
        {
            logInfoMsg("pre sel asocket, pause %u", pia->ia_bPause);
            if (! pia->ia_bPause)
            {
                /* Already Connected, just needs reading */
                setSocketToFdSet(pia->ia_psSocket, readset);
                setSocketToFdSet(pia->ia_psSocket, errorset);
            }

            if (pia->ia_psdHead != NULL)
            {
                /* If there is pending data to be sent, then we need to check
                   when the socket is writable */
                setSocketToFdSet(pia->ia_psSocket, writeset);
            }
        }
    }
    if (pia->ia_psmLock != NULL)
        releaseSyncMutex(pia->ia_psmLock);

    return u32Ret;
}

static u32 _asPostSelectSendData(internal_asocket_t * pia)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t bytesSent = 0;
    send_data_t * temp;

    /*Keep trying to send data, until we are told we can't*/
    while (u32Ret == OLERR_NO_ERROR)
    {
        bytesSent = pia->ia_psdHead->sd_sBuf - pia->ia_psdHead->sd_sBytesSent;
        u32Ret = sSend(
            pia->ia_psSocket, pia->ia_psdHead->sd_pu8Buffer +
            pia->ia_psdHead->sd_sBytesSent, &bytesSent);
        if (u32Ret == OLERR_NO_ERROR)
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
            clearPendingSendOfAsocket(pia);
            u32Ret = OLERR_FAIL_SEND_DATA;
        }
    }

    return u32Ret;
}

/** Post select handler for basic chain
 *
 *  @param pAsocket: 
 *  @param slct: 
 *  @param readset: 
 *  @param writeset: 
 *  @param errorset: 
 */
static u32 _postSelectAsocket(
    basic_chain_object_t * pAsocket, olint_t slct,
    fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bTriggerSendOK = FALSE;
    u8 u8Addr[100];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nLen;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if (pia->ia_psmLock != NULL)
        acquireSyncMutex(pia->ia_psmLock);

    /*Write Handling*/
    if ((pia->ia_psSocket != NULL) && pia->ia_bFinConnect &&
        isSocketSetInFdSet(pia->ia_psSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent*/
        u32Ret = _asPostSelectSendData(pia);

        /*This triggers OnSendOK, if all the pending data has been sent.*/
        if ((u32Ret == OLERR_NO_ERROR) && (pia->ia_psdHead == NULL))
            bTriggerSendOK = TRUE;

        if (bTriggerSendOK && (pia->ia_fnOnSendOK != NULL) &&
            (! pia->ia_bSilent))
        {
            pia->ia_fnOnSendOK(pia, pia->ia_pUser);
        }
    }

    /*Connection Handling / Read Handling*/
    if (pia->ia_psSocket != NULL)
    {
        /*close the connection if socket is in the errorset,
          maybe peer is closed*/
        if (isSocketSetInFdSet(pia->ia_psSocket, errorset) != 0)
        {
            logInfoMsg("post as, in errorset");

            /*Connection Failed*/
            if (pia->ia_fnOnConnect != NULL && ! pia->ia_bSilent)
                pia->ia_fnOnConnect(pia, FALSE, pia->ia_pUser);

            _freeAsocket(pia);
        }
        else if ((! pia->ia_bFinConnect) &&
                 (isSocketSetInFdSet(pia->ia_psSocket, writeset) != 0))
        {
            /* Connected */
            logInfoMsg("post as, connected");

            getSocketName(pia->ia_psSocket, psa, &nLen);

            convertSockAddrToIpAddr(
                psa, nLen, &(pia->ia_iaLocal), &(pia->ia_u16LocalPort));

            pia->ia_bFinConnect = TRUE;

            /*Connection Complete*/
            if (pia->ia_fnOnConnect != NULL && ! pia->ia_bSilent)
                pia->ia_fnOnConnect(pia, TRUE, pia->ia_pUser);
        }
        else if (isSocketSetInFdSet(pia->ia_psSocket, readset) != 0)
        {
            /* Data Available */
            u32Ret = _processAsocket(pia);
        }
    }

    if (pia->ia_psmLock != NULL)
        releaseSyncMutex(pia->ia_psmLock);

    return u32Ret;
}

static u32 _asConnectTo(
    internal_asocket_t * pia, ip_addr_t * piaRemote, u16 u16Port, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("as connect");

    /*If there isn't a socket already allocated, we need to allocate one*/
    if (pia->ia_psSocket == NULL)
    {
        if (piaRemote->ia_u8AddrType == IP_ADDR_TYPE_V4)
            u32Ret = createSocket(AF_INET, SOCK_STREAM, 0, &pia->ia_psSocket);
        else
            u32Ret = createSocket(AF_INET6, SOCK_STREAM, 0, &pia->ia_psSocket);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        setSocketNonblock(pia->ia_psSocket);

        /*Connect the socket, and force the chain to unblock, since the select
          statement doesn't have us in the fdset yet.*/
        u32Ret = sConnect(pia->ia_psSocket, piaRemote, u16Port);
    }

    return u32Ret;
}

static u32 _asAddPendingData(
    internal_asocket_t * pia, send_data_t * pData, u8 * pu8Buffer,
    olsize_t sBuf, as_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    send_data_t * psd = NULL;

    u32Ret = dupMemory((void **)&psd, (u8 *)pData, sizeof(*pData));
    if ((u32Ret == OLERR_NO_ERROR) && (memowner == AS_MEM_OWNER_USER))
    {
        /*If we don't own this memory, we need to copy the buffer, because the
          user may free this memory before we have a chance to send it*/
        u32Ret = dupMemory((void **)&psd->sd_pu8Buffer, pu8Buffer, sBuf);
        if (u32Ret == OLERR_NO_ERROR)
            psd->sd_amoOwner = AS_MEM_OWNER_ASOCKET;
        else
            _destroySendData(&psd);
    }

    if (u32Ret == OLERR_NO_ERROR)
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
    olsize_t sBuf, as_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t bytesSent;
    send_data_t * psd = NULL;

    bytesSent = pData->sd_sBuf - pData->sd_sBytesSent;
    u32Ret = sSend(
        pia->ia_psSocket,
        pData->sd_pu8Buffer + pData->sd_sBytesSent, &bytesSent);
    if (u32Ret == OLERR_NO_ERROR)
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
            logInfoMsg("as send data, partially send");
            u32Ret = dupMemory((void **)&psd, (u8 *)pData, sizeof(*pData));
            if ((u32Ret == OLERR_NO_ERROR) && (memowner == AS_MEM_OWNER_USER))
            {
                u32Ret = dupMemory(
                    (void **)&psd->sd_pu8Buffer, pu8Buffer, sBuf);
                if (u32Ret == OLERR_NO_ERROR)
                    psd->sd_amoOwner = AS_MEM_OWNER_ASOCKET;
                else
                    _destroySendData(&psd);
            }

            if (u32Ret == OLERR_NO_ERROR)
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
        logInfoMsg(
            "as send data, failed, add to utimer for disconnect");
        /*Send returned an error, so lets figure out what it was, as it could be
          normal. Most likely the socket closed while we tried to send*/
        pia->ia_u32Status = u32Ret;
        /*calling disconnect function, asSendData() may be called in other
          thread, use utimer just in case*/
        u32Ret = addUtimerItem(
            pia->ia_putUtimer, pia, 0, _asDisconnect, NULL);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 destroyAsocket(asocket_t ** ppAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia;

    assert((ppAsocket != NULL) && (*ppAsocket != NULL));

    pia = (internal_asocket_t *) *ppAsocket;
    logInfoMsg("destroy as");

    /*Close socket if necessary*/
    if (pia->ia_psSocket != NULL)
        destroySocket(&(pia->ia_psSocket));

    /*Free the buffer if necessary*/
    if ((! pia->ia_bNoRead) && (pia->ia_pu8Buffer != NULL))
    {
        xfree((void **)&pia->ia_pu8Buffer);
        pia->ia_sMalloc = 0;
    }

    /*Clear all the data that is pending to be sent*/
    clearPendingSendOfAsocket((asocket_t *)pia);

    if (pia->ia_putUtimer != NULL)
        destroyUtimer(&pia->ia_putUtimer);

    xfree(ppAsocket);

    return u32Ret;
}

u32 createAsocket(
    basic_chain_t * pChain, asocket_t ** ppAsocket, asocket_param_t * pap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = NULL;

    assert((pChain != NULL) && (pap != NULL) && (ppAsocket != NULL));
    assert((pap->ap_fnOnData != NULL));

    logInfoMsg("create as");

    u32Ret = xcalloc((void **)&pia, sizeof(internal_asocket_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_bcohHeader.bcoh_fnPreSelect = _preSelectAsocket;
        pia->ia_bcohHeader.bcoh_fnPostSelect = _postSelectAsocket;
        pia->ia_pbcChain = pChain;
        pia->ia_bFree = TRUE;
        pia->ia_bNoRead = pap->ap_bNoRead;
        pia->ia_psSocket = NULL;
        pia->ia_sMalloc = pap->ap_sInitialBuf;
        pia->ia_psmLock = pap->ap_psmLock;

        pia->ia_fnOnData = pap->ap_fnOnData;
        pia->ia_fnOnConnect = pap->ap_fnOnConnect;
        pia->ia_fnOnDisconnect = pap->ap_fnOnDisconnect;
        pia->ia_fnOnSendOK = pap->ap_fnOnSendOK;

        if (! pia->ia_bNoRead)
            u32Ret = xmalloc(
                (void **)&pia->ia_pu8Buffer, pap->ap_sInitialBuf);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createUtimer(pChain, &(pia->ia_putUtimer));

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = appendToBasicChain(pChain, pia);

    if (u32Ret == OLERR_NO_ERROR)
        *ppAsocket = pia;
    else if (pia != NULL)
        destroyAsocket((asocket_t **)&pia);

    return u32Ret;
}

u32 asDisconnect(asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    logInfoMsg("as disconnect");

    if (pia->ia_bFree)
    {
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;
        logErrMsg(u32Ret, "as disconnect, asocket is free");
        return u32Ret;
    }

    pia->ia_bSilent = TRUE;

    u32Ret = _freeAsocket(pia);

    return u32Ret;
}

void clearPendingSendOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;

    _clearPendingSendOfAsocket(pia);
}

u32 asSendData(
    asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    as_mem_owner_t memowner)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    send_data_t data;
    boolean_t bUnblock = FALSE;

    logInfoMsg(
        "as send data, %02x %02x %02x %02x, %d, %d",
        pu8Buffer[0], pu8Buffer[1], pu8Buffer[2], pu8Buffer[3], sBuf, memowner);

    ol_bzero(&data, sizeof(send_data_t));

    data.sd_pu8Buffer = pu8Buffer;
    data.sd_sBuf = sBuf;
    data.sd_amoOwner = memowner;

    if (pia->ia_psSocket == NULL)
        /*the socket is closed*/
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;

    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_sPendingBytesToSend += sBuf;

        if (pia->ia_psdTail != NULL)
        {
            logInfoMsg("as send data, pending data");

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
        wakeupBasicChain(pia->ia_pbcChain);
    }

    return u32Ret;
}

u32 asSendn(asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    logInfoMsg("as sendn");

    if ((! pia->ia_bFree) && (pia->ia_psSocket != NULL))
        u32Ret = sSendn(pia->ia_psSocket, pu8Buffer, psBuf);
    else
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 asConnectTo(
    asocket_t * pAsocket, ip_addr_t * piaRemote, u16 u16Port, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    olchar_t strServer[50];

    assert((pAsocket != NULL) && (piaRemote != NULL));
    assert((piaRemote->ia_u8AddrType == IP_ADDR_TYPE_V4) ||
           (piaRemote->ia_u8AddrType == IP_ADDR_TYPE_V6));

    getStringIpAddr(strServer, piaRemote);
    logInfoMsg("as connect to %s:%u", strServer, u16Port);

    if (! pia->ia_bFree)
    {
        u32Ret = OLERR_ASOCKET_IN_USE;
    }
    else
    {
        pia->ia_bFree = FALSE;

        pia->ia_pUser = pUser;

        ol_bzero(&pia->ia_iaRemote, sizeof(ip_addr_t));
        pia->ia_u16Port = u16Port;

        u32Ret = _asConnectTo(pia, piaRemote, u16Port, pUser);
        if (u32Ret == OLERR_NO_ERROR)
        {
            wakeupBasicChain(pia->ia_pbcChain);
        }
        else
        {
            logErrMsg(u32Ret, "as connect failed");

            _freeAsocket(pia);
        }
    }

    return u32Ret;
}

boolean_t isAsocketFree(asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_bFree;
}

olsize_t getPendingBytesToSendOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t toSend;

    toSend = pia->ia_sPendingBytesToSend;

    return toSend;
}

olsize_t getTotalBytesSentOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t total;

    total = pia->ia_sTotalBytesSent;

    return total;
}

void resetTotalBytesSentOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    pia->ia_sTotalBytesSent = 0;
}

void getBufferOfAsocket(
    asocket_t * pAsocket, u8 ** ppBuffer,
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

void clearBufferOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;
}

u32 useSocketForAsocket(asocket_t * pAsocket, socket_t * pSocket, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    logInfoMsg("use socket for as");

    if (! pia->ia_bFree)
    {
        u32Ret = OLERR_ASOCKET_IN_USE;
    }
    else
    {
        pia->ia_bFree = FALSE;

        pia->ia_psSocket = pSocket;

        pia->ia_bFinConnect = TRUE;

        pia->ia_pUser = pUser;

        /*make sure the socket is non-blocking*/
        setSocketNonblock(pia->ia_psSocket);
    }

    return u32Ret;
}

void setRemoteAddressOfAsocket(asocket_t * pAsocket, ip_addr_t * piaAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    memcpy(&(pia->ia_iaRemote), piaAddr, sizeof(ip_addr_t));
}

void getRemoteInterfaceOfAsocket(asocket_t * pAsocket, ip_addr_t * piaAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    memcpy(piaAddr, &(pia->ia_iaRemote), sizeof(ip_addr_t));
}

void getLocalInterfaceOfAsocket(asocket_t * pAsocket, ip_addr_t * piaAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    memcpy(piaAddr, &(pia->ia_iaLocal), sizeof(ip_addr_t));
}

u32 resumeAsocket(asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    logInfoMsg("resume as, pause %u", pia->ia_bPause);
    if (pia->ia_bPause)
    {
        pia->ia_bPause = FALSE;

        logInfoMsg("resume as");

        u32Ret = wakeupBasicChain(pia->ia_pbcChain);
    }

    return u32Ret;
}

u32 asRecvData(asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psRecv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    if ((! pia->ia_bFree) && (pia->ia_psSocket != NULL))
        u32Ret = _asRecvn(pia->ia_psSocket, pu8Buffer, psRecv);
    else
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 getAsocketOpt(
    asocket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    assert(pAsocket != NULL);

    if ((! pia->ia_bFree) && (pia->ia_psSocket != NULL))
        u32Ret = getSocketOpt(pia->ia_psSocket, level, optname, optval, optlen);
    else
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

u32 setAsocketOpt(
    socket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    assert(pAsocket != NULL);

    if ((! pia->ia_bFree) && (pia->ia_psSocket != NULL))
        u32Ret = setSocketOpt(pia->ia_psSocket, level, optname, optval, optlen);
    else
        u32Ret = OLERR_SOCKET_ALREADY_CLOSED;

    return u32Ret;
}

void * getTagOfAsocket(asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_pTag;
}

void setTagOfAsocket(asocket_t * pAsocket, void * pTag)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    pia->ia_pTag = pTag;
}

/*---------------------------------------------------------------------------*/


