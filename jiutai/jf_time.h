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

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
#if defined(LINUX)
    #define INFINITE  0xFFFFFFFF
#elif defined(WINDOWS)

#endif

/* --- functional routines ------------------------------------------------- */
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

/** Sleep some time in milliseconds
 *  
 *  @param u32Milliseconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_msleep(u32 u32Milliseconds);

/** Sleep some time in nanoseconds
 *  
 *  @param u32Nanoseconds [in] the time to sleep
 *
 *  @return the error code
 */
u32 jf_time_nsleep(u32 u32Nanoseconds);

/** Convert time in hour:min:sec to seconds
 *
 *  @param hour [in] the hour of the time
 *  @param min [in] the minute of the time
 *  @param sec [in] the second of the time
 *
 *  @return the second
 */
olint_t jf_time_convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec);

#endif /*JIUTAI_TIME_H*/

/*---------------------------------------------------------------------------*/


