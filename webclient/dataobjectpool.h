/**
 *  @file webclient/dataobjectpool.h
 *
 *  @brief Header file for webclient data object pool.
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

/** Define the webclient data object pool.
 */
typedef void  webclient_dataobject_pool_t;

/** Define the webclient data object.
 */
typedef void  webclient_dataobject_t;

typedef struct
{
    /**Number of connection in pool */
    u32 wdpcp_u32PoolSize;
    /**Buffer size of the session */
    olsize_t wdpcp_sBuffer;
} webclient_dataobject_pool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Process webclient request.
 *
 *  @param pPool [in] The webclient data object pool.
 *  @param piwr [in] The webclient request.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 processWebclientRequest(
    webclient_dataobject_pool_t * pPool, internal_webclient_request_t * piwr);

/** Destroy webclient data object pool.
 *
 *  @param ppPool [in/out] The webclient data object pool to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroyWebclientDataobjectPool(webclient_dataobject_pool_t ** ppPool);

/** Create webclient data object pool.
 *
 *  @param pjnc [in] The network chain.
 *  @param ppPool [out] The webclient data object pool to be created.
 *  @param pwdpcp [in] The parameter for the creation.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createWebclientDataobjectPool(
    jf_network_chain_t * pjnc, webclient_dataobject_pool_t ** ppPool,
    webclient_dataobject_pool_create_param_t * pwdpcp);

#endif /*WEBCLIENT_DATAOBJECT_POOL_H*/

/*------------------------------------------------------------------------------------------------*/
