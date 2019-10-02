/**
 *  @file jf_time.h
 *
 *  @brief Time header file. Provide some time routine.
 *
 *  @author Min Zhang
 *  
 *  @note Routines declared in this file are included in jf_time library
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
#if defined(LINUX)
    #define INFINITE  0xFFFFFFFF
#elif defined(WINDOWS)

#endif

/* --- functional routines ---------------------------------------------------------------------- */
#if defined(WINDOWS)
u32 jf_time_fileTimeToSecondsSince1970(FILETIME * pTime);
#endif

/** Get time
 *  
 *  @param tv [out] the time value in second and microsecond since Epoch
 *
 *  @return the error code
 */
u32 jf_time_getTimeOfDay(struct timeval * tv);

/** Get time
 *
 *  @param clkid [in] identifier of the particular clock
 *  @param tp [out] the timespec structures
 *
 *  @return the error code
 */
u32 jf_time_getClockTime(clockid_t clkid, struct timespec *tp);

/** Sleep some time in seconds
 *
 *  @param u32Seconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_sleep(u32 u32Seconds);

/** Sleep some time in milliseconds
 *  
 *  @param u32MilliSeconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_milliSleep(u32 u32MilliSeconds);

/** Sleep some time in microseconds
 *  
 *  @param u32MicroSeconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_microSleep(u32 u32MicroSeconds);

/** Sleep some time in nanoseconds
 *  
 *  @param u32NanoSeconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_nanoSleep(u32 u32NanoSeconds);

/** Convert time in hour:min:sec to seconds
 *
 *  @param hour [in] the hour of the time
 *  @param min [in] the minute of the time
 *  @param sec [in] the second of the time
 *
 *  @return the second
 */
olint_t jf_time_convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec);

/** Get the string of time period in the format of "[# hr] [# min] [# sec]"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrTime [out] the string buffer where the period string will return
 *  @param u32Period [in] the time
 */
void jf_time_getStringTimePeriod(olchar_t * pstrTime, const u32 u32Period);

/** Get the time from the string with the format hour:minute:second like
 *  15:23:58
 */
u32 jf_time_getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin, olint_t * pSec);

#endif /*JIUTAI_TIME_H*/

/*------------------------------------------------------------------------------------------------*/


