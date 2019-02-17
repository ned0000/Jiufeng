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
    basic_chain_object_header_t ia_bcohHeader;
    basic_chain_t * ia_pbaChain;

    u16 ia_u32MaxConn;
    u16 ia_u16PortNumber;
    boolean_t ia_bListening;
    u8 ia_u8Reserved[3];
    ip_addr_t ia_iaAddr;

    socket_t * ia_psListenSocket;

    fnAssocketOnData_t ia_fnOnData;
    fnAssocketOnConnect_t ia_fnOnConnect;
    fnAssocketOnDisconnect_t ia_fnOnDisconnect;
    fnAssocketOnSendOK_t ia_fnOnSendOK;

    sync_mutex_t ia_smAsocket;
    list_array_t * ia_plaAsocket;

    sync_mutex_t * ia_psmAsockets;
    asocket_t ** ia_ppaAsockets;
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
        setSocketNonblock(pia->ia_psListenSocket);

        /*Put the socket in listen, and add it to the fdset for the select*/
        pia->ia_bListening = TRUE;
        sListen(pia->ia_psListenSocket, 4);
        setSocketToFdSet(pia->ia_psListenSocket, readset);
    }
    else
    {
        /*Only put the ia_psListenSocket in the readset, if we are able to
          handle a new socket*/
        acquireSyncMutex(&pia->ia_smAsocket);
        if (! isEndOfListArray(pia->ia_plaAsocket))
            setSocketToFdSet(pia->ia_psListenSocket, readset);
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
    socket_t * pNewSocket;
    ip_addr_t ipaddr;
    u16 u16Port;
    u32 u32Index;

    if (isSocketSetInFdSet(pia->ia_psListenSocket, readset))
    {
        logInfoMsg("post select assocket, listen socket is in readset");

        /*There are pending TCP connection requests*/
        while (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = sAccept(
                pia->ia_psListenSocket, &ipaddr, &u16Port, &pNewSocket);
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

                    assert(isAsocketFree(pia->ia_ppaAsockets[u32Index]));
                    /*Instantiate a pia to contain all the data about
                      this connection*/
                    pad = &(pia->ia_padData[u32Index]);
                    pad->ad_iaAssocket = pAssocket;
                    pad->ad_pUser = NULL;

                    useSocketForAsocket(pia->ia_ppaAsockets[u32Index],
                        pNewSocket, pad);
                    setRemoteAddressOfAsocket(pia->ia_ppaAsockets[u32Index],
                        &ipaddr);

                    /*Notify the user about this new connection*/
                    if (pia->ia_fnOnConnect != NULL)
                    {
                        pia->ia_fnOnConnect(pia, pia->ia_ppaAsockets[u32Index],
                            &(pad->ad_pUser));
                    }
                }
                else
                {
                    logErrMsg(u32Ret,
                        "post select assocket, no more free asocket");
                    destroySocket(&pNewSocket);
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

static u32 _assGetTagOfAsocket(asocket_t * pAsocket)
{
    u32 u32Index;
#if defined(JIUFENG_64BIT)
    u32Index = (u32)(u64)getTagOfAsocket(pAsocket);
#else
    u32Index = (u32)getTagOfAsocket(pAsocket);
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
static u32 _assOnDisconnect(asocket_t * pAsocket, u32 u32Status, void * pUser)
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
static u32 _assOnSendOK(asocket_t * pAsocket, void * pUser)
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

u32 destroyAssocket(assocket_t ** ppAssocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) *ppAssocket;
    u32 u32Index;

    logInfoMsg("destroy assocket");

    if (pia->ia_ppaAsockets != NULL)
    {
        for (u32Index = 0;
             ((u32Index < pia->ia_u32MaxConn) && (u32Ret == OLERR_NO_ERROR));
             u32Index ++)
        {
            if (pia->ia_ppaAsockets[u32Index] != NULL)
                destroyAsocket(&pia->ia_ppaAsockets[u32Index]);
        }

        xfree((void **)&pia->ia_ppaAsockets);
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

    if (pia->ia_psListenSocket != NULL)
        destroySocket(&(pia->ia_psListenSocket));

    finiSyncMutex(&pia->ia_smAsocket);

    xfree(ppAssocket);

    return u32Ret;
}

u32 createAssocket(
    basic_chain_t * pChain, assocket_t ** ppAssocket, assocket_param_t * pap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia;
    asocket_param_t ap;
    u32 u32Index;

    assert((pChain != NULL) && (ppAssocket != NULL) && (pap != NULL));
    assert((pap->ap_u32MaxConn != 0) &&
           (pap->ap_u32MaxConn <= ASS_MAX_CONNECTIONS) &&
           (pap->ap_fnOnConnect != NULL));

    logInfoMsg("create assocket, max conn %u", pap->ap_u32MaxConn);

    /*create a new assocket*/
    u32Ret = xcalloc((void **)&pia, sizeof(internal_assocket_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_bcohHeader.bcoh_fnPreSelect = _preSelectAssocket;
        pia->ia_bcohHeader.bcoh_fnPostSelect = _postSelectAssocket;
        pia->ia_pbaChain = pChain;

        pia->ia_fnOnConnect = pap->ap_fnOnConnect;
        pia->ia_fnOnDisconnect = pap->ap_fnOnDisconnect;
        pia->ia_fnOnSendOK = pap->ap_fnOnSendOK;
        pia->ia_fnOnData = pap->ap_fnOnData;

        pia->ia_psListenSocket = NULL;
        pia->ia_u32MaxConn = pap->ap_u32MaxConn;
        pia->ia_u16PortNumber = pap->ap_u16PortNumber;
        memcpy(&(pia->ia_iaAddr), &(pap->ap_iaAddr), sizeof(ip_addr_t));

        u32Ret = xcalloc(
            (void **)&pia->ia_ppaAsockets,
            pap->ap_u32MaxConn * sizeof(asocket_t *));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xcalloc(
            (void **)&pia->ia_plaAsocket,
            sizeOfListArray(pap->ap_u32MaxConn));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        initListArray(pia->ia_plaAsocket, pap->ap_u32MaxConn);

        u32Ret = xcalloc(
            (void **)&pia->ia_padData,
            pap->ap_u32MaxConn * sizeof(assocket_data_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&pia->ia_smAsocket);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = xcalloc(
            (void **)&pia->ia_psmAsockets,
            pap->ap_u32MaxConn * sizeof(sync_mutex_t));

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&ap, 0, sizeof(asocket_param_t));

        ap.ap_sInitialBuf = pap->ap_sInitialBuf;
        ap.ap_fnOnData = _assOnData;
        ap.ap_fnOnDisconnect = _assOnDisconnect;
        ap.ap_fnOnSendOK = _assOnSendOK;

        ap.ap_bNoRead = pap->ap_bNoRead;

        /*create our socket pool*/
        for (u32Index = 0; 
             ((u32Index < pap->ap_u32MaxConn) && (u32Ret == OLERR_NO_ERROR));
             u32Index ++)
        {
            initSyncMutex(&pia->ia_psmAsockets[u32Index]);
            ap.ap_psmLock = &pia->ia_psmAsockets[u32Index];

            u32Ret = createAsocket(
                pChain, &pia->ia_ppaAsockets[u32Index], &ap);
            if (u32Ret == OLERR_NO_ERROR)
            {
#if defined(JIUFENG_64BIT)
                setTagOfAsocket(
                    pia->ia_ppaAsockets[u32Index], (void *)(u64)u32Index);
#else
                setTagOfAsocket(
                    pia->ia_ppaAsockets[u32Index], (void *)u32Index);
#endif
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = appendToBasicChain(pChain, pia);

    if (u32Ret == OLERR_NO_ERROR)
        /*Get our listening socket*/
        u32Ret = createStreamSocket(
            &pia->ia_iaAddr, &pia->ia_u16PortNumber, &pia->ia_psListenSocket);

    if (u32Ret == OLERR_NO_ERROR)
        *ppAssocket = pia;
    else if (pia != NULL)
        destroyAssocket((assocket_t **)&pia);

    return u32Ret;
}

u16 getPortNumberOfAssocket(assocket_t * pAssocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    return pia->ia_u16PortNumber;
}

void * getTagOfAssocket(assocket_t * pAssocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    return pia->ia_pTag;
}

void setTagOfAssocket(assocket_t * pAssocket, void * pTag)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    pia->ia_pTag = pTag;
}

u32 assDisconnect(assocket_t * pAssocket, asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass disconnect, index %u", u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asDisconnect(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    acquireSyncMutex(&pia->ia_smAsocket);
    putListArrayNode(pia->ia_plaAsocket, u32Index);
    releaseSyncMutex(&pia->ia_smAsocket);

    return u32Ret;
}

boolean_t isAssocketFree(assocket_t * pAssocket, asocket_t * pAsocket)
{
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);
    boolean_t bRet;

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    bRet = isAsocketFree(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return bRet;
}

u32 assSendData(
    assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    as_mem_owner_t memowner)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass send data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asSendData(pAsocket, pu8Buffer, sBuf, memowner);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 resumeAssocket(assocket_t * pAssocket, asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("resume ass");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = resumeAsocket(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 assRecvData(
    assocket_t * pAssocket, asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psRecv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass recv data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asRecvData(pAsocket, pu8Buffer, psRecv);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 getAssocketOpt(
    assocket_t * pAssocket, asocket_t * pAsocket, olint_t level,
    olint_t optname, void * optval, olsize_t * optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("get ass socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = getAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 setAssocketOpt(
    assocket_t * pAssocket, socket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("set ass socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = setAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 assSendn(
    assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetTagOfAsocket(pAsocket);

    logInfoMsg("ass sendn");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asSendn(pAsocket, pu8Buffer, psBuf);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


