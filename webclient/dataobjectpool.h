/**
 *  @file webclient/dataobjectpool.h
 *
 *  @brief Header file for webclient data object pool
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef WEBCLIENT_DATAOBJECT_POOL_H
#define WEBCLIENT_DATAOBJECT_POOL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_network.h"
#include "webclientrequest.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef void  webclient_dataobject_pool_t;

typedef void  webclient_dataobject_t;

typedef struct
{
    /** number of connection in pool */
    u32 wdpcp_u32PoolSize;
    /** buffer size of the session */
    olsize_t wdpcp_sBuffer;
} webclient_dataobject_pool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 processWebclientRequest(
    webclient_dataobject_pool_t * pPool, internal_webclient_request_t * piwr);

u32 destroyWebclientDataobjectPool(webclient_dataobject_pool_t ** ppPool);

u32 createWebclientDataobjectPool(
    jf_network_chain_t * pjnc, webclient_dataobject_pool_t ** ppPool,
    webclient_dataobject_pool_create_param_t * pwdpcp);

#endif /*WEBCLIENT_DATAOBJECT_POOL_H*/

/*------------------------------------------------------------------------------------------------*/


