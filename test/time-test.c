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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_time.h"
#include "jf_string.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

#define NUM_OF_TEST5_ENTRY  10

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
        jf_string_getTimeFromString(data[i], &hour, &min, &sec);
        seconds = jf_time_convertTimeToSeconds(hour, min, sec);
        ol_printf("%s, %d secondes\n", data[i], seconds);
    }

    ol_printf("\n");
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    test5();

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

