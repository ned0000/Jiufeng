/**
 *  @file syncsem-test.c
 *
 *  @brief test file for syncsem common object
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
#include "syncsem.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static jf_sem_t ls_jsSem;
static boolean_t ls_bToTerminate = FALSE;

#define MAX_RESOURCE_COUNT  5

/* --- private routine section---------------------------------------------- */
JF_THREAD_RETURN_VALUE consumer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
#if defined(JIUFENG_64BIT)
    u32 u32Index = (u32)(u64)pArg;
#else
    u32 u32Index = (u32)pArg;
#endif
    ol_printf("consumer %lu, %u starts\n", jf_thread_getCurrentId(), u32Index);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("consumer %u waits for resource\n", u32Index);
        u32Ret = jf_sem_down(&ls_jsSem);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("consumer %u consume 1 resource\n", u32Index);
    }


    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("consumer %u quits\n", u32Index);
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("consumer %u quits, %s\n", u32Index, strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE producer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
#if defined(JIUFENG_64BIT)
    u32 u32Index = (u32)(u64)pArg;
#else
    u32 u32Index = (u32)pArg;
#endif

    ol_printf("producer %lu, %u starts\n", jf_thread_getCurrentId(), u32Index);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("producer %u produce 1 resource\n", u32Index);
        u32Ret = jf_sem_up(&ls_jsSem);
        sleep(10);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("producer %u quits\n", u32Index);
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("producer %u quits, %s\n", u32Index, strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Index;

    u32Ret = jf_sem_init(&ls_jsSem, 0, MAX_RESOURCE_COUNT);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_thread_create(NULL, NULL, producer, (void *)(ulong)1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (u32Index = 0;
                 ((u32Index < MAX_RESOURCE_COUNT) && (u32Ret == JF_ERR_NO_ERROR));
                 u32Index ++)
            {
                u32Ret = jf_thread_create(
                    NULL, NULL, consumer, (void *)(ulong)(u32Index + 1));
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 5 minutes\n");
            sleep(300);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy semaphore\n");

        ls_bToTerminate = TRUE;

        jf_sem_fini(&ls_jsSem);

        sleep(20);

        ol_printf("main thread quits\n");
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

