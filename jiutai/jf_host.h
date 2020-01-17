/**
 *  @file jf_host.h
 *
 *  @brief Header file defines the interface for host routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_host object.
 *  -# Link with jf_ifmgmt, jf_string library.
 *  -# For windows, link with Iphlpapi.lib.
 *  
 */

#ifndef JIUTAI_HOST_H
#define JIUTAI_HOST_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_limit.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum net name length.
 */
#define JF_HOST_MAX_NET_NAME_LEN     (24)

/** Maximum number of net interface.
 */
#define JF_HOST_MAX_NET_INTERFACES   (16)

/** Maximum IP address length.
 */
#define JF_HOST_MAX_IP_ADDR_LEN      (40)

/** Maximum host name length.
 */
#define JF_HOST_MAX_HOST_NAME_LEN    (32)

/** Maximum OS name length.
 */
#define JF_HOST_MAX_OS_NAME_LEN      (256)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the net information data type.
 */
typedef struct
{
    /**The net name.*/
    olchar_t jhni_strName[JF_HOST_MAX_NET_NAME_LEN];	
    /**The IP address.*/
    olchar_t jhni_strIpAddr[JF_HOST_MAX_IP_ADDR_LEN];
    u8 jhni_u8Reserved[32];
} jf_host_net_info_t;

/** Define the host information data type.
 */
typedef struct
{
    /**The host name.*/
    olchar_t jhi_strHostName[JF_HOST_MAX_HOST_NAME_LEN];	
    /**The OS name.*/
    olchar_t jhi_strOSName[JF_HOST_MAX_OS_NAME_LEN];
    u8 jhi_u8Reserved[256];
    u16 jhi_u16Reserved[3];
    /**The number of net interface.*/
    u16 jhi_u16NetCount;
    /**The net interface list.*/
    jf_host_net_info_t jhi_jhniNet[JF_HOST_MAX_NET_INTERFACES];
} jf_host_info_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get host info.
 *
 *  @param pjhi [out] The pointer to the host_info_t struct.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_host_getInfo(jf_host_info_t * pjhi);

/** Get host name.
 *
 *  @param pstrName [in/out] The pointer to the buffer to hold the name.
 *  @param u32Len [in] The length of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
void jf_host_getName(olchar_t * pstrName, u32 u32Len);

#endif /*JIUTAI_HOST_H*/

/*------------------------------------------------------------------------------------------------*/


 

