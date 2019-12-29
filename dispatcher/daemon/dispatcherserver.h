/**
 *  @file dispatcherserver.h
 *
 *  @brief dispatcher message server header file
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef DISPATCHER_MSG_SERVER_H
#define DISPATCHER_MSG_SERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    dispatcher_serv_config_t dms_dscConfig;

    jf_network_assocket_t * dms_pjaServer;

    dispatcher_msg_id_table_t dms_dmitPushlishedMsgId;

    jf_listhead_t dms_jlList;
} dispatcher_msg_server_t;

typedef struct
{


} create_dispatcher_msg_server_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createDispatcherMsgServer(
    dispatcher_msg_server_t ** ppServer, create_dispatcher_msg_server_param_t * param);

u32 destroyDispatcherMsgServer(dispatcher_msg_server_t ** ppServer);

#endif /*DISPATCHER_MSG_SERVER_H*/

/*------------------------------------------------------------------------------------------------*/


