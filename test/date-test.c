/**
 *  @file date-test.c
 *
 *  @brief Test file for date function defined in jf_date common object.
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
#include "jf_date.h"
#include "jf_string.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestDateLeapYear = FALSE;

static boolean_t ls_bTestDaysToDate = FALSE;

static boolean_t ls_bTestDayOfWeek = FALSE;

static boolean_t ls_bTestDateToDays = FALSE;

static boolean_t ls_bTestTradingDate = FALSE;

static boolean_t ls_bTestDateString = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printDateTestUsage(void)
{
    ol_printf("\
Usage: time-test [-l] [-a] [-w] [-t] [-d] [-s]\n\
    -l test leap year.\n\
    -a test days to date.\n\
    -w test day of week.\n\
    -t test trading date.\n\
    -d test date to days.\n\
    -s test date string.\n");

    ol_printf("\n");

}

static u32 _parseDateTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lawtdsh?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printDateTestUsage();
            exit(0);
            break;
        case 'l':
            ls_bTestDateLeapYear = TRUE;
            break;
        case 'a':
            ls_bTestDaysToDate = TRUE;
            break;
        case 'w':
            ls_bTestDayOfWeek = TRUE;
            break;
        case 't':
            ls_bTestTradingDate = TRUE;
            break;
        case 'd':
            ls_bTestDateToDays = TRUE;
            break;
        case 's':
            ls_bTestDateString = TRUE;
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

/*leap year*/
static u32 _testDateLeapYear(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year[] = {234, 1600, 1700, 1900, 1950, 2000, 2002, 2004, 2007, 2008};
    olint_t i;

    for (i = 0; i < ARRAY_SIZE(year); i ++)
    {
        if (jf_date_isLeapYear(year[i]))
            ol_printf("year %d is leap year\n", year[i]);
        else
            ol_printf("year %d is not leap year\n", year[i]);
    }
    ol_printf("\n");

    return u32Ret;
}

#define NUM_OF_TEST_ENTRY  6

static u32 _testDateToDays(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t data[NUM_OF_TEST_ENTRY][3] = {
        {1970, 1, 1}, {1970, 6, 5},
        {1970, 11, 20}, {1971, 1, 5},
        {1971, 5, 1}, {1971, 10, 5}};
    olint_t i;

    for (i = 0; i < NUM_OF_TEST_ENTRY; i ++)
    {
        ol_printf("%d-%d-%d is %d days from 1970\n",
               data[i][0], data[i][1], data[i][2],
               jf_date_convertDateToDaysFrom1970(data[i][0], data[i][1], data[i][2]));
    }

    ol_printf("\n");

    return u32Ret;
}

static u32 _testDaysToDate(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t days[] = {
        1, 156, 324, 370, 486, 643, 12400, 12756, 12759, 13125, 13149, 13150, 13151};
    olint_t i;
    olint_t year, mon, day;


    for (i = 0; i < ARRAY_SIZE(days); i ++)
    {
        jf_date_convertDaysFrom1970ToDate(days[i], &year, &mon, &day);
        ol_printf("%d days from 1700 is %d-%d-%d\n",
               days[i], year, mon, day);

    }

    ol_printf("\n");

    return u32Ret;
}

#define NUM_OF_TEST4_ENTRY  10

static u32 _testDayOfWeek(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t data[NUM_OF_TEST4_ENTRY][3] = {
        {2013, 1, 1}, {2013, 6, 5},
        {2013, 11, 20}, {2014, 1, 5},
        {2014, 5, 1}, {2014, 10, 5},
        {2013, 2, 28}, {2014, 7, 31},
        {2014, 9, 30}, {2014, 12, 5}};
    olint_t i;

    for (i = 0; i < NUM_OF_TEST4_ENTRY; i ++)
    {
        ol_printf("%d-%d-%d is %d of week\n",
               data[i][0], data[i][1], data[i][2],
               jf_date_getDayOfWeekFromDate(data[i][0], data[i][1], data[i][2]));
    }

    ol_printf("\n");

    return u32Ret;
}

static u32 _testGetNextTradingDate(
    const olchar_t * pstrCurr, olchar_t * pstrNext, olsize_t sNext)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year, month, day;
    olint_t days, dw;

    jf_date_getDate2FromString(pstrCurr, &year, &month, &day);
    days = jf_date_convertDateToDaysFrom1970(year, month, day);
    dw = jf_date_getDayOfWeekFromDate(year, month, day);
    if (dw == 5)
    {
        jf_date_convertDaysFrom1970ToDate(days + 3, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, sNext, year, month, day);
    }
    else if (dw == 6)
    {
        jf_date_convertDaysFrom1970ToDate(days + 2, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, sNext, year, month, day);
    }
    else
    {
        jf_date_convertDaysFrom1970ToDate(days + 1, &year, &month, &day);
        jf_date_getStringDate2(pstrNext, sNext, year, month, day);
    }

    return u32Ret;
}

static u32 _testTradingDate(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    char curdate[16];
    char strdate[16];

    ol_strcpy(curdate, "2004-12-03");
    _testGetNextTradingDate(curdate, strdate, sizeof(strdate));
    ol_printf("curdate: %s, nextdate: %s\n", curdate, strdate);

    return u32Ret;
}

static u32 _testDateString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t year, mon, day;
    olchar_t str[128];

    jf_date_getDateToday(&year, &mon, &day);

    jf_date_getStringDate(str, sizeof(str), year, mon, day);
    ol_printf("Today is %s\n", str);

    jf_date_getStringDate2(str, sizeof(str), year, mon, day);
    ol_printf("Today is %s\n", str);


    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseDateTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bTestDateLeapYear)
            u32Ret = _testDateLeapYear();
        else if (ls_bTestDateToDays)
            u32Ret = _testDateToDays();
        else if (ls_bTestDaysToDate)
            u32Ret = _testDaysToDate();
        else if (ls_bTestDayOfWeek)
            u32Ret = _testDayOfWeek();
        else if (ls_bTestTradingDate)
            u32Ret = _testTradingDate();
        else if (ls_bTestDateString)
            u32Ret = _testDateString();
        else
            _printDateTestUsage();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
