/**
 *  @file rwlock-test.c
 *
 *  @brief Test file for read-write lock defined in jf_rwlock common object.
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
#include "jf_rwlock.h"
#include "jf_process.h"
#include "jf_thread.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_rwlock_t ls_jrLock;
static boolean_t ls_bToTerminate = FALSE;
static olint_t nRwlock = 0;

#define MAX_RESOURCE_COUNT  1

/* --- private routine section ------------------------------------------------------------------ */

JF_THREAD_RETURN_VALUE consumer1(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", jf_thread_getCurrentId());

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_rwlock_acquireReadlock(&ls_jrLock);
        ol_printf("consumer 1 readlock %d\n", nRwlock);
        jf_rwlock_releaseReadlock(&ls_jrLock);
        sleep(4);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("consumer 1 quits\n");
    else
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("consumer 1 quits, %s\n", strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE consumer2(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", jf_thread_getCurrentId());

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_rwlock_acquireReadlock(&ls_jrLock);
        ol_printf("consumer 2 readlock %d\n", nRwlock);
        jf_rwlock_releaseReadlock(&ls_jrLock);
        sleep(1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("consumer 2 quits\n");
    else
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("consumer 2 quits, %s\n", strErrMsg);
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
        jf_rwlock_acquireWritelock(&ls_jrLock);

        nRwlock += 5;
        ol_printf("producer writelock %d\n", nRwlock);

        jf_rwlock_releaseWritelock(&ls_jrLock);
        sleep(1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("producer quits\n");
    else
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("producer quits, %s\n", strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = jf_rwlock_init(&ls_jrLock);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_thread_create(NULL, NULL, producer, (void *)1);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_thread_create(NULL, NULL, consumer1, (void *)1);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_thread_create(NULL, NULL, consumer2, (void *)2);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 1 minutes\n");
            sleep(30);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy rwlock\n");

        ls_bToTerminate = TRUE;

        sleep(20);

        jf_rwlock_fini(&ls_jrLock);

        ol_printf("main thread quits\n");
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
