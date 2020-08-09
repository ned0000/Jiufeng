/**
 *  @file logger/log2server.h
 *
 *  @brief Header file for logging to server.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2SERVER_H
#define JIUFENG_LOGGER_LOG2SERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_logger.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for creating server log location.
 */
typedef struct
{
    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * csllp_pstrCallerName;

    /**The address of the log server, only IPv4 is supported.*/
    olchar_t * csllp_pstrServerAddress;
    /**The port of the log server.*/
    u16 csllp_u16ServerPort;
    u16 csllp_u16Reserved[3];

} create_server_log_location_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createServerLogLocation(
    create_server_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

u32 destroyServerLogLocation(jf_logger_log_location_t ** ppLocation);

u32 logToServer(jf_logger_log_location_t * pLocation, olchar_t * pstrLog, olsize_t sLog);

#endif /*JIUFENG_LOGGER_LOG2SERVER_H*/

/*------------------------------------------------------------------------------------------------*/
