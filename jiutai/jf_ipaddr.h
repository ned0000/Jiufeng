/**
 *  @file jf_addr.h
 *
 *  @brief Header file of network interface management library. Routines for
 *   IP address
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_ifmgmt library
 *  @note Link with olfiles and olstringparse library
 *
 */

/*--------------------------------------------------------------------------*/

#ifndef JIUFENG_IPADDR_H
#define JIUFENG_IPADDR_H

/* --- standard C lib header files ----------------------------------------- */
#if defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#elif defined(WINDOWS)
    #include <Iphlpapi.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"

#undef IFMGMTAPI
#undef IFMGMTCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_IFMGMT_DLL)
        #define IFMGMTAPI  __declspec(dllexport)
        #define IFMGMTCALL
    #else
        #define IFMGMTAPI
        #define IFMGMTCALL __cdecl
    #endif
#else
    #define IFMGMTAPI
    #define IFMGMTCALL
#endif

/* --- constant definitions ------------------------------------------------ */

#define JF_IPADDR_TYPE_V4               (0x0)

#define JF_IPADDR_TYPE_V6               (0x1)

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    u8 ji_u8AddrType;
    u8 ji_u8Reserved[7];
    union
    {
        olint_t ju_nAddr;
        u8 ju_u8Addr[16];
    } ji_uAddr;
} jf_ipaddr_t;

/* --- functional routines ------------------------------------------------- */

/*get local ip addr list, not include the loop back*/
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getLocalIpAddrList(
    u8 u8AddrType, jf_ipaddr_t * pAddr, u16 * pu16Count);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_convertSockAddrToIpAddr(
    const struct sockaddr * psa, const olint_t nSaLen,
    jf_ipaddr_t * pji, u16 * pu16Port);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_convertIpAddrToSockAddr(
    const jf_ipaddr_t * pji, const u16 u16Port,
    struct sockaddr * psa, olint_t * pnSaLen);

IFMGMTAPI void IFMGMTCALL jf_ipaddr_setIpV4Addr(
    jf_ipaddr_t * pji, olint_t nAddr);

IFMGMTAPI boolean_t IFMGMTCALL jf_ipaddr_isIpV4Addr(jf_ipaddr_t * pji);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpAddrToInaddrAny(jf_ipaddr_t * pji);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpV4AddrToInaddrAny(jf_ipaddr_t * pji);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpV6AddrToInaddrAny(jf_ipaddr_t * pji);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getStringIpAddr(
    olchar_t * pstrIp, const jf_ipaddr_t * pji);

IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getIpAddrFromString(
    const olchar_t * pstrIp, u8 u8AddrType, jf_ipaddr_t * pji);

/*the string is something like xxx.xxx.xxx.xxx:xxx*/
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getIpAddrPortFromString(
    const olchar_t * pstrIp, u8 u8AddrType, jf_ipaddr_t * pji, u16 * pu16Port);

#endif /*JIUFENG_IPADDR_H */

/*--------------------------------------------------------------------------*/

