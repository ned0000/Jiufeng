/**
 *  @file internalsocket.c
 *
 *  @brief internal socket data structure
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "internalsocket.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */
/** The minimal port number from that the allocation will be
 */
#define  NET_MIN_PORT_NUMBER          (50000)

/** The port number range for the allocation
 */
#define  NET_PORT_NUMBER_RANGE        (15000)

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */
u32 newIsocket(internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis;

    assert(ppIsocket != NULL);

    u32Ret = jf_mem_calloc((void **)&pis, sizeof(internal_socket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pis->is_isSocket = INVALID_ISOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppIsocket = pis;
    else if (pis != NULL)
        freeIsocket(&pis);

    return u32Ret;
}

u32 newIsocketWithSocket(internal_socket_t ** ppIsocket, isocket_t sock)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis;

    assert(ppIsocket != NULL);

    u32Ret = jf_mem_calloc((void **)&pis, sizeof(internal_socket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pis->is_isSocket = sock;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppIsocket = pis;
    else if (pis != NULL)
        freeIsocket(&pis);

    return u32Ret;
}

u32 freeIsocket(internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis;
    olint_t nRet;

    assert((ppIsocket != NULL) && (*ppIsocket != NULL));

    pis = *ppIsocket;

#if defined(LINUX)
    nRet = close(pis->is_isSocket);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_CLOSE_SOCKET;
#elif defined(WINDOWS)
    nRet = closesocket(pis->is_isSocket);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_CLOSE_SOCKET;
#endif

    jf_mem_free((void **)ppIsocket);

    return u32Ret;
}

/** Allocates a UDP socket for a given interface, choosing a random port number
 *  from 55000 to 65000
 *
 *  @param pjiLocal [in] the interface to bind to 
 *  @param pu16Port [in/out] the port number to bind to
 *  @param ppIsocket [out] the created UDP socket 
 *
 *  @return the error code
 */
u32 createDgramIsocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    olint_t ra = 1;
    olint_t nRet = -1;
    u8 u8Addr[100];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nAddr = sizeof(u8Addr);

    assert((pjiLocal != NULL) && (pu16Port != NULL));
    assert(ppIsocket != NULL);

    if (pjiLocal->ji_u8AddrType == JF_IPADDR_TYPE_V4)
        u32Ret = createIsocket(AF_INET, SOCK_DGRAM, 0, ppIsocket);
    else
        u32Ret = createIsocket(AF_INET6, SOCK_DGRAM, 0, ppIsocket);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pis = *ppIsocket;

        if (*pu16Port == 0)
        {
            /*Keep looping until we find a port number that isn't in use.
              Since we're using random numbers, the first try should
              usually do it. We can't just bind to 0, because we need to
              be IANA compliant.*/
            do
            {
                /*Choose a random port from 50000 to 65500, which is what
                  IANA says to use for non standard ports */
                *pu16Port = (unsigned short) (NET_MIN_PORT_NUMBER +
                    ((unsigned short) rand() % NET_PORT_NUMBER_RANGE));
                jf_ipaddr_convertIpAddrToSockAddr(pjiLocal, *pu16Port, psa, &nAddr);
            } while (bind(pis->is_isSocket, psa, nAddr) < 0);
        }
        else
        {
            /*If a specific port was specified, try to use that*/
            jf_ipaddr_convertIpAddrToSockAddr(pjiLocal, *pu16Port, psa, &nAddr);
            /*This doesn't matter if failed*/
            setsockopt(pis->is_isSocket, SOL_SOCKET, SO_REUSEADDR,
                       (olchar_t *) &ra, sizeof(ra));

            nRet = bind(pis->is_isSocket, psa, nAddr);
            if (nRet == -1)
                u32Ret = JF_ERR_FAIL_BIND_SOCKET;
        }

    }

    return u32Ret;
}

/** Allocates a TCP socket for a given interface, choosing a random port number
 *  from 50000 to 65000
 *
 *  @note If the port is 0, select a random port from the port number range
 *
 *  @param pjiLocal [in] the interface to bind to 
 *  @param pu16Port [in/out] the port number to bind to
 *  @param ppIsocket [out] the created UDP socket 
 *
 *  @return the error code
 */
u32 createStreamIsocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    olint_t ra = 1;
    olint_t nRet = -1;
    u8 u8Addr[100];
    struct sockaddr * psa = (struct sockaddr *)u8Addr;
    olint_t nAddr = sizeof(u8Addr);

    assert((pjiLocal != NULL) && (pu16Port != NULL) && (ppIsocket != NULL));

    if (pjiLocal->ji_u8AddrType == JF_IPADDR_TYPE_V4)
        u32Ret = createIsocket(AF_INET, SOCK_STREAM, 0, ppIsocket);
    else
        u32Ret = createIsocket(AF_INET6, SOCK_STREAM, 0, ppIsocket);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pis = *ppIsocket;

        if (*pu16Port == 0)
        {
            /*If *pu16Port is 0, we need to choose a random port from 
              NET_MIN_PORT_NUMBER to NET_MIN_PORT_NUMBER + NET_PORT_NUMBER_RANGE.
              By default this is 50000 + 15000, which gives us the IANA 
              defined range to use*/
            do
            {
                *pu16Port = (unsigned short) (NET_MIN_PORT_NUMBER +
                    ((unsigned short) rand() % NET_PORT_NUMBER_RANGE));
                jf_ipaddr_convertIpAddrToSockAddr(pjiLocal, *pu16Port, psa, &nAddr);
            } while (bind(pis->is_isSocket, psa, nAddr) < 0);
        }
        else
        {
            /*If a specific port was specified, try to use that*/
            jf_ipaddr_convertIpAddrToSockAddr(pjiLocal, *pu16Port, psa, &nAddr);
            /*This doesn't matter if failed*/
            setsockopt(pis->is_isSocket, SOL_SOCKET, SO_REUSEADDR,
                       (olchar_t *) &ra, sizeof(ra));

            nRet = bind(pis->is_isSocket, psa, nAddr);
            if (nRet == -1)
                u32Ret = JF_ERR_FAIL_BIND_SOCKET;
        }
    }

    return u32Ret;
}

u32 createIsocket(
    olint_t domain, olint_t type, olint_t protocol,
    internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = NULL;

    *ppIsocket = NULL;

    u32Ret = newIsocket(&pis);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pis->is_isSocket = socket(domain, type, protocol);
        if (pis->is_isSocket == INVALID_ISOCKET)
        {
            u32Ret = JF_ERR_FAIL_CREATE_SOCKET;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppIsocket = pis;
    else if (pis != NULL)
        destroyIsocket(&pis);

    return u32Ret;
}

u32 destroyIsocket(internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((ppIsocket != NULL) && (*ppIsocket != NULL));

    freeIsocket(ppIsocket);

    return u32Ret;
}

u32 ioctlIsocket(internal_socket_t * pis, olint_t req, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t bRet;

    assert(pis != NULL);

#if defined(LINUX)
    bRet = ioctl(pis->is_isSocket, req, pArg);
#elif defined(WINDOWS)
    bRet = ioctlsocket(pis->is_isSocket, req, pArg);
#endif
    if (bRet != 0)
        u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;

    return u32Ret;
}

u32 setIsocketBlock(internal_socket_t * pis)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t flags;

    assert(pis != NULL);

#if defined(LINUX)
    flags = fcntl(pis->is_isSocket, F_GETFL, 0);
    fcntl(pis->is_isSocket, F_SETFL, (~O_NONBLOCK) & flags);
#elif defined(WINDOWS)
    flags = 0;
    ioctlsocket(pis->is_isSocket, FIONBIO, &flags);
#endif

    return u32Ret;
}

u32 setIsocketNonblock(internal_socket_t * pis)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t flags;

    assert(pis != NULL);

#if defined(LINUX)
    flags = fcntl(pis->is_isSocket, F_GETFL, 0);
    fcntl(pis->is_isSocket, F_SETFL, O_NONBLOCK | flags);
#elif defined(WINDOWS)
    flags = 1;
    ioctlsocket(pis->is_isSocket, FIONBIO, &flags);
#endif

    return u32Ret;
}

u32 isJoinMulticastGroup(
    internal_socket_t * pis, jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet;
    struct ip_mreq  mreq;
    struct ipv6_mreq mreq6;

    assert(pis != NULL);
    assert((pjiAddr != NULL) && (pjiMulticaseAddr != NULL));

    if (pjiAddr->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        ol_memset(&mreq, 0, sizeof(struct ip_mreq));
        /* join using the multicast address passed in */
        mreq.imr_multiaddr.s_addr = pjiMulticaseAddr->ji_uAddr.ju_nAddr;
        /* join with specified interface */
        mreq.imr_interface.s_addr = pjiAddr->ji_uAddr.ju_nAddr;

        nRet = setsockopt(
            pis->is_isSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
            (olchar_t *)&mreq, sizeof(mreq));
        if (nRet != 0)
            u32Ret = JF_ERR_FAIL_JOIN_MULTICAST_GROUP;
    }
    else if (pjiAddr->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
        ol_memset(&mreq6, 0, sizeof(struct ipv6_mreq));
#if defined(LINUX)
        /* join using the multicast address passed in */
        ol_memcpy(
            mreq6.ipv6mr_multiaddr.s6_addr,
            pjiMulticaseAddr->ji_uAddr.ju_u8Addr, 16);
        /* join with specified interface */
        mreq6.ipv6mr_interface = pjiAddr->ji_uAddr.ju_nAddr;
#elif defined(WINDOWS)
        /* join using the multicast address passed in */
        ol_memcpy(
            mreq6.ipv6mr_multiaddr.u.Byte,
            pjiMulticaseAddr->ji_uAddr.ju_u8Addr, 16);
        /* join with specified interface */
        mreq6.ipv6mr_interface = pjiAddr->ji_uAddr.ju_nAddr;
#endif
        nRet = setsockopt(
            pis->is_isSocket, IPPROTO_IP, IPV6_ADD_MEMBERSHIP,
            (olchar_t *)&mreq6, sizeof(mreq6));
        if (nRet != 0)
            u32Ret = JF_ERR_FAIL_JOIN_MULTICAST_GROUP;
    }
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 isEnableBroadcast(internal_socket_t * pis)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = -1;
#if defined(LINUX)
    const olint_t on = 1;
#elif defined(WINDOWS)
    const olchar_t on = 1;
#endif

    assert(pis != NULL);

    nRet = setsockopt(
        pis->is_isSocket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_ENABLE_BROADCAST;

    return u32Ret;
}

/** Try to send all data but only send once, the actual sent size is in psSend
 */
u32 isSend(internal_socket_t * pis, void * pBuffer, olsize_t * psSend)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sSent;

    assert(pis != NULL);

    sSent = send(pis->is_isSocket, pBuffer, *psSend, 0);
    if (sSent == -1)
    {
#if defined(LINUX)
        if (errno != EWOULDBLOCK && errno != EINTR && errno != EAGAIN)
#elif defined(WINDOWS)
        if (WSAGetLastError() != WSAEWOULDBLOCK)
#endif
            u32Ret = JF_ERR_FAIL_SEND_DATA;

        *psSend = 0;
    }
    else
        *psSend = sSent;

    return u32Ret;
}

/** Try to send all data but only send once, unless timeout the actual sent size
 *  is in psSend
 */
u32 isSendWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend,
    u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sSent = 0;
    fd_set writeset;
    struct timeval tv;
    u32 u32Ready;

    assert(pis != NULL);

    clearIsocketFdSet(&writeset);
    setIsocketToFdSet(pis, &writeset);
    tv.tv_sec = u32Timeout;
    tv.tv_usec = 0;

    isSelect(NULL, &writeset, NULL, &tv, &u32Ready);
    if (u32Ready > 0)
    {
        if (isIsocketSetInFdSet(pis, &writeset))
        {
            /* ready to write */
            sSent = *psSend;
            u32Ret = isSend(pis, pBuffer, &sSent);
        }
        else
            u32Ret = JF_ERR_FAIL_SEND_DATA;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sSent != *psSend))
        u32Ret = JF_ERR_FAIL_SEND_DATA;

    *psSend = sSent;

    return u32Ret;
}

/** Try to send all data with possible several round, until an error occurs,
 *  the actual sent size is in psSend
 */
u32 isSendn(internal_socket_t * pis, void * pBuffer, olsize_t * psSend)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sSent = 0, sent;
    u8 * pu8Start = pBuffer;

    assert(pis != NULL);

    while ((u32Ret == JF_ERR_NO_ERROR) && (sSent < *psSend))
    {
        sent = *psSend - sSent;
        u32Ret = isSend(pis, pu8Start, &sent);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pu8Start += sent;
            sSent += sent;
        }
    }

    *psSend = sSent;

    return u32Ret;
}

/** Try to send all data with possible several round, until an error occurs or
 *  timeout, the actual sent size is in psSend
 */
u32 isSendnWithTimeout(internal_socket_t * pis, void * pBuffer,
    olsize_t * psSend, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sSent = 0, sent;
    u8 * pu8Start = pBuffer;
    fd_set writeset;
    struct timeval tv;
    u32 u32Ready;
    time_t tEnd, tCur;

    assert(pis != NULL);

    tCur = time(NULL);
    tEnd = tCur + u32Timeout;

    while ((u32Ret == JF_ERR_NO_ERROR) && (tCur <= tEnd))
    {
        clearIsocketFdSet(&writeset);
        setIsocketToFdSet(pis, &writeset);
        tv.tv_sec = tEnd - tCur;
        tv.tv_usec = 0;

        isSelect(NULL, &writeset, NULL, &tv, &u32Ready);
        if (u32Ready > 0)
        {
            if (isIsocketSetInFdSet(pis, &writeset))
            {
                /* ready to write */
                sent = *psSend - sSent;
                u32Ret = isSend(pis, pu8Start, &sent);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    pu8Start += sent;
                    sSent += sent;
                    if (sSent == *psSend)
                        break;
                }
            }
            else
                u32Ret = JF_ERR_FAIL_SEND_DATA;
        }

        tCur = time(NULL);
        if (tEnd <= tCur)
            break;
        else if (tEnd - tCur > u32Timeout)
            tCur = tEnd;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sSent != *psSend))
        u32Ret = JF_ERR_FAIL_SEND_DATA;

    *psSend = sSent;

    return u32Ret;
}

/** Try to recveive all data but only recveive once, the actual received size is
 *  in psRecv
 */
u32 isRecv(internal_socket_t * pis, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    ssize_t recved;

    assert(pis != NULL);

    recved = recv(pis->is_isSocket, pBuffer, *psRecv, 0);
    if (recved < 0)
    {
        *psRecv = 0;
#if defined(LINUX)
        if (errno != EWOULDBLOCK && errno != EINTR && errno != EAGAIN)
#elif defined(WINDOWS)
        if (WSAGetLastError() != WSAEWOULDBLOCK)
#endif
            u32Ret = JF_ERR_FAIL_RECV_DATA;
    }
    else if (recved == 0)
        u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
    else
        *psRecv = recved;

    return u32Ret;
}

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
u32 isRecvWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sRecved = 0;
    fd_set readset;
    struct timeval tv;
    u32 u32Ready;

    assert(pis != NULL);

    clearIsocketFdSet(&readset);
    setIsocketToFdSet(pis, &readset);
    tv.tv_sec = u32Timeout;
    tv.tv_usec = 0;

    isSelect(&readset, NULL, NULL, &tv, &u32Ready);
    if (u32Ready > 0)
    {
        if (isIsocketSetInFdSet(pis, &readset))
        {
            /*ready to read*/
            sRecved = *psRecv;
            u32Ret = isRecv(pis, pBuffer, &sRecved);
            if ((u32Ret == JF_ERR_NO_ERROR) && (sRecved == 0))
                u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
        }
        else
            u32Ret = JF_ERR_FAIL_RECV_DATA;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sRecved == 0))
        u32Ret = JF_ERR_FAIL_RECV_DATA;

    *psRecv = sRecved;

    return u32Ret;
}

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
u32 isRecvfromWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout, jf_ipaddr_t * pjiFrom, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sRecved = 0;
    fd_set readset;
    struct timeval tv;
    u32 u32Ready;

    assert(pis != NULL);

    clearIsocketFdSet(&readset);
    setIsocketToFdSet(pis, &readset);
    tv.tv_sec = u32Timeout;
    tv.tv_usec = 0;

    isSelect(&readset, NULL, NULL, &tv, &u32Ready);
    if (u32Ready > 0)
    {
        if (isIsocketSetInFdSet(pis, &readset))
        {
            /*ready to read*/
            sRecved = *psRecv;
            u32Ret = isRecvfrom(pis, pBuffer, &sRecved, pjiFrom, pu16Port);
            if ((u32Ret == JF_ERR_NO_ERROR) && (sRecved == 0))
                u32Ret = JF_ERR_SOCKET_PEER_CLOSED;
        }
        else
            u32Ret = JF_ERR_FAIL_RECV_DATA;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (sRecved == 0))
        u32Ret = JF_ERR_FAIL_RECV_DATA;

    *psRecv = sRecved;

    return u32Ret;
}

/** Try to recveive all data with possible several round, until an error occurs,
 *  the actual recveived size is in psRecv
 */
u32 isRecvn(internal_socket_t * pis, void * pBuffer, olsize_t * psRecv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sRecved = 0, recved = 0;
    u8 * pu8Start = pBuffer;

    assert(pis != NULL);

    while ((u32Ret == JF_ERR_NO_ERROR) && (sRecved < *psRecv))
    {
        recved = *psRecv - sRecved;
        u32Ret = isRecv(pis, pu8Start, &recved);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pu8Start += recved;
            sRecved += recved;
        }
    }

    *psRecv = sRecved;

    return u32Ret;
}

/** Try to recveive all data with possible several round, until an error occurs
 *  or timeout, the actual recveived size is in psRecv
 */
u32 isRecvnWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sRecved = 0, recved = 0;
    u8 * pu8Start = pBuffer;
    fd_set readset;
    struct timeval tv;
    time_t tEnd, tCur;
    u32 u32Ready;

    assert(pis != NULL);

    tCur = time(NULL);
    tEnd = tCur + u32Timeout;

    while ((u32Ret == JF_ERR_NO_ERROR) && (tCur < tEnd))
    {
        clearIsocketFdSet(&readset);
        setIsocketToFdSet(pis, &readset);
        tv.tv_sec = tEnd - tCur;
        tv.tv_usec = 0;

        isSelect(&readset, NULL, NULL, &tv, &u32Ready);
        if (u32Ready > 0)
        {
            if (isIsocketSetInFdSet(pis, &readset))
            {
                /*ready to read*/
                recved = *psRecv - sRecved;
                u32Ret = isRecv(pis, pu8Start, &recved);
                if ((u32Ret == JF_ERR_NO_ERROR) && (recved == 0))
                    u32Ret = JF_ERR_SOCKET_PEER_CLOSED;

                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    pu8Start += recved;
                    sRecved += recved;

                    if (sRecved == *psRecv)
                        break;
                }
            }
            else
                u32Ret = JF_ERR_FAIL_RECV_DATA;
        }

        tCur = time(NULL);
        if (tEnd <= tCur)
            break;
        if (tEnd - tCur > u32Timeout)
            tCur = tEnd;
    }

    *psRecv = sRecved;

    return u32Ret;
}

u32 isListen(internal_socket_t * pisListen, olint_t backlog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet;

    assert(pisListen != NULL);

    nRet = listen(pisListen->is_isSocket, backlog);
    if (nRet == -1)
    {
        u32Ret = JF_ERR_FAIL_LISTEN_ON_SOCKET;
    }

    return u32Ret;
}

u32 isConnect(internal_socket_t * pis, const jf_ipaddr_t * pji, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Sa[100];
    struct sockaddr * psaTo = (struct sockaddr *)u8Sa;
    olint_t salen = sizeof(u8Sa);
    olint_t nRet;

    jf_ipaddr_convertIpAddrToSockAddr(pji, u16Port, psaTo, &salen);

    nRet = connect(pis->is_isSocket, psaTo, salen);
#if defined(LINUX)
    if (nRet != 0)
    {
        if (errno != EINPROGRESS)
            u32Ret = JF_ERR_FAIL_INITIATE_CONNECTION;
    }
#elif defined(WINDOWS)
    if (nRet == INVALID_ISOCKET)
    {
        if ((WSAGetLastError() != WSAEALREADY) &&
            (WSAGetLastError() != WSAEWOULDBLOCK))
            u32Ret = JF_ERR_FAIL_INITIATE_CONNECTION;
    }
#endif

    return u32Ret;
}

u32 isConnectWithTimeout(
    internal_socket_t * pis, const jf_ipaddr_t * pji,
    u16 u16Port, u32 u32Timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bConnect = FALSE;
    fd_set writeset, errorset;
    struct timeval tv;
    time_t tEnd, tCur;
    u32 u32Ready;

    tCur = time(NULL);
    tEnd = tCur + u32Timeout;

    u32Ret = isConnect(pis, pji, u16Port);

    while ((u32Ret == JF_ERR_NO_ERROR) && (tCur < tEnd))
    {
        clearIsocketFdSet(&writeset);
        clearIsocketFdSet(&errorset);
        setIsocketToFdSet(pis, &writeset);
        setIsocketToFdSet(pis, &errorset);
        tv.tv_sec = tEnd - tCur;
        tv.tv_usec = 0;

        isSelect(NULL, &writeset, NULL, &tv, &u32Ready);
        if (u32Ready > 0)
        {
            if (isIsocketSetInFdSet(pis, &writeset))
            {
                /* Connected */
                bConnect = TRUE;
                break;
            }

            if (isIsocketSetInFdSet(pis, &errorset))
            {
                /* Error */
                u32Ret = JF_ERR_FAIL_INITIATE_CONNECTION;
                break;
            }
        }

        tCur = time(NULL);
        if (tEnd <= tCur)
            break;
        else if (tEnd - tCur > u32Timeout)
            tCur = tEnd;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (! bConnect))
        u32Ret = JF_ERR_FAIL_INITIATE_CONNECTION;

    return u32Ret;
}

u32 isAccept(
    internal_socket_t * pisListen, jf_ipaddr_t * pji, u16 * pu16Port,
    internal_socket_t ** ppIsocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pisListen;
    internal_socket_t * pisAccept = NULL;
    u8 u8Addr[100];
    struct sockaddr * psaFrom = (struct sockaddr *)u8Addr;
    olint_t nFromLen = sizeof(u8Addr);

    assert((pisListen != NULL) && (ppIsocket != NULL));

    u32Ret = jf_mem_alloc((void **)&pisAccept, sizeof(internal_socket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pisAccept, 0, sizeof(internal_socket_t));

#if defined(LINUX)
        pisAccept->is_isSocket = accept(
            pis->is_isSocket, psaFrom, (socklen_t *)&nFromLen);
#elif defined(WINDOWS)
        pisAccept->is_isSocket = accept(pis->is_isSocket, psaFrom, &nFromLen);
#endif
        if (pisAccept->is_isSocket == INVALID_ISOCKET)
            u32Ret = JF_ERR_FAIL_ACCEPT_CONNECTION;
        else
            jf_ipaddr_convertSockAddrToIpAddr(psaFrom, nFromLen, pji, pu16Port);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppIsocket = pisAccept;
    else if (pisAccept != NULL)
        destroyIsocket(&pisAccept);

    return u32Ret;
}

u32 isSendto(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend,
    const jf_ipaddr_t * pjiTo, u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Addr[100];
    struct sockaddr * psaTo = (struct sockaddr *)u8Addr;
    olint_t salen = sizeof(u8Addr);
    olsize_t sent;

    assert(pis != NULL);

    jf_ipaddr_convertIpAddrToSockAddr(pjiTo, u16Port, psaTo, &salen);

    sent = sendto(pis->is_isSocket, pBuffer, *psSend, 0, psaTo, salen);
    if (sent == -1)
    {
#if defined(LINUX)
        if (errno == EWOULDBLOCK)
#elif defined(WINDOWS)
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#endif
            *psSend = 0;
        else
            u32Ret = JF_ERR_FAIL_SEND_DATA;
    }
    else
        *psSend = sent;

    return u32Ret;
}

u32 isRecvfrom(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv,
    jf_ipaddr_t * pjiFrom, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Addr[100];
    struct sockaddr * psaFrom = (struct sockaddr *)u8Addr;
    olint_t nFromLen = sizeof(u8Addr);
    olsize_t sRecved;

    assert(pis != NULL);

#if defined(LINUX)
    sRecved = recvfrom(
        pis->is_isSocket, pBuffer, *psRecv, 0, psaFrom, (socklen_t *)&nFromLen);
#elif defined(WINDOWS)
    sRecved = recvfrom(
        pis->is_isSocket, pBuffer, *psRecv, 0, psaFrom, &nFromLen);
#endif
    if (sRecved < 0)
        u32Ret = JF_ERR_FAIL_RECV_DATA;
    else
    {
        if (pjiFrom != NULL)
            jf_ipaddr_convertSockAddrToIpAddr(psaFrom, nFromLen, pjiFrom, pu16Port);

        *psRecv = sRecved;
    }

    return u32Ret;
}

u32 isSelect(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
    struct timeval * timeout, u32 * pu32Ready)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t slct;

    *pu32Ready = 0;

    slct = select(FD_SETSIZE, readfds, writefds, exceptfds, timeout);
#if defined(LINUX)
    if (slct == -1)
#elif defined(WINDOWS)
    if (slct == SOCKET_ERROR)
#endif
    {
        u32Ret = JF_ERR_SELECT_ERROR;
    }
    else
        *pu32Ready = (u32)slct;

    return u32Ret;
}

u32 getIsocketName(
    internal_socket_t * pis, struct sockaddr * pName, olint_t * pnNameLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet;

    assert(pis != NULL);
#if defined(LINUX)
    nRet = getsockname(pis->is_isSocket, pName, (socklen_t *)pnNameLen);
#elif defined(WINDOWS)
    nRet = getsockname(pis->is_isSocket, pName, pnNameLen);
#endif
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_GET_SOCKET_NAME;

    return u32Ret;
}

#if defined(WINDOWS)
u32 WSAIoctlIsocket(
    internal_socket_t * pis, DWORD dwIoControlCode,
    LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer,
    DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t bRet;

    bRet = WSAIoctl(
        pis->is_isSocket, dwIoControlCode, lpvInBuffer, cbInBuffer,
        lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped,
        lpCompletionRoutine);
    if (bRet != 0)
        u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;

    return u32Ret;
}
#endif

void clearIsocketFromFdSet(internal_socket_t * pis, fd_set * set)
{
    assert(pis != NULL);

    FD_CLR(pis->is_isSocket, set);
}

boolean_t isIsocketSetInFdSet(internal_socket_t * pis, fd_set * set)
{
    boolean_t bRet = FALSE;

    assert(pis != NULL);

    if (FD_ISSET(pis->is_isSocket, set))
        bRet = TRUE;

    return bRet;
}

void setIsocketToFdSet(internal_socket_t * pis, fd_set * set)
{
    assert(pis != NULL);

    FD_SET(pis->is_isSocket, set);
}

void clearIsocketFdSet(fd_set * set)
{
    FD_ZERO(set);
}

u32 isGetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = -1;

    assert(pis != NULL);

    nRet = getsockopt(
        pis->is_isSocket, level, optname, optval, (socklen_t *)optlen);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_GET_SOCKET_OPT;

    return u32Ret;
}

u32 isSetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname,
    void * optval, olsize_t optlen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = -1;

    assert(pis != NULL);

    nRet = setsockopt(pis->is_isSocket, level, optname, optval, optlen);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_SET_SOCKET_OPT;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


