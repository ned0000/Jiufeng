/**
 *  @file process.h
 *
 *  @brief Process header file. Provide some functional routine for
 *   process manipulation
 *
 *  @author Min Zhang
 *
 *  @note For linux, link with stringparse library
 *  @note For Windows, link with psapi.lib
 *  
 */

#ifndef JIUTAI_PROCESS_H
#define JIUTAI_PROCESS_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
#if defined(LINUX)
    #define jf_process_getCurrentId()  getpid()
    #define jf_thread_getCurrentId()   pthread_self()
#elif defined(WINDOWS)
    #define jf_process_getCurrentId()  GetCurrentProcessId()
    #define jf_thread_getCurrentId()   GetCurrentThreadId()
    typedef DWORD pthread_t;
    typedef DWORD pid_t;
    #define sleep(seconds)  Sleep(seconds * 1000)
#endif

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u8 jpa_u8Reserved[32];
} jf_process_attr_t;

typedef struct
{
#if defined(LINUX)
    pid_t jpi_pId;
#elif defined(WINDOWS)
    HANDLE jpi_hProcess;
#endif
} jf_process_id_t;

typedef struct
{
    boolean_t jta_bDetached;
    u8 jta_u8Reserved[31];
} jf_thread_attr_t;

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

typedef void (* jf_process_fnSignalHandler_t)(olint_t signal);

/* --- functional routines ------------------------------------------------- */

void jf_process_getPidFilename(
    olchar_t * pstrPidFilename, olsize_t sFile, olchar_t * pstrDaemonName);

u32 jf_process_formCmdLineArguments(
    olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[]);

u32 jf_process_switchToDaemon(olchar_t * pstrDaemonName);

boolean_t jf_process_isAlreadyRunning(olchar_t * pstrDaemonName);

void jf_process_initId(jf_process_id_t * pProcessId);

boolean_t jf_process_isValidId(jf_process_id_t * pProcessId);

u32 jf_process_create(
    jf_process_id_t * pProcessId, jf_process_attr_t * pAttr,
    olchar_t * pstrCommandLine);

u32 jf_process_terminate(jf_process_id_t * pProcessId);

/*unknown reason*/
#define JF_PROCESS_TERMINATION_REASON_UNKNOWN           (0)
/*normal exit*/
#define JF_PROCESS_TERMINATION_REASON_EXITED            (1)
/*Signaled*/
#define JF_PROCESS_TERMINATION_REASON_SIGNALED          (10)
/*Signaled, access violation or segmentation fault*/
#define JF_PROCESS_TERMINATION_REASON_ACCESS_VIOLATION  (11)
/*Signaled, terminated, SIGTERM*/
#define JF_PROCESS_TERMINATION_REASON_TERMINATED        (12)
/*Signaled, terminated, SIGKILL*/
#define JF_PROCESS_TERMINATION_REASON_KILLED            (13)

u32 jf_process_waitForChildProcessTermination(
    jf_process_id_t pidChild[], u32 u32Count,
    u32 u32BlockTime, u32 * pu32Index, u32 * pu32Reason);

u32 jf_process_getCurrentWorkingDirectory(olchar_t * pstrDir, olsize_t sDir);

u32 jf_process_setCurrentWorkingDirectory(const olchar_t * pstrDir);

u32 jf_process_registerSignalHandlers(
    jf_process_fnSignalHandler_t fnSignalHandler);

u32 jf_process_initSocket(void);

u32 jf_process_finiSocket(void);

void jf_thread_initId(jf_thread_id_t * pThreadId);

boolean_t jf_thread_isValidId(jf_thread_id_t * pThreadId);

u32 jf_thread_create(
    jf_thread_id_t * pThreadId, jf_thread_attr_t * pAttr,
    jf_thread_fnRoutine_t fnRoutine, void * pArg);

u32 jf_thread_terminate(jf_thread_id_t * pThreadId);

u32 jf_thread_waitForThreadTermination(jf_thread_id_t threadId, u32 * pu32RetCode);


#endif /*JIUTAI_PROCESS_H*/

/*---------------------------------------------------------------------------*/


