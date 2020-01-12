/**
 *  @file messagingclient.h
 *
 *  @brief Header file for dispatcher messaging client of messaging service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef DISPATCHER_MESSAGING_CLIENT_H
#define DISPATCHER_MESSAGING_CLIENT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_messaging.h"

#include "dispatchercommon.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The parameter for creating dispatcher messaging client.
 */
typedef struct
{
    /**The directory containing the socket files.*/
    olchar_t * cdmcp_pstrSocketDir;
    /**The address to send message to.*/
    olchar_t * cdmcp_pstrMessagingOut;
    /**The name of the application.*/
    olchar_t * cdmcp_pstrName;
    /**Maximum message size.*/
    olsize_t cdmcp_sMaxMsg;
    /**Maximum number of message.*/
    u32 cdmcp_u32MaxNumMsg;

} create_dispatcher_messaging_client_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service client based on service config from file.
 */
u32 createDispatcherMessagingClient(create_dispatcher_messaging_client_param_t * pcdmcp);

u32 destroyDispatcherMessagingClient(void);

u32 startDispatcherMessagingClient(void);

u32 stopDispatcherMessagingClient(void);

u32 sendDispatcherMessagingMsg(dispatcher_msg_t * pdm);

#endif /*DISPATCHER_MESSAGING_CLIENT_H*/

/*------------------------------------------------------------------------------------------------*/


