/**
 *  @file sem-test.c
 *
 *  @brief Test file for semaphore function defined in jf_sem common object.
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
#include "jf_sem.h"
#include "jf_thread.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_sem_t ls_jsSem;
static boolean_t ls_bToTerminate = FALSE;

#define MAX_RESOURCE_COUNT  5

/* --- private routine section ------------------------------------------------------------------ */

JF_THREAD_RETURN_VALUE consumer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Id = *(u32*)pArg;

    ol_printf("consumer %lu, %u starts\n", jf_thread_getCurrentId(), u32Id);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("consumer %u waits for resource\n", u32Id);
        u32Ret = jf_sem_down(&ls_jsSem);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("consumer %u consume 1 resource\n", u32Id);
    }


    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("consumer %u quits\n", u32Id);
    else
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("consumer %u quits, %s\n", u32Id, strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE producer(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Id = *(u32 *)pArg;

    ol_printf("producer %lu, %u starts\n", jf_thread_getCurrentId(), u32Id);

    while ((! ls_bToTerminate) && (u32Ret == JF_ERR_NO_ERROR))
    {
        ol_printf("producer %u produce 1 resource\n", u32Id);
        u32Ret = jf_sem_up(&ls_jsSem);
        ol_sleep(10);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("producer %u quits\n", u32Id);
    else
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("producer %u quits, %s\n", u32Id, strErrMsg);
    }

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    u32 u32Index;
    u32 u32Id[MAX_RESOURCE_COUNT];
    u32 u32ProducerId = 1;

    u32Ret = jf_sem_init(&ls_jsSem, 0, MAX_RESOURCE_COUNT);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_thread_create(NULL, NULL, producer, &u32ProducerId);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            for (u32Index = 0;
                 ((u32Index < MAX_RESOURCE_COUNT) && (u32Ret == JF_ERR_NO_ERROR));
                 u32Index ++)
            {
                u32Id[u32Index] = u32Index + 1;
                u32Ret = jf_thread_create(NULL, NULL, consumer, &u32Id[u32Index]);
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("main thread, sleeping for 5 minutes\n");
            ol_sleep(300);
            ol_printf("prepare to exit\n");
        }

        ol_printf("destroy semaphore\n");

        ls_bToTerminate = TRUE;

        jf_sem_fini(&ls_jsSem);

        ol_sleep(20);

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
