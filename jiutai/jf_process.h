/**
 *  @file jf_process.h
 *
 *  @brief Process header file which provide some functional routine for process manipulation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_process object.
 *  -# For linux, link with stringparse library.
 *  -# For Windows, link with psapi.lib.
 *  -# Process handle and process id are 2 different concepts.
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

/** Define the process attribute data type.
 */
typedef struct
{
    u8 jpa_u8Reserved[32];
} jf_process_attr_t;

/** Define the process handle data type.
 */
typedef struct
{
#if defined(LINUX)
    pid_t jpi_pId;
#elif defined(WINDOWS)
    HANDLE jpi_hProcess;
#endif
} jf_process_handle_t;

/** The callback function to handle signal.
 */
typedef void (* jf_process_fnSignalHandler_t)(olint_t signal);

/** The termination reason code.
 */
typedef enum
{
    /**Unknown reason.*/
    JF_PROCESS_TERMINATION_REASON_UNKNOWN = 0,
    /**Normal exit.*/
    JF_PROCESS_TERMINATION_REASON_EXITED,
    /**Signaled.*/
    JF_PROCESS_TERMINATION_REASON_SIGNALED = 10,
    /**Signaled, access violation or segmentation fault.*/
    JF_PROCESS_TERMINATION_REASON_ACCESS_VIOLATION,
    /**Signaled, terminated, SIGTERM*/
    JF_PROCESS_TERMINATION_REASON_TERMINATED,
    /**Signaled, terminated, SIGKILL*/
    JF_PROCESS_TERMINATION_REASON_KILLED,
} jf_process_termination_reason_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get the PID file name for a given daemon name.
 *
 *  @param pstrPidFilename [out] The buffer for the PID file name.
 *  @param sFile [in] The size of the file name buffer.
 *  @param pstrDaemonName [in] The daemon name.
 *
 *  @return Void.
 */
void jf_process_getPidFilename(
    olchar_t * pstrPidFilename, olsize_t sFile, olchar_t * pstrDaemonName);

/** Break the command string into pointer array.
 *
 *  @param pstrCmd [out] The command string.
 *  @param psArgc [in/out] The size of the argument array as in parameter. Number of argument in the
 *   array as out parameter.
 *  @param argv [out] The pointer array for the argument.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_formCmdLineArguments(olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[]);

/** Switch the process to daemon, so it can detach themselves from the controlling terminal and run
 *  in the background.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_switchToDaemon(void);

/** Test if the process is already running.
 *  
 *  @note
 *  -# Check the pid file in /var/run directory, the name of the pid file is "daemon-name.pid".
 *  -# If the pid file is existing and the name is the same as the daemon name, another process
 *   is already running, current process will quit.
 *  -# If no pid file is found or the name is not the same as the daemon name, this function
 *   will create the pid file.
 *
 *  @param pstrDaemonName [in] The daemon name.
 *  
 *  @return The status of the daemon.
 *  @retval TRUE The daemon is running.
 *  @retval FALSE The daemon is not running.
 */
boolean_t jf_process_isAlreadyRunning(olchar_t * pstrDaemonName);

/** Initialize the process handle.
 *
 *  @param pHandle [in] The process handle to be initialized.
 *
 *  @return Void.
 */
void jf_process_initHandle(jf_process_handle_t * pHandle);

/** Check if the process handle is valid or not.
 *
 *  @param pHandle [in] The process handle to be checked.
 *
 *  @return The status of the process handle.
 *  @retval TRUE If the process handle is valid.
 *  @retval FALSE If the process handle is invalid.
 */
boolean_t jf_process_isValidHandle(jf_process_handle_t * pHandle);

/** Create a process.
 *
 *  @param pHandle [out] The process handle to be created.
 *  @param pAttr [in] The attribute for creating the process.
 *  @param pstrCommandLine [in] The command line to run as a process.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_create(
    jf_process_handle_t * pHandle, jf_process_attr_t * pAttr, olchar_t * pstrCommandLine);

/** Send SIGKILL to process to kill the process.
 *
 *  @note
 *  -# SIGKILL can not be caught by process, the process will be stopped anyway.
 *  -# It's recommended to use jf_process_terminate() firstly and then this routine after waiting
 *   serveral seconds.
 *
 *  @param pHandle [in] The process handle to be killed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_kill(jf_process_handle_t * pHandle);

/** Send SIGTERM to process to terminate the process.
 *
 *  @note
 *  -# SIGTERM can be caught by process, the process may not be terminated for some reasons.
 *
 *  @param pHandle [in] The process handle to be terminated.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_terminate(jf_process_handle_t * pHandle);

/** Wait for child process's termination.
 *
 *  @note
 *  -# The function returns if a child process terminates.
 *
 *  @param pidChild [in] The child process handle array to wait.
 *  @param u32Count [in] The count of child process array.
 *  @param u32BlockTime [in] The block time in millisecond.
 *  @param pu32Index [out] The index in array where the child is terminated.
 *  @param pu32Reason [out] The termination reason defined as jf_process_termination_reason_t.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_waitForChildProcessTermination(
    jf_process_handle_t pidChild[], u32 u32Count, u32 u32BlockTime, u32 * pu32Index, u32 * pu32Reason);

/** Get current working directory of the process.
 *
 *  @note
 *  -# Caller should make sure the buffer is big enough to hold the working directory name.
 *
 *  @param pstrDir [out] The buffer for the working directory.
 *  @param sDir [in] The size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_GET_CWD Failed to get current working directory.
 */
u32 jf_process_getCurrentWorkingDirectory(olchar_t * pstrDir, olsize_t sDir);

/** Set current working directory of the process.
 *
 *  @param pstrDir [in] The buffer for the working directory.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SET_CWD Failed to set current working directory.
 */
u32 jf_process_setCurrentWorkingDirectory(const olchar_t * pstrDir);

/** Register signal handler for the process.
 *
 *  @param fnSignalHandler [in] The signal handler.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OPERATION_FAIL Failed to set signal handler.
 */
u32 jf_process_registerSignalHandlers(jf_process_fnSignalHandler_t fnSignalHandler);

/** Ignore the specified signal.
 *
 *  @param sig [in] The signal to ignore.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_ignoreSignal(olint_t sig);

/** Initialize the socket library.
 *
 *  @note
 *  -# For Linux platform, this function do nothing.
 *  -# For Window platform, this function confirms the winsock version. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_initSocket(void);

/** Finalize the socket library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_process_finiSocket(void);

#endif /*JIUTAI_PROCESS_H*/

/*------------------------------------------------------------------------------------------------*/


