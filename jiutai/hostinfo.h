/**
 *  @file hostinfo.h
 *
 *  @brief get host information head file
 *
 *  @author Min Zhang
 *
 *  @note
 *  - link with network, stringparse library
 *  - For windows, link with Iphlpapi.lib
 *  
 */

#ifndef JIUTAI_HOSTINFO_H
#define JIUTAI_HOSTINFO_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "ollimit.h"

/* --- constant definitions ------------------------------------------------ */
#define MAX_NET_NAME_LEN     (24)
#define MAX_NET_INTERFACES   (16)
#define MAX_IP_ADDR_LEN      (40)

#define MAX_HOST_NAME_LEN    (32)
#define MAX_OS_NAME_LEN      (256)

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olchar_t ni_strName[MAX_NET_NAME_LEN];	
    olchar_t ni_strIpAddr[MAX_IP_ADDR_LEN];
    u8 ni_u8Reserved[32];
} net_info_t;

typedef struct
{
    olchar_t hi_strHostName[MAX_HOST_NAME_LEN];	
    olchar_t hi_strOSName[MAX_OS_NAME_LEN];
    u8 hi_u8Reserved[256];
    u16 hi_u16Reserved[3];
    u16 hi_u16NetCount;
    net_info_t hi_niNet[MAX_NET_INTERFACES];
} host_info_t;

/* --- functional routines ------------------------------------------------- */

/** get host info
 *
 *  @Param:    
 *       [out] phi, the pointer to the host_info_t struct
 *  @Return: PIERR_NO_ERROR: succeed, otherwise: fail.
 *
 */
u32 getHostInfo(host_info_t * phi);

void getHostName(olchar_t * pstrName, u32 u32Len);

#endif /*JIUTAI_HOSTINFO_H*/

/*---------------------------------------------------------------------------*/


 

