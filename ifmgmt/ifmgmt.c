/**
 *  @file ifmgmt.c
 *
 *  @brief The network interface management library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_logger.h"
#include "jf_ifmgmt.h"
#include "jf_mem.h"
#include "jf_filestream.h"
#include "jf_string.h"

/* --- private data/data structure section --------------------------------- */
#define SYSTEM_NET_DEV_FILE   "/proc/net/dev"


/* --- private routine section---------------------------------------------- */

#if defined(LINUX)

static u32 _getIfmgmtIfAddr(
    const olchar_t * name, olint_t sock, olint_t req, jf_ipaddr_t * pji)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, name);

    ret = ioctl(sock, req, &ifr);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_convertSockAddrToIpAddr(
            &ifr.ifr_addr, sizeof(ifr.ifr_addr), pji, NULL);
    }

    return u32Ret;
}

static u32 _getIfmgmtIfFlag(olint_t sock, jf_ifmgmt_if_t * pIf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, pIf->jii_strName);

    ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ifr.ifr_flags & IFF_UP)
            pIf->jii_bUp = TRUE;
        else
            pIf->jii_bUp = FALSE;

        if (ifr.ifr_flags & IFF_BROADCAST)
            pIf->jii_bBroadcast = TRUE;

        if (ifr.ifr_flags & IFF_POINTOPOINT)
            pIf->jii_bPointopoint = TRUE;

        if (ifr.ifr_flags & IFF_LOOPBACK)
            pIf->jii_bLoopback = TRUE;

        if (ifr.ifr_flags & IFF_RUNNING)
            pIf->jii_bRunning = TRUE;

        if (ifr.ifr_flags & IFF_PROMISC)
            pIf->jii_bPromisc = TRUE;

        if (ifr.ifr_flags & IFF_MULTICAST)
            pIf->jii_bMulticast = TRUE;
    }

    return u32Ret;
}

static u32 _getIfmgmtIfHwAddr(olint_t sock, jf_ifmgmt_if_t * pIf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, pIf->jii_strName);

    ret = ioctl(sock, SIOCGIFHWADDR, &ifr);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memcpy(pIf->jii_u8Mac, ifr.ifr_hwaddr.sa_data, JF_LIMIT_MAC_LEN);
    }

    return u32Ret;
}

#endif

static u32 _getIfmgmtIf(const olchar_t * pstrIfName, jf_ifmgmt_if_t * pif)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t sock = -1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pif, 0, sizeof(*pif));
        ol_strncpy(
            pif->jii_strName, pstrIfName, JF_IFMGMT_MAX_IF_NAME_LEN);

        u32Ret = _getIfmgmtIfFlag(sock, pif);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pif->jii_bUp && pif->jii_bRunning)
        {
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFADDR, &pif->jii_jiAddr);

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFNETMASK, &pif->jii_jiNetmask);

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFBRDADDR, &pif->jii_jiBroadcast);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _getIfmgmtIfHwAddr(sock, pif);

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

static boolean_t _isValidMacAddress(u8 u8Mac[JF_LIMIT_MAC_LEN])
{
    boolean_t bRet = FALSE;
    u8 u8Index;

    for (u8Index = 0; u8Index < JF_LIMIT_MAC_LEN; u8Index ++)
    {
        if (u8Mac[u8Index] != 0)
        {
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_ifmgmt_getAllIf(jf_ifmgmt_if_t * pif, u32 * pu32NumOfIf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    u32 u32If = 0;
    jf_filestream_t * pjf = NULL;
    olchar_t buf[1024], * pName;
    olsize_t sbyte;
    olchar_t strName[32];

    ol_memset(pif, 0, sizeof(jf_ifmgmt_if_t) * (*pu32NumOfIf));

    u32Ret = jf_filestream_open(SYSTEM_NET_DEV_FILE, "r", &pjf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sbyte = sizeof(buf);
        jf_filestream_readLine(pjf, buf, &sbyte);
        sbyte = sizeof(buf);
        jf_filestream_readLine(pjf, buf, &sbyte);

        while (u32Ret == JF_ERR_NO_ERROR)
        {
            sbyte = sizeof(buf);
            u32Ret = jf_filestream_readLine(pjf, buf, &sbyte);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pName = buf;
                while ((*pName != ':') && (pName < buf + sbyte))
                    pName ++;

                if (*pName != ':')
                    continue;

                *pName = '\0';
                strncpy(strName, buf, sizeof(strName));
                jf_string_removeLeadingSpace(strName);

                u32Ret = _getIfmgmtIf(strName, &pif[u32If]);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32If ++;
                    if (u32If == *pu32NumOfIf)
                        break;
                }
                else
                {
                    /*ignore the error and goto the next line*/
                    u32Ret = JF_ERR_NO_ERROR;
                }
            }
        }

    }

    *pu32NumOfIf = u32If;
    if (u32Ret == JF_ERR_END_OF_FILE)
        u32Ret = JF_ERR_NO_ERROR;

    if (pjf != NULL)
        jf_filestream_close(&pjf);

#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_ifmgmt_getIf(const olchar_t * pstrIfName, jf_ifmgmt_if_t * pif)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pif, 0, sizeof(jf_ifmgmt_if_t));

    u32Ret = _getIfmgmtIf(pstrIfName, pif);

    return u32Ret;
}

/*port operation*/
u32 jf_ifmgmt_upIf(const olchar_t * pstrIfName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct ifreq ifr;
    olint_t ret;
    olint_t sock = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(ifr.ifr_name, pstrIfName);

        ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ifr.ifr_flags & IFF_UP)
        {
            /*the interface is up already*/
            close(sock);
            return u32Ret;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ifr.ifr_flags |= IFF_UP;
        ifr.ifr_flags |= IFF_RUNNING;

        ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_ifmgmt_downIf(const olchar_t * pstrIfName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct ifreq ifr;
    olint_t ret;
    olint_t sock = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(ifr.ifr_name, pstrIfName);

        ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (! (ifr.ifr_flags & IFF_UP))
        {
            /*the interface is already down*/
            close(sock);
            return u32Ret;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ifr.ifr_flags &= ~IFF_UP;
        ifr.ifr_flags &= ~IFF_RUNNING;

        ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_IOCTL_SOCKET;
    }

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_ifmgmt_getStringIfFlags(olchar_t * pstrFlags, jf_ifmgmt_if_t * pIf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pstrFlags[0] = '\0';

    if (pIf->jii_bUp)
        ol_strcat(pstrFlags, "Up");
    else
        ol_strcat(pstrFlags, "Down");

    if (pIf->jii_bBroadcast)
        ol_strcat(pstrFlags, ", Broadcast");

    if (pIf->jii_bPointopoint)
        ol_strcat(pstrFlags, ", Pointtopoint");

    if (pIf->jii_bLoopback)
        ol_strcat(pstrFlags, ", Loopback");

    if (pIf->jii_bRunning)
        ol_strcat(pstrFlags, ", Running");

    if (pIf->jii_bPromisc)
        ol_strcat(pstrFlags, ", Promisc");

    if (pIf->jii_bMulticast)
        ol_strcat(pstrFlags, ", Multicast");

    return u32Ret;
}

/* Return the Media Access Control (MAC) address of the first network interface
 * card (NIC)
 */
u32 jf_ifmgmt_getMacOfFirstIf(u8 u8Mac[JF_LIMIT_MAC_LEN])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    jf_ifmgmt_if_t ifmgmts[JF_IFMGMT_MAX_IF];
    u32 u32Ifmgmt = JF_IFMGMT_MAX_IF;
    u32 u32Index;

    u32Ret = jf_ifmgmt_getAllIf(ifmgmts, &u32Ifmgmt);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0; u32Index < u32Ifmgmt; u32Index ++)
        {
            if (ifmgmts[u32Index].jii_bLoopback)
                continue;

            if (_isValidMacAddress(ifmgmts[u32Index].jii_u8Mac))
            {
                ol_memcpy(u8Mac, ifmgmts[u32Index].jii_u8Mac, JF_LIMIT_MAC_LEN);
                break;
            }
        }

        if (u32Index == u32Ifmgmt)
            u32Ret = JF_ERR_INVALID_MAC_ADDR;
   }

#elif defined(WINDOWS)
    u8 u8Buffer[6000];
    PIP_ADAPTER_INFO pAdapter;
    u32 u32Len = 6000;
    DWORD dwRet;

    dwRet = GetAdaptersInfo((PIP_ADAPTER_INFO)u8Buffer, &u32Len);
    if (dwRet != ERROR_SUCCESS)
        u32Ret = JF_ERR_FAIL_GET_ADAPTER_INFO;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pAdapter = (PIP_ADAPTER_INFO)u8Buffer;
        while (pAdapter != NULL)
        {
            if (pAdapter->Type == MIB_IF_TYPE_ETHERNET)
                break;
            else
                pAdapter = pAdapter->Next;
        }

        if (pAdapter == NULL)
            u32Ret = JF_ERR_ETHERNET_ADAPTER_NOT_FOUND;
        else
            memcpy(u8Mac, pAdapter->Address, JF_LIMIT_MAC_LEN);
    }

#endif

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


