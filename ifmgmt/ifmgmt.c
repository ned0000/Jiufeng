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
#include "olbasic.h"
#include "ollimit.h"
#include "logger.h"
#include "ifmgmt.h"
#include "xmalloc.h"
#include "files.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
#define SYSTEM_NET_DEV_FILE   "/proc/net/dev"


/* --- private routine section---------------------------------------------- */

#if defined(LINUX)

static u32 _getIfmgmtIfAddr(
    const olchar_t * name, olint_t sock, olint_t req, ip_addr_t * pia)
{
    u32 u32Ret = OLERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, name);

    ret = ioctl(sock, req, &ifr);
    if (ret != 0)
        u32Ret = OLERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        convertSockAddrToIpAddr(
            &ifr.ifr_addr, sizeof(ifr.ifr_addr), pia, NULL);
    }

    return u32Ret;
}

static u32 _getIfmgmtIfFlag(olint_t sock, ifmgmt_if_t * pIf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, pIf->ii_strName);

    ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (ret != 0)
        u32Ret = OLERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ifr.ifr_flags & IFF_UP)
            pIf->ii_bUp = TRUE;
        else
            pIf->ii_bUp = FALSE;

        if (ifr.ifr_flags & IFF_BROADCAST)
            pIf->ii_bBroadcast = TRUE;

        if (ifr.ifr_flags & IFF_POINTOPOINT)
            pIf->ii_bPointopoint = TRUE;

        if (ifr.ifr_flags & IFF_LOOPBACK)
            pIf->ii_bLoopback = TRUE;

        if (ifr.ifr_flags & IFF_RUNNING)
            pIf->ii_bRunning = TRUE;

        if (ifr.ifr_flags & IFF_PROMISC)
            pIf->ii_bPromisc = TRUE;

        if (ifr.ifr_flags & IFF_MULTICAST)
            pIf->ii_bMulticast = TRUE;
    }

    return u32Ret;
}

static u32 _getIfmgmtIfHwAddr(olint_t sock, ifmgmt_if_t * pIf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    struct ifreq ifr;
    olint_t ret;

    ol_strcpy(ifr.ifr_name, pIf->ii_strName);

    ret = ioctl(sock, SIOCGIFHWADDR, &ifr);
    if (ret != 0)
        u32Ret = OLERR_FAIL_IOCTL_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        memcpy(pIf->ii_u8Mac, ifr.ifr_hwaddr.sa_data, MAC_LEN);
    }

    return u32Ret;
}

#endif

static u32 _getIfmgmtIf(const olchar_t * pstrIfName, ifmgmt_if_t * pif)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t sock = -1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = OLERR_FAIL_CREATE_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memset(pif, 0, sizeof(*pif));
        ol_strncpy(
            pif->ii_strName, pstrIfName, MAX_IF_NAME_LEN);

        u32Ret = _getIfmgmtIfFlag(sock, pif);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pif->ii_bUp && pif->ii_bRunning)
        {
            if (u32Ret == OLERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFADDR, &pif->ii_iaAddr);

            if (u32Ret == OLERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFNETMASK, &pif->ii_iaNetmask);

            if (u32Ret == OLERR_NO_ERROR)
                u32Ret = _getIfmgmtIfAddr(
                    pstrIfName, sock, SIOCGIFBRDADDR, &pif->ii_iaBroadcast);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _getIfmgmtIfHwAddr(sock, pif);

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = OLERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 getAllIfmgmtIf(ifmgmt_if_t * pif, u32 * pu32NumOfIf)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    u32 u32If = 0;
    olfile_t * fp = NULL;
    olchar_t buf[1024], * pName;
    olsize_t sbyte;
    olchar_t strName[32];

    memset(pif, 0, sizeof(ifmgmt_if_t) * (*pu32NumOfIf));

    u32Ret = fpOpenFile(SYSTEM_NET_DEV_FILE, "r", &fp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        sbyte = sizeof(buf);
        fpReadLine(fp, buf, &sbyte);
        sbyte = sizeof(buf);
        fpReadLine(fp, buf, &sbyte);

        while (u32Ret == OLERR_NO_ERROR)
        {
            sbyte = sizeof(buf);
            u32Ret = fpReadLine(fp, buf, &sbyte);
            if (u32Ret == OLERR_NO_ERROR)
            {
                pName = buf;
                while ((*pName != ':') && (pName < buf + sbyte))
                    pName ++;

                if (*pName != ':')
                    continue;

                *pName = '\0';
                strncpy(strName, buf, sizeof(strName));
                removeLeadingSpace(strName);

                u32Ret = _getIfmgmtIf(strName, &pif[u32If]);
                if (u32Ret == OLERR_NO_ERROR)
                {
                    u32If ++;
                    if (u32If == *pu32NumOfIf)
                        break;
                }
                else
                {
                    /*ignore the error and goto the next line*/
                    u32Ret = OLERR_NO_ERROR;
                }
            }
        }

    }

    *pu32NumOfIf = u32If;
    if (u32Ret == OLERR_END_OF_FILE)
        u32Ret = OLERR_NO_ERROR;

#elif defined(WINDOWS)
    u32Ret = OLERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 getIfmgmtIf(const olchar_t * pstrIfName, ifmgmt_if_t * pif)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pif, 0, sizeof(ifmgmt_if_t));

    u32Ret = _getIfmgmtIf(pstrIfName, pif);

    return u32Ret;
}

/*port operation*/
u32 upIfmgmtIf(const olchar_t * pstrIfName)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    struct ifreq ifr;
    olint_t ret;
    olint_t sock = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = OLERR_FAIL_CREATE_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strcpy(ifr.ifr_name, pstrIfName);

        ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = OLERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ifr.ifr_flags & IFF_UP)
        {
            /*the interface is up already*/
            close(sock);
            return u32Ret;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ifr.ifr_flags |= IFF_UP;
        ifr.ifr_flags |= IFF_RUNNING;

        ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = OLERR_FAIL_IOCTL_SOCKET;
    }

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = OLERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}


u32 downIfmgmtIf(const olchar_t * pstrIfName)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    struct ifreq ifr;
    olint_t ret;
    olint_t sock = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        u32Ret = OLERR_FAIL_CREATE_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strcpy(ifr.ifr_name, pstrIfName);

        ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = OLERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (! (ifr.ifr_flags & IFF_UP))
        {
            /*the interface is already down*/
            close(sock);
            return u32Ret;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ifr.ifr_flags &= ~IFF_UP;
        ifr.ifr_flags &= ~IFF_RUNNING;

        ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
        if (ret != 0)
            u32Ret = OLERR_FAIL_IOCTL_SOCKET;
    }

    if (sock > 0)
        close(sock);

#elif defined(WINDOWS)
    u32Ret = OLERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 getStringIfmgmtIfFlags(olchar_t * pstrFlags, ifmgmt_if_t * pIf)
{
    u32 u32Ret = OLERR_NO_ERROR;

    pstrFlags[0] = '\0';

    if (pIf->ii_bUp)
        ol_strcat(pstrFlags, "Up");
    else
        ol_strcat(pstrFlags, "Down");

    if (pIf->ii_bBroadcast)
        ol_strcat(pstrFlags, ", Broadcast");

    if (pIf->ii_bPointopoint)
        ol_strcat(pstrFlags, ", Pointtopoint");

    if (pIf->ii_bLoopback)
        ol_strcat(pstrFlags, ", Loopback");

    if (pIf->ii_bRunning)
        ol_strcat(pstrFlags, ", Running");

    if (pIf->ii_bPromisc)
        ol_strcat(pstrFlags, ", Promisc");

    if (pIf->ii_bMulticast)
        ol_strcat(pstrFlags, ", Multicast");

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


