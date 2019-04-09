/**
 *  @file jf_thread.c
 *
 *  @brief Thread implementation file. Provide some functional routine for
 *   managing thread
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#if defined(LINUX)
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/wait.h>
#elif defined(WINDOWS)
    #include <time.h>
    #include <process.h>
    #include <psapi.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_thread.h"
#include "jf_err.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

#if defined(LINUX)

static u32 _setThreadAttr(jf_thread_attr_t * pjta, pthread_attr_t * ppa)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pthread_attr_init(ppa);
    if (pjta->jta_bDetached)
        pthread_attr_setdetachstate(ppa, PTHREAD_CREATE_DETACHED);
    else
        pthread_attr_setdetachstate(ppa, PTHREAD_CREATE_JOINABLE);

    return u32Ret;
}

#endif

/* --- public routine section ---------------------------------------------- */

void jf_thread_initId(jf_thread_id_t * pThreadId)
{
    ol_memset(pThreadId, 0, sizeof(jf_thread_id_t));
}

boolean_t jf_thread_isValidId(jf_thread_id_t * pThreadId)
{
    boolean_t bRet = TRUE;

#if defined(LINUX)
    if (pThreadId->jti_ptThreadId == 0)
        bRet = FALSE;
#elif defined(WINDOWS)
    if (pThreadId->jti_hThread == NULL)
        bRet = FALSE;
#endif

    return bRet;
}

u32 jf_thread_create(
    jf_thread_id_t * pThreadId, jf_thread_attr_t * pAttr,
    jf_thread_fnRoutine_t fnRoutine, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_thread_id_t jti;
#if defined(LINUX)
    pthread_attr_t attr;
    olint_t ret;

    pthread_attr_init(&attr);
    if (pAttr != NULL)
        u32Ret = _setThreadAttr(pAttr, &attr);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ret = pthread_create(&(jti.jti_ptThreadId), &attr, fnRoutine, pArg);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_CREATE_THREAD;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pThreadId != NULL)
            memcpy(pThreadId, &jti, sizeof(jf_thread_id_t));
    }
#elif defined(WINDOWS)
    jti.jti_hThread = CreateThread(NULL, 0, fnRoutine, pArg, 0, NULL);
    if (jti.jti_hThread == NULL) 
        u32Ret = JF_ERR_FAIL_CREATE_THREAD;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pThreadId != NULL)
            memcpy(pThreadId, &jti, sizeof(jf_thread_id_t));
    }
#endif

    return u32Ret;
}

u32 jf_thread_terminate(jf_thread_id_t * pThreadId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = pthread_cancel(pThreadId->jti_ptThreadId);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_TERMINATE_THREAD;
    else
        jf_thread_initId(pThreadId);
#elif defined(WINDOWS)
    boolean_t bRet;
    u32 u32ExitCode = 0;

    bRet = TerminateThread(pThreadId->jti_hThread, u32ExitCode);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_TERMINATE_THREAD;
    else
        jf_thread_initId(pThreadId);
#endif

    return u32Ret;
}

u32 jf_thread_waitForThreadTermination(jf_thread_id_t threadId, u32 * pu32RetCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t ret;

    ret = pthread_join(threadId.jti_ptThreadId, NULL);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_WAIT_THREAD_TERMINATION;
#elif defined(WINDOWS)
    u32 u32Wait;

    u32Wait = WaitForSingleObject(threadId.jti_hThread, INFINITE);
    if (u32Wait != WAIT_OBJECT_0)
        u32Ret = JF_ERR_FAIL_WAIT_THREAD_TERMINATION;
#endif

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


