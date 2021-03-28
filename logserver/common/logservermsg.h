/**
 *  @file logserver/common/logservermsg.h
 *
 *  @brief Message definition, the message is used between log control utility and daemon.
 *
 *  @author Min Zhang
 *
 */

#ifndef LOG_SERVER_MSG_H
#define LOG_SERVER_MSG_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

#include "log2servermsg.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define log server message id.
 */
typedef enum
{
    /*Get setting request.*/
    LSMI_GET_SETTING_REQ = 100,
    /*Get setting response.*/
    LSMI_GET_SETTING_RESP,
    LSMI_MAX,
} log_server_msg_id_t;

/** Define the get setting request message.
 */
typedef struct
{
    /*Message header.*/
    log_2_server_msg_header_t lsgsr_l2smhHeader;

    u32 lsgsr_u32Reserved[8];
} log_server_get_setting_req_t;

/** Define the get setting response message.
 */
typedef struct
{
    /*Message header.*/
    log_2_server_msg_header_t lsgsr_l2smhHeader;

    u32 lsgsr_u32Reserved[8];
} log_server_get_setting_resp_t;

/* --- functional routines ---------------------------------------------------------------------- */


#endif /*LOG_SERVER_MSG_H*/

/*------------------------------------------------------------------------------------------------*/
