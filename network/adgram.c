/**
 *  @file adgram.c
 *
 *  @brief The async datagram socket implementation file
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

/* --- private data/data structure section --------------------------------- */
typedef struct send_data
{
    u8 * sd_pu8Buffer;
    olsize_t sd_sBuf;
    olsize_t sd_sBytesSent;

    ip_addr_t sd_iaRemote;
    u16 sd_u16RemotePort;
    u16 sd_u16Reserved[3];

    ad_mem_owner_t sd_amoOwner;

    struct send_data * sd_psdNext;
} send_data_t;

typedef struct
{
    basic_chain_object_header_t ia_bcohHeader;
    basic_chain_t * ia_pbcChain;

    socket_t * ia_psSocket;

    ip_addr_t ia_iaRemote;
    u16 ia_u16RemotePort;
    u16 is_u16Reserved3;

    fnAdgramOnData_t ia_fnOnData;
    fnAdgramOnSendOK_t ia_fnOnSendOK;

    void * ia_pUser;

    utimer_t * ia_puUtimer;

    boolean_t ia_bFree;
    boolean_t ia_bPause;
    u8 ia_u8Reserved2[6];

    olsize_t ia_sBeginPointer;
    olsize_t ia_sEndPointer;

    u8 * ia_pu8Buffer;
    olsize_t ia_sMalloc;
    u32 ia_u32Reserved;

    olsize_t ia_sPendingBytesToSend;
    olsize_t ia_sTotalBytesSent;

    send_data_t * ia_psdHead;
    send_data_t * ia_psdTail;

    sync_mutex_t ia_smLock;
} internal_adgram_t;

/* --- private routine section---------------------------------------------- */
static u32 _adRecvfrom(socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    ip_addr_t * piaRemote, u16 * pu16Port)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = sRecvfrom(pSocket, pBuffer, psRecv, piaRemote, pu16Port);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (*psRecv == 0)
            u32Ret = OLERR_SOCKET_PEER_CLOSED;
    }

    return u32Ret;
}

static u32 _destroySendData(send_data_t ** ppsd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    send_data_t * psd = *ppsd;

    if (psd->sd_amoOwner == AD_MEM_OWNER_ADGRAM)
    {
        xfree((void **)&(psd->sd_pu8Buffer));
    }
    xfree((void **)ppsd);

    return u32Ret;
}

/** Internal method called when data is ready to be processed on an asocket
 *
 *  @param pia: The asocket with pending data
 */
static u32 _processAdgram(internal_adgram_t * pia)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t bytesReceived;

    if (pia->ia_bPause)
        return u32Ret;

    bytesReceived = pia->ia_sMalloc - pia->ia_sEndPointer;
    if (bytesReceived == 0)
    {
        /*buffer is full, let's pause adgram first and then send the interrupt
          event then */
        pia->ia_bPause = TRUE;

        return u32Ret;
    }

    u32Ret = _adRecvfrom(pia->ia_psSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer,
        &bytesReceived, &(pia->ia_iaRemote), &(pia->ia_u16RemotePort));
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*Data was read, so increment our counters*/
        pia->ia_sEndPointer += bytesReceived;

        /*Tell the user we have some data*/
        if (pia->ia_fnOnData != NULL)
        {
            pia->ia_fnOnData(pia, pia->ia_pu8Buffer, &(pia->ia_sBeginPointer),
                pia->ia_sEndPointer, pia->ia_pUser, &(pia->ia_bPause),
                &(pia->ia_iaRemote), pia->ia_u16RemotePort);
        }

        /*If the user consumed all of the buffer, we can recycle it*/
        if (pia->ia_sBeginPointer == pia->ia_sEndPointer)
        {
            pia->ia_sBeginPointer = 0;
            pia->ia_sEndPointer = 0;
        }
    }

    return u32Ret;
}

/** Pre select handler of adgram object for the basic chain
 *
 *  @param pAdgram [in] the async dgram socket 
 *  @param readset [out] the read fd set
 *  @param writeset [out] the write fd set
 *  @param errorset [out] the error fd set
 *  @param pu32BlockTime [out] the block time in millisecond
 *
 *  @return the error code
 */
static u32 _preSelectAdgram(
    basic_chain_object_t * pAdgram, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    logInfoMsg("before select adgram");

    if (pia->ia_psSocket != NULL)
    {
        if (! pia->ia_bPause)
        {
            /* Already Connected, just needs reading */
            setSocketToFdSet(pia->ia_psSocket, readset);
            setSocketToFdSet(pia->ia_psSocket, errorset);
        }

        acquireSyncMutex(&(pia->ia_smLock));
        if (pia->ia_psdHead != NULL)
        {
            /* If there is pending data to be sent, then we need to check
               when the socket is writable */
            setSocketToFdSet(pia->ia_psSocket, writeset);
        }
        releaseSyncMutex(&(pia->ia_smLock));
    }

    return u32Ret;
}

/** Chained PostSelect handler for asocket
 *
 *  @param pAdgram: 
 *  @param slct: 
 *  @param readset: 
 *  @param writeset: 
 *  @param errorset: 
 */
static u32 _postSelectAdgram(basic_chain_object_t * pAdgram, olint_t slct,
    fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = OLERR_NO_ERROR;
    boolean_t bTriggerSendOK = FALSE;
    send_data_t * temp, * psd;
    olsize_t bytesSent = 0;
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    logInfoMsg("after select adgram");

    /*Write Handling*/
    if (pia->ia_psSocket != NULL &&
        isSocketSetInFdSet(pia->ia_psSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent*/
        acquireSyncMutex(&(pia->ia_smLock));
        /*Keep trying to send data, until we are told we can't*/
        while (u32Ret == OLERR_NO_ERROR)
        {
            psd = pia->ia_psdHead;
            bytesSent = psd->sd_sBuf - psd->sd_sBytesSent;
            u32Ret = sSendto(pia->ia_psSocket,
                psd->sd_pu8Buffer + psd->sd_sBytesSent,
                &bytesSent, &(psd->sd_iaRemote), psd->sd_u16RemotePort);
            if (u32Ret == OLERR_NO_ERROR)
            {
                pia->ia_sPendingBytesToSend -= bytesSent;
                pia->ia_sTotalBytesSent += bytesSent;
                pia->ia_psdHead->sd_sBytesSent += bytesSent;
                if (pia->ia_psdHead->sd_sBytesSent ==
                    pia->ia_psdHead->sd_sBuf)
                {
                    /*Finished Sending this block*/
                    if (pia->ia_psdHead == pia->ia_psdTail)
                    {
                        pia->ia_psdTail = NULL;
                    }
                    temp = pia->ia_psdHead->sd_psdNext;
                    _destroySendData(&(pia->ia_psdHead));
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
                clearPendingSendOfAdgram(pAdgram);
            }
        }

        /*This triggers OnSendOK, if all the pending data has been sent.*/
        if (pia->ia_psdHead == NULL && u32Ret == OLERR_NO_ERROR)
        {
            bTriggerSendOK = TRUE;
        }
        releaseSyncMutex(&(pia->ia_smLock));
        if (bTriggerSendOK)
        {
            pia->ia_fnOnSendOK(pia, pia->ia_pUser);
        }
    }

    /*needs reading*/
    if ((pia->ia_psSocket != NULL) &&
        (isSocketSetInFdSet(pia->ia_psSocket, readset) != 0))
    {
        /*Data Available*/
        u32Ret = _processAdgram(pia);
    }

    return u32Ret;
}

static u32 _createDgramSocket(ip_addr_t * piaRemote, socket_t ** ppSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    ip_addr_t ipaddr;
    u16 u16Port;

    memcpy(&ipaddr, piaRemote, sizeof(ip_addr_t));

    setIpAddrToInaddrAny(&ipaddr);

    u32Ret = createDgramSocket(&ipaddr, &u16Port, ppSocket);

    return u32Ret;
}

static u32 _adAddPendingData(
    internal_adgram_t * pia, send_data_t ** ppData, u8 * pu8Buffer,
    olsize_t sBuf, ad_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    send_data_t * pData = *ppData;

    if (memowner == AD_MEM_OWNER_USER)
    {
        /*If we don't own this memory, we need to copy the buffer, because the
          user may free this memory before we have a chance to send it*/
        u32Ret = dupMemory(
            (void **)&pData->sd_pu8Buffer, pu8Buffer, pData->sd_sBuf);
        if (u32Ret == OLERR_NO_ERROR)
        {
            pData->sd_amoOwner = AD_MEM_OWNER_ADGRAM;
        }
        else
        {
            _destroySendData(ppData);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /*There are still bytes that are pending to be sent, so we need to queue
          this up */
        pia->ia_psdTail->sd_psdNext = pData;
        pia->ia_psdTail = pData;
        *pbUnblock = TRUE;
    }

    return u32Ret;
}
                    
static u32 _adTrySendData(
    internal_adgram_t * pia, send_data_t ** ppData, u8 * pu8Buffer,
    olsize_t sBuf, ad_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t bytesSent;
    send_data_t * pData = *ppData;

    bytesSent = pData->sd_sBuf - pData->sd_sBytesSent;
    u32Ret = sSendto(
        pia->ia_psSocket, pData->sd_pu8Buffer + pData->sd_sBytesSent,
        &bytesSent, &pData->sd_iaRemote, pData->sd_u16RemotePort);
    if (u32Ret == OLERR_NO_ERROR)
    {
        /*We were able to send something, so lets increment the counters*/
        pData->sd_sBytesSent += bytesSent;
        pia->ia_sPendingBytesToSend -= bytesSent;
        pia->ia_sTotalBytesSent += bytesSent;
    }
    else
    {
        /*Send returned an error, so lets figure out what it was, as it could be
          normal. Most likely the socket closed while we tried to send*/
        _destroySendData(ppData);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pData->sd_sBytesSent == pData->sd_sBuf)
        {
            /*All of the data has been sent*/
            _destroySendData(ppData);
        }
        else
        {
            /*All of the data wasn't sent, so we need to copy the buffer if we
              don't own the memory, because the user may free the memory, before
              we have a chance to complete sending it*/
            if (memowner == AD_MEM_OWNER_USER)
            {
                u32Ret = dupMemory(
                    (void **)&pData->sd_pu8Buffer, pu8Buffer, pData->sd_sBuf);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    pData->sd_amoOwner = AD_MEM_OWNER_ADGRAM;
                }
                else
                {
                    _destroySendData(ppData);
                }
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                /*There is data pending to be sent, so lets go ahead and try to
                  send it*/
                pia->ia_psdTail = pData;
                pia->ia_psdHead = pData;

                *pbUnblock = TRUE;
            }
        }
    }


    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 destroyAdgram(adgram_t ** ppAdgram)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t *pia = (internal_adgram_t *) *ppAdgram;

    /*Close socket if necessary*/
    if (pia->ia_psSocket != NULL)
    {
        destroySocket(&(pia->ia_psSocket));
    }

    /*Free the buffer if necessary*/
    if (pia->ia_pu8Buffer != NULL)
    {
        xfree((void **)&(pia->ia_pu8Buffer));
        pia->ia_sMalloc = 0;
    }

    /*Clear all the data that is pending to be sent*/
    clearPendingSendOfAdgram((adgram_t *)pia);

    if (pia->ia_puUtimer != NULL)
        destroyUtimer(&(pia->ia_puUtimer));

    finiSyncMutex(&(pia->ia_smLock));

    xfree(ppAdgram);

    return u32Ret;
}

u32 createAdgram(
    basic_chain_t * pChain, adgram_t ** ppAdgram, adgram_param_t * pap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia;

    u32Ret = xcalloc((void **)&pia, sizeof(internal_adgram_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_bcohHeader.bcoh_fnPreSelect = _preSelectAdgram;
        pia->ia_bcohHeader.bcoh_fnPostSelect = _postSelectAdgram;
        pia->ia_psSocket = NULL;
        pia->ia_pbcChain = pChain;
        pia->ia_bFree = TRUE;

        pia->ia_sMalloc = pap->ap_sInitialBuf;
        pia->ia_pUser = pap->ap_pUser;

        pia->ia_fnOnData = pap->ap_fnOnData;
        pia->ia_fnOnSendOK = pap->ap_fnOnSendOK;

        u32Ret = xmalloc(
            (void **)&(pia->ia_pu8Buffer), pap->ap_sInitialBuf);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createUtimer(pChain, &(pia->ia_puUtimer));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = initSyncMutex(&(pia->ia_smLock));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = appendToBasicChain(pChain, pia);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppAdgram = pia;
    else if (pia != NULL)
        destroyAdgram((adgram_t **)&pia);

    return u32Ret;
}

u32 clearPendingSendOfAdgram(adgram_t * pAdgram)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;
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

    pia->ia_sPendingBytesToSend = 0;
    pia->ia_sTotalBytesSent = 0;

    return u32Ret;
}

u32 adSendData(
    adgram_t * pAdgram, u8 * pu8Buffer, olsize_t sBuf,
    ad_mem_owner_t memowner, ip_addr_t * piaRemote, u16 u16RemotePort)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;
    send_data_t * data;
    boolean_t bUnblock = FALSE;

    u32Ret = xcalloc((void **)&data, sizeof(send_data_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        data->sd_pu8Buffer = pu8Buffer;
        data->sd_sBuf = sBuf;
        data->sd_amoOwner = memowner;

        memcpy(&(data->sd_iaRemote), piaRemote, sizeof(ip_addr_t));
        data->sd_u16RemotePort = u16RemotePort;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = acquireSyncMutex(&(pia->ia_smLock));
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (pia->ia_psSocket == NULL)
            {
                /*invalid socket, let's create one*/
                u32Ret = _createDgramSocket(piaRemote, &(pia->ia_psSocket));
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                pia->ia_sPendingBytesToSend += sBuf;
                if (pia->ia_psdTail != NULL)
                {
                    u32Ret = _adAddPendingData(
                        pia, &data, pu8Buffer, sBuf, memowner, &bUnblock);
                }
                else
                {
                    u32Ret = _adTrySendData(
                        pia, &data, pu8Buffer, sBuf, memowner, &bUnblock);
                }
            }

            releaseSyncMutex(&(pia->ia_smLock));

            if (bUnblock)
            {
                wakeupBasicChain(pia->ia_pbcChain);
            }
        }
    }

    return u32Ret;
}

boolean_t isAdgramFree(adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_bFree;
}

olsize_t getPendingBytesToSendOfAdgram(adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sPendingBytesToSend;
}

olsize_t getTotalBytesSentOfAdgram(adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sTotalBytesSent;
}

void resetTotalBytesSentOfAdgram(adgram_t * pAdgram)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_sTotalBytesSent = 0;
}

void getBufferOfAdgram(adgram_t * pAdgram, u8 ** ppBuffer,
    olsize_t * psBeginPointer, olsize_t * psEndPointer)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    *ppBuffer = pia->ia_pu8Buffer;
    *psBeginPointer = pia->ia_sBeginPointer;
    *psEndPointer = pia->ia_sEndPointer;
}

void clearBufferOfAdgram(adgram_t * pAdgram)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;
}

u32 useSocketForAdgram(adgram_t * pAdgram, socket_t * pSocket, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    /* make sure the socket is invalid, if it's valid, we may create before*/
    assert(pia->ia_psSocket == NULL);

    pia->ia_sPendingBytesToSend = 0;
    pia->ia_sTotalBytesSent = 0;

    pia->ia_psSocket = pSocket;

    pia->ia_bFree = FALSE;
    pia->ia_bPause = FALSE;

    pia->ia_pUser = pUser;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    /* Make sure the socket is non-blocking, so we can play nice and share
       the thread */
    setSocketNonblock(pia->ia_psSocket);

    return u32Ret;
}

u32 freeSocketForAdgram(adgram_t * pAdgram)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_bFree = TRUE;
    pia->ia_bPause = TRUE;

    acquireSyncMutex(&(pia->ia_smLock));
    clearPendingSendOfAdgram(pia);
    releaseSyncMutex(&(pia->ia_smLock));

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    if (pia->ia_psSocket != NULL)
        destroySocket(&(pia->ia_psSocket));

    return u32Ret;
}

u32 resumeAdgram(adgram_t * pAdgram)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    if (pia->ia_bPause)
    {
        pia->ia_bPause = FALSE;
        u32Ret = wakeupBasicChain(pia->ia_pbcChain);
    }

    return u32Ret;
}

u32 adJoinMulticastGroup(
    adgram_t * pAdgram, ip_addr_t * piaAddr, ip_addr_t * piaMulticaseAddr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = joinMulticastGroup(pia->ia_psSocket, piaAddr, piaMulticaseAddr);

    return u32Ret;
}

u32 adEnableBroadcast(adgram_t * pAdgram)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = enableBroadcast(pia->ia_psSocket);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


