/**
 *  @file jf_time.c
 *
 *  @brief The implementation file for time object. 
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
#include "jf_err.h"
#include "jf_time.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static void _removeTrailingNewLineChar(olchar_t * pstrTime)
{
    olsize_t sTime = 0;

    sTime = ol_strlen(pstrTime);
    if ((sTime > 0) && pstrTime[sTime - 1] == '\n')
        pstrTime[sTime - 1] = '\0';
}

/* --- public routine section ------------------------------------------------------------------- */

#if defined(WINDOWS)
u32 jf_time_fileTimeToSecondsSince1970(FILETIME * pTime)
{
	u64 u64Time;

    /*In 100-nanosecond since January 1, 1601*/
    u64Time = ((u64)pTime->dwHighDateTime << 32) + pTime->dwLowDateTime;
    /*In 100-nanosecond since January 1, 1970*/
    u64Time -= 116444736000000000;
    /*In second since January 1, 1970*/
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

    /*In 100-nanosecond since January 1, 1601*/
    u64Time = ((u64)systime.dwHighDateTime << 32) + systime.dwLowDateTime;
    /*In 100-nanosecond since January 1, 1970*/
    u64Time -= 116444736000000000;
    /*In macrosecond since January 1, 1970*/
    u64Time /= 10;

    tv->tv_sec = (long)(u64Time / JF_TIME_SECOND_TO_MICROSECOND);
    tv->tv_usec = (long)(u64Time % JF_TIME_SECOND_TO_MICROSECOND);

#endif

    return u32Ret;
}

u32 jf_time_getClockTime(clockid_t clkid, struct timespec * tp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    s32 ret = 0;

    ret = clock_gettime(clkid, tp);
    if (ret == -1)
        u32Ret = JF_ERR_FAIL_GET_CLOCK_TIME;

#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_time_sleep(u32 u32Seconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    sleep(u32Seconds);
#elif defined(WINDOWS)
    Sleep(u32Seconds * 1000);
#endif

    return u32Ret;
}

u32 jf_time_milliSleep(u32 u32MilliSeconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    usleep(u32MilliSeconds * 1000);
#elif defined(WINDOWS)
    Sleep(u32MilliSeconds);
#endif

    return u32Ret;
}

u32 jf_time_microSleep(u32 u32MicroSeconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    usleep(u32MicroSeconds);
#elif defined(WINDOWS)
    TO_BE_DONE;
#endif

    return u32Ret;
}

u32 jf_time_nanoSleep(u32 u32NanoSeconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct timespec ts;

    ts.tv_sec  = 0;
    ts.tv_nsec = u32NanoSeconds;
    nanosleep(&ts, NULL);
#elif defined(WINDOWS)
    TO_BE_DONE;
#endif

    return u32Ret;
}

olint_t jf_time_convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec)
{
    return hour * 3600 + min * 60 + sec;
}

u32 jf_time_getStringTimePeriod(olchar_t * pstrTime, olsize_t sStrTime, const u32 u32Period)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Temp = 0, u32Seconds = 0, u32Minutes = 0, u32Hours = 0;
    olchar_t strTemp[16];

    ol_bzero(pstrTime, sStrTime);

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
                ol_strcat(pstrTime, ", ");
            }
            ol_strcat(pstrTime, strTemp);
        }

        if (u32Seconds > 0)
        {
            ol_sprintf(strTemp, "%d sec", u32Seconds);
            if (ol_strlen(pstrTime) > 0)
            {
                ol_strcat(pstrTime, ", ");
            }
            ol_strcat(pstrTime, strTemp);
        }
    }

    return u32Ret;
}

u32 jf_time_getTimeFromString(
    const olchar_t * pstrTime, olint_t * pHour, olint_t * pMin, olint_t * pSec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * firstChar, * psubStr, cCol = ':';
    olchar_t strTime[100];
    u32 u32Value;
    olsize_t size;

    ol_memset(strTime, 0, sizeof(strTime));
    ol_strncpy(strTime, pstrTime, sizeof(strTime) - 1);
    firstChar = strTime;

    /*Hour.*/
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

    /*Minute.*/
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

    /*Second.*/
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

u32 jf_time_getStringTime(
    olchar_t * pstrTime, olsize_t sTime, const olint_t hour, const olint_t min, const olint_t sec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pstrTime, sTime);

    if ((hour > 24) || (min > 60) || (sec > 60))
        ol_strncpy(pstrTime, JF_STRING_NOT_APPLICABLE, sTime - 1);
    else
        ol_snprintf(pstrTime, sTime - 1, "%02d:%02d:%02d", hour, min, sec);

    return u32Ret;
}

u32 jf_time_getMonotonicRawTimeInSecond(olint_t * pSec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timespec tp;
    olint_t rawtime = 0;

    u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    if (u32Ret == JF_ERR_NO_ERROR)
        rawtime = (olint_t)tp.tv_sec;

    *pSec = rawtime;

    return u32Ret;
}

u32 jf_time_getUtcTimeInSecondFromTimeDate(
    u64 * pu64Sec, const olint_t hour, const olint_t min, const olint_t sec,
    const olint_t year, const olint_t mon, const olint_t day)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct tm result;

    if (year < 1900)
        u32Ret = JF_ERR_INVALID_DATE;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&result, sizeof(result));
        result.tm_hour = hour;
        result.tm_min = min;
        result.tm_sec = sec;
        result.tm_year = year - 1900;
        result.tm_mon = mon - 1;
        result.tm_mday = day;

        *pu64Sec = (u64)mktime(&result);
    }

    return u32Ret;
}

u32 jf_time_getUtcTimeInSecond(u64 * pu64Sec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *pu64Sec = (u64)time(NULL);

    return u32Ret;
}

u32 jf_time_getUtcTimeInMicroSecond(u64 * pu64MicroSec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timeval tv;

    *pu64MicroSec = 0;

    u32Ret = jf_time_getTimeOfDay(&tv);

    if (u32Ret == JF_ERR_NO_ERROR)
        *pu64MicroSec = (u64)tv.tv_sec * JF_TIME_SECOND_TO_MICROSECOND + (u64)tv.tv_usec;

    return u32Ret;
}

u32 jf_time_getUtcTimeInSecondOfNextDay(const u64 u64Sec, u64 * pu64Next)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    time_t tSec = (time_t)u64Sec;
    struct tm result;

    /*Use local time instead utc time as mktime() needs local broken-down time.*/
    localtime_r(&tSec, &result);
    result.tm_mday ++;
    /*mktime() take local broken-down time as parameter and return utc time.*/
    *pu64Next = (u64)mktime(&result);

    return u32Ret;
}

u32 jf_time_getUtcTimeInSecondOfNextWeek(const u64 u64Sec, u64 * pu64Next)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    time_t tSec = (time_t)u64Sec;
    struct tm result;

    /*Use local time instead utc time as mktime() needs local broken-down time.*/
    localtime_r(&tSec, &result);
    result.tm_mday += 7;
    /*mktime() take local broken-down time as parameter and return utc time.*/
    *pu64Next = (u64)mktime(&result);

    return u32Ret;
}

u32 jf_time_getStringLocalTime(olchar_t * pstrTime, olsize_t sStrTime, const u64 u64Sec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const time_t tSec = (time_t)u64Sec;

    ol_bzero(pstrTime, sStrTime);

    if (sStrTime < 32)
        u32Ret = JF_ERR_BUFFER_TOO_SMALL;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ctime_r(&tSec, pstrTime);
        /*Remove the '\n' at the end of string.*/
        _removeTrailingNewLineChar(pstrTime);
    }

    return u32Ret;
}

u32 jf_time_getStringUtcTime(olchar_t * pstrTime, olsize_t sStrTime, const u64 u64Sec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const time_t tSec = (time_t)u64Sec;
    struct tm result;

    ol_bzero(pstrTime, sStrTime);

    if (sStrTime < 32)
        u32Ret = JF_ERR_BUFFER_TOO_SMALL;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        gmtime_r(&tSec, &result);
        asctime_r(&result, pstrTime);
        /*Remove the '\n' at the end of string.*/
        _removeTrailingNewLineChar(pstrTime);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
