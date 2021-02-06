/**
 *  @file acsocket.c
 *
 *  @brief Implementation file for async client asocket.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_network.h"
#include "jf_mutex.h"
#include "jf_jiukun.h"
#include "jf_listarray.h"

#include "asocket.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Pre-declare the internal async client socket data type.
 */
struct internal_acsocket;

/** Define the private data for async socket.
 */
typedef struct acsocket_data
{
    /**The internal async client socket the async socket belongs to.*/
    struct internal_acsocket * ad_iaAcsocket;
    /**User object.*/
    void * ad_pUser;
    u32 ad_u32Reserved[2];
} acsocket_data_t;

/** Define the internal async client socket data type.
 */
typedef struct internal_acsocket
{
    /**The network chain object header. MUST BE the first field.*/
    jf_network_chain_object_header_t ia_jncohHeader;
    /**The network chain.*/
    jf_network_chain_t * ia_pjncChain;

    /**Name of this object.*/
    olchar_t ia_strName[JF_NETWORK_MAX_NAME_LEN];

    /**Maximum connection.*/
    u16 ia_u32MaxConn;
    u16 ia_u16Reserved[3];

    u8 ia_u8Reserved[8];

    /**Callback function for incoming data.*/
    jf_network_fnAcsocketOnData_t ia_fnOnData;
    /**Callback function for connection.*/
    jf_network_fnAcsocketOnConnect_t ia_fnOnConnect;
    /**Callback function for disconnection.*/
    jf_network_fnAcsocketOnDisconnect_t ia_fnOnDisconnect;
    /**Callback function for sent data.*/
    jf_network_fnAcsocketOnSendData_t ia_fnOnSendData;

    /*Start of lock protected section.*/
    /**Mutex lock.*/
    jf_mutex_t ia_jmAsocket;
    /**List array for free async sockets.*/
    jf_listarray_t * ia_pjlAsocket;
    /*End of lock protected section.*/

    /**Async socket array.*/
    jf_network_asocket_t ** ia_pjnaAsockets;
    /**Private data array for async sockets.*/
    acsocket_data_t * ia_padData;

    /**Accessed by outside, async client socket should not touch it.*/
    void * ia_pTag;
} internal_acsocket_t;

/** Maximum connections in async client socket.
 */
#define ACSOCKET_MAX_CONNECTIONS    (100)

/* --- private routine section ------------------------------------------------------------------ */

/** Pre select handler for the chain
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param readset [out] The read set of socket.
 *  @param writeset [out] The write set of socket.
 *  @param errorset [out] The error set of socket.
 *  @param pu32BlockTime [out] The block time in millisecond.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _preSelectAcsocket(
    void * pAcsocket, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_acsocket_t * pia = (internal_acsocket_t *) pAcsocket;


    return u32Ret;
}

/** Post select handler for the chain.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param slct [in] Number of ready socket.
 *  @param readset [in] The read set of socket.
 *  @param writeset [in] The write set of socket.
 *  @param errorset [in] The error set of socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _postSelectAcsocket(
    void * pAcsocket, olint_t slct, fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    acsocket_data_t * pad;
//    internal_acsocket_t * pia = (internal_acsocket_t *)pAcsocket;


    return u32Ret;
}

/** Internal method dispatched by the data event of the underlying async socket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param pu8Buffer [in] The buffer.
 *  @param psBeginPointer [in/out] The beging pointer of the data.
 *  @param sEndPointer [in] The end pointer of the data.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _acsOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnData event up.*/
    if (pia->ia_fnOnData != NULL)
    {
        pia->ia_fnOnData(
            pad->ad_iaAcsocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer, pad->ad_pUser);
    }

    return u32Ret;
}

/** Internal method dispatched by the OnConnect event of the underlying async socket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The connection status.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _acsOnConnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnConnect event up*/
    u32Ret = pia->ia_fnOnConnect(pia, pAsocket, u32Status, pad->ad_pUser);

    return u32Ret;
}

static u32 _acsGetIndexOfAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Index;

    u32Index = getIndexOfAsocket(pAsocket);

    return u32Index;
}

/** Internal method dispatched by the OnDisconnect event of the underlying async socket.
 *
 *  @param pAsocket [in] The async sync socket.
 *  @param u32Status [in] The status code for the disconnection.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _acsOnDisconnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;
    u32 u32Index = _acsGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, put %u", pia->ia_strName, u32Index);

    /*Put the async socket to free list.*/
    jf_mutex_acquire(&pia->ia_jmAsocket);
    jf_listarray_putNode(pia->ia_pjlAsocket, u32Index);
    jf_mutex_release(&pia->ia_jmAsocket);

    /*Pass this OnDisconnect event up.*/
    if (pia->ia_fnOnDisconnect != NULL)
    {
        pia->ia_fnOnDisconnect(
            pad->ad_iaAcsocket, pAsocket, u32Status, pad->ad_pUser);
    }

    return u32Ret;
}

/** Internal method dispatched by the OnSendData event of the underlying async socket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status of data transmission.
 *  @param pu8Buffer [in] The receive buffer.
 *  @param sBuf [in] The size of the buffer.
 *  @param pUser [in] The associated user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _acsOnSendData(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    acsocket_data_t * pad = (acsocket_data_t *) pUser;
    internal_acsocket_t * pia = pad->ad_iaAcsocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnSendData event up.*/
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
        /*Iterate the async socket array.*/
        for (u32Index = 0;
             ((u32Index < pia->ia_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            /*Destroy the async socket.*/
            if (pia->ia_pjnaAsockets[u32Index] != NULL)
                destroyAsocket(&pia->ia_pjnaAsockets[u32Index]);
        }

        /*Free memory of the async socket array.*/
        jf_jiukun_freeMemory((void **)&pia->ia_pjnaAsockets);
    }

    /*Free memory of the free async socket list array.*/
    if (pia->ia_pjlAsocket != NULL)
        jf_jiukun_freeMemory((void **)&pia->ia_pjlAsocket);

    /*Free memory of the private data.*/
    if (pia->ia_padData != NULL)
        jf_jiukun_freeMemory((void **)&pia->ia_padData);

    /*Finalize the mutex.*/
    jf_mutex_fini(&pia->ia_jmAsocket);

    /*Free memory of the async client socket.*/
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
    u32 u32Index = 0;
    olchar_t strName[JF_NETWORK_MAX_NAME_LEN];

    assert((pChain != NULL) && (ppAcsocket != NULL) && (pjnacp != NULL));
    assert((pjnacp->jnacp_sInitialBuf != 0) && (pjnacp->jnacp_u32MaxConn != 0) &&
           (pjnacp->jnacp_u32MaxConn <= ACSOCKET_MAX_CONNECTIONS));
    assert((pjnacp->jnacp_fnOnConnect != NULL) && (pjnacp->jnacp_fnOnData != NULL) &&
           (pjnacp->jnacp_fnOnDisconnect != NULL));

    JF_LOGGER_DEBUG("name: %s", pjnacp->jnacp_pstrName);

    /*Allocate memory for internal async client socket.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_acsocket_t));

    /*Initialize the internal async client socket.*/
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

        /*Allocate memory for the connections.*/
        u32Ret = jf_jiukun_allocMemory(
            (void **)&(pia->ia_pjnaAsockets),
            pjnacp->jnacp_u32MaxConn * sizeof(jf_network_asocket_t *));
    }

    /*Allocate memory for the free async socket list array.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_pjnaAsockets, pjnacp->jnacp_u32MaxConn * sizeof(jf_network_asocket_t *));

        u32Ret = jf_jiukun_allocMemory(
            (void **)&pia->ia_pjlAsocket, jf_listarray_getSize(pjnacp->jnacp_u32MaxConn));
    }

    /*Allocate memory of private data for each connection.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_pjlAsocket, jf_listarray_getSize(pjnacp->jnacp_u32MaxConn));
        jf_listarray_init(pia->ia_pjlAsocket, pjnacp->jnacp_u32MaxConn);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&(pia->ia_padData), pjnacp->jnacp_u32MaxConn * sizeof(acsocket_data_t));
    }

    /*Initialize the mutex.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(acsocket_data_t));

        u32Ret = jf_mutex_init(&pia->ia_jmAsocket);
    }

    /*Create async socket pool.*/
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

        /*Create one async socket for each connection.*/
        for (u32Index = 0; 
             ((u32Index < pjnacp->jnacp_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            /*Generate the name of async socket.*/
            ol_snprintf(
                strName, JF_NETWORK_MAX_NAME_LEN - 1, "%s-%d", pjnacp->jnacp_pstrName,
                u32Index + 1);

            /*Create async socket.*/
            u32Ret = createAsocket(pChain, &pia->ia_pjnaAsockets[u32Index], &acp);

            /*Set index of the async socket.*/
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                setIndexOfAsocket(pia->ia_pjnaAsockets[u32Index], u32Index);
            }
        }
    }

    /*Add the async client socket to chain.*/
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

    /*Get free async socket from pool.*/
    jf_mutex_acquire(&pia->ia_jmAsocket);
    u32Index = jf_listarray_getNode(pia->ia_pjlAsocket);
    jf_mutex_release(&pia->ia_jmAsocket);

    JF_LOGGER_DEBUG("name: %s, index %u", pia->ia_strName, u32Index);

    /*No async socket in the free list if the index is the end.*/
    if (u32Index == JF_LISTARRAY_END)
        u32Ret = JF_ERR_SOCKET_POOL_EMPTY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Instantiate a private data to contain all the data about this connection.*/
        pad = &pia->ia_padData[u32Index];
        pad->ad_iaAcsocket = pAcsocket;
        pad->ad_pUser = pUser;

        /*Make the connection.*/
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
