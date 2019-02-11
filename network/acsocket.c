/**
 *  @file acsocket.c
 *
 *  @brief The client server socket
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
struct internal_acsocket;

typedef struct acsocket_data
{
    struct internal_acsocket * ad_iaAcsocket;
    void * ad_pUser;
    u32 ad_u32Reserved[2];
} acsocket_data_t;

typedef struct internal_acsocket
{
    basic_chain_object_header_t ia_bcohHeader;
    basic_chain_t * ia_pbaChain;

    u16 ia_u32MaxConn;
    u16 ia_u16Reserved[3];
    u8 ia_u8Reserved[8];

    fnAcsocketOnData_t ia_fnOnData;
    fnAcsocketOnConnect_t ia_fnOnConnect;
    fnAcsocketOnDisconnect_t ia_fnOnDisconnect;
    fnAcsocketOnSendOK_t ia_fnOnSendOK;

    sync_mutex_t ia_smAsocket;
    list_array_t * ia_plaAsocket;

    sync_mutex_t * ia_psmAsockets;
    asocket_t ** ia_ppaAsockets;
    acsocket_data_t * ia_padData;

    void * ia_pTag;
} internal_acsocket_t;

#define ACS_MAX_CONNECTIONS    (100)

/* --- private routine section---------------------------------------------- */

/** Pre select handler for the chain
 *
 *  @param: pAcsocket: 
 *  @param: readset: 
 *  @param: writeset: 
 *  @param: errorset: 
 *  @param: pu32BlockTime: 
 */
static u32 _preSelectAcsocket(void * pAcsocket, fd_set * readset,
    fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;


    return u32Ret;
}

/** Post select handler for the chain
 *
 *  @param: pAcsocket: 
 *  @param: slct: 
 *  @param: readset: 
 *  @param: writeset: 
 *  @param: errorset: 
 */
static u32 _postSelectAcsocket(
    void * pAcsocket, olint_t slct, fd_set * readset,
    fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    acsocket_data_t * pad;
//    internal_acsocket_t * pia = (internal_acsocket_t *)pAcsocket;


    return u32Ret;
}

/** Internal method dispatched by the data event of the underlying asocket
 *
 *  @param: pAsocket: 
 *  @param: puBuffer: 
 *  @param: psBeginPointer: 
 *  @param: u32EndPointer: 
 *  @param: pUser: 
 *  @param: pPause: 
 */
static u32 _acsOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * pu32BeginPointer,
    olsize_t u32EndPointer, void * pUser, boolean_t * bPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    logInfoMsg("acs on data");

    /*Pass the received data up*/
    if (pia->ia_fnOnData != NULL)
    {
        pia->ia_fnOnData(
            pad->ad_iaAcsocket, pAsocket, pu8Buffer,
            pu32BeginPointer, u32EndPointer, pad->ad_pUser, bPause);
    }

    return u32Ret;
}

/** Internal method dispatched by the connect event of the underlying asocket
 *
 *  @param: pAsocket: 
 *  @param: bOK: 
 *  @param: pUser: 
 */
static u32 _acsOnConnect(asocket_t * pAsocket, boolean_t bOK, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    logInfoMsg("acs on connect");

    /*Pass the received data up*/
    if (pia->ia_fnOnConnect != NULL)
    {
        pia->ia_fnOnConnect(pia, pAsocket, bOK, pad->ad_pUser);
    }

    return u32Ret;
}

static u32 _acsGetTagOfAsocket(asocket_t * pAsocket)
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
 *  @param: pAsocket: 
 *  @param: pUser: 
 */
static u32 _acsOnDisconnect(asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("acs on disconnect, put %u", u32Index);
    acquireSyncMutex(&pia->ia_smAsocket);
    putListArrayNode(pia->ia_plaAsocket, u32Index);
    releaseSyncMutex(&pia->ia_smAsocket);

    /*Pass this Disconnect event up*/
    if (pia->ia_fnOnDisconnect != NULL)
    {
        pia->ia_fnOnDisconnect(
            pad->ad_iaAcsocket, pAsocket, u32Status, pad->ad_pUser);
    }

    return u32Ret;
}

/** Internal method dispatched by the send ok event of the underlying asocket
 *
 *  @param: pAsocket: 
 *  @param: pUser: 
 */
static u32 _acsOnSendOK(asocket_t * pAsocket, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    logInfoMsg("acs on send ok");

    /*Pass the OnSendOK event up*/
    if (pia->ia_fnOnSendOK != NULL)
    {
        pia->ia_fnOnSendOK(pad->ad_iaAcsocket, pAsocket, pad->ad_pUser);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
/** Destroy acsocket module
 *
 *  @param: pAcsocket: 
 */
u32 destroyAcsocket(acsocket_t ** ppAcsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) *ppAcsocket;
    u32 u32Index;

    logInfoMsg("destroy acsocket");

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

    finiSyncMutex(&pia->ia_smAsocket);

    xfree(ppAcsocket);

    return u32Ret;
}

u32 createAcsocket(
    basic_chain_t * pChain, acsocket_t ** ppAcsocket, acsocket_param_t * pap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) *ppAcsocket;
    asocket_param_t ap;
    u32 u32Index;

    assert((pChain != NULL) && (ppAcsocket != NULL) && (pap != NULL));
    assert((pap->ap_sInitialBuf != 0) && (pap->ap_u32MaxConn != 0) &&
           (pap->ap_u32MaxConn <= ACS_MAX_CONNECTIONS) &&
           (pap->ap_fnOnConnect != NULL) && (pia->ia_fnOnData != NULL));

    logInfoMsg("create acsocket");

    /*create a new acsocket*/
    u32Ret = xcalloc((void **)&pia, sizeof(internal_acsocket_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pia->ia_bcohHeader.bcoh_fnPreSelect = _preSelectAcsocket;
        pia->ia_bcohHeader.bcoh_fnPostSelect = _postSelectAcsocket;
        pia->ia_pbaChain = pChain;

        pia->ia_fnOnConnect = pap->ap_fnOnConnect;
        pia->ia_fnOnDisconnect = pap->ap_fnOnDisconnect;
        pia->ia_fnOnSendOK = pap->ap_fnOnSendOK;
        pia->ia_fnOnData = pap->ap_fnOnData;

        pia->ia_u32MaxConn = pap->ap_u32MaxConn;

        u32Ret = xcalloc(
            (void **)&(pia->ia_ppaAsockets),
            pap->ap_u32MaxConn * sizeof(asocket_t *));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xcalloc(
            (void **)&pia->ia_plaAsocket, sizeOfListArray(pap->ap_u32MaxConn));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        initListArray(pia->ia_plaAsocket, pap->ap_u32MaxConn);

        u32Ret = xcalloc((void **)&(pia->ia_padData),
            pap->ap_u32MaxConn * sizeof(acsocket_data_t));
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
        ap.ap_fnOnData = _acsOnData;
        ap.ap_fnOnConnect = _acsOnConnect;
        ap.ap_fnOnDisconnect = _acsOnDisconnect;
        ap.ap_fnOnSendOK = _acsOnSendOK;

        ap.ap_bNoRead = pap->ap_bNoRead;

        /*Create our socket pool*/
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
        *ppAcsocket = pia;
    else if (pia != NULL)
        destroyAcsocket((acsocket_t **)&pia);

    return u32Ret;
}

void * getTagOfAcsocket(acsocket_t * pAcsocket)
{
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;

    return pia->ia_pTag;
}

void setTagOfAcsocket(acsocket_t * pAcsocket, void * pTag)
{
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;

    pia->ia_pTag = pTag;
}

u32 acsDisconnect(acsocket_t * pAcsocket, asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("acs disconnect, index %u", u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asDisconnect(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    acquireSyncMutex(&pia->ia_smAsocket);
    putListArrayNode(pia->ia_plaAsocket, u32Index);
    releaseSyncMutex(&pia->ia_smAsocket);

    return u32Ret;
}

boolean_t isAcsocketFree(acsocket_t * pAcsocket, asocket_t * pAsocket)
{
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);
    boolean_t bRet;

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    bRet = isAsocketFree(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return bRet;
}

u32 acsSendData(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    as_mem_owner_t memowner)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("acs send data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asSendData(pAsocket, pu8Buffer, sBuf, memowner);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 resumeAcsocket(acsocket_t * pAcsocket, asocket_t * pAsocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("resume acs");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = resumeAsocket(pAsocket);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 acsRecvData(
    acsocket_t * pAcsocket, asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psRecv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("acs recv data");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asRecvData(pAsocket, pu8Buffer, psRecv);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 getAcsocketOpt(
    acsocket_t * pAcsocket, asocket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("get acs socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = getAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 setAcsocketOpt(
    acsocket_t * pAcsocket, socket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("set acs socket opt");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = setAsocketOpt(pAsocket, level, optname, optval, optlen);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 acsSendn(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetTagOfAsocket(pAsocket);

    logInfoMsg("acs sendn");

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asSendn(pAsocket, pu8Buffer, psBuf);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

u32 acsConnectTo(
    acsocket_t * pAcsocket,
    ip_addr_t * piaRemote, u16 u16RemotePortNumber, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    acsocket_data_t * pad;
    u32 u32Index;

    /*Check to see if we have available resources to handle this connection
      request*/
    acquireSyncMutex(&pia->ia_smAsocket);
    u32Index = getListArrayNode(pia->ia_plaAsocket);
    releaseSyncMutex(&pia->ia_smAsocket);

    logInfoMsg("acs connect, index %u", u32Index);

    /*Instantiate a acsocket_data_t to contain all the data about this
      connection*/
    pad = &pia->ia_padData[u32Index];
    pad->ad_iaAcsocket = pAcsocket;
    pad->ad_pUser = pUser;

    acquireSyncMutex(&pia->ia_psmAsockets[u32Index]);
    u32Ret = asConnectTo(
        pia->ia_ppaAsockets[u32Index], piaRemote, u16RemotePortNumber, pad);
    releaseSyncMutex(&pia->ia_psmAsockets[u32Index]);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


