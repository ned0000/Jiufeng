/**
 *  @file ifmgmt.h
 *
 *  @brief Header file of network interface management library. Including
 *   physical port.
 *
 *  @author Min Zhang
 *
 *  @note Link with olfiles and olstringparse library
 *
 */

/*--------------------------------------------------------------------------*/

#ifndef JIUFENG_IFMGMT_H
#define JIUFENG_IFMGMT_H

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
#include "olbasic.h"
#include "errcode.h"

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
#define MAC_LEN                      (6)

/*maximum physical interface*/
#define MAX_IFMGMT_IF                (6)

/*maximum interface name length*/
#define MAX_IF_NAME_LEN              (16)
/*maximum interface ip address length*/
#define MAX_IF_IP_LEN                (16)
/*maximum interface mac length*/
#define IF_MAC_LEN                   (MAC_LEN)

#define IP_ADDR_TYPE_V4              (0x0)
#define IP_ADDR_TYPE_V6              (0x1)

#define IF_FLAGS_STR_SIZE            (128)

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    u8 ia_u8AddrType;
    u8 ia_u8Reserved[7];
    union
    {
        olint_t iu_nAddr;
        u8 iu_u8Addr[16];
    } ia_uAddr;
} ip_addr_t;

typedef struct ifmgmt_if
{
    boolean_t ii_bUp;
    boolean_t ii_bBroadcast;
    boolean_t ii_bPointopoint;
    boolean_t ii_bLoopback;
    boolean_t ii_bRunning;
    boolean_t ii_bPromisc;
    boolean_t ii_bMulticast;
    u8 ii_u8Reserved;

    olchar_t ii_strName[MAX_IF_NAME_LEN];

    ip_addr_t ii_iaAddr;
    ip_addr_t ii_iaNetmask;
    ip_addr_t ii_iaBroadcast;

    u8 ii_u8Mac[MAC_LEN];
    /*mac address is 6 byte long, to align the mac address*/
    u8 ii_u8Reserved2[2];
} ifmgmt_if_t;

/* --- functional routines ------------------------------------------------- */

/*get interface config*/
IFMGMTAPI u32 IFMGMTCALL getAllIfmgmtIf(ifmgmt_if_t * pif, u32 * pu32NumOfIf);

IFMGMTAPI u32 IFMGMTCALL getIfmgmtIf(
    const olchar_t * pstrIfName, ifmgmt_if_t * pif);

/*port operation*/
IFMGMTAPI u32 IFMGMTCALL upIfmgmtIf(const olchar_t * pstrIfName);

IFMGMTAPI u32 IFMGMTCALL downIfmgmtIf(const olchar_t * pstrIfName);

/*the size of string "pstrFlags" should be more than IF_FLAGS_STR_SIZE bytes*/
IFMGMTAPI u32 IFMGMTCALL getStringIfmgmtIfFlags(
    olchar_t * pstrFlags, ifmgmt_if_t * pIf);

/* ip addr */
/*get local ip addr list, not include the loop back*/
IFMGMTAPI u32 IFMGMTCALL getLocalIpAddrList(
    u8 u8AddrType, ip_addr_t * pAddr, u16 * pu16Count);

IFMGMTAPI u32 IFMGMTCALL getMacOfFirstIf(u8 u8Mac[MAC_LEN]);

IFMGMTAPI u32 IFMGMTCALL convertSockAddrToIpAddr(
    const struct sockaddr * psa, const olint_t nSaLen,
    ip_addr_t * pia, u16 * pu16Port);

IFMGMTAPI u32 IFMGMTCALL convertIpAddrToSockAddr(
    const ip_addr_t * pia, const u16 u16Port,
    struct sockaddr * psa, olint_t * pnSaLen);

IFMGMTAPI void IFMGMTCALL setIpV4Addr(ip_addr_t * pia, olint_t nAddr);

IFMGMTAPI boolean_t IFMGMTCALL isIpV4Addr(ip_addr_t * pia);

IFMGMTAPI u32 IFMGMTCALL setIpAddrToInaddrAny(ip_addr_t * pia);

IFMGMTAPI u32 IFMGMTCALL setIpV4AddrToInaddrAny(ip_addr_t * pia);

IFMGMTAPI u32 IFMGMTCALL setIpV6AddrToInaddrAny(ip_addr_t * pia);

IFMGMTAPI u32 IFMGMTCALL getStringIpAddr(
    olchar_t * pstrIp, const ip_addr_t * pia);

IFMGMTAPI u32 IFMGMTCALL getIpAddrFromString(
    const olchar_t * pstrIp, u8 u8AddrType, ip_addr_t * pia);

/*the string is something like xxx.xxx.xxx.xxx:xxx*/
IFMGMTAPI u32 IFMGMTCALL getIpAddrPortFromString(
    const olchar_t * pstrIp, u8 u8AddrType, ip_addr_t * pia, u16 * pu16Port);

#endif /*JIUFENG_IFMGMT_H */

/*--------------------------------------------------------------------------*/

