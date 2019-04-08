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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "network.h"
#include "jf_mutex.h"
#include "jf_mem.h"

/* --- private data/data structure section --------------------------------- */
typedef struct send_data
{
    u8 * sd_pu8Buffer;
    olsize_t sd_sBuf;
    olsize_t sd_sBytesSent;

    jf_ipaddr_t sd_jiRemote;
    u16 sd_u16RemotePort;
    u16 sd_u16Reserved[3];

    jf_network_mem_owner_t sd_jnmoOwner;

    struct send_data * sd_psdNext;
} send_data_t;

typedef struct
{
    jf_network_chain_object_header_t ia_jncohHeader;
    jf_network_chain_t * ia_pjncChain;

    jf_network_socket_t * ia_pjnsSocket;

    jf_ipaddr_t ia_iaRemote;
    u16 ia_u16RemotePort;
    u16 is_u16Reserved3;

    jf_network_fnAdgramOnData_t ia_fnOnData;
    jf_network_fnAdgramOnSendOK_t ia_fnOnSendOK;

    void * ia_pUser;

    jf_network_utimer_t * ia_pjnuUtimer;

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

    jf_mutex_t ia_jmLock;
} internal_adgram_t;

/* --- private routine section---------------------------------------------- */
static u32 _adRecvfrom(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    jf_ipaddr_t * pjiRemote, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_network_recvfrom(pSocket, pBuffer, psRecv, pjiRemote, pu16Port);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (*psRecv == 0)
            u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
    }

    return u32Ret;
}

static u32 _destroySendData(send_data_t ** ppsd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    send_data_t * psd = *ppsd;

    if (psd->sd_jnmoOwner == JF_NETWORK_MEM_OWNER_LIB)
    {
        jf_mem_free((void **)&(psd->sd_pu8Buffer));
    }
    jf_mem_free((void **)ppsd);

    return u32Ret;
}

/** Internal method called when data is ready to be processed on an asocket
 *
 *  @param pia: The asocket with pending data
 */
static u32 _processAdgram(internal_adgram_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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

    u32Ret = _adRecvfrom(pia->ia_pjnsSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer,
        &bytesReceived, &(pia->ia_iaRemote), &(pia->ia_u16RemotePort));
    if (u32Ret == JF_ERR_NO_ERROR)
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

/** Pre select handler of adgram object for the chain
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
    jf_network_chain_object_t * pAdgram, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    jf_logger_logInfoMsg("before select adgram");

    if (pia->ia_pjnsSocket != NULL)
    {
        if (! pia->ia_bPause)
        {
            /* Already Connected, just needs reading */
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, readset);
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);
        }

        jf_mutex_acquire(&(pia->ia_jmLock));
        if (pia->ia_psdHead != NULL)
        {
            /* If there is pending data to be sent, then we need to check
               when the socket is writable */
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
        }
        jf_mutex_release(&(pia->ia_jmLock));
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
static u32 _postSelectAdgram(
    jf_network_chain_object_t * pAdgram, olint_t slct,
    fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bTriggerSendOK = FALSE;
    send_data_t * temp, * psd;
    olsize_t bytesSent = 0;
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    jf_logger_logInfoMsg("after select adgram");

    /*Write Handling*/
    if (pia->ia_pjnsSocket != NULL &&
        jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent*/
        jf_mutex_acquire(&(pia->ia_jmLock));
        /*Keep trying to send data, until we are told we can't*/
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            psd = pia->ia_psdHead;
            bytesSent = psd->sd_sBuf - psd->sd_sBytesSent;
            u32Ret = jf_network_sendto(pia->ia_pjnsSocket,
                psd->sd_pu8Buffer + psd->sd_sBytesSent,
                &bytesSent, &(psd->sd_jiRemote), psd->sd_u16RemotePort);
            if (u32Ret == JF_ERR_NO_ERROR)
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
                jf_network_clearPendingSendOfAdgram(pAdgram);
            }
        }

        /*This triggers OnSendOK, if all the pending data has been sent.*/
        if (pia->ia_psdHead == NULL && u32Ret == JF_ERR_NO_ERROR)
        {
            bTriggerSendOK = TRUE;
        }
        jf_mutex_release(&(pia->ia_jmLock));
        if (bTriggerSendOK)
        {
            pia->ia_fnOnSendOK(pia, pia->ia_pUser);
        }
    }

    /*needs reading*/
    if ((pia->ia_pjnsSocket != NULL) &&
        (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, readset) != 0))
    {
        /*Data Available*/
        u32Ret = _processAdgram(pia);
    }

    return u32Ret;
}

static u32 _createDgramSocket(
    jf_ipaddr_t * pjiRemote, jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t ipaddr;
    u16 u16Port;

    memcpy(&ipaddr, pjiRemote, sizeof(jf_ipaddr_t));

    jf_ipaddr_setIpAddrToInaddrAny(&ipaddr);

    u32Ret = jf_network_createDgramSocket(&ipaddr, &u16Port, ppSocket);

    return u32Ret;
}

static u32 _adAddPendingData(
    internal_adgram_t * pia, send_data_t ** ppData, u8 * pu8Buffer,
    olsize_t sBuf, jf_network_mem_owner_t memowner, boolean_t * pbUnblock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    send_data_t * pData = *ppData;

    if (memowner == JF_NETWORK_MEM_OWNER_USER)
    {
        /*If we don't own this memory, we need to copy the buffer, because the
          user may free this memory before we have a chance to send it*/
        u32Ret = jf_mem_duplicate(
            (void **)&pData->sd_pu8Buffer, pu8Buffer, pData->sd_sBuf);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pData->sd_jnmoOwner = JF_NETWORK_MEM_OWNER_LIB;
        }
        else
        {
            _destroySendData(ppData);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
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
    olsize_t sBuf, jf_network_mem_owner_t memowner,
    boolean_t * pbUnblock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent;
    send_data_t * pData = *ppData;

    bytesSent = pData->sd_sBuf - pData->sd_sBytesSent;
    u32Ret = jf_network_sendto(
        pia->ia_pjnsSocket, pData->sd_pu8Buffer + pData->sd_sBytesSent,
        &bytesSent, &pData->sd_jiRemote, pData->sd_u16RemotePort);
    if (u32Ret == JF_ERR_NO_ERROR)
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

    if (u32Ret == JF_ERR_NO_ERROR)
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
            if (memowner == JF_NETWORK_MEM_OWNER_USER)
            {
                u32Ret = jf_mem_duplicate(
                    (void **)&pData->sd_pu8Buffer, pu8Buffer, pData->sd_sBuf);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    pData->sd_jnmoOwner = JF_NETWORK_MEM_OWNER_LIB;
                }
                else
                {
                    _destroySendData(ppData);
                }
            }

            if (u32Ret == JF_ERR_NO_ERROR)
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

u32 jf_network_destroyAdgram(jf_network_adgram_t ** ppAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t *pia = (internal_adgram_t *) *ppAdgram;

    /*Close socket if necessary*/
    if (pia->ia_pjnsSocket != NULL)
    {
        jf_network_destroySocket(&(pia->ia_pjnsSocket));
    }

    /*Free the buffer if necessary*/
    if (pia->ia_pu8Buffer != NULL)
    {
        jf_mem_free((void **)&(pia->ia_pu8Buffer));
        pia->ia_sMalloc = 0;
    }

    /*Clear all the data that is pending to be sent*/
    jf_network_clearPendingSendOfAdgram((jf_network_adgram_t *)pia);

    if (pia->ia_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&(pia->ia_pjnuUtimer));

    jf_mutex_fini(&(pia->ia_jmLock));

    jf_mem_free(ppAdgram);

    return u32Ret;
}

u32 jf_network_createAdgram(
    jf_network_chain_t * pChain, jf_network_adgram_t ** ppAdgram,
    jf_network_adgram_create_param_t * pjnacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia;

    u32Ret = jf_mem_calloc((void **)&pia, sizeof(internal_adgram_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAdgram;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAdgram;
        pia->ia_pjnsSocket = NULL;
        pia->ia_pjncChain = pChain;
        pia->ia_bFree = TRUE;

        pia->ia_sMalloc = pjnacp->jnacp_sInitialBuf;
        pia->ia_pUser = pjnacp->jnacp_pUser;

        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;
        pia->ia_fnOnSendOK = pjnacp->jnacp_fnOnSendOK;

        u32Ret = jf_mem_alloc(
            (void **)&(pia->ia_pu8Buffer), pjnacp->jnacp_sInitialBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createUtimer(pChain, &(pia->ia_pjnuUtimer));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mutex_init(&(pia->ia_jmLock));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_appendToChain(pChain, pia);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAdgram = pia;
    else if (pia != NULL)
        jf_network_destroyAdgram((jf_network_adgram_t **)&pia);

    return u32Ret;
}

u32 jf_network_clearPendingSendOfAdgram(jf_network_adgram_t * pAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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

u32 jf_network_sendAdgramData(
    jf_network_adgram_t * pAdgram, u8 * pu8Buffer, olsize_t sBuf,
    jf_network_mem_owner_t memowner, jf_ipaddr_t * pjiRemote, u16 u16RemotePort)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;
    send_data_t * data;
    boolean_t bUnblock = FALSE;

    u32Ret = jf_mem_calloc((void **)&data, sizeof(send_data_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        data->sd_pu8Buffer = pu8Buffer;
        data->sd_sBuf = sBuf;
        data->sd_jnmoOwner = memowner;

        memcpy(&(data->sd_jiRemote), pjiRemote, sizeof(jf_ipaddr_t));
        data->sd_u16RemotePort = u16RemotePort;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mutex_acquire(&(pia->ia_jmLock));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pia->ia_pjnsSocket == NULL)
            {
                /*invalid socket, let's create one*/
                u32Ret = _createDgramSocket(pjiRemote, &(pia->ia_pjnsSocket));
            }

            if (u32Ret == JF_ERR_NO_ERROR)
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

            jf_mutex_release(&(pia->ia_jmLock));

            if (bUnblock)
            {
                jf_network_wakeupChain(pia->ia_pjncChain);
            }
        }
    }

    return u32Ret;
}

boolean_t jf_network_isAdgramFree(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_bFree;
}

olsize_t jf_network_getPendingBytesToSendOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sPendingBytesToSend;
}

olsize_t jf_network_getTotalBytesSentOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sTotalBytesSent;
}

void jf_network_resetTotalBytesSentOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_sTotalBytesSent = 0;
}

void jf_network_getBufferOfAdgram(
    jf_network_adgram_t * pAdgram, u8 ** ppBuffer,
    olsize_t * psBeginPointer, olsize_t * psEndPointer)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    *ppBuffer = pia->ia_pu8Buffer;
    *psBeginPointer = pia->ia_sBeginPointer;
    *psEndPointer = pia->ia_sEndPointer;
}

void jf_network_clearBufferOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;
}

u32 jf_network_useSocketForAdgram(
    jf_network_adgram_t * pAdgram, jf_network_socket_t * pSocket, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    /* make sure the socket is invalid, if it's valid, we may create before*/
    assert(pia->ia_pjnsSocket == NULL);

    pia->ia_sPendingBytesToSend = 0;
    pia->ia_sTotalBytesSent = 0;

    pia->ia_pjnsSocket = pSocket;

    pia->ia_bFree = FALSE;
    pia->ia_bPause = FALSE;

    pia->ia_pUser = pUser;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    /* Make sure the socket is non-blocking, so we can play nice and share
       the thread */
    jf_network_setSocketNonblock(pia->ia_pjnsSocket);

    return u32Ret;
}

u32 jf_network_freeSocketForAdgram(jf_network_adgram_t * pAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    pia->ia_bFree = TRUE;
    pia->ia_bPause = TRUE;

    jf_mutex_acquire(&(pia->ia_jmLock));
    jf_network_clearPendingSendOfAdgram(pia);
    jf_mutex_release(&(pia->ia_jmLock));

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    if (pia->ia_pjnsSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsSocket));

    return u32Ret;
}

u32 jf_network_resumeAdgram(jf_network_adgram_t * pAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    if (pia->ia_bPause)
    {
        pia->ia_bPause = FALSE;
        u32Ret = jf_network_wakeupChain(pia->ia_pjncChain);
    }

    return u32Ret;
}

u32 jf_network_joinMulticastGroupOfAdgram(
    jf_network_adgram_t * pAdgram,
    jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = jf_network_joinMulticastGroup(
        pia->ia_pjnsSocket, pjiAddr, pjiMulticaseAddr);

    return u32Ret;
}

u32 jf_network_enableBroadcastOfAdgram(jf_network_adgram_t * pAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = jf_network_enableBroadcast(pia->ia_pjnsSocket);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


