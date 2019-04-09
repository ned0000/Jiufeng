/**
 *  @file jf_time.c
 *
 *  @brief The time common object. 
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#if defined(LINUX)
    #include <sys/time.h>
#elif defined(WINDOWS)
    #include <time.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_time.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */
#if defined(WINDOWS)
u32 jf_time_fileTimeToSecondsSince1970(FILETIME * pTime)
{
	u64 u64Time;

    /* in 100-nanosecond since January 1, 1601 */
    u64Time = ((u64)pTime->dwHighDateTime << 32) + pTime->dwLowDateTime;
    /* in 100-nanosecond since January 1, 1970 */
    u64Time -= 116444736000000000;
    /* in second since January 1, 1970 */
    u64Time /= 10000000;

	return (u32)u64Time;
}
#endif

u32 jf_time_getTimeOfDay(struct timeval * tv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = gettimeofday(tv, NULL);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_GET_TIME;
#elif defined(WINDOWS)
    FILETIME systime;
    u64 u64Time;

    GetSystemTimeAsFileTime(&systime);

    /* in 100-nanosecond since January 1, 1601 */
    u64Time = ((u64)systime.dwHighDateTime << 32) + systime.dwLowDateTime;
    /* in 100-nanosecond since January 1, 1970 */
    u64Time -= 116444736000000000;
    /* in macrosecond since January 1, 1970 */
    u64Time /= 10;

    tv->tv_sec = (long)(u64Time / 1000000);
    tv->tv_usec = (long)(u64Time % 1000000);

#endif

    return u32Ret;
}

u32 jf_time_msleep(u32 u32Milliseconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    usleep(u32Milliseconds * 1000);
#elif defined(WINDOWS)
    Sleep(u32Milliseconds);
#endif

    return u32Ret;
}

u32 jf_time_nsleep(u32 u32Nanoseconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct timespec ts;

    ts.tv_sec  = 0;
    ts.tv_nsec = 500;
    nanosleep(&ts, NULL);
#elif defined(WINDOWS)
    Sleep(1);
#endif

    return u32Ret;
}

olint_t jf_time_convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec)
{
    return hour * 3600 + min * 60 + sec;
}

/*---------------------------------------------------------------------------*/


