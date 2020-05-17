/**
 *  @file xfer/xferpool.h
 *
 *  @brief Header file for webclient data object pool.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef DISPATCHER_XFER_OBJECT_POOL_H
#define DISPATCHER_XFER_OBJECT_POOL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#include "dispatcherxfer.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the xfer object pool data type.
 */
typedef void  dispatcher_xfer_object_pool_t;

typedef void  dispatcher_xfer_object_t;

/** The possible value for event in function fnOnDispatcherXferObjectEvent_t.
 */
typedef enum dispatcher_xfer_object_event
{
    /*Unknown xfer event type.*/
    DISPATCHER_XFER_OBJECT_EVENT_UNKNOWN = 0,
    /**The message is sent successfully.*/
    DISPATCHER_XFER_OBJECT_EVENT_MSG_SENT,
} dispatcher_xfer_object_event_t;

/** Callback function for dispatcher xfer object event.
 */
typedef u32 (* fnOnDispatcherXferObjectEvent_t)(
    dispatcher_xfer_object_event_t event, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser);

/** The parameter for creating the dispatcher xfer pool.
 */
typedef struct
{
    /**Maximum message size.*/
    olsize_t dxpcp_sMaxMsg;
    /**Maximum number of message.*/
    u32 dxpcp_u32MaxNumMsg;

    /**Maximum address.*/
    u32 dxpcp_u32MaxAddress;
    /**The address of remote server, the remote server may be have several address.*/
    jf_ipaddr_t * dxpcp_pjiRemote[DISPATCHER_XFER_MAX_NUM_OF_ADDRESS];
    /**The port of remote server.*/
    u16 dxpcp_u16RemotePort[DISPATCHER_XFER_MAX_NUM_OF_ADDRESS];

    /**The name of the application.*/
    olchar_t * dxpcp_pstrName;
} dispatcher_xfer_pool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Destroy dispatcher xfer object pool.
 */
u32 destroyDispatcherXferObjectPool(dispatcher_xfer_object_pool_t ** ppPool);

/** Create dispatcher xfer object pool.
 */
u32 createDispatcherXferObjectPool(
    jf_network_chain_t * pjnc, dispatcher_xfer_object_pool_t ** ppPool,
    dispatcher_xfer_pool_create_param_t * pdxpcp);

/** Send dispatcher xfer pool message.
 */
u32 sendMsgByDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool, dispatcher_msg_t * pdm);

/** Pause dispatcher xfer object pool
 */
u32 pauseDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool);

/** Resume dispatcher xfer object pool
 */
u32 resumeDispatcherXferObjectPool(dispatcher_xfer_object_pool_t * pPool);


#endif /*DISPATCHER_XFER_OBJECT_POOL_H*/

/*------------------------------------------------------------------------------------------------*/


