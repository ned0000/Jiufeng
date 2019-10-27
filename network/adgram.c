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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_network.h"
#include "jf_mutex.h"
#include "jf_jiukun.h"
#include "jf_listhead.h"

#include "adgram.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct send_data
{
    u8 * asd_pu8Buffer;
    olsize_t asd_sBuf;
    olsize_t asd_sBytesSent;

    jf_ipaddr_t asd_jiRemote;
    u16 asd_u16RemotePort;
    u16 asd_u16Reserved[3];

    jf_listhead_t asd_jlList;
} adgram_send_data_t;

typedef struct
{
    jf_network_chain_object_header_t ia_jncohHeader;
    jf_network_chain_t * ia_pjncChain;

    jf_network_socket_t * ia_pjnsSocket;

    jf_ipaddr_t ia_iaRemote;
    u16 ia_u16RemotePort;
    u16 is_u16Reserved3;

    fnAdgramOnData_t ia_fnOnData;
    fnAdgramOnSendData_t ia_fnOnSendData;

    void * ia_pUser;

    jf_network_utimer_t * ia_pjnuUtimer;

    u8 ia_u8Reserved2[8];

    olsize_t ia_sBeginPointer;
    olsize_t ia_sEndPointer;

    u8 * ia_pu8Buffer;
    olsize_t ia_sMalloc;
    u32 ia_u32Reserved;

    olsize_t ia_sTotalDataSent;
    olsize_t ia_sTotalBytesSent;

    u32 ia_u32Status;
    
    jf_listhead_t ia_jlSendData;
    
    /*start of lock protected section*/
    jf_mutex_t ia_jmLock;
    /**wait data list*/
    jf_listhead_t ia_jlWaitData;
    u8 ia_u8Reserved3[8];
    /*end of lock protected section*/

} internal_adgram_t;

/* --- private routine section ------------------------------------------------------------------ */

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

static u32 _destroyAdgramSendData(adgram_send_data_t ** ppsd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    adgram_send_data_t * psd = *ppsd;

    if (psd->asd_pu8Buffer != NULL)
    {
        jf_jiukun_freeMemory((void **)&(psd->asd_pu8Buffer));
    }
    jf_jiukun_freeMemory((void **)ppsd);

    return u32Ret;
}

/** Clears all the pending data to be sent for an async socket
 *
 *  @param pia [in] the asocket to clear
 */
static void _clearPendingSendOfAdgram(internal_adgram_t * pia)
{
    adgram_send_data_t * pasd = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;

    jf_listhead_forEachSafe(&pia->ia_jlSendData, pos, temppos)
    {
        pasd = jf_listhead_getEntry(pos, adgram_send_data_t, asd_jlList);

        pia->ia_fnOnSendData(
            pia, pia->ia_u32Status, pasd->asd_pu8Buffer, pasd->asd_sBuf, pia->ia_pUser);

        _destroyAdgramSendData(&pasd);
    }
    
}

/** Internal method called when data is ready to be processed on an asocket
 *
 *  @param pia: The asocket with pending data
 */
static u32 _processAdgram(internal_adgram_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesReceived;

    u32Ret = _adRecvfrom(
        pia->ia_pjnsSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer,
        &bytesReceived, &pia->ia_iaRemote, &pia->ia_u16RemotePort);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Data was read, so increment our counters*/
        pia->ia_sEndPointer += bytesReceived;

        /*Tell the user we have some data*/
        pia->ia_fnOnData(
            pia, pia->ia_pu8Buffer, &pia->ia_sBeginPointer, pia->ia_sEndPointer, pia->ia_pUser,
            &pia->ia_iaRemote, pia->ia_u16RemotePort);

        /*If the user consumed all of the buffer, we can recycle it*/
        if (pia->ia_sBeginPointer == pia->ia_sEndPointer)
        {
            pia->ia_sBeginPointer = 0;
            pia->ia_sEndPointer = 0;
        }

        if (pia->ia_sMalloc == pia->ia_sEndPointer)
        {
            /*buffer is full, clear the buffer*/
            pia->ia_sBeginPointer = pia->ia_sEndPointer = 0;
        }
    }

    return u32Ret;
}

static u32 _handleAdgramRequest(internal_adgram_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    adgram_send_data_t * pasd = NULL;

    jf_mutex_acquire(&pia->ia_jmLock);
    if (! jf_listhead_isEmpty(&pia->ia_jlWaitData))
        jf_listhead_spliceTail(&pia->ia_jlSendData, &pia->ia_jlWaitData);
    jf_mutex_release(&pia->ia_jmLock);

    if ((pia->ia_pjnsSocket == NULL) && (! jf_listhead_isEmpty(&pia->ia_jlSendData)))
    {
        pasd = jf_listhead_getEntry(pia->ia_jlSendData.jl_pjlNext, adgram_send_data_t, asd_jlList);
        
        u32Ret = jf_network_createTypeDgramSocket(
            pasd->asd_jiRemote.ji_u8AddrType, &pia->ia_pjnsSocket);
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

    _handleAdgramRequest(pia);
    
    if (pia->ia_pjnsSocket != NULL)
    {
        /* needs reading */
        jf_network_setSocketToFdSet(pia->ia_pjnsSocket, readset);
        jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);

        if (! jf_listhead_isEmpty(&pia->ia_jlSendData))
        {
            /* If there is pending data to be sent, then we need to check
               when the socket is writable */
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
        }
    }

    return u32Ret;
}

static u32 _adgramPostSelectSendData(internal_adgram_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent = 0;
    adgram_send_data_t * pasd = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;
    
    /*Keep trying to send data, until we are told we can't*/
    jf_listhead_forEachSafe(&pia->ia_jlSendData, pos, temppos)
    {
        pasd = jf_listhead_getEntry(pos, adgram_send_data_t, asd_jlList);
        bytesSent = pasd->asd_sBuf - pasd->asd_sBytesSent;

        u32Ret = jf_network_sendto(
            pia->ia_pjnsSocket, pasd->asd_pu8Buffer + pasd->asd_sBytesSent,
            &bytesSent, &pasd->asd_jiRemote, pasd->asd_u16RemotePort);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pia->ia_sTotalBytesSent += bytesSent;

            pasd->asd_sBytesSent += bytesSent;
            if (pasd->asd_sBytesSent == pasd->asd_sBuf)
            {
                pia->ia_sTotalDataSent ++;
                /*Finished Sending this block*/
                jf_listhead_del(&pasd->asd_jlList);

                pia->ia_fnOnSendData(
                    pia, u32Ret, pasd->asd_pu8Buffer, pasd->asd_sBytesSent, pia->ia_pUser);

                _destroyAdgramSendData(&pasd);
            }
            else
            {
                /*partial data is sent, the left data will be sent later*/
                break;
            }
        }
        else
        {
            /*There was an error sending*/
            u32Ret = JF_ERR_FAIL_SEND_DATA;
            jf_logger_logErrMsg(u32Ret, "asocket fails to send data");
            pia->ia_u32Status = u32Ret;
            /*disconnect the connection*/
            _clearPendingSendOfAdgram(pia);

            break;
        }
    }

    return u32Ret;
}

/** Chained post select handler for adgram
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
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    jf_logger_logInfoMsg("after select adgram");

    /*Write Handling*/
    if (pia->ia_pjnsSocket != NULL &&
        jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent*/
        /*Keep trying to send data, until we are told we can't*/
        u32Ret = _adgramPostSelectSendData(pia);
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

static u32 _onAdgramSendData(
    jf_network_adgram_t * pAdgram, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyAdgram(jf_network_adgram_t ** ppAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t *pia = (internal_adgram_t *) *ppAdgram;

    /*Clear all the data that is pending to be sent*/
    _clearPendingSendOfAdgram((jf_network_adgram_t *)pia);

    /*Close socket if necessary*/
    if (pia->ia_pjnsSocket != NULL)
    {
        jf_network_destroySocket(&(pia->ia_pjnsSocket));
    }

    /*Free the buffer if necessary*/
    if (pia->ia_pu8Buffer != NULL)
    {
        jf_jiukun_freeMemory((void **)&(pia->ia_pu8Buffer));
    }

    if (pia->ia_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&(pia->ia_pjnuUtimer));

    jf_mutex_fini(&(pia->ia_jmLock));

    jf_jiukun_freeMemory(ppAdgram);

    return u32Ret;
}

u32 createAdgram(
    jf_network_chain_t * pChain, jf_network_adgram_t ** ppAdgram,
    adgram_create_param_t * pjnacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = NULL;

    assert((pChain != NULL) && (pjnacp != NULL) && (ppAdgram != NULL));
    assert((pjnacp->jnacp_fnOnData != NULL));

    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_adgram_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia, sizeof(internal_adgram_t));
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAdgram;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAdgram;
        pia->ia_pjnsSocket = NULL;
        pia->ia_pjncChain = pChain;
        pia->ia_sMalloc = pjnacp->jnacp_sInitialBuf;
        pia->ia_pUser = pjnacp->jnacp_pUser;
        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;
        pia->ia_fnOnSendData = pjnacp->jnacp_fnOnSendData;
        if (pia->ia_fnOnSendData == NULL)
            pia->ia_fnOnSendData = _onAdgramSendData;
        jf_listhead_init(&pia->ia_jlSendData);
        jf_listhead_init(&pia->ia_jlWaitData);

        u32Ret = jf_jiukun_allocMemory(
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
        destroyAdgram((jf_network_adgram_t **)&pia);

    return u32Ret;
}

u32 sendAdgramData(
    jf_network_adgram_t * pAdgram, u8 * pu8Buffer, olsize_t sBuf,
    jf_ipaddr_t * pjiRemote, u16 u16RemotePort)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;
    adgram_send_data_t * data = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&data, sizeof(adgram_send_data_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(data, sizeof(adgram_send_data_t));
        data->asd_pu8Buffer = pu8Buffer;
        data->asd_sBuf = sBuf;

        ol_memcpy(&data->asd_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        data->asd_u16RemotePort = u16RemotePort;

        jf_listhead_init(&data->asd_jlList);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_mutex_acquire(&pia->ia_jmLock);
        jf_listhead_addTail(&pia->ia_jlWaitData, &data->asd_jlList);
        jf_mutex_release(&pia->ia_jmLock);

        u32Ret = jf_network_wakeupChain(pia->ia_pjncChain);
    }
#if 0    
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (pia->ia_pjnsSocket == NULL)
            {
                /*invalid socket, let's create one*/
                u32Ret = _createDgramSocket(pjiRemote, &(pia->ia_pjnsSocket));
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
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



            if (bUnblock)
            {
                jf_network_wakeupChain(pia->ia_pjncChain);
            }
        }
#endif

    return u32Ret;
}

olsize_t getTotalDataSentOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sTotalDataSent;
}

olsize_t getTotalBytesSentOfAdgram(jf_network_adgram_t * pAdgram)
{
    internal_adgram_t *pia = (internal_adgram_t *) pAdgram;

    return pia->ia_sTotalBytesSent;
}

u32 useSocketForAdgram(
    jf_network_adgram_t * pAdgram, jf_network_socket_t * pSocket, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    /* make sure the socket is invalid, if it's valid, we may create before*/
    assert(pia->ia_pjnsSocket == NULL);

    pia->ia_sTotalDataSent = 0;
    pia->ia_sTotalBytesSent = 0;

    pia->ia_pjnsSocket = pSocket;

    pia->ia_pUser = pUser;

    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    /* Make sure the socket is non-blocking, so we can play nice and share
       the thread */
    jf_network_setSocketNonblock(pia->ia_pjnsSocket);

    return u32Ret;
}

u32 joinMulticastGroupOfAdgram(
    jf_network_adgram_t * pAdgram, jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = jf_network_joinMulticastGroup(
        pia->ia_pjnsSocket, pjiAddr, pjiMulticaseAddr);

    return u32Ret;
}

u32 enableBroadcastOfAdgram(jf_network_adgram_t * pAdgram)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_adgram_t * pia = (internal_adgram_t *) pAdgram;

    u32Ret = jf_network_enableBroadcast(pia->ia_pjnsSocket);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


