/**
 *  @file time-test.c
 *
 *  @brief test file for jf_time common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_time.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

#define NUM_OF_TEST5_ENTRY  10

static void testClockTime(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timespec tp;
    olint_t rawtime = 0;

    jf_time_getMonotonicRawTimeSecond(&rawtime);
    u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %ld, tv_nsec: %ld\n", tp.tv_sec, tp.tv_nsec);
        ol_printf("raw time: %d\n", rawtime);

        jf_time_sleep(5);

        jf_time_getMonotonicRawTimeSecond(&rawtime);
        u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %ld, tv_nsec: %ld\n", tp.tv_sec, tp.tv_nsec);
        ol_printf("raw time: %d\n", rawtime);

        jf_time_sleep(3);

        jf_time_getMonotonicRawTimeSecond(&rawtime);
        u32Ret = jf_time_getClockTime(CLOCK_MONOTONIC_RAW, &tp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %ld, tv_nsec: %ld\n", tp.tv_sec, tp.tv_nsec);
        ol_printf("raw time: %d\n", rawtime);
    }

    ol_printf("\n");
}

static void test5(void)
{
    olchar_t * data[NUM_OF_TEST5_ENTRY] = {"00:00:00", "00:00:59",
                                       "11:59:59", "12:00:00",
                                       "12:59:59", "13:00:00",
                                       "13:00:01", "15:00:00",
                                       "23:00:00", "23:59:59"};
    olint_t i;
    olint_t hour, min, sec, seconds;

    for (i = 0; i < NUM_OF_TEST5_ENTRY; i ++)
    {
        jf_time_getTimeFromString(data[i], &hour, &min, &sec);
        seconds = jf_time_convertTimeToSeconds(hour, min, sec);
        ol_printf("%s, %d secondes\n", data[i], seconds);
    }

    ol_printf("\n");
}

static void testTimePeriod(void)
{
    olchar_t strTime[100];
    u32 u32Peroid[] = {0, 1, 60, 61, 600, 601, 86400, 86401, 87000, 87001};
    olint_t i = 0;

    for (i = 0; i < ARRAY_SIZE(u32Peroid); i ++)
    {
        jf_time_getStringTimePeriod(strTime, u32Peroid[i]);
        ol_printf("%u: %s\n", u32Peroid[i], strTime);
    }

}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    testClockTime();

    test5();

    testTimePeriod();

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

