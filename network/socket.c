/**
 *  @file socket.c
 *
 *  @brief routines for socket 
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(LINUX)
    #include <sys/param.h>
    #include <sys/socket.h>
    #include <sys/file.h>
    #include <sys/time.h>
    #include <sys/times.h>
    #include <sys/signal.h>

    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#elif defined(WINDOWS)
    #include <Winsock2.h>
    #include <Ws2tcpip.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_network.h"
#include "jf_mem.h"
#include "internalsocket.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */


/* --- public routine section ---------------------------------------------- */

u32 jf_network_createDgramSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pjiLocal != NULL) && (pu16Port != NULL));
    assert(ppSocket != NULL);

    u32Ret = createDgramIsocket(pjiLocal, pu16Port,
        (internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_createStreamSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pjiLocal != NULL) && (pu16Port != NULL) && (ppSocket != NULL));

    u32Ret = createStreamIsocket(pjiLocal, pu16Port,
        (internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_createTypeStreamSocket(
    u8 u8AddrType, jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(ppSocket != NULL);

    if (u8AddrType == JF_IPADDR_TYPE_V6)
        u32Ret = createIsocket(
            AF_INET6, SOCK_STREAM, 0, (internal_socket_t **)ppSocket);
    else
        u32Ret = createIsocket(
            AF_INET, SOCK_STREAM, 0, (internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_createSocket(
    olint_t domain, olint_t type, olint_t protocol,
    jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(ppSocket != NULL);

    u32Ret = createIsocket(
        domain, type, protocol, (internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_destroySocket(jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((ppSocket != NULL) && (*ppSocket != NULL));

    destroyIsocket((internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_ioctlSocket(
    jf_network_socket_t * pSocket, olint_t req, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = ioctlIsocket(pis, req, pArg);

    return u32Ret;
}

u32 jf_network_setSocketBlock(jf_network_socket_t * pSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = setIsocketBlock(pis);

    return u32Ret;
}

u32 jf_network_setSocketNonblock(jf_network_socket_t * pSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = setIsocketNonblock(pis);

    return u32Ret;
}

u32 jf_network_joinMulticastGroup(
    jf_network_socket_t * pSocket, jf_ipaddr_t * pjiAddr,
    jf_ipaddr_t * pjiMulticaseAddr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);
    assert((pjiAddr != NULL) && (pjiMulticaseAddr != NULL));

    u32Ret = isJoinMulticastGroup(pis, pjiAddr, pjiMulticaseAddr);

    return u32Ret;
}

u32 jf_network_enableBroadcast(jf_network_socket_t * pSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isEnableBroadcast(pis);

    return u32Ret;
}

/** Try to send all data but only send once, the actual sent size is in psSend
 */
u32 jf_network_send(jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isSend(pis, pBuffer, psSend);

    return u32Ret;
}

/** Try to send all data but only send once, unless timeout the actual sent size
 *  is in psSend
 */
u32 jf_network_sendWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isSendWithTimeout(pis, pBuffer, psSend, u32Timeout);

    return u32Ret;
}

/** Try to send all data with possible several round, until an error occurs, the
 *  actual sent size is in psSend
 */
u32 jf_network_sendn(jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isSendn(pis, pBuffer, psSend);

    return u32Ret;
}

/** Try to send all data with possible several round, until an error occurs or
 *  timeout, the actual sent size is in psSend
 */
u32 jf_network_sendnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isSendnWithTimeout(pis, pBuffer, psSend, u32Timeout);

    return u32Ret;
}

/** Try to recveive all data but only recveive once, the actual received size is
 *  in psRecv
 */
u32 jf_network_recv(jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecv(pis, pBuffer, psRecv);

    return u32Ret;
}

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
u32 jf_network_recvWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecvWithTimeout(pis, pBuffer, psRecv, u32Timeout);

    return u32Ret;
}

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
u32 jf_network_recvfromWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout, jf_ipaddr_t * pjiFrom, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecvfromWithTimeout(
        pis, pBuffer, psRecv, u32Timeout, pjiFrom, pu16Port);

    return u32Ret;
}

/** Try to recveive all data with possible several round, until an error occurs,
 *  the actual recveived size is in psRecv
 */
u32 jf_network_recvn(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecvn(pis, pBuffer, psRecv);

    return u32Ret;
}

/** Try to recveive all data with possible several round, until an error occurs
 *  or timeout, the actual recveived size is in psRecv
 */
u32 jf_network_recvnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecvnWithTimeout(pis, pBuffer, psRecv, u32Timeout);

    return u32Ret;
}

u32 jf_network_listen(jf_network_socket_t * pListen, olint_t backlog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pListen;

    assert(pListen != NULL);

    u32Ret = isListen(pis, backlog);

    return u32Ret;
}

u32 jf_network_connect(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isConnect(pis, pji, u16Port);

    return u32Ret;
}

u32 jf_network_connectWithTimeout(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isConnectWithTimeout(pis, pji, u16Port, u32Timeout);

    return u32Ret;
}

u32 jf_network_accept(
    jf_network_socket_t * pListen, jf_ipaddr_t * pji, u16 * pu16Port,
    jf_network_socket_t ** ppSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pListen;

    assert((pListen != NULL) && (ppSocket != NULL));

    u32Ret = isAccept(pis, pji, pu16Port, (internal_socket_t **)ppSocket);

    return u32Ret;
}

u32 jf_network_sendto(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend,
    const jf_ipaddr_t * pjiTo, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isSendto(pis, pBuffer, psSend, pjiTo, u16Port);

    return u32Ret;
}

u32 jf_network_recvfrom(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    jf_ipaddr_t * pjiFrom, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = isRecvfrom(pis, pBuffer, psRecv, pjiFrom, pu16Port);

    return u32Ret;
}

u32 jf_network_select(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
    struct timeval * timeout, u32 * pu32Ready)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = isSelect(readfds, writefds, exceptfds, timeout, pu32Ready);

    return u32Ret;
}

u32 jf_network_getSocketName(
    jf_network_socket_t * pSocket, struct sockaddr * pName, olint_t * pnNameLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    u32Ret = getIsocketName(pis, pName, pnNameLen);

    return u32Ret;
}

void jf_network_clearSocketFromFdSet(
    jf_network_socket_t * pSocket, fd_set * set)
{
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    clearIsocketFromFdSet(pis, set);
}

boolean_t jf_network_isSocketSetInFdSet(
    jf_network_socket_t * pSocket, fd_set * set)
{
    boolean_t bRet = FALSE;
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    bRet = isIsocketSetInFdSet(pis, set);

    return bRet;
}

void jf_network_setSocketToFdSet(jf_network_socket_t * pSocket, fd_set * set)
{
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    setIsocketToFdSet(pis, set);
}

void jf_network_clearFdSet(fd_set * set)
{
    clearIsocketFdSet(set);
}

u32 jf_network_getSocketOpt(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen)
{
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    return isGetSockOpt(pis, level, optname, optval, optlen);
}

u32 jf_network_setSocketOpt(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    internal_socket_t * pis = (internal_socket_t *)pSocket;

    assert(pSocket != NULL);

    return isSetSockOpt(pis, level, optname, optval, optlen);
}

/*--------------------------------------------------------------------------*/

