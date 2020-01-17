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
    /**Number of object in pool.*/
    u32 dxpcp_u32PoolSize;
    /**Buffer size of the object.*/
    olsize_t dxpcp_sBuffer;
    /**The address of remote server.*/
    jf_ipaddr_t * dxpcp_pjiRemote;
    /**The port of remote server.*/
    u16 dxpcp_u16RemotePort;
    u16 dxpcp_u16Reserved[3];
    /**The name of the application.*/
    olchar_t * dxpcp_pstrName;
    /**Callback function to process the dispather xfer object event.*/
    fnOnDispatcherXferObjectEvent_t dxpcp_fnOnEvent;
    /**The argument of the callback function.*/
    void * dxpcp_pUser;
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
u32 sendDispatcherXferPoolMsg(dispatcher_xfer_object_pool_t * pPool, dispatcher_msg_t * pdm);

#endif /*DISPATCHER_XFER_OBJECT_POOL_H*/

/*------------------------------------------------------------------------------------------------*/


