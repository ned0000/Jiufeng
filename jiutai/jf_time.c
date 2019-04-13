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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#if defined(LINUX)
    #include <sys/time.h>
#elif defined(WINDOWS)
    #include <time.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_time.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */
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

void jf_time_getStringTimePeriod(olchar_t * pstrTime, const u32 u32Period)
{
    u32 u32Temp, u32Seconds, u32Minutes, u32Hours;
    olchar_t strTemp[16];

    if (u32Period == 0)
    {
        ol_sprintf(pstrTime, "0 sec");
    }
    else
    {
        u32Hours = u32Period / 3600;
        u32Temp = u32Period % 3600;

        u32Minutes = u32Temp / 60;
        u32Seconds = u32Temp % 60;

        pstrTime[0] = 0;
        if (u32Hours > 0)
        {
            ol_sprintf(pstrTime, "%d hr", u32Hours);
        }

        if (u32Minutes > 0)
        {
            ol_sprintf(strTemp, "%d min", u32Minutes);
            if (ol_strlen(pstrTime) > 0)
            {
                ol_strcat(pstrTime, " ");
            }
            ol_strcat(pstrTime, strTemp);
        }

        if (u32Seconds > 0)
        {
            ol_sprintf(strTemp, "%d sec", u32Seconds);
            if (ol_strlen(pstrTime) > 0)
            {
                ol_strcat(pstrTime, " ");
            }
            ol_strcat(pstrTime, strTemp);
        }
    }
}

/** Get the time from the string with the format hour:minute:second
 *
 */
u32 jf_time_getTimeFromString(
    const olchar_t * pstrTime, olint_t * pHour, olint_t * pMin, olint_t * pSec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * firstChar, * psubStr, cCol = ':';
    olchar_t strTime[100];
    u32 u32Value;
    olsize_t size;

    memset(strTime, 0, sizeof(strTime));
    ol_strncpy(strTime, pstrTime, sizeof(strTime) - 1);
    firstChar = strTime;

    /* hour */
    psubStr = ol_strchr(firstChar, cCol);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (ol_sscanf(firstChar, "%02d", &u32Value) != 1)
        {
            return JF_ERR_INVALID_TIME;
        }
        else if (u32Value > 23)
        {
            return JF_ERR_INVALID_TIME;
        }
        else
        {
            firstChar = psubStr + 1;
            *pHour = u32Value;
        }
    }
    else
    {
        return JF_ERR_INVALID_TIME;
    }

    /* Minute */
    psubStr = ol_strchr(firstChar, cCol);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (ol_sscanf(firstChar, "%02d", &u32Value) != 1)
        {
            return JF_ERR_INVALID_TIME;
        }
        else if (u32Value >= 60)
        {
            return JF_ERR_INVALID_TIME;
        }
        else
        {
            firstChar = psubStr + 1;
            *pMin = u32Value;
        }
    }
    else
    {
        return JF_ERR_INVALID_TIME;
    }

    /* Second */
    if (ol_sscanf(firstChar, "%02d", &u32Value) != 1)
    {
        return JF_ERR_INVALID_TIME;
    }
    else if (u32Value >= 60)
    {
        return JF_ERR_INVALID_TIME;
    }
    else
    {
        *pSec = u32Value;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


