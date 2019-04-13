/**
 *  @file jf_host.h
 *
 *  @brief get host information head file
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_host object
 *  @note Link with ifmgmt, stringparse library
 *  @note For windows, link with Iphlpapi.lib
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
#define JF_HOST_MAX_NET_NAME_LEN     (24)
#define JF_HOST_MAX_NET_INTERFACES   (16)
#define JF_HOST_MAX_IP_ADDR_LEN      (40)

#define JF_HOST_MAX_HOST_NAME_LEN    (32)
#define JF_HOST_MAX_OS_NAME_LEN      (256)

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
    olchar_t jhni_strName[JF_HOST_MAX_NET_NAME_LEN];	
    olchar_t jhni_strIpAddr[JF_HOST_MAX_IP_ADDR_LEN];
    u8 jhni_u8Reserved[32];
} jf_host_net_info_t;

typedef struct
{
    olchar_t jhi_strHostName[JF_HOST_MAX_HOST_NAME_LEN];	
    olchar_t jhi_strOSName[JF_HOST_MAX_OS_NAME_LEN];
    u8 jhi_u8Reserved[256];
    u16 jhi_u16Reserved[3];
    u16 jhi_u16NetCount;
    jf_host_net_info_t jhi_jhniNet[JF_HOST_MAX_NET_INTERFACES];
} jf_host_info_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get host info
 *
 *  @param pjhi [out] the pointer to the host_info_t struct
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_host_getInfo(jf_host_info_t * pjhi);

/** Get host name
 *
 *  @param pstrName [in/out] the pointer to the buffer to hold the name
 *  @param u32Len [in] the length of the buffer
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 */
void jf_host_getName(olchar_t * pstrName, u32 u32Len);

#endif /*JIUTAI_HOST_H*/

/*------------------------------------------------------------------------------------------------*/


 

