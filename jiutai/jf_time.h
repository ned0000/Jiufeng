/**
 *  @file jf_time.h
 *
 *  @brief Header file which defines time routine.
 *
 *  @author Min Zhang
 *  
 *  @note
 *  -# Routines declared in this file are included in jf_time object.
 *
 */

#ifndef JIUTAI_TIME_H
#define JIUTAI_TIME_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include <time.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

#if defined(LINUX)
/** Define the INFINITE macro.
 */
    #define INFINITE                                   (0xFFFFFFFF)
#elif defined(WINDOWS)
/** Define the clock id data type.
 */
    typedef olint_t                                    clockid_t;
#endif

enum jf_time_clock_t
{
#if defined(LINUX)
    /**Real time clock id.*/
    JF_TIME_CLOCK_REALTIME = CLOCK_REALTIME,
    /**Monotonic clock id.*/
    JF_TIME_CLOCK_MONOTONIC = CLOCK_MONOTONIC,
    /**Process cputime id.*/
    JF_TIME_CLOCK_PROCESS_CPUTIME_ID = CLOCK_PROCESS_CPUTIME_ID,
    /**Thread cputime id.*/
    JF_TIME_CLOCK_THREAD_CPUTIME_ID = CLOCK_THREAD_CPUTIME_ID,
    /**Monotonic raw clock id.*/
    JF_TIME_CLOCK_MONOTONIC_RAW = CLOCK_MONOTONIC_RAW,
#elif defined(WINDOWS)
    /**Real time clock id.*/
    JF_TIME_CLOCK_REALTIME = 0,
    /**Monotonic clock id.*/
    JF_TIME_CLOCK_MONOTONIC,
    /**Process cputime id.*/
    JF_TIME_CLOCK_PROCESS_CPUTIME_ID,
    /**Thread cputime id.*/
    JF_TIME_CLOCK_THREAD_CPUTIME_ID,
    /**Monotonic raw clock id.*/
    JF_TIME_CLOCK_MONOTONIC_RAW,
#endif
};

/** Second to milli-second.
 */
#define JF_TIME_SECOND_TO_MILLISECOND              (1000)

/** Second to micro-second.
 */
#define JF_TIME_SECOND_TO_MICROSECOND              (1000000)

/** Milli-second to nano-second.
 */
#define JF_TIME_MILLISECOND_TO_NANOSECOND          (1000000)

/** Define the time spec data type.
 */
typedef struct jf_time_spec
{
    /**Second.*/
    u64 jts_u64Second;
    /**Nanosecond.*/
    u64 jts_u64NanoSecond;
} jf_time_spec_t;

/** Define the time val data type.
 */
typedef struct jf_time_val
{
    /**Second.*/
    u64 jtv_u64Second;
    /**Microsecond.*/
    u64 jtv_u64MicroSecond;
} jf_time_val_t;

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

#if defined(WINDOWS)
/** Convert the file time to seconds since Epoch.
 *  
 *  @param pTime [out] The file time.
 *
 *  @return The seconds since 1970, Epoch.
 */
u64 jf_time_fileTimeToSecondsSince1970(FILETIME * pTime);
#endif

/** Get time of day.
 *  
 *  @param pjtv [out] The time value in second and microsecond since Epoch.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getTimeOfDay(jf_time_val_t * pjtv);

/** Get clock time.
 *
 *  @param clkid [in] Identifier of the particular clock.
 *  @param pjts [out] The time spec data structures.
 *
 *  @return The error code
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getClockTime(clockid_t clkid, jf_time_spec_t * pjts);

/** Sleep some time in seconds.
 *
 *  @param u32Seconds [in] The time to sleep.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_sleep(u32 u32Seconds);

/** Sleep some time in milliseconds.
 *  
 *  @param u32MilliSeconds [in] The time to sleep.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_milliSleep(u32 u32MilliSeconds);

/** Sleep some time in microseconds.
 *  
 *  @param u32MicroSeconds [in] The time to sleep.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_microSleep(u32 u32MicroSeconds);

/** Sleep some time in nanoseconds.
 *  
 *  @param u32NanoSeconds [in] The time to sleep.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_nanoSleep(u32 u32NanoSeconds);

/** Convert time in hour:min:sec to seconds.
 *
 *  @param hour [in] The hour of the time.
 *  @param min [in] The minute of the time.
 *  @param sec [in] The second of the time.
 *
 *  @return The second.
 */
olint_t jf_time_convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec);

/** Get the string of time period in the format of "[# hr] [# min] [# sec]".
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big
 *   enough to avoid memory access violation.
 *
 *  @par Example
 *  <table>
 *  <tr><th>Time Period             <th>String                        </tr>
 *  <tr><td>0                       <td>0 sec                         </tr>
 *  <tr><td>1                       <td>1 sec                         </tr>
 *  <tr><td>60                      <td>1 min                         </tr>
 *  <tr><td>61                      <td>1 min, 1 sec                  </tr>
 *  <tr><td>600                     <td>10 min                        </tr>
 *  <tr><td>601                     <td>10 min, 1 sec                 </tr>
 *  <tr><td>86400                   <td>24 hr                         </tr>
 *  <tr><td>86401                   <td>24 hr, 1 sec                  </tr>
 *  <tr><td>87000                   <td>24 hr, 10 min                 </tr>
 *  <tr><td>87001                   <td>24 hr, 10 min, 1 sec          </tr>
 *  </table>
 *
 *  @param pstrTime [out] The string buffer where the period string will return.
 *  @param sStrTime [in] The size of the buffer.
 *  @param u32Period [in] The time.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getStringTimePeriod(olchar_t * pstrTime, olsize_t sStrTime, const u32 u32Period);

/** Get the time from the string with the format "hour:minute:second" like 15:23:58.
 *
 *  @param pstrTimeString [in] The string to parse.
 *  @param pHour [out] The hour.
 *  @param pMin [out] The minute.
 *  @param pSec [out] The second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin, olint_t * pSec);

/** Get the string of time in the format "hour:minute:second" like 15:03:08.
 *
 *  @param pstrTime [out] The string buffer where the time string will return.
 *  @param sTime [in] Size of the string buffer. 
 *  @param hour [in] The year.
 *  @param min [in] The month of the year.
 *  @param sec [in] The day of the month.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getStringTime(
    olchar_t * pstrTime, olsize_t sTime, const olint_t hour, const olint_t min, const olint_t sec);

/** Get monotonic raw time in second.
 *
 *  @note
 *  -# The monotonic time is from unspecified starting point, it's a relative time.
 *  -# The time is not affected by discontinuous jumps in the system time (Eg. system administrator
 *   manually changes the clock), and not affected by incremental adjustments performed by adjtime.
 *
 *  @param pu64Second [out] The monotonic raw time in second.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getMonotonicRawTimeInSecond(u64 * pu64Second);

/** Get UTC time in second since the Epoch, 1970-01-01 00:00:00 +0000 (UTC) from broken-down time
 *  and date.
 *
 *  @note
 *  -# The broken-down time and date are local time.
 *
 *  @param pu64Sec [out] The UTC time in second.
 *  @param hour [in] The hour of the time.
 *  @param min [in] The minute of the time.
 *  @param sec [in] The second of the time.
 *  @param year [in] The year of the date.
 *  @param mon [in] The month of the date.
 *  @param day [in] The day of the date.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getUtcTimeInSecondFromTimeDate(
    u64 * pu64Sec, const olint_t hour, const olint_t min, const olint_t sec,
    const olint_t year, const olint_t mon, const olint_t day);

/** Get current UTC time in second since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 *
 *  @param pu64Sec [out] The UTC time in second.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getUtcTimeInSecond(u64 * pu64Sec);

/** Get current UTC time in micro-second since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 *
 *  @param pu64MicroSec [out] The UTC time in micro-second.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getUtcTimeInMicroSecond(u64 * pu64MicroSec);

/** Get UTC time in second of next day based on the given UTC time.
 *
 *  @par Example
 *  @code
 *  UTC Second : 1551348626
 *  Local Time : Thu Feb 28 18:10:26 2019
 *  UTC Time   : Thu Feb 28 10:10:26 2019
 *
 *  Next Day
 *  UTC Second : 1551435026
 *  Local Time : Fri Mar  1 18:10:26 2019
 *  UTC Time   : Fri Mar  1 10:10:26 2019
 *  @endcode
 *
 *  @param u64Sec [in] The UTC time in second.
 *  @param pu64Next [out] The UTC time in second of next day.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getUtcTimeInSecondOfNextDay(const u64 u64Sec, u64 * pu64Next);

/** Get UTC time in second of next week based on the given UTC time.
 *
 *  @par Example
 *  @code
 *  UTC Second : 1580465426
 *  Local Time : Fri Jan 31 18:10:26 2020
 *  UTC Time   : Fri Jan 31 10:10:26 2020
 *
 *  Next Week
 *  UTC Second : 1581070226
 *  Local Time : Fri Feb  7 18:10:26 2020
 *  UTC Time   : Fri Feb  7 10:10:26 2020
 *  @endcode
 *
 *  @param u64Sec [in] The UTC time in second.
 *  @param pu64Next [out] The UTC time in second of next day.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getUtcTimeInSecondOfNextWeek(const u64 u64Sec, u64 * pu64Next);

/** Get current local time string.
 *
 *  @note
 *  -# The size of the string buffer should not less than 32 bytes.
 *
 *  @par Example
 *  @code
 *  UTC Second : 1551348626
 *  Local Time : Thu Feb 28 18:10:26 2019
 *  UTC Time   : Thu Feb 28 10:10:26 2019
 *  @endcode
 *
 *  @param pstrTime [out] The buffer for the time string.
 *  @param sStrTime [in] The size of the buffer.
 *  @param u64Sec [in] The UTC time in second.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getStringLocalTime(olchar_t * pstrTime, olsize_t sStrTime, const u64 u64Sec);

/** Get current UTC time string.
 *
 *  @note
 *  -# The size of the string buffer should not less than 32 bytes.
 *
 *  @par Example
 *  @code
 *  UTC Second : 1551953426
 *  Local Time : Thu Mar  7 18:10:26 2019
 *  UTC Time   : Thu Mar  7 10:10:26 2019
 *  @endcode
 *
 *  @param pstrTime [out] The buffer for the time string.
 *  @param sStrTime [in] The size of the buffer.
 *  @param u64Sec [in] The UTC time in second.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_time_getStringUtcTime(olchar_t * pstrTime, olsize_t sStrTime, const u64 u64Sec);

#endif /*JIUTAI_TIME_H*/

/*------------------------------------------------------------------------------------------------*/
