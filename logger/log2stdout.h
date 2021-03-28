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

/** Log message to stdout.
 *
 *  @param pLocation [in] The log location object.
 *  @param bBanner [in] Log banner if it's TRUE.
 *  @param pstrLog [in] The log message.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 logToStdout(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog);

/** Save log to stdout.
 *
 *  @note
 *  -# The routine is used by log server only.
 *
 *  @param pLocation [in] The log location object.
 *  @param u64Time [in] The time stamp.
 *  @param pstrSource [in] The log message source, it identifies the modules who send the log.
 *  @param pstrLog [in] The log message.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 saveLogToStdout(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    const olchar_t * pstrLog);

/** Destroy the tty log location.
 *
 *  @param ppLocation [in/out] The log location to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroyStdoutLogLocation(jf_logger_log_location_t ** ppLocation);

/** Create the stdout log location.
 *
 *  @param pParam [in] The parameter for creating the log location.
 *  @param ppLocation [out] The log location created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createStdoutLogLocation(
    create_stdout_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

#endif /*JIUFENG_LOGGER_LOG2STDOUT_H*/

/*------------------------------------------------------------------------------------------------*/
