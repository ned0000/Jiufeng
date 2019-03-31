/**
 *  @file assocket.c
 *
 *  @brief The async server socket
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
#include "bases.h"

/* --- private data/data structure section --------------------------------- */
struct internal_assocket;

typedef struct assocket_data
{
    struct internal_assocket * ad_iaAssocket;
    void * ad_pUser;
    u32 ad_u32Reserved[2];
} assocket_data_t;

typedef struct internal_assocket
{
    jf_network_chain_object_header_t ia_jncohHeader;
    jf_network_chain_t * ia_pjncChain;

    u16 ia_u32MaxConn;
    u16 ia_u16PortNumber;
    boolean_t ia_bListening;
    u8 ia_u8Reserved[3];
    ip_addr_t ia_iaAddr;

    jf_network_socket_t * ia_pjnsListenSocket;

    jf_network_fnAssocketOnData_t ia_fnOnData;
    jf_network_fnAssocketOnConnect_t ia_fnOnConnect;
    jf_network_fnAssocketOnDisconnect_t ia_fnOnDisconnect;
    jf_network_fnAssocketOnSendOK_t ia_fnOnSendOK;

    sync_mutex_t ia_smAsocket;
    list_array_t * ia_plaAsocket;

    sync_mutex_t * ia_psmAsockets;
    jf_network_asocket_t ** ia_pjnaAsockets;
    assocket_data_t * ia_padData;

    void * ia_pTag;
} internal_assocket_t;

#define ASS_MAX_CONNECTIONS   4000

/* --- private routine section---------------------------------------------- */

/** preselect handler for basic chain
 */
static u32 _preSelectAssocket(
    void * pAssocket, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    /*The socket isn't put in listening mode, until the chain is started.
      If this variable is TRUE, that means we need to do that.*/
    if (! pia->ia_bListening)
    {
        logInfoMsg("pre sel ass, listening on the socket");
        /*Set the socket to non-block mode, so we can play nice and share the
          thread*/
        jf_network_setSocketNonblock(pia->ia_pjnsListenSocket);

        /*Put the socket in listen, and add it to the fdset for the select*/
        pia->ia_bListening = TRUE;
        jf_network_listen(pia->ia_pjnsListenSocket, 4);
        jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
    }
    else
    {
        /*Only put the ia_pjnsListenSocket in the readset, if we are able to
          handle a new socket*/
        acquireSyncMutex(&pia->ia_smAsocket);
        if (! isEndOfListArray(pia->ia_plaAsocket))
            jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
        else
            logInfoMsg("pre sel ass, no free asocket on the socket");
        releaseSyncMutex(&pia->ia_smAsocket);
    }

    return u32Ret;
}

/** Post select handler for basic chain
 */
static u32 _postSelectAssocket(
    void * pAssocket, olint_t slct, fd_set * readset, fd_set * writeset,
    fd_set * errorset)
{
    u32 u32Ret = OLERR_NO_ERROR;
    assocket_data_t * pad;
    internal_assocket_t * pia = (internal_assocket_t *)pAssocket;
    jf_network_socket_t * pNewSocket;
    ip_addr_t ipaddr;
    u16 u16Port;
    u32 u32Index;

    if (jf_network_isSocketSetInFdSet(pia->ia_pjnsListenSocket, readset))
    {
        logInfoMsg("post select assocket, listen socket is in readset");

        /*There are pending TCP connection requests*/
        while (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = jf_network_accept(
                pia->ia_pjnsListenSocket, &ipaddr, &u16Port, &pNewSocket);
            if (u32Ret == OLERR_NO_ERROR)
            {
                /*Check to see if we have available resources to handle
                  this connection request*/
                acquireSyncMutex(&pia->ia_smAsocket);
                u32Index = getListArrayNode(pia->ia_plaAsocket);
                releaseSyncMutex(&pia->ia_smAsocket);
                if (u32Index != LIST_ARRAY_END)
                {
                    logInfoMsg(
                        "post select assocket, new connection, use %u",
                        u32Index);

                    assert(jf_network_isAsocketFree(
                               pia->ia_pjnaAsockets[u32Index]));
                    /*Instantiate a pia to contain all the data about
                      this connection*/
                    pad = &(pia->ia_padData[u32Index]);
                    pad->ad_iaAssocket = pAssocket;
                    pad->ad_pUser = NULL;

                    jf_network_useSocketForAsocket(
                        pia->ia_pjnaAsockets[u32Index], pNewSocket, pad);
                    jf_network_setRemoteAddressOfAsocket(
                        pia->ia_pjnaAsockets[u32Index], &ipaddr);

                    /*Notify the user about this new connection*/
                    if (pia->ia_fnOnConnect != NULL)
                    {
                        pia->ia_fnOnConnect(
                            pia, pia->ia_pjnaAsockets[u32Index],
                            &(pad->ad_pUser));
                    }
                }
                else
                {
                    logErrMsg(u32Ret,
                        "post select assocket, no more free asocket");
                    jf_network_destroySocket(&pNewSocket);
                    break;
                }
            }
            else
            {
                /*iterate all free asockets until all connections are
                  accepted, if the number of pending connection is less than
                  the number of free asocket, an error occurred and that
                  is all right*/
                break;
            }
        }
    }

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying asocket
 *
 *  @param pAsocket [in] the async socket
 *  @param pu8Buffer [in] the buffer
 *  @param psBeginPointer [in/out] the beging pointer of the data 
 *  @param sEndPointer [in] the end pointer of the data 
 *  @param pUser [in] the user
 *  @param pbPause [in] the pause flag
 *
 *  @return the error code
 */
static u32 _assOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser, boolean_t * pbPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    logInfoMsg("ass on data");

    /*Pass the received data up*/
    if (pia->ia_fnOnData != NULL)
    {
        pia->ia_fnOnData(
            pad->ad_iaAssocket, pAsocket, pu8Buffer,
            psBeginPointer, sEndPointer, pad->ad_pUser, pbPause);
    }

    return u32Ret;
}

static u32 _assGetTagOfAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Index;
#if defined(JIUFENG_64BIT)
    u32Index = (u32)(u64)jf_network_getTagOfAsocket(pAsocket);
#else
    u32Index = (u32)jf_network_getTagOfAsocket(pAsocket);
#endif
    return u32Index;
}

/** Internal method dispatched by the disconnect event of the underlying asocket
 *
 *  @param pAsocket [in] the async socket 
 *  @param u32Status [in] the status code for the disconnection
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
static u32 _assOnDisconnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass on disconnect, put %u", u32Index);
    acquireSyncMutex(&pia->ia_smAsocket);
    putListArrayNode(pia->ia_plaAsocket, u32Index);
    releaseSyncMutex(&pia->ia_smAsocket);

    /*Pass this Disconnect event up*/
    if (pia->ia_fnOnDisconnect != NULL)
    {
        pia->ia_fnOnDisconnect(
            pad->ad_iaAssocket, pAsocket, u32Status, pad->ad_pUser);
    }

    return u32Ret;
}

/** Internal method dispatched by the OnSendOK event of the underlying asocket
 *
 *  @param pAsocket [in] the async socket 
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
static u32 _assOnSendOK(jf_network_asocket_t * pAsocket, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    logInfoMsg("ass on send ok");

    /*Pass the OnSendOK event up*/
    if (pia->ia_fnOnSendOK != NULL)
    {
        pia->ia_fnOnSendOK(
            pad->ad_iaAssocket, pAsocket, pad->ad_pUser);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_network_destroyAssocket(jf_network_assocket_t ** ppAssocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) *ppAssocket;
    u32 u32Index;

    logInfoMsg("destroy assocket");

    if (pia->ia_pjnaAsockets != NULL)
    {
        for (u32Index = 0;
             ((u32Index < pia->ia_u32MaxConn) && (u32Ret == OLERR_NO_ERROR));
             u32Index ++)
        {
            if (pia->ia_pjnaAsockets[u32Index] != NULL)
                jf_network_destroyAsocket(&pia->ia_pjnaAsockets[u32Index]);
        }

        xfree((void **)&pia->ia_pjnaAsockets);
    }

    if (pia->ia_psmAsockets != NULL)
    {
        for (u32Index = 0; u32Index < pia->ia_u32MaxConn; u32Index ++)
            finiSyncMutex(&pia->ia_psmAsockets[u32Index]);

        xfree((void **)&pia->ia_psmAsockets);
    }

    if (pia->ia_plaAsocket != NULL)
        xfree((void **)&pia->ia_plaAsocket);

    if (pia->ia_padData != NULL)
        xfree((void **)&pia->ia_padData);

    if (pia->ia_pjnsListenSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsListenSocket));

    finiSyncMutex(&pia->ia_smAsocket);

    xfree(ppAssocket);

    return u32Ret;
}

u32 jf_network_createAssocket(
    jf_network_chain_t * pChain, jf_network_assocket_t ** ppAssocket,
    jf_network_assocket_create_param_t * pjnacp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia;
    jf_network_asocket_create_param_t jnacp;
    u32 u32Index;

    assert((pChain != NULL) && (ppAssocket != NULL) && (pjnacp != NULL));
    assert((pjnacp->jnacp_u32MaxConn != 0) &&
           (pjnacp->jnacp_u32MaxConn <= ASS_MAX_CONNECTIONS) &&
           (pjnacp->jnacp_fnOnConnect != NULL));

    logInfoMsg("create assocket, max conn %u", pjnacp->jnacp_u32MaxConn);

    /*create a new assocket*/
    u32Ret = xcalloc((void **)&pia, sizeof(internal_assocket_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAssocket;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAssocket;
        pia->ia_pjncChain = pChain;

        pia->ia_fnOnConnect = pjnacp->jnacp_fnOnConnect;
        pia->ia_fnOnDisconnect = pjnacp->jnacp_fnOnDisconnect;
        pia->ia_fnOnSendOK = pjnacp->jnacp_fnOnSendOK;
        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;

        pia->ia_pjnsListenSocket = NULL;
        pia->ia_u32MaxConn = pjnacp->jnacp_u32MaxConn;
        pia->ia_u16PortNumber = pjnacp->jnacp_u16PortNumber;
        memcpy(&(pia->ia_iaAddr), &(pjnacp->jnacp_iaAddr), sizeof(ip_addr_t));

        u32Ret = xcalloc(
            (void **)&pia->ia_pjnaAsockets,
            pjnacp->jnacp_u32MaxConn * sizeof(jf_network_asocket_t *));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xcalloc(
            (void **)&pia->ia_plaAsocket,
            sizeOfListArray(pjnacp->jnacp_u32MaxConn));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        initListArray(pia->ia_plaAsocket, pjnacp->jnacp_u32MaxConn);

        u32Ret = xcalloc(
            (void **)&pia->ia_padData,
            pjnacp->jnacp_u32MaxConn * sizeof(assocket_data_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&pia->ia_smAsocket);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = xcalloc(
            (void **)&pia->ia_psmAsockets,
            pjnacp->jnacp_u32MaxConn * sizeof(sync_mutex_t));

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memset(&jnacp, 0, sizeof(jnacp));

        jnacp.jnacp_sInitialBuf = pjnacp->jnacp_sInitialBuf;
        jnacp.jnacp_fnOnData = _assOnData;
        jnacp.jnacp_fnOnDisconnect = _assOnDisconnect;
        jnacp.jnacp_fnOnSendOK = _assOnSendOK;

        jnacp.jnacp_bNoRead = pjnacp->jnacp_bNoRead;

        /*create our socket pool*/
        for (u32Index = 0; 
             ((u32Index < pjnacp->jnacp_u32MaxConn) && (u32Ret == OLERR_NO_ERROR));
             u32Index ++)
        {
            initSyncMutex(&pia->ia_psmAsockets[u32Index]);
            jnacp.jnacp_psmLock = &pia->ia_psmAsockets[u32Index];

            u32Ret = jf_network_createAsocket(
                pChain, &pia->ia_pjnaAsockets[u32Index], &jnacp);
            if (u32Ret == OLERR_NO_ERROR)
            {
#if defined(JIUFENG_64BIT)
                jf_network_setTagOfAsocket(
                    pia->ia_pjnaAsockets[u32Index], (void *)(u64)u32Index);
#else
                jf_network_setTagOfAsocket(
                    pia->ia_pjnaAsockets[u32Index], (void *)u32Index);
#endif
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pChain, pia);

    if (u32Ret == OLERR_NO_ERROR)
        /*Get our listening socket*/
        u32Ret = jf_network_createStreamSocket(
            &pia->ia_iaAddr, &pia->ia_u16PortNumber, &pia->ia_pjnsListenSocket);

    if (u32Ret == OLERR_NO_ERROR)
        *ppAssocket = pia;
    else if (pia != NULL)
        jf_network_destroyAssocket((jf_network_assocket_t **)&pia);

    return u32Ret;
}

u16 jf_network_getPortNumberOfAssocket(jf_network_assocket_t * pAssocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    return pia->ia_u16PortNumber;
}

void * jf_network_getTagOfAssocket(jf_network_assocket_t * pAssocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    return pia->ia_pTag;
}

void jf_network_setTagOfAssocket(jf_network_assocket_t * pAssocket, void * pTag)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    pia->ia_pTag = pTag;
}

u32 jf_network_disconnectAssocket(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass disconnect, index %u", u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_disconnectAsocket(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    acquireSyncMutex(&pia->ia_smAsocket);
    putListArrayNode(pia->ia_plaAsocket, u32Index);
    releaseSyncMutex(&pia->ia_smAsocket);

    return u32Ret;
}

boolean_t jf_network_isAssocketFree(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);
    boolean_t bRet;

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    bRet = jf_network_isAsocketFree(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return bRet;
}

u32 jf_network_sendAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sBuf, jf_network_mem_owner_t memowner)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass send data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_sendAsocketData(pAsocket, pu8Buffer, sBuf, memowner);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 jf_network_resumeAssocket(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("resume ass");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_resumeAsocket(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 jf_network_recvAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psRecv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass recv data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_recvAsocketData(pAsocket, pu8Buffer, psRecv);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 jf_network_getAssocketOpt(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    olint_t level, olint_t optname, void * optval, olsize_t * optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("get ass socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_getAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 jf_network_setAssocketOpt(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, olint_t level,
    olint_t optname, void * optval, olsize_t optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("set ass socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_setAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 jf_network_sendnAssocket(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass sendn");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = jf_network_sendnAsocket(pAsocket, pu8Buffer, psBuf);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


