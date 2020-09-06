/**
 *  @file jf_ipaddr.h
 *
 *  @brief Header file contains data structure and routines for IP address manipulation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_ifmgmt library.
 */

/*------------------------------------------------------------------------------------------------*/

#ifndef JIUFENG_IPADDR_H
#define JIUFENG_IPADDR_H

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(WINDOWS)
    #include <Iphlpapi.h>
#elif defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
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

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the address type.
 */
enum
{
    /**IPV4 address type.*/
    JF_IPADDR_TYPE_V4 = 0,
    /**IPV6 address type.*/
    JF_IPADDR_TYPE_V6,
    /**Unix domain socket address type.*/
    JF_IPADDR_TYPE_UDS,
};

/* --- data structures -------------------------------------------------------------------------- */

/** Define the IP address data type.
 */
typedef struct
{
    /**Address type.*/
    u8 ji_u8AddrType;
    u8 ji_u8Reserved[7];
    union
    {
        /**IPV4 address.*/
        olint_t ju_nAddr;
        /**IPV6 address.*/
        u8 ju_u8Addr[16];
        /**Unix domain socket.*/
        olchar_t ju_strPath[120];
    } ji_uAddr;
} jf_ipaddr_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get local ip address list.
 *
 *  @note
 *  -# Loop back address are excluded.
 *
 *  @param u8AddrType [in] The address type.
 *  @param pAddr [in] The address data type array.
 *  @param pu16Count [in/out] The array size as in parameter and number of address in array as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getLocalIpAddrList(
    u8 u8AddrType, jf_ipaddr_t * pAddr, u16 * pu16Count);

/** Convert socket address to ip address.
 *
 *  @param psa [in] The socket address.
 *  @param nSaLen [in] Size of the socket address.
 *  @param pji [out] The ip address returned.
 *  @param pu16Port [out] The port returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_IP_ADDR_TYPE Invalid IP address type.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_convertSockAddrToIpAddr(
    const struct sockaddr * psa, const olint_t nSaLen, jf_ipaddr_t * pji, u16 * pu16Port);

/** Convert ip address to socket address.
 *
 *  @param pji [in] The ip address.
 *  @param u16Port [in] The port.
 *  @param psa [out] The socket address.
 *  @param pnSaLen [in/out] Size of the socket address buffer as in parameter, size of socket
 *   address returned as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_convertIpAddrToSockAddr(
    const jf_ipaddr_t * pji, const u16 u16Port, struct sockaddr * psa, olint_t * pnSaLen);

/** Set the ip address to IPV4 address.
 *
 *  @param pji [in] The ip address.
 *  @param nAddr [in] The IPV4 ip address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI void IFMGMTCALL jf_ipaddr_setIpV4Addr(jf_ipaddr_t * pji, const olint_t nAddr);

/** Set the ip address to UDS address.
 *
 *  @param pji [in] The ip address.
 *  @param pstrPath [in] The unix domain socket path.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI void IFMGMTCALL jf_ipaddr_setUdsAddr(jf_ipaddr_t * pji, const olchar_t * pstrPath);

/** Check if the address type is IPV4.
 *
 *  @param pji [in] The ip address.
 *
 *  @return The check status.
 *  @retval TRUE The address type is IPV4.
 *  @retval FALSE The address type is not IPV4.
 */
IFMGMTAPI boolean_t IFMGMTCALL jf_ipaddr_isIpV4Addr(jf_ipaddr_t * pji);

/** Set the ip address to INADDR_ANY.
 *
 *  @param pji [in] The ip address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpAddrToInaddrAny(jf_ipaddr_t * pji);

/** Set the IPV4 address to INADDR_ANY.
 *
 *  @param pji [in] The ip address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpV4AddrToInaddrAny(jf_ipaddr_t * pji);

/** Set the IPV6 address to INADDR_ANY.
 *
 *  @param pji [in] The ip address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_setIpV6AddrToInaddrAny(jf_ipaddr_t * pji);

/** Get the string of IP address according to the IP address type.
 *
 *  @param pstrIp [out] The string buffer where the IP address string will return.
 *  @param sIp [in] Size of the string buffer.
 *  @param pji [in] The ip address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_IP_ADDR_TYPE Invalid address type.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getStringIpAddr(
    olchar_t * pstrIp, olsize_t sIp, const jf_ipaddr_t * pji);

/** Get the string of IP address and port.
 *
 *  @param pstrIp [out] The string buffer where the string will return.
 *  @param sIp [in] Size of the string buffer.
 *  @param pji [in] The ip address.
 *  @param u16Port [out] The port.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_IP_ADDR_TYPE Invalid address type.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getStringIpAddrPort(
    olchar_t * pstrIp, olsize_t sIp, const jf_ipaddr_t * pji, const u16 u16Port);

/** Get IP address from the string.
 *
 *  @param pstrIp [in] The string buffer.
 *  @param u8AddrType [in] The address type.
 *  @param pji [out] The ip address parsed from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_IP_ADDR_TYPE Invalid address type.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getIpAddrFromString(
    const olchar_t * pstrIp, u8 u8AddrType, jf_ipaddr_t * pji);

/** Get IP address and port from the string.
 *
 *  @note
 *  -# For IPV4, the string is something like xxx.xxx.xxx.xxx:xxx.
 *
 *  @param pstrIp [in] The string buffer.
 *  @param u8AddrType [in] The address type.
 *  @param pji [out] The ip address parsed from string.
 *  @param pu16Port [out] The port parsed from string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_IP Invalid IP address.
 *  @retval JF_ERR_INVALID_IP_ADDR_TYPE Invalid address type.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ipaddr_getIpAddrPortFromString(
    const olchar_t * pstrIp, u8 u8AddrType, jf_ipaddr_t * pji, u16 * pu16Port);

#endif /*JIUFENG_IPADDR_H */

/*------------------------------------------------------------------------------------------------*/
