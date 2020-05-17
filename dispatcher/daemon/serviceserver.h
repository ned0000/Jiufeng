/**
 *  @file serviceserver.h
 *
 *  @brief Header file for message server of dispather service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_SERVICE_SERVER_H
#define JIUFENG_SERVICE_SERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The callback function to queue the message for service.
 */
typedef u32 (* fnQueueServiceServerMsg_t)(u8 * pu8Msg, olsize_t sMsg);

/** The parameter for creating dispatcher service server.
 */
typedef struct
{
    /**Max socket connection in async server socket for a service.*/
    u32 cdssp_u32MaxConnInServer;
    /**The directory containing the socket files.*/
    olchar_t * cdssp_pstrSocketDir;
    /**The callback function to queue the message.*/
    fnQueueServiceServerMsg_t cdssp_fnQueueMsg;
} create_dispatcher_service_server_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service server based on service config from file.
 */
u32 createDispatcherServiceServers(
    jf_linklist_t * pjlServConfig, create_dispatcher_service_server_param_t * pcdssp);

u32 destroyDispatcherServiceServers(void);

u32 startDispatcherServiceServers(void);

u32 stopDispatcherServiceServers(void);

#endif /*JIUFENG_SERVICE_SERVER_H*/

/*------------------------------------------------------------------------------------------------*/


