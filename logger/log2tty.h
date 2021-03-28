/**
 *  @file logger/log2tty.h
 *
 *  @brief Header file for logging to tty.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2TTY_H
#define JIUFENG_LOGGER_LOG2TTY_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for creating tty log location.
 */
typedef struct
{
    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * ctllp_pstrCallerName;

    /**The TTY file name.*/
    olchar_t * ctllp_pstrTtyFile;

} create_tty_log_location_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the tty log location.
 *
 *  @param pParam [in] The parameter for creating the log location.
 *  @param ppLocation [out] The log location created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createTtyLogLocation(
    create_tty_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

/** Destroy the tty log location.
 *
 *  @param ppLocation [in/out] The log location to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroyTtyLogLocation(jf_logger_log_location_t ** ppLocation);

/** Log message to tty.
 *
 *  @param pLocation [in] The log location object.
 *  @param bBanner [in] Log banner if it's TRUE.
 *  @param pstrLog [in] The log message.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 logToTty(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog);

/** Save log to tty.
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
u32 saveLogToTty(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    olchar_t * pstrLog);

#endif /*JIUFENG_LOGGER_LOG2TTY_H*/

/*------------------------------------------------------------------------------------------------*/
