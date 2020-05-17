/**
 *  @file acsocket.c
 *
 *  @brief The client asocket
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
#include "jf_listarray.h"

#include "asocket.h"

/* --- private data/data structure section ------------------------------------------------------ */

struct internal_acsocket;

typedef struct acsocket_data
{
    struct internal_acsocket * ad_iaAcsocket;
    void * ad_pUser;
    u32 ad_u32Reserved[2];
} acsocket_data_t;

typedef struct internal_acsocket
{
    jf_network_chain_object_header_t ia_jncohHeader;
    jf_network_chain_t * ia_pjncChain;

    olchar_t ia_strName[JF_NETWORK_MAX_NAME_LEN];

    u16 ia_u32MaxConn;
    u16 ia_u16Reserved[3];

    u8 ia_u8Reserved[8];

    jf_network_fnAcsocketOnData_t ia_fnOnData;
    jf_network_fnAcsocketOnConnect_t ia_fnOnConnect;
    jf_network_fnAcsocketOnDisconnect_t ia_fnOnDisconnect;
    jf_network_fnAcsocketOnSendData_t ia_fnOnSendData;

    jf_mutex_t ia_jmAsocket;
    jf_listarray_t * ia_pjlAsocket;

    jf_network_asocket_t ** ia_pjnaAsockets;
    acsocket_data_t * ia_padData;

    void * ia_pTag;
} internal_acsocket_t;

/** Maximum connections in async client socket.
 */
#define ACSOCKET_MAX_CONNECTIONS    (100)

/* --- private routine section ------------------------------------------------------------------ */

/** Pre select handler for the chain
 *
 *  @param pAcsocket [in] the async client socket 
 *  @param readset [out] the read fd set
 *  @param writeset [out] the write fd set
 *  @param errorset [out] the error fd set
 *  @param pu32BlockTime [out] the block time in millisecond
 *
 *  @return the error code
 */
static u32 _preSelectAcsocket(
    void * pAcsocket, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;


    return u32Ret;
}

/** Post select handler for the chain
 *
 *  @param pAcsocket [in] the async client socket
 *  @param slct [in] number of ready socket 
 *  @param readset [in] the read fd set
 *  @param writeset [in] the write fd set
 *  @param errorset [in] the error fd set
 *
 *  @return the error code
 */
static u32 _postSelectAcsocket(
    void * pAcsocket, olint_t slct, fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    acsocket_data_t * pad;
//    internal_acsocket_t * pia = (internal_acsocket_t *)pAcsocket;


    return u32Ret;
}

/** Internal method dispatched by the data event of the underlying asocket
 *
 *  @param pAsocket [in] the async socket
 *  @param pu8Buffer [in] the buffer
 *  @param psBeginPointer [in/out] the beging pointer of the data 
 *  @param sEndPointer [in] the end pointer of the data 
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
static u32 _acsOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the received data up*/
    if (pia->ia_fnOnData != NULL)
    {
        pia->ia_fnOnData(
            pad->ad_iaAcsocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer, pad->ad_pUser);
    }

    return u32Ret;
}

/** Internal method dispatched by the connect event of the underlying asocket
 *
 *  @param pAsocket [in] the async socket 
 *  @param u32Status [in] the connection status
 *  @param pUser [in] the user
 *
 *  @return the error code
 */
static u32 _acsOnConnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the received data up*/
    u32Ret = pia->ia_fnOnConnect(pia, pAsocket, u32Status, pad->ad_pUser);

    return u32Ret;
}

static u32 _acsGetIndexOfAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Index;

    u32Index = getIndexOfAsocket(pAsocket);

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
static u32 _acsOnDisconnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;
    u32 u32Index = _acsGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, put %u", pia->ia_strName, u32Index);

    jf_mutex_acquire(&pia->ia_jmAsocket);
    jf_listarray_putNode(pia->ia_pjlAsocket, u32Index);
    jf_mutex_release(&pia->ia_jmAsocket);

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
 *  @param pAsocket [in] the async socket
 *  @param u32Status [in] the status of data transmission
 *  @param pu8Buffer [in] the receive buffer
 *  @param sBuf [in] the size of the buffer
 *  @param pUser [in] the associated webclient data object
 *
 *  @return the error code
 */
static u32 _acsOnSendData(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnSendOK event up*/
    pia->ia_fnOnSendData(pad->ad_iaAcsocket, pAsocket, u32Status, pu8Buffer, sBuf, pad->ad_pUser);

    return u32Ret;
}

static u32 _acsocketOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_destroyAcsocket(jf_network_acsocket_t ** ppAcsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) *ppAcsocket;
    u32 u32Index;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    if (pia->ia_pjnaAsockets != NULL)
    {
        for (u32Index = 0;
             ((u32Index < pia->ia_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            if (pia->ia_pjnaAsockets[u32Index] != NULL)
                destroyAsocket(&pia->ia_pjnaAsockets[u32Index]);
        }

        jf_jiukun_freeMemory((void **)&pia->ia_pjnaAsockets);
    }

    if (pia->ia_pjlAsocket != NULL)
        jf_jiukun_freeMemory((void **)&pia->ia_pjlAsocket);

    if (pia->ia_padData != NULL)
        jf_jiukun_freeMemory((void **)&pia->ia_padData);

    jf_mutex_fini(&pia->ia_jmAsocket);

    jf_jiukun_freeMemory(ppAcsocket);

    return u32Ret;
}

u32 jf_network_createAcsocket(
    jf_network_chain_t * pChain, jf_network_acsocket_t ** ppAcsocket,
    jf_network_acsocket_create_param_t * pjnacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) *ppAcsocket;
    asocket_create_param_t acp;
    u32 u32Index;
    olchar_t strName[JF_NETWORK_MAX_NAME_LEN];

    assert((pChain != NULL) && (ppAcsocket != NULL) && (pjnacp != NULL));
    assert((pjnacp->jnacp_sInitialBuf != 0) && (pjnacp->jnacp_u32MaxConn != 0) &&
           (pjnacp->jnacp_u32MaxConn <= ACSOCKET_MAX_CONNECTIONS));
    assert((pjnacp->jnacp_fnOnConnect != NULL) && (pjnacp->jnacp_fnOnData != NULL) &&
           (pjnacp->jnacp_fnOnDisconnect != NULL));

    JF_LOGGER_DEBUG("name: %s", pjnacp->jnacp_pstrName);

    /*create a new acsocket*/
    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_acsocket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia, sizeof(internal_acsocket_t));
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAcsocket;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAcsocket;
        pia->ia_pjncChain = pChain;

        pia->ia_fnOnConnect = pjnacp->jnacp_fnOnConnect;
        pia->ia_fnOnDisconnect = pjnacp->jnacp_fnOnDisconnect;
        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;
        pia->ia_fnOnSendData = pjnacp->jnacp_fnOnSendData;
        if (pia->ia_fnOnSendData == NULL)
            pia->ia_fnOnSendData = _acsocketOnSendData;

        pia->ia_u32MaxConn = pjnacp->jnacp_u32MaxConn;
        ol_strncpy(pia->ia_strName, pjnacp->jnacp_pstrName, JF_NETWORK_MAX_NAME_LEN - 1);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&(pia->ia_pjnaAsockets),
            pjnacp->jnacp_u32MaxConn * sizeof(jf_network_asocket_t *));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_pjnaAsockets, pjnacp->jnacp_u32MaxConn * sizeof(jf_network_asocket_t *));

        u32Ret = jf_jiukun_allocMemory(
            (void **)&pia->ia_pjlAsocket, jf_listarray_getSize(pjnacp->jnacp_u32MaxConn));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_pjlAsocket, jf_listarray_getSize(pjnacp->jnacp_u32MaxConn));
        jf_listarray_init(pia->ia_pjlAsocket, pjnacp->jnacp_u32MaxConn);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&(pia->ia_padData), pjnacp->jnacp_u32MaxConn * sizeof(acsocket_data_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(acsocket_data_t));

        u32Ret = jf_mutex_init(&pia->ia_jmAsocket);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&acp, 0, sizeof(acp));

        acp.acp_sInitialBuf = pjnacp->jnacp_sInitialBuf;
        acp.acp_fnOnData = _acsOnData;
        acp.acp_fnOnConnect = _acsOnConnect;
        acp.acp_fnOnDisconnect = _acsOnDisconnect;
        acp.acp_fnOnSendData = _acsOnSendData;
        strName[JF_NETWORK_MAX_NAME_LEN - 1] = '\0';
        acp.acp_pstrName = strName;

        /*Create our socket pool*/
        for (u32Index = 0; 
             ((u32Index < pjnacp->jnacp_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            ol_snprintf(
                strName, JF_NETWORK_MAX_NAME_LEN - 1, "%s-%d", pjnacp->jnacp_pstrName,
                u32Index + 1);

            u32Ret = createAsocket(pChain, &pia->ia_pjnaAsockets[u32Index], &acp);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                setIndexOfAsocket(pia->ia_pjnaAsockets[u32Index], u32Index);
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pChain, pia);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppAcsocket = pia;
    else if (pia != NULL)
        jf_network_destroyAcsocket((jf_network_acsocket_t **)&pia);

    return u32Ret;
}

void * jf_network_getTagOfAcsocket(jf_network_acsocket_t * pAcsocket)
{
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;

    return pia->ia_pTag;
}

void jf_network_setTagOfAcsocket(jf_network_acsocket_t * pAcsocket, void * pTag)
{
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;

    pia->ia_pTag = pTag;
}

u32 jf_network_disconnectAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, index %u", pia->ia_strName, u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    u32Ret = disconnectAsocket(pAsocket);

    return u32Ret;
}

u32 jf_network_sendAcsocketData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    u32 u32Index = _acsGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, index: %u", pia->ia_strName, u32Index);

    u32Ret = sendAsocketData(pAsocket, pu8Buffer, sBuf);

    return u32Ret;
}

u32 jf_network_connectAcsocketTo(
    jf_network_acsocket_t * pAcsocket, jf_ipaddr_t * pjiRemote, u16 u16RemotePort, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;
    acsocket_data_t * pad;
    u32 u32Index;

    /*Check to see if we have available resources to handle this connection request*/
    jf_mutex_acquire(&pia->ia_jmAsocket);
    u32Index = jf_listarray_getNode(pia->ia_pjlAsocket);
    jf_mutex_release(&pia->ia_jmAsocket);

    JF_LOGGER_DEBUG("name: %s, index %u", pia->ia_strName, u32Index);

    if (u32Index == JF_LISTARRAY_END)
        u32Ret = JF_ERR_SOCKET_POOL_EMPTY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Instantiate a acsocket_data_t to contain all the data about this connection*/
        pad = &pia->ia_padData[u32Index];
        pad->ad_iaAcsocket = pAcsocket;
        pad->ad_pUser = pUser;

        u32Ret = connectAsocketTo(
            pia->ia_pjnaAsockets[u32Index], pjiRemote, u16RemotePort, pad);
    }

    return u32Ret;
}

void jf_network_getLocalInterfaceOfAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr)
{
    getLocalInterfaceOfAsocket(pAsocket, pjiAddr);
}

/*------------------------------------------------------------------------------------------------*/


