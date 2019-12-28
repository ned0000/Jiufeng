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

/* --- data structures -------------------------------------------------------------------------- */

/** Define the INFINITE macro.
 */
#if defined(LINUX)
    #define INFINITE  0xFFFFFFFF
#elif defined(WINDOWS)

#endif

/* --- functional routines ---------------------------------------------------------------------- */

#if defined(WINDOWS)
u32 jf_time_fileTimeToSecondsSince1970(FILETIME * pTime);
#endif

/** Get time.
 *  
 *  @param tv [out] The time value in second and microsecond since Epoch.
 *
 *  @return The error code.
 */
u32 jf_time_getTimeOfDay(struct timeval * tv);

/** Get clock time.
 *
 *  @param clkid [in] Identifier of the particular clock.
 *  @param tp [out] The timespec structures.
 *
 *  @return The error code
 */
u32 jf_time_getClockTime(clockid_t clkid, struct timespec *tp);

/** Sleep some time in seconds.
 *
 *  @param u32Seconds [in] The time to sleep.
 *
 *  @return The error code.
 */
u32 jf_time_sleep(u32 u32Seconds);

/** Sleep some time in milliseconds.
 *  
 *  @param u32MilliSeconds [in] The time to sleep.
 *
 *  @return The error code.
 */
u32 jf_time_milliSleep(u32 u32MilliSeconds);

/** Sleep some time in microseconds.
 *  
 *  @param u32MicroSeconds [in] The time to sleep.
 *
 *  @return The error code.
 */
u32 jf_time_microSleep(u32 u32MicroSeconds);

/** Sleep some time in nanoseconds.
 *  
 *  @param u32NanoSeconds [in] The time to sleep.
 *
 *  @return The error code.
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
 *  @param pstrTime [out] The string buffer where the period string will return.
 *  @param u32Period [in] The time.
 */
void jf_time_getStringTimePeriod(olchar_t * pstrTime, const u32 u32Period);

/** Get the time from the string with the format hour:minute:second like 15:23:58.
 */
u32 jf_time_getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin, olint_t * pSec);

#endif /*JIUTAI_TIME_H*/

/*------------------------------------------------------------------------------------------------*/


