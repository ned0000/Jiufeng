/**
 *  @file servclient.h
 *
 *  @brief Header file for message client of dispather service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_SERVCLIENT_H
#define JIUFENG_SERVCLIENT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The parameter for creating dispatcher service client.
 */
typedef struct
{
    /**Max socket connection in async client socket for a service.*/
    u32 cdscp_u32MaxConnInClient;
    /**The directory containing the socket files.*/
    olchar_t * cdscp_pstrSocketDir;
} create_dispatcher_serv_client_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create dispatcher service client based on service config from file.
 */
u32 createDispatcherServClients(
    jf_linklist_t * pjlServConfig, create_dispatcher_serv_client_param_t * pcdscp);

u32 destroyDispatcherServClients(void);

u32 startDispatcherServClients(void);

u32 stopDispatcherServClients(void);

u32 dispatchMsgToServ(u8 * pu8Msg, olsize_t sMsg);

#endif /*JIUFENG_SERVCLIENT_H*/

/*------------------------------------------------------------------------------------------------*/


