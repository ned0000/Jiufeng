/**
 *  @file logger/log2file.h
 *
 *  @brief Header file for logging to file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2FILE_H
#define JIUFENG_LOGGER_LOG2FILE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_logger.h"

#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for creating file log location.
 */
typedef struct
{
    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * cfllp_pstrCallerName;

    /**The log file name, the log file will be "callername.log" if it's not specified.*/
    olchar_t * cfllp_pstrLogFile;
    /**The size of the log file in byte. If 0, no limit.*/
    olsize_t cfllp_sLogFile;

} create_file_log_location_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 logToFile(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog);

/** Save log to file.
 *
 *  @note
 *  -# The routine is used by log server only.
 */
u32 saveLogToFile(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    olchar_t * pstrLog);

u32 destroyFileLogLocation(jf_logger_log_location_t ** ppLocation);

u32 createFileLogLocation(
    create_file_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation);

#endif /*JIUFENG_LOGGER_LOG2FILE_H*/

/*------------------------------------------------------------------------------------------------*/
