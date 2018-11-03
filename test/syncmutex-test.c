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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "syncmutex.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static sync_mutex_t ls_ssLock;
static boolean_t ls_bToTerminate = FALSE;
static olint_t nMutex = 0;

#define MAX_RESOURCE_COUNT  1

/* --- private routine section---------------------------------------------- */
THREAD_RETURN_VALUE consumer(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    ol_printf("consumer %lu starts\n", getCurrentThreadId());

    while ((! ls_bToTerminate) && (u32Ret == OLERR_NO_ERROR))
    {
        acquireSyncMutex(&ls_ssLock);
        sleep(5);
        nMutex --;
        ol_printf("consumer mutex %d\n", nMutex);
        sleep(5);
        releaseSyncMutex(&ls_ssLock);
    }

    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("consumer quits\n");
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("consumer quits, %s\n", strErrMsg);
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
        acquireSyncMutex(&ls_ssLock);

        nMutex ++;
        ol_printf("producer mutex %d\n", nMutex);

        releaseSyncMutex(&ls_ssLock);
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

    u32Ret = initSyncMutex(&ls_ssLock);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createThread(NULL, NULL, producer, (void *)1);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = createThread(NULL, NULL, consumer, (void *)1);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 1 minutes\n");
            sleep(30);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy mutex\n");

        ls_bToTerminate = TRUE;

        sleep(20);

        finiSyncMutex(&ls_ssLock);

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



