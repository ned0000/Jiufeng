/**
 *  @file assocket.c
 *
 *  @brief Implementation file for async server asocket.
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

/** Pre-declare the internal async server socket data type.
 */
struct internal_assocket;

/** Define the private data for async socket.
 */
typedef struct assocket_data
{
    /**The internal async server socket the async socket belongs to.*/
    struct internal_assocket * ad_iaAssocket;
    /**User object.*/
    void * ad_pUser;
    u32 ad_u32Reserved[2];
} assocket_data_t;

/** Define the internal async server socket data type.
 */
typedef struct internal_assocket
{
    /**The network chain object header. MUST BE the first field.*/
    jf_network_chain_object_header_t ia_jncohHeader;
    /**The network chain.*/
    jf_network_chain_t * ia_pjncChain;

    /**Maximum connection.*/
    u32 ia_u32MaxConn;
    /**Port number of the server.*/
    u16 ia_u16PortNumber;
    /**Server is in listening mode.*/
    boolean_t ia_bListening;
    u8 ia_u8Reserved[1];
    /**IP address of the server.*/
    jf_ipaddr_t ia_jiAddr;

    /**Name of this object.*/
    olchar_t ia_strName[JF_NETWORK_MAX_NAME_LEN];

    /**Listen socket of the server.*/
    jf_network_socket_t * ia_pjnsListenSocket;

    /**Callback function for incoming data.*/
    jf_network_fnAssocketOnData_t ia_fnOnData;
    /**Callback function for connection.*/
    jf_network_fnAssocketOnConnect_t ia_fnOnConnect;
    /**Callback function for disconnection.*/
    jf_network_fnAssocketOnDisconnect_t ia_fnOnDisconnect;
    /**Callback function for sent data.*/
    jf_network_fnAssocketOnSendData_t ia_fnOnSendData;

    /*Start of lock protected section.*/
    /**Mutex lock.*/
    jf_mutex_t ia_jmAsocket;
    /**List array for free async sockets.*/
    jf_listarray_t * ia_pjlAsocket;
    /*End of lock protected section.*/

    /**Async socket array.*/
    jf_network_asocket_t ** ia_pjnaAsockets;
    /**Private data array for async sockets.*/
    assocket_data_t * ia_padData;

    /**Accessed by outside, async server socket should not touch it.*/
    void * ia_pTag;
} internal_assocket_t;

/** Maximum connections in async server socket.
 */
#define ASS_MAX_CONNECTIONS                 (4000)

/* --- private routine section ------------------------------------------------------------------ */

/** preselect handler for basic chain.
 */
static u32 _preSelectAssocket(
    void * pAssocket, fd_set * readset, fd_set * writeset, fd_set * errorset, u32 * pu32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    /*The socket isn't put in listening mode, until the chain is started. If this variable is TRUE,
      that means we need to do that.*/
    if (! pia->ia_bListening)
    {
        JF_LOGGER_INFO("name: %s, listening", pia->ia_strName);
        /*Set the socket to non-block mode.*/
        jf_network_setSocketNonblock(pia->ia_pjnsListenSocket);

        /*Put the socket in listen, and add it to the read set for the select().*/
        pia->ia_bListening = TRUE;
        jf_network_listen(pia->ia_pjnsListenSocket, pia->ia_u32MaxConn);
        jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
    }
    else
    {
        /*Only put the ia_pjnsListenSocket in the readset, if free async socket is available.*/
        if (! jf_listarray_isEnd(pia->ia_pjlAsocket))
            jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
        else
            JF_LOGGER_INFO("name: %s, no free asocket", pia->ia_strName);
    }

    return u32Ret;
}

/** Post select handler for basic chain.
 */
static u32 _postSelectAssocket(
    void * pAssocket, olint_t slct, fd_set * readset, fd_set * writeset, fd_set * errorset)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = NULL;
    internal_assocket_t * pia = (internal_assocket_t *)pAssocket;
    jf_network_socket_t * pNewSocket = NULL;
    jf_ipaddr_t ipaddr;
    u16 u16Port = 0;
    u32 u32Index = 0;

    if (jf_network_isSocketSetInFdSet(pia->ia_pjnsListenSocket, readset))
    {
        JF_LOGGER_DEBUG("name: %s, in readset", pia->ia_strName);

        /*There are pending TCP connection requests*/
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_network_accept(
                pia->ia_pjnsListenSocket, &ipaddr, &u16Port, &pNewSocket);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*Get a free async socket from pool.*/
                u32Index = jf_listarray_getNode(pia->ia_pjlAsocket);
                if (u32Index != JF_LISTARRAY_END)
                {
                    /*Success.*/
                    JF_LOGGER_DEBUG(
                        "name: %s, new connection, index: %u", pia->ia_strName, u32Index);

                    assert(isAsocketFree(pia->ia_pjnaAsockets[u32Index]));
                    /*Instantiate a private data to contain all the data about this connection.*/
                    pad = &(pia->ia_padData[u32Index]);
                    pad->ad_iaAssocket = pAssocket;
                    pad->ad_pUser = NULL;

                    /*Use the accepted socket for the async socket.*/
                    u32Ret = useSocketForAsocket(
                        pia->ia_pjnaAsockets[u32Index], pNewSocket, &ipaddr, u16Port, pad);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        /*Notify the upper layer about this new connection.*/
                        pia->ia_fnOnConnect(
                            pia, pia->ia_pjnaAsockets[u32Index], &(pad->ad_pUser));
                    }
                }
                else
                {
                    /*No free async socket.*/
                    u32Ret = JF_ERR_SOCKET_POOL_EMPTY;
                    JF_LOGGER_ERR(u32Ret, "name: %s, no more free asocket", pia->ia_strName);
                    /*Close the connection.*/
                    jf_network_destroySocket(&pNewSocket);
                    break;
                }
            }
        }
    }

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying async socket.
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
static u32 _assOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnData event up.*/
    u32Ret = pia->ia_fnOnData(
        pad->ad_iaAssocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer, pad->ad_pUser);

    return u32Ret;
}

static u32 _assGetIndexOfAsocket(jf_network_asocket_t * pAsocket)
{
    u32 u32Index;

    u32Index = getIndexOfAsocket(pAsocket);

    return u32Index;
}

/** Internal method dispatched by the OnDisconnect event of the underlying async socket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status code for the disconnection.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _assOnDisconnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;
    u32 u32Index = _assGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, index: %u", pia->ia_strName, u32Index);

    /*Pass this OnDisconnect event up.*/
    pia->ia_fnOnDisconnect(
            pad->ad_iaAssocket, pAsocket, u32Status, pad->ad_pUser);

    /*Put the async socket to free list.*/
    jf_mutex_acquire(&pia->ia_jmAsocket);
    jf_listarray_putNode(pia->ia_pjlAsocket, u32Index);
    jf_mutex_release(&pia->ia_jmAsocket);

    return u32Ret;
}

/** Internal method dispatched by the OnSendData event of the underlying async socket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status of the data transmission.
 *  @param pu8Buffer [in] The buffer containing the sent data.
 *  @param sBuf [in] The size of the buffer.
 *  @param pUser [in] The user object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _assOnSendData(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    /*Pass the OnSendData event up.*/
    u32Ret = pia->ia_fnOnSendData(
        pad->ad_iaAssocket, pAsocket, u32Status, pu8Buffer, sBuf, pad->ad_pUser);

    return u32Ret;
}

static u32 _assocketOnSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    return JF_ERR_NO_ERROR;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_destroyAssocket(jf_network_assocket_t ** ppAssocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) *ppAssocket;
    u32 u32Index;

    JF_LOGGER_INFO("name: %s", pia->ia_strName);

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

    /*Destroy the listen socket.*/
    if (pia->ia_pjnsListenSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsListenSocket));

    /*Finalize the mutex.*/
    jf_mutex_fini(&pia->ia_jmAsocket);

    /*Free memory of the async server socket.*/
    jf_jiukun_freeMemory(ppAssocket);

    return u32Ret;
}

u32 jf_network_createAssocket(
    jf_network_chain_t * pChain, jf_network_assocket_t ** ppAssocket,
    jf_network_assocket_create_param_t * pjnacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_assocket_t * pia = NULL;
    asocket_create_param_t acp;
    u32 u32Index = 0;
    olchar_t strName[JF_NETWORK_MAX_NAME_LEN];

    assert((pChain != NULL) && (ppAssocket != NULL) && (pjnacp != NULL));
    assert((pjnacp->jnacp_u32MaxConn != 0) &&
           (pjnacp->jnacp_u32MaxConn <= ASS_MAX_CONNECTIONS));
    assert((pjnacp->jnacp_fnOnConnect != NULL) && (pjnacp->jnacp_fnOnDisconnect != NULL) &&
           (pjnacp->jnacp_fnOnData != NULL));
    assert(pjnacp->jnacp_pstrName != NULL);

    JF_LOGGER_INFO("name: %s, max conn: %u", pjnacp->jnacp_pstrName, pjnacp->jnacp_u32MaxConn);

    /*Allocate memory for internal async server socket.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_assocket_t));

    /*Initialize the internal async server socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia, sizeof(internal_assocket_t));
        pia->ia_jncohHeader.jncoh_fnPreSelect = _preSelectAssocket;
        pia->ia_jncohHeader.jncoh_fnPostSelect = _postSelectAssocket;
        pia->ia_pjncChain = pChain;

        pia->ia_fnOnConnect = pjnacp->jnacp_fnOnConnect;
        pia->ia_fnOnDisconnect = pjnacp->jnacp_fnOnDisconnect;
        pia->ia_fnOnData = pjnacp->jnacp_fnOnData;
        pia->ia_fnOnSendData = pjnacp->jnacp_fnOnSendData;
        if (pia->ia_fnOnSendData == NULL)
            pia->ia_fnOnSendData = _assocketOnSendData;

        pia->ia_pjnsListenSocket = NULL;
        pia->ia_u32MaxConn = pjnacp->jnacp_u32MaxConn;
        pia->ia_u16PortNumber = pjnacp->jnacp_u16ServerPort;
        ol_memcpy(&(pia->ia_jiAddr), &(pjnacp->jnacp_jiServer), sizeof(jf_ipaddr_t));
        ol_snprintf(
            pia->ia_strName, sizeof(pia->ia_strName) - 1, "%s-ass", pjnacp->jnacp_pstrName);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&pia->ia_pjnaAsockets,
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
            (void **)&pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(assocket_data_t));
    }

    /*Initialize the mutex.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(assocket_data_t));

        u32Ret = jf_mutex_init(&pia->ia_jmAsocket);
    }

    /*Create async socket pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&acp, sizeof(acp));

        acp.acp_sInitialBuf = pjnacp->jnacp_sInitialBuf;
        acp.acp_fnOnData = _assOnData;
        acp.acp_fnOnDisconnect = _assOnDisconnect;
        acp.acp_fnOnSendData = _assOnSendData;
        strName[JF_NETWORK_MAX_NAME_LEN - 1] = '\0';
        acp.acp_pstrName = strName;

        /*Create async socket for each connection.*/
        for (u32Index = 0; 
             ((u32Index < pjnacp->jnacp_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            /*Generate the name of the async socket.*/
            ol_snprintf(
                strName, sizeof(strName) - 1, "%s-as-%d", pia->ia_strName, u32Index);

            /*Create async socket.*/
            u32Ret = createAsocket(pChain, &pia->ia_pjnaAsockets[u32Index], &acp);

            /*Set index of the async socket.*/
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                setIndexOfAsocket(pia->ia_pjnaAsockets[u32Index], u32Index);
            }
        }
    }

    /*Add the async server socket to chain.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_appendToChain(pChain, pia);

    /*Create listening socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("name: %s, create listening socket", pjnacp->jnacp_pstrName);

        u32Ret = jf_network_createStreamSocket(
            &pia->ia_jiAddr, &pia->ia_u16PortNumber, &pia->ia_pjnsListenSocket);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
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
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;
    u32 u32Index = _assGetIndexOfAsocket(pAsocket);

    JF_LOGGER_DEBUG("name: %s, index: %u", pia->ia_strName, u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    u32Ret = disconnectAsocket(pAsocket);

    return u32Ret;
}

u32 jf_network_sendAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_assocket_t * pia = (internal_assocket_t *) pAssocket;

    JF_LOGGER_DEBUG("name: %s", pia->ia_strName);

    u32Ret = sendAsocketData(pAsocket, pu8Buffer, sBuf);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
