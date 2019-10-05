/**
 *  @file jf_process.h
 *
 *  @brief Process header file. Provide some functional routine for
 *   process manipulation
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_process object
 *  @note For linux, link with stringparse library
 *  @note For Windows, link with psapi.lib
 *  
 */

#ifndef JIUTAI_PROCESS_H
#define JIUTAI_PROCESS_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */
#if defined(LINUX)
    #define jf_process_getCurrentId()  getpid()
#elif defined(WINDOWS)
    #define jf_process_getCurrentId()  GetCurrentProcessId()
    typedef DWORD pid_t;
    #define sleep(seconds)  Sleep(seconds * 1000)
#endif

/* --- data structures -------------------------------------------------------------------------- */
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

typedef void (* jf_process_fnSignalHandler_t)(olint_t signal);

/* --- functional routines ---------------------------------------------------------------------- */

void jf_process_getPidFilename(
    olchar_t * pstrPidFilename, olsize_t sFile, olchar_t * pstrDaemonName);

u32 jf_process_formCmdLineArguments(
    olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[]);

u32 jf_process_switchToDaemon(olchar_t * pstrDaemonName);

/** Test if the process is already running.
 *  
 *  @note Check the pid file in /var/run directory, the name of the pid file is daemon-name.pid
 *  @note If the pid file is existing and the name is the same as the daemon name, another process
 *   is already running, current process will quit
 *  @note If the no pid file is found or the name is not the same as the daemon name, this function
 *   will create the pid file
 *
 *  @param pstrDaemonName [in] the daemon name
 *  
 *  @return the status of the daemon 
 *
 */
boolean_t jf_process_isAlreadyRunning(olchar_t * pstrDaemonName);

void jf_process_initId(jf_process_id_t * pProcessId);

boolean_t jf_process_isValidId(jf_process_id_t * pProcessId);

u32 jf_process_create(
    jf_process_id_t * pProcessId, jf_process_attr_t * pAttr,
    olchar_t * pstrCommandLine);

/** Send SIGKILL to process to kill the process, SIGKILL cannot be caught by process
 */
u32 jf_process_kill(jf_process_id_t * pProcessId);

/** Send SIGTERM to process to terminate the process, SIGKILL can be caught by process
 */
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

#endif /*JIUTAI_PROCESS_H*/

/*------------------------------------------------------------------------------------------------*/


