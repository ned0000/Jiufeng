/**
 *  @file webclient/dataobject.h
 *
 *  @brief Header file for webclient data object
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef WEBCLIENT_DATAOBJECT_H
#define WEBCLIENT_DATAOBJECT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_network.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef void  webclient_dataobject_pool_t;

typedef void  webclient_dataobject_t;

typedef void  webclient_request_t;

typedef struct
{
    /** number of connection in pool */
    u32 wdpcp_u32PoolSize;
    /** buffer size of the session */
    olsize_t wdpcp_sBuffer;
} webclient_dataobject_pool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createWebclientRequestSendData(
    webclient_dataobject_pool_t * pPool, webclient_request_t ** ppRequest, u8 ** ppu8Data,
    olsize_t * psData, u16 u16Num, jf_ipaddr_t * pjiRemote, u16 u16Port,
    jf_webclient_fnOnEvent_t fnOnEvent, void * user);

u32 createWebclientRequestDeleteRequest(
    webclient_dataobject_pool_t * pPool, webclient_request_t ** ppRequest, jf_ipaddr_t * pjiRemote,
    u16 u16Port);

u32 processWebclientRequest(webclient_dataobject_pool_t * pPool, webclient_request_t * request);

u32 destroyWebclientRequest(webclient_request_t ** ppRequest);

u32 destroyWebclientDataobjectPool(webclient_dataobject_pool_t ** ppPool);

u32 createWebclientDataobjectPool(
    jf_network_chain_t * pjnc, webclient_dataobject_pool_t ** ppPool,
    webclient_dataobject_pool_create_param_t * pwdpcp);

#endif /*WEBCLIENT_DATAOBJECT_H*/

/*------------------------------------------------------------------------------------------------*/


