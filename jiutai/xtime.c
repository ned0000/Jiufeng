/**
 *  @file xtime.c
 *
 *  @brief The xtime library
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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */
#if defined(WINDOWS)
u32 fileTimeToSecondsSince1970(FILETIME * pTime)
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

u32 getTimeOfDay(struct timeval * tv)
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

u32 msleep(u32 u32Milliseconds)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    usleep(u32Milliseconds * 1000);
#elif defined(WINDOWS)
    Sleep(u32Milliseconds);
#endif

    return u32Ret;
}

u32 nsleep(u32 u32Nanoseconds)
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

boolean_t isLeapYear(olint_t year)
{
    if (((year % 4 == 0) && (year % 100 != 0)) ||
        (year % 400 == 0))
        return TRUE;

    return FALSE;
}

olint_t getDaysOfMonth(olint_t year, olint_t mon)
{
    olint_t md;

    switch (mon)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        md = 31;
        break; 
    case 2:
        if (isLeapYear(year))
            md = 29;
        else
            md = 28;
        break; 
    default:
        md = 30;
        break;
    }

    return md;
}

olint_t getDaysOfYear(olint_t year)
{
    if (isLeapYear(year))
        return 366;

    return 365;
}

/*from 1970*/
olint_t convertDateToDaysFrom1970(olint_t year, olint_t mon, olint_t day)
{
    olint_t ret = day;
    olint_t i, a[12] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};
    olint_t start_year = 1970;

    if (isLeapYear(year)) a[2] = 29;
    for (i = 0; i <= mon - 1; i ++) 
        ret += a[i];

    while (start_year < year)
    {
        ret += getDaysOfYear(start_year);
        start_year ++;
    }

    return ret;
}

olint_t convertTodayToDaysFrom1970(void)
{
    olint_t year, mon, day;

    getDateToday(&year, &mon, &day);
    return convertDateToDaysFrom1970(year, mon, day);
}

void convertDaysFrom1970ToDate(const olint_t days, olint_t * year, olint_t * mon, olint_t * day)
{
    olint_t ret = days;
    olint_t i, s = 0, a[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    olint_t start_year = 1970;

    while (ret > 0)
    {
        ret -= getDaysOfYear(start_year);
        if (ret > 0)
            start_year ++;
    }

    *year = start_year;
    ret += getDaysOfYear(start_year);

    if (isLeapYear(start_year))
        a[2] = 29;
    for (i = 0; i < 12; i ++) 
    {
        s += a[i];
        if ((ret > s) && (ret <= s + a[i + 1])) 
        {
            *mon = i + 1;
            *day = ret - s;
            break;
        }
    }

}

olint_t getDayOfWeekFromDate(olint_t year, olint_t mon, olint_t day)
{
    olint_t m = mon, d = day;
    olint_t c, y, w;

    if (m <= 2)
    {
        year -= 1;
        m += 12;
    }
    c = year / 100;
    y = year % 100; 

    w = (int)(c / 4) - 2 * c + y + (int)(y / 4)
        + (int)(13 * (m + 1) / 5) + d - 1;

    w = w % 7;
    if (w < 0)
        w += 7; 

    return w;
}

olint_t getDayOfWeekForToday(void)
{
    olint_t year, mon, day;

    getDateToday(&year, &mon, &day);
    return getDayOfWeekFromDate(year, mon, day);
}

boolean_t isWeekendForDate(olint_t year, olint_t mon, olint_t day)
{
    boolean_t bRet = FALSE;
    olint_t dw;

    dw = getDayOfWeekFromDate(year, mon, day);
    if ((dw == 6) || /*Saturday*/
        (dw == 0)) /*Sunday*/
        bRet = TRUE;

    return bRet;
}

void getDateToday(olint_t * year, olint_t * mon, olint_t * day)
{
    time_t curtime = time(NULL);
    struct tm * tmdate;

    tmdate = localtime(&curtime);
    *year = 1900 + tmdate->tm_year;
    *mon = 1 + tmdate->tm_mon;
    *day = tmdate->tm_mday;
}

olint_t convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec)
{
    return hour * 3600 + min * 60 + sec;
}

/*---------------------------------------------------------------------------*/


