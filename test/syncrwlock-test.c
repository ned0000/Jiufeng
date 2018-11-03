/**
 *  @file syncrwlock-test.c
 *
 *  @brief test file for syncrwlock common object
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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "syncrwlock.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static sync_rwlock_t ls_ssLock;
static boolean_t ls_bToTerminate = FALSE;
static olint_t nRwlock = 0;

#define MAX_RESOURCE_COUNT  1

/* --- private routine section---------------------------------------------- */
THREAD_RETURN_VALUE consumer1(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", getCurrentThreadId());

    while ((! ls_bToTerminate) && (u32Ret == OLERR_NO_ERROR))
    {
        acquireSyncReadlock(&ls_ssLock);
        ol_printf("consumer 1 readlock %d\n", nRwlock);
        releaseSyncReadlock(&ls_ssLock);
        sleep(4);
    }

    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("consumer 1 quits\n");
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("consumer 1 quits, %s\n", strErrMsg);
    }

    THREAD_RETURN(u32Ret);
}

THREAD_RETURN_VALUE consumer2(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", getCurrentThreadId());

    while ((! ls_bToTerminate) && (u32Ret == OLERR_NO_ERROR))
    {
        acquireSyncReadlock(&ls_ssLock);
        ol_printf("consumer 2 readlock %d\n", nRwlock);
        releaseSyncReadlock(&ls_ssLock);
        sleep(1);
    }

    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("consumer 2 quits\n");
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("consumer 2 quits, %s\n", strErrMsg);
    }

    THREAD_RETURN(u32Ret);
}

THREAD_RETURN_VALUE producer(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("producer %lu starts\n", getCurrentThreadId());

    while ((! ls_bToTerminate) && (u32Ret == OLERR_NO_ERROR))
    {
        acquireSyncWritelock(&ls_ssLock);

        nRwlock += 5;
        ol_printf("producer writelock %d\n", nRwlock);

        releaseSyncWritelock(&ls_ssLock);
        sleep(1);
    }

    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("producer quits\n");
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("producer quits, %s\n", strErrMsg);
    }

    THREAD_RETURN(u32Ret);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = initSyncRwlock(&ls_ssLock);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createThread(NULL, NULL, producer, (void *)1);
        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = createThread(NULL, NULL, consumer1, (void *)1);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = createThread(NULL, NULL, consumer2, (void *)2);

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 1 minutes\n");
            sleep(30);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy rwlock\n");

        ls_bToTerminate = TRUE;

        sleep(20);

        finiSyncRwlock(&ls_ssLock);

        ol_printf("main thread quits\n");
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

