/**
 *  @file jf_date.h
 *
 *  @brief Header file which provide some date routines.
 *
 *  @author Min Zhang
 *  
 *  @note
 *  -# Routines declared in this file are included in jf_date object.
 *
 */

#ifndef JIUTAI_DATE_H
#define JIUTAI_DATE_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the day of week.
 */
enum
{
    /**It's Sunday.*/
    JF_DATE_SUNDAY = 0,
    /**It's Sunday.*/
    JF_DATE_MONDAY,
    /**It's Tuesday.*/
    JF_DATE_TUESDAY,
    /**It's Wednesday.*/
    JF_DATE_WEDNESDAY,
    /**It's Thursday.*/
    JF_DATE_THURSDAY,
    /**It's Friday.*/
    JF_DATE_FRIDAY,
    /**It's Saturday.*/
    JF_DATE_SATURDAY,
};


/* --- functional routines ---------------------------------------------------------------------- */

/** Check if the year is leap year.
 *  
 *  @param year [in] The year to check.
 *
 *  @return If it's a leap year.
 *  @retval TRUE It's a leap year.
 *  @retval FALSE It's not a leap year.
 */
boolean_t jf_date_isLeapYear(olint_t year);

/** Get the number of days in the year.
 *  
 *  @param year [in] The year to check.
 *
 *  @return Number of days.
 *  @retval 366 for leap year.
 *  @retval 365 for not leap year.
 */
olint_t jf_date_getDaysOfYear(olint_t year);

/** Get the number of days in month of the year.
 *  
 *  @param year [in] The year to check.
 *  @param mon [in] The month to check.
 *
 *  @return Number of days.
 *  @retval 28 for the 2th month of not leap year.
 *  @retval 29 for the 2th month of leap year.
 *  @retval 30 for the 4th, 6th, 9th, 11th month.
 *  @retval 31 for the 1th, 3th, 5th, 7th, 8th, 10th, 12th month.
 */
olint_t jf_date_getDaysOfMonth(olint_t year, olint_t mon);

/** Get number of days from 1970 for date specified in parameters.
 *  
 *  @param year [in] The year of the date.
 *  @param mon [in] The month of the date.
 *  @param day [in] The day of the date.
 *
 *  @return Number of days from 1970.
 */
olint_t jf_date_convertDateToDaysFrom1970(olint_t year, olint_t mon, olint_t day);

/** Get number of days from 1970 for today.
 *  
 *  @return Number of days from 1970.
 */
olint_t jf_date_convertTodayToDaysFrom1970(void);

/** Convert the days from 1970 to year, month and day.
 *
 *  @param days [in] The days from 1970.
 *  @param year [out] The year of the date.
 *  @param mon [out] The month of the date.
 *  @param day [out] The day of the date.
 *
 *  @return Void.
 */
void jf_date_convertDaysFrom1970ToDate(
    const olint_t days, olint_t * year, olint_t * mon, olint_t * day);

/** Get day of week from date.
 *  
 *  @param year [in] The year of the date.
 *  @param mon [in] The month of the date.
 *  @param day [in] The day of the date.
 *
 *  @return The day of week.
 *  @retval 0 Sunday.
 *  @retval 1 Monday.
 *  @retval 2 Tuesday.
 *  @retval 3 Wednesday.
 *  @retval 4 Thursday.
 *  @retval 5 Friday.
 *  @retval 6 Saturday.
 */
olint_t jf_date_getDayOfWeekFromDate(olint_t year, olint_t mon, olint_t day);

/** Get day of week for today.
 *  
 *  @return The day of week.
 *  @retval 0 Sunday.
 *  @retval 1 Monday.
 *  @retval 2 Tuesday.
 *  @retval 3 Wednesday.
 *  @retval 4 Thursday.
 *  @retval 5 Friday.
 *  @retval 6 Saturday.
 */
olint_t jf_date_getDayOfWeekForToday(void);


/** Get next day of week.
 *
 *  @param dayofweek [in] The current day of week.
 *
 *  @return The next day of week.
 *  @retval 0 Sunday.
 *  @retval 1 Monday.
 *  @retval 2 Tuesday.
 *  @retval 3 Wednesday.
 *  @retval 4 Thursday.
 *  @retval 5 Friday.
 *  @retval 6 Saturday.
 */
olint_t jf_date_getNextDayOfWeek(olint_t dayofweek);


/** Check if it's weekend from date.
 *  
 *  @param year [in] The year of the date.
 *  @param mon [in] The month of the date.
 *  @param day [in] The day of the date.
 *
 *  @return If the date is weekend or not.
 *  @retval TRUE It's weekend.
 *  @retval FALSE It's not weekend.
 */
boolean_t jf_date_isWeekendForDate(olint_t year, olint_t mon, olint_t day);

/** Get date for today.
 *
 *  @param year [out] The year of the date.
 *  @param mon [out] The month of the date.
 *  @param day [out] The day of the date.
 *
 *  @return Void.
 */
void jf_date_getDateToday(olint_t * year, olint_t * mon, olint_t * day);

/** Get the string of date in the format of "<mon> dd, yyyy".
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big
 *   enough to avoid memory access violation.
 *
 *  @param pstrDate [out] The string buffer where the date string will return.
 *  @param sDate [in] Size of the string.
 *  @param year [in] The year.
 *  @param mon [in] The month of the year.
 *  @param day [in] The day of the month.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_date_getStringDate(
    olchar_t * pstrDate, olsize_t sDate, const olint_t year, const olint_t mon, const olint_t day);

/** Get the string of date in the format of "yyyy-mm-dd".
 *
 *  @param pstrDate [out] The string buffer where the date string will return.
 *  @param sDate [in] Size of the string.
 *  @param year [in] The year.
 *  @param mon [in] The month of the year.
 *  @param day [in] The day of the month.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_date_getStringDate2(
    olchar_t * pstrDate, olsize_t sDate, const olint_t year, const olint_t mon, const olint_t day);

/** Get the string of date in format "yyyy-mm-dd".
 *
 *  @param pstrDate [out] The string buffer where the date string will return.
 *  @param sDate [in] Size of the string.
 *  @param nDays [in] The days from 1970.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_date_getStringDate2ForDaysFrom1970(olchar_t * pstrDate, olsize_t sDate, const olint_t nDays);

/** Get the string of time in the format of "hh:mm:ss <month> <day>, <year>".
 *
 *  @note
 *  -# The time is local time in second.
 *  -# The size of the string buffer should not less than 32 bytes.
 *
 *  @param pstrTime [out] The string buffer where the date string will return.
 *  @param sTime [in] Size of the string.
 *  @param u64Time [in] The time in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_TIME Invalid time.
 */
u32 jf_date_getStringLocalDateTime(olchar_t * pstrTime, olsize_t sTime, const u64 u64Time);

/** Get the string of time in the format of "hh:mm:ss <month> <day>, <year>".
 *
 *  @note
 *  -# The time is UTC time in second.
 *  -# The size of the string buffer should not less than 32 bytes.
 *
 *  @param pstrTime [out] The string buffer where the date string will return.
 *  @param sTime [in] Size of the string.
 *  @param u64Time [in] The time in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_TIME Invalid time.
 */
u32 jf_date_getStringUtcDateTime(olchar_t * pstrTime, olsize_t sTime, const u64 u64Time);

/** Get date from the string with the format year/month/date like 2005/10/20.
 *  
 *  @param pstrDate [in] The date string.
 *  @param pYear [out] The year of the date.
 *  @param pMon [out] The month of the date.
 *  @param pDay [out] The day of the date.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_date_getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

/** Get date from the string with the format year-month-date like 2005-10-20.
 *  
 *  @param pstrDate [in] The date string.
 *  @param pYear [out] The year of the date.
 *  @param pMon [out] The month of the date.
 *  @param pDay [out] The day of the date.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_date_getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

#endif /*JIUTAI_DATE_H*/

/*------------------------------------------------------------------------------------------------*/
