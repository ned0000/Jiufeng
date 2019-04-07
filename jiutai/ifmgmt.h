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

/*maximum physical interface*/
#define JF_IFMGMT_MAX_IF                (6)

/*maximum interface name length*/
#define JF_IFMGMT_MAX_IF_NAME_LEN       (16)

/*maximum interface ip address length*/
#define JF_IFMGMT_MAX_IF_IP_LEN         (16)

/*maximum interface mac length*/
#define JF_IFMGMT_MAC_LEN               (MAC_LEN)

#define JF_IPADDR_TYPE_V4               (0x0)

#define JF_IPADDR_TYPE_V6               (0x1)

#define JF_IFMGMT_IF_FLAGS_STR_SIZE     (128)

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

typedef struct jf_ifmgmt_if
{
    boolean_t jii_bUp;
    boolean_t jii_bBroadcast;
    boolean_t jii_bPointopoint;
    boolean_t jii_bLoopback;
    boolean_t jii_bRunning;
    boolean_t jii_bPromisc;
    boolean_t jii_bMulticast;
    u8 jii_u8Reserved;

    olchar_t jii_strName[JF_IFMGMT_MAX_IF_NAME_LEN];

    jf_ipaddr_t jii_jiAddr;
    jf_ipaddr_t jii_jiNetmask;
    jf_ipaddr_t jii_jiBroadcast;

    u8 jii_u8Mac[JF_LIMIT_MAC_LEN];
    /*mac address is 6 byte long, to align the mac address*/
    u8 jii_u8Reserved2[2];
} jf_ifmgmt_if_t;

/* --- functional routines ------------------------------------------------- */

/*get interface config*/
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getAllIf(
    jf_ifmgmt_if_t * pjii, u32 * pu32NumOfIf);

IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getIf(
    const olchar_t * pstrIfName, jf_ifmgmt_if_t * pjii);

/*port operation*/
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_upIf(const olchar_t * pstrIfName);

IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_downIf(const olchar_t * pstrIfName);

/*the size of string "pstrFlags" should be more than IF_FLAGS_STR_SIZE bytes*/
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getStringIfFlags(
    olchar_t * pstrFlags, jf_ifmgmt_if_t * pjii);

IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getMacOfFirstIf(u8 u8Mac[JF_LIMIT_MAC_LEN]);

/* ip addr */
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

#endif /*JIUFENG_IFMGMT_H */

/*--------------------------------------------------------------------------*/

