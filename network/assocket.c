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

    u32 ia_u32MaxConn;
    u16 ia_u16PortNumber;
    boolean_t ia_bListening;
    u8 ia_u8Reserved[1];
    jf_ipaddr_t ia_jiAddr;

    olchar_t ia_strName[JF_NETWORK_MAX_NAME_LEN];

    jf_network_socket_t * ia_pjnsListenSocket;

    jf_network_fnAssocketOnData_t ia_fnOnData;
    jf_network_fnAssocketOnConnect_t ia_fnOnConnect;
    jf_network_fnAssocketOnDisconnect_t ia_fnOnDisconnect;
    jf_network_fnAssocketOnSendData_t ia_fnOnSendData;

    /*start of lock protected section*/
    jf_mutex_t ia_jmAsocket;
    jf_listarray_t * ia_pjlAsocket;
    /*end of lock protected section*/

    jf_network_asocket_t ** ia_pjnaAsockets;
    assocket_data_t * ia_padData;

    void * ia_pTag;
} internal_assocket_t;

#define ASS_MAX_CONNECTIONS   4000

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
        jf_logger_logInfoMsg("pre sel ass, listening on the socket %s", pia->ia_strName);
        /*Set the socket to non-block mode, so we can play nice and share the thread*/
        jf_network_setSocketNonblock(pia->ia_pjnsListenSocket);

        /*Put the socket in listen, and add it to the fdset for the select*/
        pia->ia_bListening = TRUE;
        jf_network_listen(pia->ia_pjnsListenSocket, pia->ia_u32MaxConn);
        jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
    }
    else
    {
        /*Only put the ia_pjnsListenSocket in the readset, if we are able to handle a new socket*/
        if (! jf_listarray_isEnd(pia->ia_pjlAsocket))
            jf_network_setSocketToFdSet(pia->ia_pjnsListenSocket, readset);
        else
            jf_logger_logInfoMsg("pre sel ass, no free asocket on the socket");
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
        jf_logger_logInfoMsg(
            "post select assocket, listen socket %s is in readset", pia->ia_strName);

        /*There are pending TCP connection requests*/
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_network_accept(
                pia->ia_pjnsListenSocket, &ipaddr, &u16Port, &pNewSocket);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*Check to see if we have available resources to handle
                  this connection request*/
                u32Index = jf_listarray_getNode(pia->ia_pjlAsocket);
                if (u32Index != JF_LISTARRAY_END)
                {
                    jf_logger_logInfoMsg(
                        "post select assocket, new connection, use %u", u32Index);

                    assert(isAsocketFree(pia->ia_pjnaAsockets[u32Index]));
                    /*Instantiate a pia to contain all the data about
                      this connection*/
                    pad = &(pia->ia_padData[u32Index]);
                    pad->ad_iaAssocket = pAssocket;
                    pad->ad_pUser = NULL;

                    u32Ret = useSocketForAsocket(
                        pia->ia_pjnaAsockets[u32Index], pNewSocket, &ipaddr, u16Port, pad);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        /*Notify the user about this new connection*/
                        pia->ia_fnOnConnect(
                            pia, pia->ia_pjnaAsockets[u32Index], &(pad->ad_pUser));
                    }
                }
                else
                {
                    u32Ret = JF_ERR_SOCKET_POOL_EMPTY;
                    jf_logger_logErrMsg(
                        u32Ret, "post select assocket, no more free asocket");
                    jf_network_destroySocket(&pNewSocket);
                    break;
                }
            }
        }
    }

    return u32Ret;
}

/** Internal method dispatched by the OnData event of the underlying asocket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param pu8Buffer [in] The buffer.
 *  @param psBeginPointer [in/out] The beging pointer of the data.
 *  @param sEndPointer [in] The end pointer of the data.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
static u32 _assOnData(
    void * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    jf_logger_logInfoMsg("ass on data");

    /*Pass the received data up.*/
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

/** Internal method dispatched by the disconnect event of the underlying asocket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status code for the disconnection.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
static u32 _assOnDisconnect(jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;
    u32 u32Index = _assGetIndexOfAsocket(pAsocket);

    jf_logger_logInfoMsg("ass on disconnect, put %u", u32Index);

    /*Pass this Disconnect event up*/
    pia->ia_fnOnDisconnect(
            pad->ad_iaAssocket, pAsocket, u32Status, pad->ad_pUser);

    jf_mutex_acquire(&pia->ia_jmAsocket);
    jf_listarray_putNode(pia->ia_pjlAsocket, u32Index);
    jf_mutex_release(&pia->ia_jmAsocket);

    return u32Ret;
}

/** Internal method dispatched by the OnSendOK event of the underlying asocket.
 *
 *  @param pAsocket [in] The async socket.
 *  @param u32Status [in] The status of the data transmission.
 *  @param pu8Buffer [in] The buffer containing the sent data.
 *  @param sBuf [in] The size of the buffer.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
static u32 _assOnSendData(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    assocket_data_t * pad = (assocket_data_t *) pUser;
    internal_assocket_t * pia = pad->ad_iaAssocket;

    jf_logger_logInfoMsg("ass on send ok");

    /*Pass the OnSendOK event up*/
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

    jf_logger_logInfoMsg("destroy assocket");

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

    if (pia->ia_pjnsListenSocket != NULL)
        jf_network_destroySocket(&(pia->ia_pjnsListenSocket));

    jf_mutex_fini(&pia->ia_jmAsocket);

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
    u32 u32Index;
    olchar_t strName[JF_NETWORK_MAX_NAME_LEN];

    assert((pChain != NULL) && (ppAssocket != NULL) && (pjnacp != NULL));
    assert((pjnacp->jnacp_u32MaxConn != 0) &&
           (pjnacp->jnacp_u32MaxConn <= ASS_MAX_CONNECTIONS));
    assert((pjnacp->jnacp_fnOnConnect != NULL) && (pjnacp->jnacp_fnOnDisconnect != NULL) &&
           (pjnacp->jnacp_fnOnData != NULL));

    jf_logger_logInfoMsg(
        "create assocket %s, max conn %u", pjnacp->jnacp_pstrName, pjnacp->jnacp_u32MaxConn);

    /*Allocate memory for async server socket.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pia, sizeof(internal_assocket_t));
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
        ol_strncpy(pia->ia_strName, pjnacp->jnacp_pstrName, JF_NETWORK_MAX_NAME_LEN - 1);

        u32Ret = jf_jiukun_allocMemory(
            (void **)&pia->ia_pjnaAsockets,
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
            (void **)&pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(assocket_data_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pia->ia_padData, pjnacp->jnacp_u32MaxConn * sizeof(assocket_data_t));

        u32Ret = jf_mutex_init(&pia->ia_jmAsocket);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&acp, sizeof(acp));

        acp.acp_sInitialBuf = pjnacp->jnacp_sInitialBuf;
        acp.acp_fnOnData = _assOnData;
        acp.acp_fnOnDisconnect = _assOnDisconnect;
        acp.acp_fnOnSendData = _assOnSendData;
        strName[JF_NETWORK_MAX_NAME_LEN - 1] = '\0';
        acp.acp_pstrName = strName;

        /*Create async socket pool.*/
        for (u32Index = 0; 
             ((u32Index < pjnacp->jnacp_u32MaxConn) && (u32Ret == JF_ERR_NO_ERROR));
             u32Index ++)
        {
            ol_snprintf(
                strName, JF_NETWORK_MAX_NAME_LEN - 1, "%s-%d", pjnacp->jnacp_pstrName, u32Index);

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
    {
        jf_logger_logDebugMsg("create listening socket for assocket %s", pjnacp->jnacp_pstrName);
        /*Create listening socket.*/
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

    jf_logger_logInfoMsg("ass disconnect, index %u", u32Index);
    assert(u32Index < pia->ia_u32MaxConn);

    u32Ret = disconnectAsocket(pAsocket);

    return u32Ret;
}

u32 jf_network_sendAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("ass send data");

    u32Ret = sendAsocketData(pAsocket, pu8Buffer, sBuf);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

