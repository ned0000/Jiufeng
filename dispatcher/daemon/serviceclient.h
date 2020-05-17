/**
 *  @file serviceclient.h
 *
 *  @brief Header file for message client of dispather service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_SERVICE_CLIENT_H
#define JIUFENG_SERVICE_CLIENT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"

#include "dispatchercommon.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The parameter for creating dispatcher service client.
 */
typedef struct
{
    /**The directory containing the socket files.*/
    olchar_t * cdscp_pstrSocketDir;
} create_dispatcher_service_client_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service client based on service config from file.
 */
u32 createDispatcherServiceClients(
    jf_linklist_t * pjlServConfig, create_dispatcher_service_client_param_t * pcdscp);

/** Destroy dispatcher service client.
 */
u32 destroyDispatcherServiceClients(void);

/** Start dispatcher service client.
 */
u32 startDispatcherServiceClients(void);

/** Stop dispatcher service client.
 */
u32 stopDispatcherServiceClients(void);

/** Pause dispatcher service client, the client will stop sending out the message.
 */
u32 pauseDispatcherServiceClient(u32 servId);

/** Resume dispatcher service client, the client will start sending out the message.
 */
u32 resumeDispatcherServiceClient(u32 servId);

/** Dispatch message to service.
 */
u32 dispatchMsgToServiceClients(dispatcher_msg_t * pdm);

#endif /*JIUFENG_SERVICE_CLIENT_H*/

/*------------------------------------------------------------------------------------------------*/


