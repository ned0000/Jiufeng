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
    #define getCurrentProcessId()  getpid()
    #define getCurrentThreadId()   pthread_self()
#elif defined(WINDOWS)
    #define getCurrentProcessId()  GetCurrentProcessId()
    #define getCurrentThreadId()   GetCurrentThreadId()
    typedef DWORD pthread_t;
    typedef DWORD pid_t;
    #define sleep(seconds)  Sleep(seconds * 1000)
#endif

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u8 pa_u8Reserved[32];
} process_attr_t;

typedef struct
{
#if defined(LINUX)
    pid_t pi_pId;
#elif defined(WINDOWS)
    HANDLE pi_hProcess;
#endif
} process_id_t;

typedef struct
{
    boolean_t ta_bDetached;
    u8 ta_u8Reserved[31];
} thread_attr_t;

typedef struct
{
#if defined(LINUX)
    pthread_t ti_ptThreadId;
#elif defined(WINDOWS)
    HANDLE ti_hThread;
#endif
} thread_id_t;

#if defined(LINUX)
    #if defined(JIUFENG_64BIT)
        #define THREAD_RETURN_VALUE       void *
        #define THREAD_RETURN(retval)     pthread_exit((void *)(u64)retval);
        typedef void * (* fnThreadRoutine_t)(void * pArg);
    #else
        #define THREAD_RETURN_VALUE  void *
        #define THREAD_RETURN(retval)       pthread_exit((void *)retval);
        typedef void * (* fnThreadRoutine_t)(void * pArg);
    #endif
#elif defined(WINDOWS)
    #define THREAD_RETURN_VALUE  u32 WINAPI
    #define THREAD_RETURN return
    typedef LPTHREAD_START_ROUTINE fnThreadRoutine_t;
#endif

/* --- functional routines ------------------------------------------------- */
void getPidFilename(olchar_t * pstrPidFilename, olsize_t sFile,
    olchar_t * pstrDaemonName);

u32 formCmdLineArguments(olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[]);

u32 switchToDaemon(olchar_t * pstrDaemonName);

boolean_t bAlreadyRunning(olchar_t * pstrDaemonName);



void initProcessId(process_id_t * pProcessId);

boolean_t isValidProcessId(process_id_t * pProcessId);

u32 createProcess(process_id_t * pProcessId, process_attr_t * pAttr,
    olchar_t * pstrCommandLine);

u32 terminateProcess(process_id_t * pProcessId);

/*unknown reason*/
#define PROCESS_TERMINATION_REASON_UNKNOWN  0
/*normal exit*/
#define PROCESS_TERMINATION_REASON_EXITED  1
/*Signaled*/
#define PROCESS_TERMINATION_REASON_SIGNALED  10
/*Signaled, access violation or segmentation fault*/
#define PROCESS_TERMINATION_REASON_ACCESS_VIOLATION  11
/*Signaled, terminated, SIGTERM*/
#define PROCESS_TERMINATION_REASON_TERMINATED  12
/*Signaled, terminated, SIGKILL*/
#define PROCESS_TERMINATION_REASON_KILLED  13

u32 waitForChildProcessTermination(process_id_t pidChild[], u32 u32Count,
    u32 u32BlockTime, u32 * pu32Index, u32 * pu32Reason);



void initThreadId(thread_id_t * pThreadId);

boolean_t isValidThreadId(thread_id_t * pThreadId);

u32 createThread(thread_id_t * pThreadId, thread_attr_t * pAttr,
    fnThreadRoutine_t fnThreadRoutine, void * pArg);

u32 terminateThread(thread_id_t * pThreadId);

u32 waitForThreadTermination(thread_id_t threadId, u32 * pu32RetCode);



typedef void (* fnSignalHandler_t)(olint_t signal);

u32 registerSignalHandlers(fnSignalHandler_t fnSignalHandler);


u32 getCurrentWorkingDirectory(olchar_t * pstrDir, olsize_t sDir);

u32 setCurrentWorkingDirectory(const olchar_t * pstrDir);

#endif /*JIUTAI_PROCESS_H*/

/*---------------------------------------------------------------------------*/


