/**
 *  @file ipaddr.c
 *
 *  @brief The functions to manipulate ip address
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#if defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#elif defined(WINDOWS)
    #if _MSC_VER >= 1500
        #include <ws2tcpip.h>
    #endif
    #include <Winsock2.h>
    #include <Iphlpapi.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "ifmgmt.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
static u32 _getIpV4LocalIpAddrList(jf_ipaddr_t * pAddr, u16 * pu16Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olchar_t strBuffer[16 * sizeof(struct ifreq)];
    struct ifconf ifConf;
    struct ifreq ifReq;
    olint_t sock = -1;
    olint_t ret;
    struct sockaddr_in localaddr;
    olint_t tempresults[16];
    u16 u16Count = 0, u16Index;
    olint_t i;

    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* Get the interface configuration information... */
        ifConf.ifc_len = sizeof strBuffer;
        ifConf.ifc_ifcu.ifcu_buf = (caddr_t)strBuffer;
        ret = ioctl(sock, SIOCGIFCONF, &ifConf);
        if (ret == -1)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* Cycle through the list of interfaces looking for IP addresses. */
        for (i = 0; (i < ifConf.ifc_len); )
        {
            struct ifreq * pifReq =
                (struct ifreq *)((caddr_t)ifConf.ifc_req + i);

            i += sizeof(*pifReq);
            /* See if this is the sort of interface we want to deal with. */
            ol_strcpy(ifReq.ifr_name, pifReq->ifr_name);
            ret = ioctl(sock, SIOCGIFFLAGS, &ifReq);
            if (ret == -1)
            {
                u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
                break;
            }

            /* Skip loopback, point-to-poolint_t and down interfaces, */
            /* except don't skip down interfaces */
            /* if we're trying to get a list of configurable interfaces. */
            if (ifReq.ifr_flags & IFF_LOOPBACK || ! (ifReq.ifr_flags & IFF_UP))
            {
                continue;
            }   
            if (pifReq->ifr_addr.sa_family == AF_INET)
            {
                /* Get a pointer to the address... */
                memcpy(&localaddr, &pifReq->ifr_addr, sizeof(pifReq->ifr_addr));
                if (localaddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK))
                {
                    tempresults[u16Count] = localaddr.sin_addr.s_addr;
                    ++u16Count;
                }
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u16Count > *pu16Count)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16Count; u16Index ++)
        {
            pAddr[u16Index].ji_u8AddrType = JF_IPADDR_TYPE_V4;
            pAddr[u16Index].ji_uAddr.ju_nAddr = tempresults[u16Index];
        }

        *pu16Count = u16Count;
    }

    if (sock != -1)
        close(sock);

#elif defined(WINDOWS)
    olint_t i;
    olchar_t buffer[16 * sizeof(SOCKET_ADDRESS_LIST)];
    DWORD bufferSize;
    SOCKET sock = INVALID_SOCKET;
    olint_t tempresults[16];
    u16 u16Count = 0, u16Index;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY,
             NULL, 0, buffer, 16 * sizeof(SOCKET_ADDRESS_LIST), &bufferSize,
             NULL, NULL);
        u16Count = (u16)(((SOCKET_ADDRESS_LIST*)buffer)->iAddressCount);
        for (i = 0; i < u16Count; ++i)
        {
            tempresults[i] = ((struct sockaddr_in *)
                (((SOCKET_ADDRESS_LIST*)buffer)->Address[i].lpSockaddr))->sin_addr.s_addr;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u16Count > *pu16Count)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16Count; u16Index ++)
        {
            pAddr[u16Index].ji_u8AddrType = JF_IPADDR_TYPE_V4;
            pAddr[u16Index].ji_uAddr.iu_nAddr = tempresults[u16Index];
        }

        *pu16Count = u16Count;
    }

    if (sock != INVALID_SOCKET)
        closesocket(sock);
#endif

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_ipaddr_getLocalIpAddrList(
    u8 u8AddrType, jf_ipaddr_t * pAddr, u16 * pu16Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((u8AddrType == JF_IPADDR_TYPE_V4) || (u8AddrType == JF_IPADDR_TYPE_V6));
    assert((pAddr != NULL) && (pu16Count != NULL));

    if (u8AddrType == JF_IPADDR_TYPE_V4)
        u32Ret = _getIpV4LocalIpAddrList(pAddr, pu16Count);
    else if (u8AddrType == JF_IPADDR_TYPE_V6)
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 jf_ipaddr_convertSockAddrToIpAddr(
    const struct sockaddr * psa, const olint_t nSaLen,
    jf_ipaddr_t * pji, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct sockaddr_in * pInet4Addr;
    struct sockaddr_in6 * pInet6Addr;

    memset(pji, 0, sizeof(jf_ipaddr_t));

    switch (psa->sa_family)
    {
    case AF_INET:
        pInet4Addr = (struct sockaddr_in *)psa;

        pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
        pji->ji_uAddr.ju_nAddr = pInet4Addr->sin_addr.s_addr;
        if (pu16Port != NULL)
            *pu16Port = ntohs(pInet4Addr->sin_port);
        break;
    case AF_INET6:
        pInet6Addr = (struct sockaddr_in6 *)psa;

        pji->ji_u8AddrType = JF_IPADDR_TYPE_V6;
#if defined(LINUX)
        memcpy(pji->ji_uAddr.ju_u8Addr, pInet6Addr->sin6_addr.s6_addr, 16);
#elif defined(WINDOWS)
        memcpy(pji->ji_uAddr.ju_u8Addr, pInet6Addr->sin6_addr.u.Byte, 16);
#endif
        if (pu16Port != NULL)
            *pu16Port = ntohs(pInet6Addr->sin6_port);
        break;
    default:
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
        break;
    }

    return u32Ret;
}

u32 jf_ipaddr_convertIpAddrToSockAddr(
    const jf_ipaddr_t * pji, const u16 u16Port,
    struct sockaddr * psa, olint_t * pnSaLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct sockaddr_in * pInet4Addr;
    struct sockaddr_in6 * pInet6Addr;

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        assert(*pnSaLen >= sizeof(struct sockaddr_in));

        pInet4Addr = (struct sockaddr_in *)psa;

        memset(pInet4Addr, 0, sizeof(struct sockaddr_in));

        pInet4Addr->sin_family = AF_INET;
        pInet4Addr->sin_addr.s_addr = pji->ji_uAddr.ju_nAddr;
        pInet4Addr->sin_port = htons(u16Port);

        *pnSaLen = sizeof(struct sockaddr_in);
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
        assert(*pnSaLen >= sizeof(struct sockaddr_in6));

        pInet6Addr = (struct sockaddr_in6 *)psa;

        memset(pInet6Addr, 0, sizeof(struct sockaddr_in6));

        pInet6Addr->sin6_family = AF_INET6;
#if defined(LINUX)
        memcpy(pInet6Addr->sin6_addr.s6_addr, pji->ji_uAddr.ju_u8Addr, 16);
#elif defined(WINDOWS)
        memcpy(pInet6Addr->sin6_addr.u.Byte, pji->ji_uAddr.ju_u8Addr, 16);
#endif
        pInet6Addr->sin6_port = htons(u16Port);

        *pnSaLen = sizeof(struct sockaddr_in6);
    }
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 jf_ipaddr_setIpAddrToInaddrAny(jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct in6_addr in6addr;

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        pji->ji_uAddr.ju_nAddr = INADDR_ANY;
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
#if defined(LINUX)
        in6addr = in6addr_any;
        memcpy(pji->ji_uAddr.ju_u8Addr, in6addr.s6_addr, 16);
#elif defined(WINDOWS)

#endif
    }
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 jf_ipaddr_setIpV4AddrToInaddrAny(jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
    jf_ipaddr_setIpAddrToInaddrAny(pji);

    return u32Ret;
}

u32 jf_ipaddr_setIpV6AddrToInaddrAny(jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pji->ji_u8AddrType = JF_IPADDR_TYPE_V6;
    jf_ipaddr_setIpAddrToInaddrAny(pji);

    return u32Ret;
}

void jf_ipaddr_setIpV4Addr(jf_ipaddr_t * pji, olint_t nAddr)
{
    memset(pji, 0, sizeof(jf_ipaddr_t));

    pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
    pji->ji_uAddr.ju_nAddr = nAddr;
}

boolean_t jf_ipaddr_isIpV4Addr(jf_ipaddr_t * pji)
{
    boolean_t bRet = TRUE;

    if (pji->ji_u8AddrType != JF_IPADDR_TYPE_V4)
        bRet = FALSE;

    return bRet;
}

u32 jf_ipaddr_getIpAddrFromString(
    const olchar_t * pIpString, u8 u8AddrType, jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t bRet;
    struct in_addr inaddr;

    memset(pji, 0, sizeof(jf_ipaddr_t));

    if (u8AddrType == JF_IPADDR_TYPE_V4)
    {
        pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
#if defined(LINUX)
        bRet = inet_aton(pIpString, &inaddr);
        if (bRet != 0)
            pji->ji_uAddr.ju_nAddr = inaddr.s_addr;
        else
            u32Ret = JF_ERR_INVALID_IP;
#elif defined(WINDOWS)
        pji->ji_uAddr.ju_nAddr = inet_addr(pIpString);
        if (pji->ji_uAddr.ju_nAddr == INADDR_NONE)
            u32Ret = JF_ERR_INVALID_IP;
#endif
    }
    else if (u8AddrType == JF_IPADDR_TYPE_V6)
    {
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    }
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 jf_ipaddr_getIpAddrPortFromString(
    const olchar_t * pIpString, u8 u8AddrType, jf_ipaddr_t * pji, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t bRet;
    struct in_addr inaddr;
    olchar_t strIp[100];
    olchar_t * port;

    ol_strncpy(strIp, pIpString, 99);
    strIp[99] = '\0';

    port = strIp + ol_strlen(pIpString);
    while (port != strIp && *port != ':')
        port --;

    if (*port != ':')
        return JF_ERR_INVALID_IP;

    *port = '\0';
    port ++;

    sscanf(port, "%hu", pu16Port);

    memset(pji, 0, sizeof(jf_ipaddr_t));

    if (u8AddrType == JF_IPADDR_TYPE_V4)
    {
        pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
#if defined(LINUX)
        bRet = inet_aton(strIp, &inaddr);
        if (bRet != 0)
            pji->ji_uAddr.ju_nAddr = inaddr.s_addr;
        else
            u32Ret = JF_ERR_INVALID_IP;
#elif defined(WINDOWS)
        pji->ji_uAddr.ju_nAddr = inet_addr(strIp);
        if (pji->ji_uAddr.ju_nAddr == INADDR_NONE)
            u32Ret = JF_ERR_INVALID_IP;
#endif
    }
    else if (u8AddrType == JF_IPADDR_TYPE_V6)
    {
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    }
    else
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;

    return u32Ret;
}

u32 jf_ipaddr_validateIpAddr(const olchar_t * pstrIp, const u8 u8AddrType)
{
    jf_ipaddr_t ji;

    return jf_ipaddr_getIpAddrFromString(pstrIp, u8AddrType, &ji);
}

/* get the string of IP Address according to the IP address type
 *
 * the function does not check the size of the string buffer.
 * please make sure it is big enough to avoid memory access violation.
 *
 * @param
 *   [in] pstrIpAddr, the string buffer where the IP address string will return
 *   [out] pji, the ip address.
 *
 * @return: JF_ERR_NO_ERROR, JF_ERR_INVALID_IP_ADDR_TYPE.
 */
u32 jf_ipaddr_getStringIpAddr(olchar_t * pstrIpAddr, const jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct in_addr inaddr;

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        inaddr.s_addr = pji->ji_uAddr.ju_nAddr;
#if defined(LINUX)
        if (inet_ntop(AF_INET, (void *)&inaddr, pstrIpAddr, 16) == NULL)
            u32Ret = JF_ERR_INVALID_IP;
#elif defined(WINDOWS)
        ol_strcpy(pstrIpAddr, inet_ntoa(inaddr));
#endif
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {

    }
    else
    {
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


