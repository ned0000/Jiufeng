/**
 *  @file asocket.c
 *
 *  @brief Implementation file for async socket.
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
#include "jf_string.h"
#include "jf_listhead.h"

#include "asocket.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the send data data type. 
 */
typedef struct asocket_send_data
{
    /**Data buffer.*/
    u8 * asd_pu8Buffer;
    /**Data size.*/
    olsize_t asd_sBuf;
    /**Bytes have been sent already.*/
    olsize_t asd_sBytesSent;

    /**Linked list of send data.*/
    jf_listhead_t asd_jlList;

    u8 asd_u8Reserved[8];
} asocket_send_data_t;

/** Define the internal async socket data type.
 */
typedef struct
{
    /**The network chain object header. MUST BE the first field.*/
    jf_network_chain_object_header_t ia_jncohHeader;
    /**The network chain.*/
    jf_network_chain_t * ia_pjncChain;

    /**Name of the object.*/
    olchar_t ia_strName[JF_NETWORK_MAX_NAME_LEN];

    /**Total data which have been sent.*/
    olsize_t ia_sTotalSendData;
    /**Total bytes which have been sent.*/
    olsize_t ia_sTotalBytesSent;

    /**Network socket of this async socket.*/
    jf_network_socket_t * ia_pjnsSocket;

    /**Remote address of the connection.*/
    jf_ipaddr_t ia_jiRemote;    
    /**Remote port of the connection.*/
    u16 ia_u16RemotePort;
    u16 ia_u16Reserved[3];

    /**Local address of the connection.*/
    jf_ipaddr_t ia_jiLocal;
    /**Local port of the connection.*/
    u16 ia_u16LocalPort;
    u16 ia_u16Reserved2[3];

    /**Pointer to the user data.*/
    void * ia_pUser;

    /**Internal timer of async socket.*/
    jf_network_utimer_t * ia_pjnuUtimer;

    /**List of data to be sent.*/
    jf_listhead_t ia_jlSendData;

    /**Connection is established.*/
    boolean_t ia_bFinConnect;
    u8 ia_u8Reserved2[7];

    /**Buffer for the data.*/
    u8 * ia_pu8Buffer;
    /**Size of the buffer.*/
    olsize_t ia_sBuffer;
    /**Index used by async server socket and async client socket. Asocket should not touch it.*/
    u32 ia_u32Index;

    /**Begin pointer of the data in the buffer.*/
    olsize_t ia_sBeginPointer;
    /**End pointer of the data in the buffer.*/
    olsize_t ia_sEndPointer;

    /**Saved error code.*/
    u32 ia_u32Status;
    u32 ia_u32Reserved3;

    /**Callback function for incoming data.*/
    fnAsocketOnData_t ia_fnOnData;
    /**Callback function for connection.*/
    fnAsocketOnConnect_t ia_fnOnConnect;
    /**Callback function for disconnection.*/
    fnAsocketOnDisconnect_t ia_fnOnDisconnect;
    /**Callback function for sent data.*/
    fnAsocketOnSendData_t ia_fnOnSendData;

    /**Accessed by outside, asocket should not touch it.*/
    void * ia_pTag;

    /*start of lock protected section.*/
    /**Mutex lock.*/
    jf_mutex_t ia_jmLock;
    /**Wait data list.*/
    jf_listhead_t ia_jlWaitData;
    /**If the asocket is free or not.*/
    boolean_t ia_bFree;
    u8 ia_u8Reserved3[7];
    /*end of lock protected section.*/
    
} internal_asocket_t;

/* --- private routine section ------------------------------------------------------------------ */

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

static void _destroyAsocketSendData(asocket_send_data_t ** ppasd)
{
    asocket_send_data_t * pasd = *ppasd;

    if (pasd->asd_pu8Buffer != NULL)
    {
        jf_jiukun_freeMemory((void **)&pasd->asd_pu8Buffer);
    }
    jf_jiukun_freeMemory((void **)ppasd);
}

/** Clears all the pending data to be sent for an async socket.
 *
 *  @param pia [in] The asocket to clear.
 */
static void _clearPendingSendOfAsocket(internal_asocket_t * pia)
{
    asocket_send_data_t * pasd = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);
    
    /*Move the data from waiting list to send list as Wait data should be also freed.*/
    jf_mutex_acquire(&pia->ia_jmLock);
    if (! jf_listhead_isEmpty(&pia->ia_jlWaitData))
        jf_listhead_spliceTail(&pia->ia_jlSendData, &pia->ia_jlWaitData);
    jf_mutex_release(&pia->ia_jmLock);
            
    /*Free the send data one by one.*/
    jf_listhead_forEachSafe(&pia->ia_jlSendData, pos, temppos)
    {
        pasd = jf_listhead_getEntry(pos, asocket_send_data_t, asd_jlList);

        jf_listhead_del(&pasd->asd_jlList);

        /*Invoke the callback function to notify the data is failed to be sent.*/
        pia->ia_fnOnSendData(
            pia, pia->ia_u32Status, pasd->asd_pu8Buffer, pasd->asd_sBuf, pia->ia_pUser);

        /*Free the data.*/
        _destroyAsocketSendData(&pasd);
    }
    
}

static u32 _freeAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    pia->ia_sTotalSendData = 0;
    pia->ia_sTotalBytesSent = 0;
    pia->ia_bFinConnect = FALSE;

    pia->ia_pUser = NULL;
    /*Initialise the buffer pointers, since no data is in them yet.*/
    pia->ia_sBeginPointer = 0;
    pia->ia_sEndPointer = 0;

    pia->ia_u32Status = 0;

    ol_memset(&(pia->ia_jiRemote), 0, sizeof(jf_ipaddr_t));
    pia->ia_u16RemotePort = 0;

    ol_memset(&(pia->ia_jiLocal), 0, sizeof(jf_ipaddr_t));
    pia->ia_u16LocalPort = 0;

    pia->ia_bFree = TRUE;

    return u32Ret;
}

/** Disconnect the asocket.
 *
 *  @param pia [in] The asocket to disconnect.
 */
static u32 _asDisconnect(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Since the socket is closing, we need to clear the data that is pending to be sent.*/
    _clearPendingSendOfAsocket(pia);

    if (pia->ia_pjnsSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsSocket));

    if (pia->ia_fnOnDisconnect != NULL)
        /*Trigger the OnDissconnect event if necessary.*/
        pia->ia_fnOnDisconnect(pia, pia->ia_u32Status, pia->ia_pUser);

    _freeAsocket(pia);

    return u32Ret;
}

/** Internal method called when data is ready to be processed on an asocket.
 *
 *  @param pia [in] The asocket with pending data.
 */
static u32 _processAsocket(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesReceived;

    bytesReceived = pia->ia_sBuffer - pia->ia_sEndPointer;

    u32Ret = _asRecvn(
        pia->ia_pjnsSocket, pia->ia_pu8Buffer + pia->ia_sEndPointer, &bytesReceived);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Data was read, so increment our counters*/
        pia->ia_sEndPointer += bytesReceived;

        pia->ia_fnOnData(
            pia, pia->ia_pu8Buffer, &pia->ia_sBeginPointer, pia->ia_sEndPointer, pia->ia_pUser);

        JF_LOGGER_DEBUG(
            "name: %s, beginp: %d, endp: %d", pia->ia_strName, pia->ia_sBeginPointer,
            pia->ia_sEndPointer);

        if (pia->ia_sBeginPointer == pia->ia_sEndPointer)
        {
            /*If the user consumed all of the buffer, we can recycle it*/
            pia->ia_sBeginPointer = 0;
            pia->ia_sEndPointer = 0;
        }
        else if ((pia->ia_sBeginPointer != pia->ia_sEndPointer) && (pia->ia_sBeginPointer != 0))
        {
            /*partial data is consumed, delete the consumed data*/
            ol_memmove(
                pia->ia_pu8Buffer, pia->ia_pu8Buffer + pia->ia_sBeginPointer,
                pia->ia_sEndPointer - pia->ia_sBeginPointer);

            pia->ia_sEndPointer -= pia->ia_sBeginPointer;
            pia->ia_sBeginPointer = 0;
        }
        else if (pia->ia_sBuffer == pia->ia_sEndPointer)
        {
            /*buffer is full, clear the buffer*/
            JF_LOGGER_ERR(
                JF_ERR_BUFFER_IS_FULL, "name: %s, buffer is full, clear it", pia->ia_strName);
            pia->ia_sBeginPointer = pia->ia_sEndPointer = 0;
        }
    }
    else    
    {
        JF_LOGGER_ERR(u32Ret, "name: %s", pia->ia_strName);
        /*This means the socket was gracefully closed by peer*/
        pia->ia_u32Status = u32Ret;
        _asDisconnect(pia);
    }

    return u32Ret;
}

static u32 _asConnectTo(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*If there isn't a socket already allocated, we need to allocate one.*/
    if (pia->ia_pjnsSocket == NULL)
    {
        JF_LOGGER_DEBUG("name: %s, create stream socket", pia->ia_strName);
        u32Ret = jf_network_createTypeStreamSocket(
            pia->ia_jiRemote.ji_u8AddrType, &pia->ia_pjnsSocket);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_network_setSocketNonblock(pia->ia_pjnsSocket);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Connect to the server, no need to wakeup the chain, as this function is called in utimer
          and utimer is called before pre-select function of async socket.*/
        u32Ret = jf_network_connect(pia->ia_pjnsSocket, &pia->ia_jiRemote, pia->ia_u16RemotePort);
    }

    return u32Ret;
}

/** Pre select handler for asocket.
 *
 *  @param pAsocket [in] The async socket. 
 *  @param readset [out] The read fd set.
 *  @param writeset [out] The write fd set.
 *  @param errorset [out] The error fd set.
 *  @param pu32BlockTime [out] The block time in millisecond.
 *
 *  @return The error code.
 */
static u32 _preSelectAsocket(
    jf_network_chain_object_t * pAsocket, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
#if defined(DEBUG_ASOCKET)
    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);
#endif
    if (pia->ia_pjnsSocket != NULL)
    {
        if (! pia->ia_bFinConnect)
        {
            /*Not connected yet.*/
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);
        }
        else
        {
            /*Already connected, just needs reading.*/
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, readset);
            jf_network_setSocketToFdSet(pia->ia_pjnsSocket, errorset);

            jf_mutex_acquire(&pia->ia_jmLock);
            if (! jf_listhead_isEmpty(&pia->ia_jlWaitData))
                jf_listhead_spliceTail(&pia->ia_jlSendData, &pia->ia_jlWaitData);
            jf_mutex_release(&pia->ia_jmLock);
            
            if (! jf_listhead_isEmpty(&pia->ia_jlSendData))
            {
                /*If there is pending data to be sent, then we need to check when the socket is
                  writable.*/
                jf_network_setSocketToFdSet(pia->ia_pjnsSocket, writeset);
            }
        }
    }

    return u32Ret;
}

static u32 _asPostSelectSendData(internal_asocket_t * pia)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t bytesSent = 0;
    asocket_send_data_t * pasd = NULL;
    jf_listhead_t * pos = NULL, * temppos = NULL;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Keep trying to send data, until we are told we can't.*/
    jf_listhead_forEachSafe(&pia->ia_jlSendData, pos, temppos)
    {
        pasd = jf_listhead_getEntry(pos, asocket_send_data_t, asd_jlList);
        bytesSent = pasd->asd_sBuf - pasd->asd_sBytesSent;

        u32Ret = jf_network_send(
            pia->ia_pjnsSocket, pasd->asd_pu8Buffer + pasd->asd_sBytesSent, &bytesSent);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pia->ia_sTotalBytesSent += bytesSent;

            pasd->asd_sBytesSent += bytesSent;
            if (pasd->asd_sBytesSent == pasd->asd_sBuf)
            {
                pia->ia_sTotalSendData ++;
                /*Finished Sending this block.*/
                jf_listhead_del(&pasd->asd_jlList);

                pia->ia_fnOnSendData(
                    pia, u32Ret, pasd->asd_pu8Buffer, pasd->asd_sBytesSent, pia->ia_pUser);

                _destroyAsocketSendData(&pasd);
            }
            else
            {
                /*Partial data is sent, the left data will be sent later.*/
                break;
            }
        }
        else
        {
            /*There was an error sending.*/
            u32Ret = JF_ERR_FAIL_SEND_DATA;
            JF_LOGGER_ERR(u32Ret, "name: %s, fails to send data", pia->ia_strName);
            pia->ia_u32Status = u32Ret;
            /*Disconnect the connection.*/
            _asDisconnect(pia);

            break;
        }
    }

    return u32Ret;
}

/** Post select handler for chain.
 *
 *  @param pAsocket [in] The async socket.
 *  @param slct [in] Number of ready socket. 
 *  @param readset [in] The read fd set.
 *  @param writeset [in] The write fd set.
 *  @param errorset [in] The error fd set.
 *
 *  @return The error code.
 */
static u32 _postSelectAsocket(
    jf_network_chain_object_t * pAsocket, olint_t slct,
    fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Addr[256];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nLen = sizeof(u8Addr);
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
#if defined(DEBUG_ASOCKET)
    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);
#endif
    /*Write handling.*/
    if ((pia->ia_pjnsSocket != NULL) && pia->ia_bFinConnect &&
        jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0)
    {
        /*The socket is writable, and data needs to be sent.*/
        u32Ret = _asPostSelectSendData(pia);
    }

    /*Connection handling / read handling.*/
    if (pia->ia_pjnsSocket != NULL)
    {
        /*Close the connection if socket is in the errorset, maybe peer is closed.*/
        if (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, errorset) != 0)
        {
            JF_LOGGER_DEBUG("name: %s, in errorset", pia->ia_strName);

            /*Connection failed.*/
            pia->ia_u32Status = JF_ERR_SOCKET_CONNECTION_NOT_SETUP;
            pia->ia_fnOnConnect(pia, pia->ia_u32Status, pia->ia_pUser);

            _freeAsocket(pia);
        }
        else if ((! pia->ia_bFinConnect) &&
                 (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, writeset) != 0))
        {
            /*Connected.*/
            JF_LOGGER_DEBUG("name: %s, connected", pia->ia_strName);

            jf_network_getSocketName(pia->ia_pjnsSocket, psa, &nLen);
            jf_ipaddr_convertSockAddrToIpAddr(
                psa, nLen, &pia->ia_jiLocal, &pia->ia_u16LocalPort);

            pia->ia_bFinConnect = TRUE;

            /*Connection complete.*/
            pia->ia_fnOnConnect(pia, JF_ERR_NO_ERROR, pia->ia_pUser);
        }
        else if (jf_network_isSocketSetInFdSet(pia->ia_pjnsSocket, readset) != 0)
        {
            /*Data Available.*/
            u32Ret = _processAsocket(pia);
        }
    }

    return u32Ret;
}

static u32 _asAddSendData(
    internal_asocket_t * pia, u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    asocket_send_data_t * pasd = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pasd, sizeof(*pasd));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pasd, sizeof(*pasd));
        pasd->asd_pu8Buffer = pu8Buffer;
        pasd->asd_sBuf = sBuf;
        jf_listhead_init(&pasd->asd_jlList);

        /*Clone the data.*/
        u32Ret = jf_jiukun_cloneMemory((void **)&pasd->asd_pu8Buffer, pu8Buffer, sBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Queue up the data to wait data list.*/
        JF_LOGGER_DEBUG("name: %s, add send data to wait list", pia->ia_strName);
        jf_mutex_acquire(&pia->ia_jmLock);
        jf_listhead_addTail(&pia->ia_jlWaitData, &pasd->asd_jlList);
        jf_mutex_release(&pia->ia_jmLock);
    }

    if ((u32Ret != JF_ERR_NO_ERROR) && (pasd != NULL))
        _destroyAsocketSendData(&pasd);
    
    return u32Ret;
}

static u32 _asocketOnData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

static u32 _asocketOnConnect(
    jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

static u32 _asocketOnDisconnect(
    jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

static u32 _asocketOnSendData(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

static void _setInternalCallbackFunction(
    internal_asocket_t * pia, asocket_create_param_t * pacp)
{
    pia->ia_fnOnData = pacp->acp_fnOnData;
    if (pia->ia_fnOnData == NULL)
        pia->ia_fnOnData = _asocketOnData;

    pia->ia_fnOnConnect = pacp->acp_fnOnConnect;
    if (pia->ia_fnOnConnect == NULL)
        pia->ia_fnOnConnect = _asocketOnConnect;

    pia->ia_fnOnDisconnect = pacp->acp_fnOnDisconnect;
    if (pia->ia_fnOnDisconnect == NULL)
        pia->ia_fnOnDisconnect = _asocketOnDisconnect;

    pia->ia_fnOnSendData = pacp->acp_fnOnSendData;
    if (pia->ia_fnOnSendData == NULL)
        pia->ia_fnOnSendData = _asocketOnSendData;

}

static u32 _asUtimerConnect(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pData;

    JF_LOGGER_DEBUG("name: %s, utimer trigger connect to", pia->ia_strName);
    
    u32Ret = _asConnectTo(pia);

    return u32Ret;
}

static u32 _asUtimerDisconnect(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pData;

    JF_LOGGER_DEBUG("name: %s, disconnect", pia->ia_strName);
    pia->ia_u32Status = JF_ERR_SOCKET_LOCAL_CLOSED;

    u32Ret = _asDisconnect(pia);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyAsocket(jf_network_asocket_t ** ppAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = NULL;

    assert((ppAsocket != NULL) && (*ppAsocket != NULL));

    pia = (internal_asocket_t *) *ppAsocket;
    JF_LOGGER_INFO("name: %s", pia->ia_strName);

    /*Clear all the data that is pending to be sent*/
    pia->ia_u32Status = JF_ERR_SOCKET_LOCAL_CLOSED;
    _clearPendingSendOfAsocket(pia);

    /*Close socket if necessary*/
    if (pia->ia_pjnsSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsSocket));

    /*Free the buffer if necessary*/
    if (pia->ia_pu8Buffer != NULL)
    {
        jf_jiukun_freeMemory((void **)&pia->ia_pu8Buffer);
        pia->ia_sBuffer = 0;
    }

    jf_mutex_fini(&pia->ia_jmLock);
    
    if (pia->ia_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&pia->ia_pjnuUtimer);

    jf_jiukun_freeMemory(ppAsocket);

    return u32Ret;
}

u32 createAsocket(
    jf_network_chain_t * pChain, jf_network_asocket_t ** ppAsocket, asocket_create_param_t * pacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = NULL;

    assert((pChain != NULL) && (pacp != NULL) && (ppAsocket != NULL));
    assert((pacp->acp_fnOnData != NULL));

    JF_LOGGER_INFO("name: %s", pacp->acp_pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_asocket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia, sizeof(internal_asocket_t));
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAsocket;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAsocket;
        pia->ia_pjncChain = pChain;
        pia->ia_bFree = TRUE;
        pia->ia_pjnsSocket = NULL;
        jf_listhead_init(&pia->ia_jlSendData);
        jf_listhead_init(&pia->ia_jlWaitData);
        _setInternalCallbackFunction(pia, pacp);
        pia->ia_sBuffer = pacp->acp_sInitialBuf;
        ol_strncpy(pia->ia_strName, pacp->acp_pstrName, JF_NETWORK_MAX_NAME_LEN - 1);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&pia->ia_pu8Buffer, pacp->acp_sInitialBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pia->ia_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(pChain, &pia->ia_pjnuUtimer, pia->ia_strName);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pChain, pia);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAsocket = pia;
    else if (pia != NULL)
        destroyAsocket((jf_network_asocket_t **)&pia);

    return u32Ret;
}

u32 disconnectAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    jf_mutex_acquire(&pia->ia_jmLock);
    if (pia->ia_bFree)
    {
        u32Ret = JF_ERR_SOCKET_ALREADY_CLOSED;
        JF_LOGGER_ERR(u32Ret, "name: %s, free", pia->ia_strName);
    }
    jf_mutex_release(&pia->ia_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_addUtimerItem(
            pia->ia_pjnuUtimer, pia, 0, _asUtimerDisconnect, NULL);
    }

    return u32Ret;
}

u32 sendAsocketData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    JF_LOGGER_DEBUG(
        "name: %s, %02x %02x %02x %02x, %d", pia->ia_strName,
        pu8Buffer[0], pu8Buffer[1], pu8Buffer[2], pu8Buffer[3], sBuf);

    jf_mutex_acquire(&pia->ia_jmLock);
    if (pia->ia_bFree)
        /*the socket is not connected*/
        u32Ret = JF_ERR_SOCKET_CONNECTION_NOT_SETUP;
    jf_mutex_release(&pia->ia_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _asAddSendData(pia, pu8Buffer, sBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_wakeupChain(pia->ia_pjncChain);
    }

    return u32Ret;
}

u32 connectAsocketTo(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiRemote, u16 u16Port, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    olchar_t strServer[50];

    assert((pAsocket != NULL) && (pjiRemote != NULL));

    jf_ipaddr_getStringIpAddrPort(strServer, sizeof(strServer), pjiRemote, u16Port);
    JF_LOGGER_DEBUG("name: %s, server: %s", pia->ia_strName, strServer);

    jf_mutex_acquire(&pia->ia_jmLock);
    if (! pia->ia_bFree)
    {
        u32Ret = JF_ERR_ASOCKET_IN_USE;
    }
    else
    {
        ol_memcpy(&pia->ia_jiRemote, pjiRemote, sizeof(jf_ipaddr_t));
        pia->ia_u16RemotePort = u16Port;
        pia->ia_pUser = pUser;

        pia->ia_bFree = FALSE;
    }
    jf_mutex_release(&pia->ia_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_addUtimerItem(pia->ia_pjnuUtimer, pia, 0, _asUtimerConnect, NULL);
    }
    
    return u32Ret;
}

boolean_t isAsocketFree(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;
    boolean_t bFree = FALSE;

    jf_mutex_acquire(&pia->ia_jmLock);
    bFree = pia->ia_bFree;
    jf_mutex_release(&pia->ia_jmLock);

    return bFree;
}

olsize_t getTotalSendDataOfAsocket(
    jf_network_asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t toSend;

    toSend = pia->ia_sTotalSendData;

    return toSend;
}

olsize_t getTotalBytesSentOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t *pia = (internal_asocket_t *) pAsocket;
    olsize_t total;

    total = pia->ia_sTotalBytesSent;

    return total;
}

u32 useSocketForAsocket(
    jf_network_asocket_t * pAsocket, jf_network_socket_t * pSocket,
    jf_ipaddr_t * pjiRemote, u16 u16RemotePort, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *)pAsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    if (! pia->ia_bFree)
        u32Ret = JF_ERR_ASOCKET_IN_USE;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*make sure the socket is non-blocking*/
        jf_network_setSocketNonblock(pSocket);

        pia->ia_pjnsSocket = pSocket;
        ol_memcpy(&pia->ia_jiRemote, pjiRemote, sizeof(*pjiRemote));
        pia->ia_u16RemotePort = u16RemotePort;
        pia->ia_pUser = pUser;

        pia->ia_bFinConnect = TRUE;        
        pia->ia_bFree = FALSE;
    }

    return u32Ret;
}

void getRemoteInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    ol_memcpy(pjiAddr, &(pia->ia_jiRemote), sizeof(jf_ipaddr_t));
}

void getLocalInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    ol_memcpy(pjiAddr, &(pia->ia_jiLocal), sizeof(jf_ipaddr_t));
}

void * getTagOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_pTag;
}

void setTagOfAsocket(jf_network_asocket_t * pAsocket, void * pTag)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    pia->ia_pTag = pTag;
}

u32 getIndexOfAsocket(jf_network_asocket_t * pAsocket)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    return pia->ia_u32Index;
}

void setIndexOfAsocket(jf_network_asocket_t * pAsocket, u32 u32Index)
{
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    pia->ia_u32Index = u32Index;
}

u32 jf_network_getAsocketOption(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t * psOptval)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    u32Ret = jf_network_getSocketOption(pia->ia_pjnsSocket, level, optname, pOptval, psOptval);

    return u32Ret;
}

u32 jf_network_setAsocketOption(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t sOptval)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_asocket_t * pia = (internal_asocket_t *) pAsocket;

    u32Ret = jf_network_setSocketOption(pia->ia_pjnsSocket, level, optname, pOptval, sOptval);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
