/**
 *  @file jf_ifmgmt.h
 *
 *  @brief Header file defines network interface management function.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_ifmgmt library.
 *  =# Link with olfiles and olstringparse library.
 *
 */

/*------------------------------------------------------------------------------------------------*/

#ifndef JIUFENG_IFMGMT_H
#define JIUFENG_IFMGMT_H

/* --- standard C lib header files -------------------------------------------------------------- */
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

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_ipaddr.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum physical interface.
 */
#define JF_IFMGMT_MAX_IF                (6)

/** Maximum interface name length.
 */
#define JF_IFMGMT_MAX_IF_NAME_LEN       (16)

/** Maximum interface ip address length
 */
#define JF_IFMGMT_MAX_IF_IP_LEN         (16)

/** Maximum interface mac length.
 */
#define JF_IFMGMT_MAC_LEN               (JF_LIMIT_MAC_LEN)

/** String size for interface flag.
 */
#define JF_IFMGMT_IF_FLAGS_STR_SIZE     (128)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the network interface type.
 */
typedef struct jf_ifmgmt_if
{
    /**The interface is up if it's TRUE.*/
    boolean_t jii_bUp;
    /**The address is broadcast if it's TRUE.*/
    boolean_t jii_bBroadcast;
    /**The interface is in point-to-point mode if it's TRUE.*/
    boolean_t jii_bPointToPoint;
    /**It's a loopback interface if it's TRUE.*/
    boolean_t jii_bLoopback;
    /**The interface is running if it's TRUE.*/
    boolean_t jii_bRunning;
    /**The interface is in promisc mode if it's TRUE.*/
    boolean_t jii_bPromisc;
    /**The address is multicast if it's TRUE.*/
    boolean_t jii_bMulticast;
    u8 jii_u8Reserved;
    /**The interface name.*/
    olchar_t jii_strName[JF_IFMGMT_MAX_IF_NAME_LEN];
    /**The address of the interface.*/
    jf_ipaddr_t jii_jiAddr;
    /**The net mask of the interface address.*/
    jf_ipaddr_t jii_jiNetmask;
    /**The broadcast address of the interface.*/
    jf_ipaddr_t jii_jiBroadcast;
    /**The MAC address.*/
    u8 jii_u8Mac[JF_LIMIT_MAC_LEN];
    /*mac address is 6 byte long, to align the mac address*/
    u8 jii_u8Reserved2[2];
} jf_ifmgmt_if_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get all interfaces' configuration.
 *
 *  @param pjii [out] The interface configuration array.
 *  @param pu32NumOfIf [in/out] The array size as in parameter, number of interface returned as out
 *   parameter.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getAllIf(jf_ifmgmt_if_t * pjii, u32 * pu32NumOfIf);

/** Get a specified interface's configuration.
 *
 *  @param pstrIfName [in] The name of the interface.
 *  @param pjii [out] The interface configuration.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getIf(const olchar_t * pstrIfName, jf_ifmgmt_if_t * pjii);

/** Up an interface.
 *
 *  @param pstrIfName [in] The name of the interface.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_upIf(const olchar_t * pstrIfName);

/** Down an interface.
 *
 *  @param pstrIfName [in] The name of the interface.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_downIf(const olchar_t * pstrIfName);

/** Get string for interface flags.
 *
 *  @note
 *  -# The size of string buffer should be more than IF_FLAGS_STR_SIZE bytes.
 *
 *  @param pstrFlags [out] The string buffer.
 *  @param pjii [in] The interface.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getStringIfFlags(olchar_t * pstrFlags, jf_ifmgmt_if_t * pjii);

/** Get MAC address of the first interface.
 *
 *  @param u8Mac [out] The buffer for MAC address.
 *
 *  @return The error code.
 */
IFMGMTAPI u32 IFMGMTCALL jf_ifmgmt_getMacOfFirstIf(u8 u8Mac[JF_LIMIT_MAC_LEN]);

#endif /*JIUFENG_IFMGMT_H */

/*------------------------------------------------------------------------------------------------*/
