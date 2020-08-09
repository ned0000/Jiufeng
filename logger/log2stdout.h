/**
 *  @file logger/log2stdout.h
 *
 *  @brief Header file for logging to stdout.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2STDOUT_H
#define JIUFENG_LOGGER_LOG2STDOUT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for creating stdout log location.
 */
typedef struct
{
    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * csllp_pstrCallerName;

    u8 csllp_u8Reserved[32];

} create_stdout_log_location_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 logToStdout(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog);

/** Save log to stdout.
 *
 *  @note
 *  -# The routine is used by log server only.
 */
u32 saveLogToStdout(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    const olchar_t * pstrLog);

u32 destroyStdoutLogLocation(jf_logger_log_location_t ** ppLocation);

u32 createStdoutLogLocation(
    create_stdout_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

#endif /*JIUFENG_LOGGER_LOG2STDOUT_H*/

/*------------------------------------------------------------------------------------------------*/
