/**
 *  @file logger/common.h
 *
 *  @brief Header file defines the internal common routine used by logger library.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_COMMON_H
#define JIUFENG_LOGGER_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Retry count for writing log to log location.
 */
#define JF_LOGGER_RETRY_COUNT                       (3)

/** Default caller name if it's not specified.
 */
#define JF_LOGGER_DEF_CALLER_NAME                   "NONAME"

/** The maximum common banner length.
 */
#define JF_LOGGER_MAX_COMMON_BANNER_LEN             (128)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the log location data type.
 */
typedef void   jf_logger_log_location_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Check if the error code is system error.
 *
 *  @param u32ErrCode [in] The error code.
 *
 *  @return The status.
 *  @retval TRUE The error code is system error.
 *  @retval FALSE The error code is not system error.
 */
boolean_t isSysErrorCode(u32 u32ErrCode);

/** Get common log banner.
 *
 *  @note
 *  -# The common banner's format is "<time stamp> [<caller name>:<threadid>] ".
 *  -# The size of the buffer should more than JF_LOGGER_MAX_COMMON_HEADER_LEN.
 *
 *  @param pstrCallerName [in] The caller name.
 *  @param pstrBanner [in] String buffer for the header.
 *  @param sBanner [in] The size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getCommonLogBanner(const olchar_t * pstrCallerName, olchar_t * pstrBanner, olsize_t sBanner);

/** Get time stamp string from the time.
 *
 *  @param u64Time [in] The time stamp.
 *  @param pstrStamp [out] The buffer for the time stamp string.
 *  @param sStamp [out] Size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
olsize_t getSaveLogTimeStamp(const u64 u64Time, olchar_t * pstrStamp, olsize_t sStamp);

/** Get log banner from save log message.
 *
 *  @note
 *  -# The routine is used by log server only.
 *  -# The banner's format is "<time stamp> source ".
 *
 *  @param u64Time [in] The time stamp.
 *  @param pstrSource [in] The log message source, it identifies the modules who send the log.
 *  @param pstrBanner [out] The buffer for the banner.
 *  @param sBanner [out] Size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getSaveLogBanner(
    const u64 u64Time, const olchar_t * pstrSource, olchar_t * pstrBanner, olsize_t sBanner);

#endif /*JIUFENG_LOGGER_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/
