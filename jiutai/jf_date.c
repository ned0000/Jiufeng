/**
 *  @file jf_date.c
 *
 *  @brief The implementation file for date routines.
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
#include "jf_date.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */

const static olchar_t * ls_pstrMonth[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* --- private routine section ------------------------------------------------------------------ */

static boolean_t _isDayInRange(olint_t year, olint_t month, olint_t day)
{
    olint_t dm = jf_date_getDaysOfMonth(year, month);

    if (day <= 0 || day > dm)
        return FALSE;

    return TRUE;
}

/** Get date from the string with the format yyyy%mm%dd where '%' is the seperator.
 */
static u32 _getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay, olchar_t sep)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olsize_t size = 0;
    olchar_t * firstChar, * psubStr;
    olchar_t u8Data[100];
    u32 u32Value;

    ol_memset(u8Data, 0, 100);
    ol_strncpy(u8Data, pstrDate, 99);
    firstChar = u8Data;

    /*Year.*/
    psubStr = ol_strchr(firstChar, sep);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (ol_sscanf(firstChar, "%04d", &u32Value) == 1)
        {
            if (u32Value >= 1970 && u32Value <= 2037)
            {
                firstChar = psubStr + 1;
                *pYear = u32Value;
                u32Ret = JF_ERR_NO_ERROR;
            }
        }
    }
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        return u32Ret;
    }

    /*Month.*/
    u32Ret = JF_ERR_INVALID_DATE;
    psubStr = ol_strchr(firstChar, sep);
    if(psubStr != NULL)
    {
        u32Ret = JF_ERR_INVALID_DATE;
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (ol_sscanf(firstChar, "%02d", &u32Value) == 1)
        {
            if (u32Value <= 12 && u32Value >= 1)
            {
                firstChar = psubStr + 1;
                *pMon = u32Value;
                u32Ret = JF_ERR_NO_ERROR;
            }
        }
    }
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        return u32Ret;
    }

    /*Day.*/
    u32Ret = JF_ERR_INVALID_DATE;
    if (ol_sscanf(firstChar, "%02d", &u32Value) == 1)
    {
        if (_isDayInRange(*pYear, *pMon, u32Value) == TRUE)
        {
            *pDay = u32Value;
            u32Ret = JF_ERR_NO_ERROR;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

boolean_t jf_date_isLeapYear(olint_t year)
{
    if (((year % 4 == 0) && (year % 100 != 0)) ||
        (year % 400 == 0))
        return TRUE;

    return FALSE;
}

olint_t jf_date_getDaysOfMonth(olint_t year, olint_t mon)
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
        if (jf_date_isLeapYear(year))
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

olint_t jf_date_getDaysOfYear(olint_t year)
{
    if (jf_date_isLeapYear(year))
        return 366;

    return 365;
}

/*from 1970*/
olint_t jf_date_convertDateToDaysFrom1970(olint_t year, olint_t mon, olint_t day)
{
    olint_t ret = day;
    olint_t i, a[12] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};
    olint_t start_year = 1970;

    if (jf_date_isLeapYear(year)) a[2] = 29;
    for (i = 0; i <= mon - 1; i ++) 
        ret += a[i];

    while (start_year < year)
    {
        ret += jf_date_getDaysOfYear(start_year);
        start_year ++;
    }

    return ret;
}

olint_t jf_date_convertTodayToDaysFrom1970(void)
{
    olint_t year, mon, day;

    jf_date_getDateToday(&year, &mon, &day);
    return jf_date_convertDateToDaysFrom1970(year, mon, day);
}

void jf_date_convertDaysFrom1970ToDate(
    const olint_t days, olint_t * year, olint_t * mon, olint_t * day)
{
    olint_t ret = days;
    olint_t i, s = 0, a[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    olint_t start_year = 1970;

    while (ret > 0)
    {
        ret -= jf_date_getDaysOfYear(start_year);
        if (ret > 0)
            start_year ++;
    }

    *year = start_year;
    ret += jf_date_getDaysOfYear(start_year);

    if (jf_date_isLeapYear(start_year))
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

olint_t jf_date_getDayOfWeekFromDate(olint_t year, olint_t mon, olint_t day)
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

    w = (olint_t)(c / 4) - 2 * c + y + (olint_t)(y / 4)
        + (olint_t)(13 * (m + 1) / 5) + d - 1;

    w = w % 7;
    if (w < 0)
        w += 7; 

    return w;
}

olint_t jf_date_getDayOfWeekForToday(void)
{
    olint_t year, mon, day;

    jf_date_getDateToday(&year, &mon, &day);
    return jf_date_getDayOfWeekFromDate(year, mon, day);
}

olint_t jf_date_getNextDayOfWeek(olint_t dayofweek)
{
    olint_t ret = dayofweek + 1;

    if (dayofweek == JF_DATE_SATURDAY)
        ret = JF_DATE_SUNDAY;

    return ret;
}

boolean_t jf_date_isWeekendForDate(olint_t year, olint_t mon, olint_t day)
{
    boolean_t bRet = FALSE;
    olint_t dw;

    dw = jf_date_getDayOfWeekFromDate(year, mon, day);
    if ((dw == 6) || /*Saturday*/
        (dw == 0)) /*Sunday*/
        bRet = TRUE;

    return bRet;
}

void jf_date_getDateToday(olint_t * year, olint_t * mon, olint_t * day)
{
    time_t curtime = time(NULL);
    struct tm * tmdate;

    tmdate = localtime(&curtime);
    *year = 1900 + tmdate->tm_year;
    *mon = 1 + tmdate->tm_mon;
    *day = tmdate->tm_mday;
}

u32 jf_date_getStringDate(
    olchar_t * pstrDate, olsize_t sDate, const olint_t year, const olint_t mon, const olint_t day)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pstrDate, sDate);

    if ((mon == 0) && (day == 0))
        ol_strncpy(pstrDate, JF_STRING_NOT_APPLICABLE, sDate - 1);
    else if ((mon > 12) || (mon == 0))
        ol_snprintf(pstrDate, sDate - 1, "Month(%d) %d, %04d", mon, day, year);
    else
        ol_snprintf(pstrDate, sDate - 1, "%s %d, %04d", ls_pstrMonth[mon - 1], day, year);

    return u32Ret;
}

u32 jf_date_getStringDate2(
    olchar_t * pstrDate, olsize_t sDate, const olint_t year, const olint_t mon, const olint_t day)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pstrDate, sDate);

    if ((mon == 0) && (day == 0))
        ol_strncpy(pstrDate, JF_STRING_NOT_APPLICABLE, sDate - 1);
    else
        ol_snprintf(pstrDate, sDate - 1, "%4d-%02d-%02d", year, mon, day);

    return u32Ret;
}

u32 jf_date_getStringDate2ForDaysFrom1970(olchar_t * pstrDate, olsize_t sDate, const olint_t nDays)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year = 0, mon = 0, day = 0;

    ol_bzero(pstrDate, sDate);

    jf_date_convertDaysFrom1970ToDate(nDays, &year, &mon, &day);
    ol_snprintf(pstrDate, sDate - 1, "%4d-%02d-%02d", year, mon, day);

    return u32Ret;
}

u32 jf_date_getStringUTCTime(olchar_t * pstrTime, olsize_t sTime, const u64 u64Time)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct tm * ptmLocal = NULL;
    time_t t = u64Time;
    olchar_t strDate[64];

    ol_bzero(pstrTime, sTime);

    ptmLocal = gmtime(&t);
    if (ptmLocal == NULL)
    {
        ol_snprintf(pstrTime, sTime - 1, "invalid time stamp (%llu)", u64Time);
        u32Ret = JF_ERR_INVALID_TIME;
    }
    else
    {
        jf_date_getStringDate(
            strDate, sizeof(strDate), ptmLocal->tm_year + 1900, ptmLocal->tm_mon + 1,
            ptmLocal->tm_mday);

        ol_snprintf(
            pstrTime, sTime - 1, "%s %02d:%02d:%02d", strDate, ptmLocal->tm_hour, ptmLocal->tm_min,
            ptmLocal->tm_sec);
    }

    return u32Ret;
}

u32 jf_date_getStringLocalTime(olchar_t * pstrTime, olsize_t sTime, const u64 u64Time)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct tm * ptmLocal = NULL;
    time_t t = u64Time;
    olchar_t strDate[64];

    ol_bzero(pstrTime, sTime);

    ptmLocal = localtime(&t);
    if (ptmLocal == NULL)
    {
        ol_snprintf(pstrTime, sTime - 1, "invalid time stamp (%llu)", u64Time);
        u32Ret = JF_ERR_INVALID_TIME;
    }
    else
    {
        jf_date_getStringDate(
            strDate, sizeof(strDate), ptmLocal->tm_year + 1900, ptmLocal->tm_mon + 1,
            ptmLocal->tm_mday);

        ol_snprintf(
            pstrTime, sTime - 1, "%s %02d:%02d:%02d", strDate, ptmLocal->tm_hour, ptmLocal->tm_min,
            ptmLocal->tm_sec);
    }

    return u32Ret;
}

u32 jf_date_getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olchar_t cSlash = '/';

    u32Ret = _getDateFromString(pstrDate, pYear, pMon, pDay, cSlash);

    return u32Ret;
}

u32 jf_date_getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olchar_t cSlash = '-';

    u32Ret = _getDateFromString(pstrDate, pYear, pMon, pDay, cSlash);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


