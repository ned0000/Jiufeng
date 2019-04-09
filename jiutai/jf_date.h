/**
 *  @file jf_date.h
 *
 *  @brief Date header file. Provide some date routine.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef JIUTAI_DATE_H
#define JIUTAI_DATE_H

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

/** Check if the year is leap year
 *  
 *  @param year [in] the year to check
 *
 *  @return if it's a leap year
 *  @retval TRUE it's a leap year
 *  @retval FALSE it's not a leap year
 */
boolean_t jf_date_isLeapYear(olint_t year);

/** Get the number of days in the year
 *  
 *  @param year [in] the year to check
 *
 *  @return number of days
 *  @retval 366 for leap year
 *  @retval 365 for not leap year
 */
olint_t jf_date_getDaysOfYear(olint_t year);

/** Get the number of days in month of the year
 *  
 *  @param year [in] the year to check
 *  @param mon [in] the month to check
 *
 *  @return number of days
 *  @retval 28 for the 2th month of not leap year
 *  @retval 29 for the 2th month of leap year
 *  @retval 30 for the 4th, 6th, 9th, 11th month
 *  @retval 31 for the 1th, 3th, 5th, 7th, 8th, 10th, 12th month
 */
olint_t jf_date_getDaysOfMonth(olint_t year, olint_t mon);

/** Get number of days from 1970 for date specified in parameters
 *  
 *  @param year [in] the year of the date
 *  @param mon [in] the month of the date
 *  @param day [in] the day of the date
 *
 *  @return number of days from 1970 
 */
olint_t jf_date_convertDateToDaysFrom1970(olint_t year, olint_t mon, olint_t day);

/** Get number of days from 1970 for today
 *  
 *  @return number of days from 1970 
 */
olint_t jf_date_convertTodayToDaysFrom1970(void);

/** Convert the days from 1970 to year, month and day
 *
 *  @param days [in] the days from 1970
 *  @param year [out] the year of the date
 *  @param mon [out] the month of the date
 *  @param day [out] the day of the date
 *
 *  @return void
 */
void jf_date_convertDaysFrom1970ToDate(
    const olint_t days, olint_t * year, olint_t * mon, olint_t * day);

/** Get day of week from date
 *  
 *  @param year [in] the year of the date
 *  @param mon [in] the month of the date
 *  @param day [in] the day of the date
 *
 *  @return the day of week
 *  @retval 0 sunday
 *  @retval 1 monday
 *  @retval 2 tuesday
 *  @retval 3 wednesday
 *  @retval 4 thursday
 *  @retval 5 friday
 *  @retval 6 saturday
 */
olint_t jf_date_getDayOfWeekFromDate(olint_t year, olint_t mon, olint_t day);

/** Get day of week for today
 *  
 *  @return the day of week
 *  @retval 0 sunday
 *  @retval 1 monday
 *  @retval 2 tuesday
 *  @retval 3 wednesday
 *  @retval 4 thursday
 *  @retval 5 friday
 *  @retval 6 saturday
 */
olint_t jf_date_getDayOfWeekForToday(void);

/** Check if it's weekend from date
 *  
 *  @param year [in] the year of the date
 *  @param mon [in] the month of the date
 *  @param day [in] the day of the date
 *
 *  @return if the date is weekend or not
 *  @retval TRUE it's weekend
 *  @retval FALSE it's not weekend
 */
boolean_t jf_date_isWeekendForDate(olint_t year, olint_t mon, olint_t day);

/** Get date for today
 *
 *  @param year [out] the year of the date
 *  @param mon [out] the month of the date
 *  @param day [out] the day of the date
 *
 *  @return void
 */
void jf_date_getDateToday(olint_t * year, olint_t * mon, olint_t * day);

#endif /*JIUTAI_DATE_H*/

/*---------------------------------------------------------------------------*/

