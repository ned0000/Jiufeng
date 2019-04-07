/**
 *  @file syncmutex-test.c
 *
 *  @brief test file for syncmutex common object
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
#include "syncmutex.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static jf_mutex_t ls_jmLock;
static boolean_t ls_bToTerminate = FALSE;
static olint_t nMutex = 0;

#define MAX_RESOURCE_COUNT  1

/* --- private routine section---------------------------------------------- */
JF_THREAD_RETURN_VALUE consumer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", jf_thread_getCurrentId());

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_mutex_acquire(&ls_jmLock);
        sleep(5);
        nMutex --;
        ol_printf("consumer mutex %d\n", nMutex);
        sleep(5);
        jf_mutex_release(&ls_jmLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("consumer quits\n");
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("consumer quits, %s\n", strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE producer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("producer %lu starts\n", jf_thread_getCurrentId());

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_mutex_acquire(&ls_jmLock);

        nMutex ++;
        ol_printf("producer mutex %d\n", nMutex);

        jf_mutex_release(&ls_jmLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("producer quits\n");
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("producer quits, %s\n", strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = jf_mutex_init(&ls_jmLock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_thread_create(NULL, NULL, producer, (void *)1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_thread_create(NULL, NULL, consumer, (void *)1);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 1 minutes\n");
            sleep(30);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy mutex\n");

        ls_bToTerminate = TRUE;

        sleep(20);

        jf_mutex_fini(&ls_jmLock);

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



