/**
 *  @file ipaddr.c
 *
 *  @brief Implementation file for functions to manipulate ip address.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_ipaddr.h"
#include "jf_string.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _getIpV4LocalIpAddrList(jf_ipaddr_t * pAddr, u16 * pu16Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olchar_t strBuffer[16 * sizeof(struct ifreq)];
    struct ifconf ifConf;
    struct ifreq ifReq;
    olint_t sock = -1;
    olint_t ret = 0;
    struct sockaddr_in localaddr;
    olint_t tempresults[16];
    u16 u16Count = 0, u16Index = 0;
    olint_t i = 0;

    /*Create an unbound datagram socket to do the SIOCGIFADDR ioctl on.*/
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Get the interface configuration information.*/
        ifConf.ifc_len = sizeof strBuffer;
        ifConf.ifc_ifcu.ifcu_buf = (caddr_t)strBuffer;
        ret = ioctl(sock, SIOCGIFCONF, &ifConf);
        if (ret == -1)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Cycle through the list of interfaces looking for IP addresses.*/
        for (i = 0; (i < ifConf.ifc_len); )
        {
            struct ifreq * pifReq =
                (struct ifreq *)((caddr_t)ifConf.ifc_req + i);

            i += sizeof(*pifReq);
            /*See if this is the sort of interface we want to deal with.*/
            ol_strcpy(ifReq.ifr_name, pifReq->ifr_name);
            ret = ioctl(sock, SIOCGIFFLAGS, &ifReq);
            if (ret == -1)
            {
                u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
                break;
            }

            /*Skip loopback, point-to-poolint_t and down interfaces, except don't skip down
              interfaces if we're trying to get a list of configurable interfaces.*/
            if (ifReq.ifr_flags & IFF_LOOPBACK || ! (ifReq.ifr_flags & IFF_UP))
            {
                continue;
            }   
            if (pifReq->ifr_addr.sa_family == AF_INET)
            {
                /*Get a pointer to the address.*/
                ol_memcpy(&localaddr, &pifReq->ifr_addr, sizeof(pifReq->ifr_addr));
                if (localaddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK))
                {
                    tempresults[u16Count] = localaddr.sin_addr.s_addr;
                    ++u16Count;
                }
            }
        }
    }

    /*Check buffer size.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u16Count > *pu16Count)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    /*Copy the address list.*/
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
    olint_t i = 0;
    olchar_t buffer[16 * sizeof(SOCKET_ADDRESS_LIST)];
    DWORD bufferSize = 0;
    SOCKET sock = INVALID_SOCKET;
    olint_t tempresults[16];
    u16 u16Count = 0, u16Index = 0;

    /*Create a socket.*/
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    /*Get address list.*/
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

    /*Check buffer size.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u16Count > *pu16Count)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    /*Copy the address list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16Count; u16Index ++)
        {
            pAddr[u16Index].ji_u8AddrType = JF_IPADDR_TYPE_V4;
            pAddr[u16Index].ji_uAddr.ju_nAddr = tempresults[u16Index];
        }

        *pu16Count = u16Count;
    }

    if (sock != INVALID_SOCKET)
        closesocket(sock);
#endif

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

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
    const struct sockaddr * psa, const olint_t nSaLen, jf_ipaddr_t * pji, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct sockaddr_in * pInet4Addr = NULL;
    struct sockaddr_in6 * pInet6Addr = NULL;
#if defined(LINUX)
    struct sockaddr_un * pUnAddr = NULL;
#endif

    ol_memset(pji, 0, sizeof(jf_ipaddr_t));

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
        ol_memcpy(pji->ji_uAddr.ju_u8Addr, pInet6Addr->sin6_addr.s6_addr, 16);
#elif defined(WINDOWS)
        ol_memcpy(pji->ji_uAddr.ju_u8Addr, pInet6Addr->sin6_addr.u.Byte, 16);
#endif

        if (pu16Port != NULL)
            *pu16Port = ntohs(pInet6Addr->sin6_port);
        break;
#if defined(LINUX)
    case AF_UNIX:
        pUnAddr = (struct sockaddr_un *)psa;

        pji->ji_u8AddrType = JF_IPADDR_TYPE_UDS;
        ol_strncpy(pji->ji_uAddr.ju_strPath, pUnAddr->sun_path, sizeof(pji->ji_uAddr.ju_strPath) - 1);

        if (pu16Port != NULL)
            *pu16Port = 0;
        break;
#endif
    default:
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
        break;
    }

    return u32Ret;
}

u32 jf_ipaddr_convertIpAddrToSockAddr(
    const jf_ipaddr_t * pji, const u16 u16Port, struct sockaddr * psa, olint_t * pnSaLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct sockaddr_in * pInet4Addr = NULL;
    struct sockaddr_in6 * pInet6Addr = NULL;
#if defined(LINUX)
    struct sockaddr_un * pUnAddr = NULL;
#endif

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        /*IPv4.*/
        assert(*pnSaLen >= sizeof(struct sockaddr_in));

        pInet4Addr = (struct sockaddr_in *)psa;

        ol_bzero(pInet4Addr, sizeof(struct sockaddr_in));

        pInet4Addr->sin_family = AF_INET;
        pInet4Addr->sin_addr.s_addr = pji->ji_uAddr.ju_nAddr;
        pInet4Addr->sin_port = htons(u16Port);

        *pnSaLen = sizeof(struct sockaddr_in);
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
        /*IPv6.*/
        assert(*pnSaLen >= sizeof(struct sockaddr_in6));

        pInet6Addr = (struct sockaddr_in6 *)psa;

        ol_bzero(pInet6Addr, sizeof(struct sockaddr_in6));

        pInet6Addr->sin6_family = AF_INET6;
#if defined(LINUX)
        ol_memcpy(pInet6Addr->sin6_addr.s6_addr, pji->ji_uAddr.ju_u8Addr, 16);
#elif defined(WINDOWS)
        ol_memcpy(pInet6Addr->sin6_addr.u.Byte, pji->ji_uAddr.ju_u8Addr, 16);
#endif
        pInet6Addr->sin6_port = htons(u16Port);

        *pnSaLen = sizeof(struct sockaddr_in6);
    }
#if defined(LINUX)
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_UDS)
    {
        /*UDS.*/
        assert(*pnSaLen >= sizeof(struct sockaddr_un));

        pUnAddr = (struct sockaddr_un *)psa;

        ol_bzero(pUnAddr, sizeof(struct sockaddr_un));

        pUnAddr->sun_family = AF_UNIX;
        ol_strncpy(pUnAddr->sun_path, pji->ji_uAddr.ju_strPath, sizeof(pUnAddr->sun_path) - 1);

        *pnSaLen = sizeof(struct sockaddr_un);
    }
#endif
    else
    {
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
    }

    return u32Ret;
}

u32 jf_ipaddr_setIpAddrToInaddrAny(jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct in6_addr in6addr;

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        /*IPv4.*/
        pji->ji_uAddr.ju_nAddr = INADDR_ANY;
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
        /*IPv6.*/
#if defined(LINUX)
        in6addr = in6addr_any;
        memcpy(pji->ji_uAddr.ju_u8Addr, in6addr.s6_addr, 16);
#elif defined(WINDOWS)

#endif
    }
    else
    {
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
    }

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

void jf_ipaddr_setIpV4Addr(jf_ipaddr_t * pji, const olint_t nAddr)
{
    ol_memset(pji, 0, sizeof(jf_ipaddr_t));

    pji->ji_u8AddrType = JF_IPADDR_TYPE_V4;
    pji->ji_uAddr.ju_nAddr = nAddr;
}

void jf_ipaddr_setUdsAddr(jf_ipaddr_t * pji, const olchar_t * pstrPath)
{
    ol_memset(pji, 0, sizeof(jf_ipaddr_t));

    pji->ji_u8AddrType = JF_IPADDR_TYPE_UDS;
    ol_strncpy(pji->ji_uAddr.ju_strPath, pstrPath, sizeof(pji->ji_uAddr.ju_strPath) - 1);
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
    olint_t bRet = 0;
    struct in_addr inaddr;

    ol_bzero(pji, sizeof(jf_ipaddr_t));

    if (u8AddrType == JF_IPADDR_TYPE_V4)
    {
        /*IPv4.*/
        pji->ji_u8AddrType = u8AddrType;
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
        /*IPv6.*/
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    }
#if defined(LINUX)
    else if (u8AddrType == JF_IPADDR_TYPE_UDS)
    {
        /*UDS.*/
        pji->ji_u8AddrType = u8AddrType;

        ol_strncpy(pji->ji_uAddr.ju_strPath, pIpString, sizeof(pji->ji_uAddr.ju_strPath) - 1);
    }
#endif
    else
    {
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
    }

    return u32Ret;
}

u32 jf_ipaddr_getIpAddrPortFromString(
    const olchar_t * pIpString, u8 u8AddrType, jf_ipaddr_t * pji, u16 * pu16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /*Buffer should be big enough to hold string of IPv6 and port.*/
    olchar_t strIp[128];
    olchar_t * port = NULL;

    /*Copy the string.*/
    ol_strncpy(strIp, pIpString, sizeof(strIp) - 1);
    strIp[sizeof(strIp) - 1] = JF_STRING_NULL_CHAR;

    /*Find port string. Move to the end of string.*/
    port = strIp + ol_strlen(strIp);

    /*Find the delimiter ':'.*/
    while ((port != strIp) && (*port != JF_STRING_COLON_CHAR))
        port --;

    /*Invalid port number if colon is not found.*/
    if (*port != JF_STRING_COLON_CHAR)
        u32Ret = JF_ERR_INVALID_PORT_NUMBER;

    /*Parse port.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *port = '\0';
        port ++;

        u32Ret = jf_option_getU16FromString(port, pu16Port);
    }

    /*Parse IP address.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ipaddr_getIpAddrFromString(strIp, u8AddrType, pji);

    return u32Ret;
}

u32 jf_ipaddr_validateIpAddr(const olchar_t * pstrIp, const u8 u8AddrType)
{
    jf_ipaddr_t ji;

    return jf_ipaddr_getIpAddrFromString(pstrIp, u8AddrType, &ji);
}

u32 jf_ipaddr_getStringIpAddr(olchar_t * pstrIp, olsize_t sIp, const jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct in_addr inaddr;

    ol_bzero(pstrIp, sIp);

    if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V4)
    {
        /*IPv4.*/
        inaddr.s_addr = pji->ji_uAddr.ju_nAddr;
#if defined(LINUX)
        if (inet_ntop(AF_INET, (void *)&inaddr, pstrIp, sIp) == NULL)
            u32Ret = JF_ERR_INVALID_IP;
#elif defined(WINDOWS)
        ol_strncpy(pstrIp, inet_ntoa(inaddr), sIp - 1);
#endif
    }
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_V6)
    {
        /*IPv6.*/
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    }
#if defined(LINUX)
    else if (pji->ji_u8AddrType == JF_IPADDR_TYPE_UDS)
    {
        /*UDS.*/
        ol_strncpy(pstrIp, pji->ji_uAddr.ju_strPath, sIp - 1);
    }
#endif
    else
    {
        u32Ret = JF_ERR_INVALID_IP_ADDR_TYPE;
    }

    return u32Ret;
}

u32 jf_ipaddr_getStringIpAddrPort(
    olchar_t * pstrIp, olsize_t sIp, const jf_ipaddr_t * pji, const u16 u16Port)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[512];

    ol_bzero(pstrIp, sIp);

    /*Get string with ip address.*/
    u32Ret = jf_ipaddr_getStringIpAddr(str, sizeof(str), pji);

    /*add port.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pji->ji_u8AddrType != JF_IPADDR_TYPE_UDS)
            ol_snprintf(pstrIp, sIp - 1, "%s:%u", str, u16Port);
        else /*Unix domain socket doesn't have port.*/
            ol_snprintf(pstrIp, sIp - 1, "%s", str);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
