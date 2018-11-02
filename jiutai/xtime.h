/**
 *  @file xtime.h
 *
 *  @brief time header file
 *	 provide some time routine
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef JIUTAI_XTIME_H
#define JIUTAI_XTIME_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
#if defined(LINUX)
    #define INFINITE  0xFFFFFFFF
#elif defined(WINDOWS)

#endif

/* --- functional routines ------------------------------------------------- */
#if defined(WINDOWS)
u32 fileTimeToSecondsSince1970(FILETIME * pTime);
#endif

u32 getTimeOfDay(struct timeval * tv);

u32 msleep(u32 u32Milliseconds);

u32 nsleep(u32 u32Nanoseconds);

boolean_t isLeapYear(olint_t year);

olint_t getDaysOfYear(olint_t year);

olint_t getDaysOfMonth(olint_t year, olint_t mon);

olint_t convertDateToDaysFrom1970(olint_t year, olint_t mon, olint_t day);
olint_t convertTodayToDaysFrom1970(void);

void convertDaysFrom1970ToDate(
    const olint_t days, olint_t * year, olint_t * mon, olint_t * day);

/*0 is Sunday, 1 is Monday, 2 is Tuesday, 3 is Wednesday,
 4 is Thursday, 5 is Friday, 6 is Saturday*/
olint_t getDayOfWeekFromDate(olint_t year, olint_t mon, olint_t day);
olint_t getDayOfWeekForToday(void);
boolean_t isWeekendForDate(olint_t year, olint_t mon, olint_t day);

void getDateToday(olint_t * year, olint_t * mon, olint_t * day);

olint_t convertTimeToSeconds(olint_t hour, olint_t min, olint_t sec);

#endif /*JIUTAI_XTIME_H*/

/*---------------------------------------------------------------------------*/


