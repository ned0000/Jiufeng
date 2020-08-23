/**
 *  @file time-test.c
 *
 *  @brief Test file for time function defined in jf_time common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_time.h"
#include "jf_string.h"
#include "jf_date.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTimeTestClock = FALSE;

static boolean_t ls_bTimeTestPeriod = FALSE;

static boolean_t ls_bTimeTestRecur = FALSE;

static boolean_t ls_bTimeTestString = FALSE;

static boolean_t ls_bTimeTestSystemTime = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printTimeTestUsage(void)
{
    ol_printf("\
Usage: time-test [-c] [-p] [-r] [-t] [-s]\n\
    -c test clock time.\n\
    -p test time period.\n\
    -r test time recur.\n\
    -t test system time.\n\
    -s test time string.\n");

    ol_printf("\n");

}

static u32 _parseTimeTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "cprtsh?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printTimeTestUsage();
            exit(0);
            break;
        case 'c':
            ls_bTimeTestClock = TRUE;
            break;
        case 'p':
            ls_bTimeTestPeriod = TRUE;
            break;
        case 'r':
            ls_bTimeTestRecur = TRUE;
            break;
        case 't':
            ls_bTimeTestSystemTime = TRUE;
            break;
        case 's':
            ls_bTimeTestString = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testTimeClock(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_time_spec_t jts;
    u64 rawtime = 0;

    jf_time_getMonotonicRawTimeInSecond(&rawtime);
    u32Ret = jf_time_getClockTime(JF_TIME_CLOCK_MONOTONIC_RAW, &jts);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %llu, tv_nsec: %llu\n", jts.jts_u64Second, jts.jts_u64NanoSecond);
        ol_printf("raw time: %llu\n", rawtime);

        jf_time_sleep(5);

        jf_time_getMonotonicRawTimeInSecond(&rawtime);
        u32Ret = jf_time_getClockTime(JF_TIME_CLOCK_MONOTONIC_RAW, &jts);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %llu, tv_nsec: %llu\n", jts.jts_u64Second, jts.jts_u64NanoSecond);
        ol_printf("raw time: %llu\n", rawtime);

        jf_time_sleep(3);

        jf_time_getMonotonicRawTimeInSecond(&rawtime);
        u32Ret = jf_time_getClockTime(JF_TIME_CLOCK_MONOTONIC_RAW, &jts);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("clock time, tv_sec: %llu, tv_nsec: %llu\n", jts.jts_u64Second, jts.jts_u64NanoSecond);
        ol_printf("raw time: %llu\n", rawtime);
    }

    ol_printf("\n");

    return u32Ret;
}

static u32 _testTimeString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * data[] = {
        "00:00:00", "00:00:59",
        "11:59:59", "12:00:00",
        "12:59:59", "13:00:00",
        "13:00:01", "15:00:00",
        "23:00:00", "23:59:59"};
    olint_t i;
    olint_t hour, min, sec, seconds;

    for (i = 0; i < ARRAY_SIZE(data); i ++)
    {
        jf_time_getTimeFromString(data[i], &hour, &min, &sec);
        seconds = jf_time_convertTimeToSeconds(hour, min, sec);
        ol_printf("%s, %d secondes\n", data[i], seconds);
    }

    ol_printf("\n");

    return u32Ret;
}

static u32 _testTimePeriod(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strTime[100];
    u32 u32Peroid[] = {0, 1, 60, 61, 600, 601, 86400, 86401, 87000, 87001};
    olint_t i = 0;

    for (i = 0; i < ARRAY_SIZE(u32Peroid); i ++)
    {
        jf_time_getStringTimePeriod(strTime, sizeof(strTime), u32Peroid[i]);
        ol_printf("%u: %s\n", u32Peroid[i], strTime);
    }

    return u32Ret;
}

static u32 _printTimeRecur(u64 u64Sec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[128];

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("UTC Second : %llu\n", u64Sec);

        u32Ret = jf_time_getStringLocalTime(str, sizeof(str), u64Sec);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Local Time : %s\n", str);

        u32Ret = jf_time_getStringUtcTime(str, sizeof(str), u64Sec);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("UTC Time   : %s\n", str);

    }

    return u32Ret;
}

static u32 _testTimeRecur(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Next = 0;
    u64 u64Sec[] = {
        1580551826,
        1580465426,
        1585649426,
        1582884626,
        1551348626,
    };
    u32 index = 0;
    olchar_t * pstr[] = {
        "2005/10/20", "15:23:58",
        "2019/3/7",   "18:10:26",
        "2020/2/29",  "18:10:26",
    };
    olint_t hour, min, sec, year, mon, day;

    u32Ret = jf_time_getUtcTimeInSecond(&u64Next);
    if (u32Ret == JF_ERR_NO_ERROR)
        _printTimeRecur(u64Next);

    for (index = 0; index < ARRAY_SIZE(u64Sec); index ++)
    {
        printf("---------------------------------------------------------\n");

        _printTimeRecur(u64Sec[index]);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_time_getUtcTimeInSecondOfNextDay(u64Sec[index], &u64Next);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("\nNext Day\n");
            _printTimeRecur(u64Next);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_time_getUtcTimeInSecondOfNextWeek(u64Sec[index], &u64Next);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("\nNext Week\n");
            _printTimeRecur(u64Next);
        }
    }

    ol_printf("\n");
    for (index = 0; index < ARRAY_SIZE(pstr); index += 2)
    {
        printf("---------------------------------------------------------\n");
        ol_printf("%s, %s\n", pstr[index], pstr[index + 1]);

        jf_date_getDateFromString(pstr[index], &year, &mon, &day);

        jf_time_getTimeFromString(pstr[index + 1], &hour, &min, &sec);

        jf_time_getUtcTimeInSecondFromTimeDate(&u64Next, hour, min, sec, year, mon, day);

        _printTimeRecur(u64Next);
    }

    return u32Ret;
}

static u32 _testTimeSystemTime(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_time_val_t jtv;
    struct tm tmLocal;
    oltime_t time = 0;
#if defined(WINDOWS)
    olint_t err = 0;
#elif defined(LINUX)

#endif

    u32Ret = jf_time_getTimeOfDay(&jtv);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Second      : %llu\n", jtv.jtv_u64Second);
        ol_printf("Microsecond : %llu\n", jtv.jtv_u64MicroSecond);

        /*Necessary, incase localtime() returns error. On windows, time_t may not be 64 bits*/
        time = (oltime_t)jtv.jtv_u64Second;

#if defined(WINDOWS)
        err = localtime_s(&tmLocal, &time);
        if (err == 0)
        {
            ol_printf("tm_mon: %d\n", tmLocal.tm_mon);
            ol_printf("tm_year: %d\n", tmLocal.tm_year);
        }
        else
        {
            ol_printf("localtime error\n");
        }
#elif defined(LINUX)
        localtime_r(&time, &tmLocal);

        ol_printf("tm_mon: %d\n", tmLocal.tm_mon);
        ol_printf("tm_year: %d\n", tmLocal.tm_year);
#endif
    }

    ol_printf("\n");

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseTimeTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bTimeTestClock)
            u32Ret = _testTimeClock();
        else if (ls_bTimeTestString)
            u32Ret = _testTimeString();
        else if (ls_bTimeTestPeriod)
            u32Ret = _testTimePeriod();
        else if (ls_bTimeTestRecur)
            u32Ret = _testTimeRecur();
        else if (ls_bTimeTestSystemTime)
            u32Ret = _testTimeSystemTime();
        else
            _printTimeTestUsage();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
