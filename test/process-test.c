/**
 *  @file process-test.c
 *
 *  @brief test file for process object
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
#include "errcode.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bToTerminate = FALSE;

#define MAX_THREAD_COUNT  5

/* --- private routine section---------------------------------------------- */
JF_THREAD_RETURN_VALUE _testThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = (u32)(ulong)pArg;

    ol_printf("_testThread %u starts\n", u32Index);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("_testThread %u shout\n", u32Index);
        sleep(5);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("_testThread %u quits\n", u32Index);

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Index;

    for (u32Index = 0;
         ((u32Index < MAX_THREAD_COUNT) && (u32Ret == JF_ERR_NO_ERROR));
         u32Index ++)
    {
        ol_printf("main tread, create thread %u\n", u32Index);
        u32Ret = jf_thread_create(
            NULL, NULL, _testThread, (void *)(ulong)(u32Index + 1));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("main thread, sleeping for 5 minutes\n");
        sleep(300);
        ol_printf("prepare to exit\n");
    }

    ls_bToTerminate = TRUE;

    sleep(20);

    ol_printf("main thread quits\n");

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

