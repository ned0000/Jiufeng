/**
 *  @file dispatcher/xfer/dispatcherxfer.h
 *
 *  @brief Header file defines the interface of xfer library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_dispatcher_xfer library.
 *  -# Link with jf_network library for the underlying socket.
 *  -# Link with jf_jiukun library for memory allocation.
 */

#ifndef DISPATCHER_XFER_H
#define DISPATCHER_XFER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_network.h"

#include "dispatchercommon.h"

#undef DISPATCHERXFERAPI
#undef DISPATCHERXFERCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_DISPATCHERXFER_DLL)
        #define DISPATCHERXFERAPI  __declspec(dllexport)
        #define DISPATCHERXFERCALL
    #else
        #define DISPATCHERXFERAPI
        #define DISPATCHERXFERCALL __cdecl
    #endif
#else
    #define DISPATCHERXFERAPI
    #define DISPATCHERXFERCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the dispatcher xfer data type.
 */
typedef void  dispatcher_xfer_t;

/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for creating dispatcher xfer data type.
 */
typedef struct
{
    /**Maximum message size.*/
    olsize_t dxcp_sMaxMsg;
    /**Maximum number of message.*/
    u32 dxcp_u32MaxNumMsg;
    /**The address of remote server.*/
    jf_ipaddr_t * dxcp_pjiRemote;
    /**The port of remote server.*/
    u16 dxcp_u16RemotePort;
    /**The name of the application.*/
    olchar_t * dxcp_pstrName;
} dispatcher_xfer_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a new dispatcher xfer.
 *
 *  @param pjnc [in] The chain to add this module to.
 *  @param ppXfer [out] The created dispatcher xfer.
 *  @param pdxcp [in] The parameters for creating dispatcher xfer.
 *
 *  @return The error code.
 */
DISPATCHERXFERAPI u32 DISPATCHERXFERCALL dispatcher_xfer_create(
    jf_network_chain_t * pjnc, dispatcher_xfer_t ** ppXfer, dispatcher_xfer_create_param_t * pdxcp);

/** Send message to remote server.
 *
 *  @note
 *  -# The data array is cloned in this funcion.
 *
 *  @param pXfer [in] The dispatcher xfer to queue the requests to.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *  @param pu8Msg [in] The message to send.
 *  @param sMsg [in] The message size.
 *  @param fnOnEvent [in] data reception handler.
 *  @param pUser [in] The user.
 *
 *  @return The error code.
 */
DISPATCHERXFERAPI u32 DISPATCHERXFERCALL dispatcher_xfer_sendMsg(
    dispatcher_xfer_t * pXfer, dispatcher_msg_t * pdm);

/** Clear the message queue for a specific remote server.
 *
 *  @param pXfer [in] The dispatcher xfer.
 *  @param pjiRemote [in] The address of remote server.
 *  @param u16Port [in] The port of remote server.
 *
 *  @return The error code.
 */
DISPATCHERXFERAPI u32 DISPATCHERXFERCALL dispatcher_xfer_clearMsgQueue(dispatcher_xfer_t * pXfer);

/** Destory dispatcher xfer.
 *
 *  @param ppXfer [in/out] The dispatcher xfer to free.
 */
DISPATCHERXFERAPI u32 DISPATCHERXFERCALL dispatcher_xfer_destroy(
    dispatcher_xfer_t ** ppXfer);

#endif /*DISPATCHER_XFER_H*/

/*------------------------------------------------------------------------------------------------*/


