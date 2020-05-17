/**
 *  @file messagingserver.h
 *
 *  @brief Header file for dispatcher messaging server of messaging library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef DISPATCHER_MESSAGING_SERVER_H
#define DISPATCHER_MESSAGING_SERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_messaging.h"

#include "dispatchercommon.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The callback function to process the incoming message.
 */
typedef u32 (* fnProcessMessagingServerMsg_t)(u8 * pu8Msg, olsize_t sMsg);

/** The parameter for creating dispatcher service server.
 */
typedef struct
{
    /**Max socket connection in async server socket for a service.*/
    u32 cdmsp_u32MaxConnInServer;
    /**The directory containing the socket files.*/
    olchar_t * cdmsp_pstrSocketDir;
    /**The callback function to process the incoming message.*/
    jf_messaging_fnProcessMsg_t cdmsp_fnProcessMsg;
    /**The address to receive message from.*/
    olchar_t * cdmsp_pstrMessagingIn;
    /**The name of the application.*/
    olchar_t * cdmsp_pstrName;
    /**Maximum message size.*/
    olsize_t cdmsp_sMaxMsg;
    /**Maximum number of message.*/
    u32 cdmsp_u32MaxNumMsg;
} create_dispatcher_messaging_server_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service server based on service config from file.
 */
u32 createDispatcherMessagingServer(create_dispatcher_messaging_server_param_t * pcdmsp);

u32 destroyDispatcherMessagingServer(void);

u32 startDispatcherMessagingServer(void);

u32 stopDispatcherMessagingServer(void);

/** Add message to incoming message queue.
 */
u32 addDispatcherMessagingMsg(dispatcher_msg_t * pdm);

#endif /*DISPATCHER_MESSAGING_SERVER_H*/

/*------------------------------------------------------------------------------------------------*/


