/**
 *  @file jf_thread.h
 *
 *  @brief Header file which provide some functional routine for thread manipulation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_thread object.
 *  -# For linux, link with jf_string library.
 *  -# For Windows, link with psapi.lib.
 *  
 */

#ifndef JIUTAI_THREAD_H
#define JIUTAI_THREAD_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <signal.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */
#if defined(LINUX)
    #define jf_thread_getCurrentId()   pthread_self()
#elif defined(WINDOWS)
    #define jf_thread_getCurrentId()   GetCurrentThreadId()
    typedef DWORD pthread_t;
    #define sleep(seconds)  Sleep(seconds * 1000)
#endif

/* --- data structures -------------------------------------------------------------------------- */

/** Define the thread attribute data type.
 */
typedef struct
{
    /**Detach the thread if it's TRUE.*/
    boolean_t jta_bDetached;
    u8 jta_u8Reserved[31];
} jf_thread_attr_t;

/** Define the thread id data type.
 */
typedef struct
{
#if defined(LINUX)
    pthread_t jti_ptThreadId;
#elif defined(WINDOWS)
    HANDLE jti_hThread;
#endif
} jf_thread_id_t;

#if defined(LINUX)
    #if defined(JIUFENG_64BIT)
        #define JF_THREAD_RETURN_VALUE       void *
        #define JF_THREAD_RETURN(retval)     pthread_exit((void *)(u64)retval);
        typedef void * (* jf_thread_fnRoutine_t)(void * pArg);
    #else
        #define JF_THREAD_RETURN_VALUE       void *
        #define JF_THREAD_RETURN(retval)     pthread_exit((void *)retval);
        typedef void * (* jf_thread_fnRoutine_t)(void * pArg);
    #endif
#elif defined(WINDOWS)
    #define JF_THREAD_RETURN_VALUE           u32 WINAPI
    #define JF_THREAD_RETURN                 return
    typedef LPTHREAD_START_ROUTINE           jf_thread_fnRoutine_t;
#endif

/** The callback function for signal handler.
 */
typedef void (* jf_thread_fnSignalHandler_t)(olint_t signal);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the thread id.
 *
 *  @param pThreadId [out] The thread ID.
 *
 *  @return Void.
 */
void jf_thread_initId(jf_thread_id_t * pThreadId);

/** Check if it's a valid thread id.
 *
 *  @param pThreadId [in] The thread id to be checked.
 *
 *  @return The status of the thread id.
 *  @retval TRUE If the thread id is valid.
 *  @retval FALSE If the thread id is invalid.
 */
boolean_t jf_thread_isValidId(jf_thread_id_t * pThreadId);

/** Create thread and run the specified routine.
 *
 *  @param pThreadId [out] The thread id created.
 *  @param pAttr [in] The attribute for creating the thread.
 *  @param fnRoutine [in] The routine to be called for the thread.
 *  @param pArg [in] The argument for the routine.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_thread_create(
    jf_thread_id_t * pThreadId, jf_thread_attr_t * pAttr, jf_thread_fnRoutine_t fnRoutine,
    void * pArg);

/** Terminate the thread.
 *
 *  @note
 *  -# If the thread is terminated successfully, the ID is reinitialized.
 *
 *  @param pThreadId [in] The thread id to be terminated.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_TERMINATE_THREAD Failed to terminate the thread.
 */
u32 jf_thread_terminate(jf_thread_id_t * pThreadId);

/** Wait for thread termination.
 *
 *  @note
 *  -# The thread must be joinable.
 *  -# If the thread has already terminated, the function returns immediately.
 *
 *  @param threadId [in] The thread id to wait.
 *  @param pu32RetCode [out] The thread termination reason.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_thread_waitForThreadTermination(jf_thread_id_t threadId, u32 * pu32RetCode);

/** Register signal handler for thread.
 *
 *  @note
 *  -# The function will create a dedicated thread to handle the signal. Different from traditional
 *   signal handler scheme, this scheme will handle the signal asynchronously. So other threads
 *   won't be interrupted by signal.
 *
 *  @param fnSignalHandler [in] The signal handler.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_thread_registerSignalHandlers(jf_thread_fnSignalHandler_t fnSignalHandler);

#endif /*JIUTAI_THREAD_H*/

/*------------------------------------------------------------------------------------------------*/


