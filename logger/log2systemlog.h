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

/** Create the system log log location.
 *
 *  @param pParam [in] The parameter for creating the log location.
 *  @param ppLocation [out] The log location created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createSystemlogLogLocation(
    create_systemlog_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

/** Destroy the system log log location.
 *
 *  @param ppLocation [in/out] The log location to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroySystemlogLogLocation(jf_logger_log_location_t ** ppLocation);

/** Log message to system log.
 *
 *  @param pLocation [in] The log location object.
 *  @param u8LogLevel [in] The internal log level.
 *  @param bBanner [in] Log banner if it's TRUE.
 *  @param pstrLog [in] The log message.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 logToSystemlog(
    jf_logger_log_location_t * pLocation, u8 u8LogLevel, boolean_t bBanner, olchar_t * pstrLog);

#endif /*JIUFENG_LOGGER_LOG2SYSTEMLOG_H*/

/*------------------------------------------------------------------------------------------------*/
