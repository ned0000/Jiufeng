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

u32 createTtyLogLocation(
    create_tty_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

u32 destroyTtyLogLocation(jf_logger_log_location_t ** ppLocation);

u32 logToTty(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog);

/** Save log to tty.
 *
 *  @note
 *  -# The routine is used by log server only.
 */
u32 saveLogToTty(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    olchar_t * pstrLog);

#endif /*JIUFENG_LOGGER_LOG2TTY_H*/

/*------------------------------------------------------------------------------------------------*/
