/**
 *  @file servserver.h
 *
 *  @brief Header file for message server of dispather service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_SERVSERVER_H
#define JIUFENG_SERVSERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The callback function to queue the message for service.
 */
typedef u32 (* fnQueueServServerMsg_t)(u8 * pu8Msg, olsize_t sMsg);

/** The parameter for creating dispatcher service server.
 */
typedef struct
{
    /**Max socket connection in async server socket for a service.*/
    u32 cdssp_u32MaxConnInServer;
    /**The directory containing the socket files.*/
    olchar_t * cdssp_pstrSocketDir;
    /**The callback function to queue the message.*/
    fnQueueServServerMsg_t cdssp_fnQueueMsg;
} create_dispatcher_serv_server_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service server based on service config from file.
 */
u32 createDispatcherServServers(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_server_param_t * pcdssp);

u32 destroyDispatcherServServers(void);

u32 startDispatcherServServers(void);

u32 stopDispatcherServServers(void);

#endif /*JIUFENG_SERVSERVER_H*/

/*------------------------------------------------------------------------------------------------*/


