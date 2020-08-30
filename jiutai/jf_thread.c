/**
 *  @file jf_thread.c
 *
 *  @brief Thread implementation file. Provide some functional routine for managing thread.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdlib.h>
#if defined(LINUX)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/wait.h>
#elif defined(WINDOWS)
    #include <time.h>
    #include <process.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_thread.h"
#include "jf_err.h"
#include "jf_time.h"

/* --- private data/data structure section ------------------------------------------------------ */

#if defined(LINUX)

typedef struct
{
    sigset_t sha_ssSet;
    jf_thread_fnSignalHandler_t sha_fnHandler;
} signal_handler_arg;

static signal_handler_arg ls_shaSignalArg;

#endif

/* --- private routine section ------------------------------------------------------------------ */

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

/** The thread to handle signal.
 *
 *  @note
 *  -# If the signal(not real signal) is sent to this process more than once at the same time,
 *   there is possibility that only 1 signal is received and others are missed.
 *   for SIGCHLD, this problem will cause the defunct process cannot be handled by parent process.
 */
JF_THREAD_RETURN_VALUE _signalHandlerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t sig, ret;
    signal_handler_arg * pSha = (signal_handler_arg *)pArg;
    boolean_t bToTerminate = FALSE;
    sigset_t set = pSha->sha_ssSet;
    jf_thread_fnSignalHandler_t fnHandler = pSha->sha_fnHandler;

    while (! bToTerminate)
    {
        ret = sigwait(&set, &sig);
        if (ret == 0)
        {
            fnHandler(sig);
            if ((sig == SIGTERM) || (sig == SIGINT))
                break;
        }
    }

    JF_THREAD_RETURN(u32Ret);
}

#endif

/* --- public routine section ------------------------------------------------------------------- */

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
    jf_thread_id_t * pThreadId, jf_thread_attr_t * pAttr, jf_thread_fnRoutine_t fnRoutine,
    void * pArg)
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
            ol_memcpy(pThreadId, &jti, sizeof(jf_thread_id_t));
    }
#elif defined(WINDOWS)
    jti.jti_hThread = CreateThread(NULL, 0, fnRoutine, pArg, 0, NULL);
    if (jti.jti_hThread == NULL) 
        u32Ret = JF_ERR_FAIL_CREATE_THREAD;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pThreadId != NULL)
            ol_memcpy(pThreadId, &jti, sizeof(jf_thread_id_t));
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

    u32Wait = WaitForSingleObject(threadId.jti_hThread, JF_TIME_INFINITE);
    if (u32Wait != WAIT_OBJECT_0)
        u32Ret = JF_ERR_FAIL_WAIT_THREAD_TERMINATION;
#endif

    return u32Ret;
}

u32 jf_thread_registerSignalHandlers(jf_thread_fnSignalHandler_t fnSignalHandler)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nIndex = 0;
#if defined(WINDOWS)
    olint_t nSignals[] = {SIGTERM, SIGINT};
    olint_t nSignalCount = 2;

    while ((u32Ret == JF_ERR_NO_ERROR) && (nIndex < nSignalCount))
    {
        signal(nSignals[nIndex], fnSignalHandler);
        nIndex ++;
    }

#elif defined(LINUX)

    olint_t nSignals[] = {SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGTSTP, SIGINT, SIGCHLD};
    olint_t nRet = 0, nSignalCount = 8;
    sigset_t ssSignalSet;

    sigemptyset(&ssSignalSet);

    for (nIndex = 0; nIndex < nSignalCount; nIndex ++)
    {
        sigaddset(&ssSignalSet, nSignals[nIndex]);
    }

    /*Block all the signals in this thread.*/
    nRet = pthread_sigmask(SIG_BLOCK, &ssSignalSet, NULL);
    if (nRet != 0)
        u32Ret = JF_ERR_OPERATION_FAIL;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ls_shaSignalArg.sha_ssSet = ssSignalSet;
        ls_shaSignalArg.sha_fnHandler = fnSignalHandler;

        /*Create a thread, the new thread inherits the signal handle.*/
        u32Ret = jf_thread_create(NULL, NULL, _signalHandlerThread, (void *)&ls_shaSignalArg);
    }

#endif
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

