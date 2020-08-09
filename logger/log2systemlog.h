/**
 *  @file logger/log2systemlog.h
 *
 *  @brief Header file for logging to system log.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2SYSTEMLOG_H
#define JIUFENG_LOGGER_LOG2SYSTEMLOG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for creating systemlog log location.
 */
typedef struct
{
    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * csllp_pstrCallerName;

    u8 csllp_u8Reserved[32];

} create_systemlog_log_location_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 createSystemlogLogLocation(
    create_systemlog_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

u32 destroySystemlogLogLocation(jf_logger_log_location_t ** ppLocation);

u32 logToSystemlog(
    jf_logger_log_location_t * pLocation, u8 u8LogLevel, boolean_t bBanner, olchar_t * pstrLog);

#endif /*JIUFENG_LOGGER_LOG2SYSTEMLOG_H*/

/*------------------------------------------------------------------------------------------------*/
