/**
 *  @file jf_process.c
 *
 *  @brief Implementation file provide some functional routine for managing process.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <signal.h>

#if defined(WINDOWS)
    #include <time.h>
    #include <process.h>
#elif defined(LINUX)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <errno.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_err.h"
#include "jf_time.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

#if defined(LINUX)

static u32 _runCommandLine(olchar_t * pstrCommandLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t argc = 80;
    olchar_t * argv[80];
    olint_t ret = 0;

    u32Ret = jf_process_formCmdLineArguments(pstrCommandLine, &argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ret = execv(argv[0], (olchar_t **)argv);

        exit(ret);
    }

    return u32Ret;
}

static void _setProcessTerminationReason(olint_t nStatus, u8 * pu8Reason)
{
    olint_t nSignal = 0;

    if (WIFEXITED(nStatus))
    {
        *pu8Reason = JF_PROCESS_TERMINATION_REASON_EXITED;
    }
    else if (WIFSIGNALED(nStatus))
    {
        nSignal = WTERMSIG(nStatus);
        if (nSignal == SIGKILL)
            *pu8Reason = JF_PROCESS_TERMINATION_REASON_KILLED;
        else if (nSignal == SIGTERM)
            *pu8Reason = JF_PROCESS_TERMINATION_REASON_TERMINATED;
        else if (nSignal == SIGSEGV)
            *pu8Reason = JF_PROCESS_TERMINATION_REASON_ACCESS_VIOLATION;
        else
            *pu8Reason = JF_PROCESS_TERMINATION_REASON_SIGNALED;
    }
    else
    {
        *pu8Reason = JF_PROCESS_TERMINATION_REASON_UNKNOWN;
    }
}
#endif

#if defined(LINUX)

/** Get the process name by PID.
 *
 *  @note
 *  -# The name of the process is retrieved from proc file at "/proc/PID/status". The first line
 *   of the file is like "Name:\tPROCESS-NAME".
 */
static boolean_t _getProcessNameByPid(pid_t pid, olchar_t * pstrName, olsize_t sName)
{
    boolean_t bFound = FALSE;
    olchar_t strBuf[128];
    olint_t fdStatus = 0;
    olint_t nRet;

    /*Check if the given process id is matched by current process ID.*/
    if (pid != jf_process_getCurrentId())
    {
        /*Open the proc file.*/
        ol_sprintf(strBuf, "/proc/%d/status", pid);
        fdStatus = ol_open(strBuf, O_RDONLY);

        if (fdStatus > 0)
        {
            /*Read the proc file.*/
            ol_bzero(strBuf, sizeof(strBuf));
            nRet = ol_read(fdStatus, strBuf, sizeof(strBuf) - 1);
            if (nRet > 0)
            {
                strBuf[nRet] = '\0';
                /*Scan the buffer to get the name.*/
                nRet = ol_sscanf(strBuf, "Name:\t%s", pstrName);
                if (nRet == 1)
                {
                    bFound = TRUE;
                }
            }

            ol_close(fdStatus);
        }
    }

    return bFound;
}

static boolean_t _isProcessAlreadyRunning(olchar_t * pstrPidFile, olchar_t * pstrDaemonName)
{
    boolean_t bRunning = FALSE;
    olint_t fd = 0;
    olchar_t strBuf[128];
    pid_t pid = 0;
    olint_t nRet = 0;

    /*Open the PID file.*/
    fd = ol_open(pstrPidFile, O_RDONLY);
    if (fd > 0)
    {
        /*Read the PID file.*/
        ol_bzero(strBuf, sizeof(strBuf));

        nRet = ol_read(fd, strBuf, sizeof(strBuf) - 1);
        if (nRet > 0)
        {
            /*Scan the PID file to get the PID.*/
            strBuf[nRet] = 0;
            ol_sscanf(strBuf, "%d\n", &pid);

            /*Get the process name by PID.*/
            if (_getProcessNameByPid(pid, strBuf, sizeof(strBuf)))
            {
                /*Compare the daemon name.*/
                if (ol_strcmp(strBuf, pstrDaemonName) == 0)
                    bRunning = TRUE;
            }
        }

        ol_close(fd);
    }

    return bRunning;
}
#endif

/* --- public routine section ------------------------------------------------------------------- */

void jf_process_getPidFilename(
    olchar_t * pstrPidFilename, olsize_t sFile, olchar_t * pstrDaemonName)
{
    pstrPidFilename[0] = '\0';
#if defined(LINUX)
    ol_snprintf(pstrPidFilename, sFile - 1, "/var/run/%s.pid", pstrDaemonName);
    pstrPidFilename[sFile - 1] = '\0';
#endif
}

u32 jf_process_formCmdLineArguments(
    olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t cDelimiter = ' ';
    olchar_t cQuote = '"';
    u32 u32Length = 0, u32Index = 0, u32Position = 0;
    olsize_t argc = 0;

    ol_bzero(argv, sizeof(olchar_t **) * (*psArgc));

    u32Length = (u32)ol_strlen(pstrCmd);

    while ((u32Index < u32Length) && (argc < *psArgc))
    {
        /*The start quote.*/
        if (pstrCmd[u32Index] == cQuote)
        {
            while ((pstrCmd[u32Index + 1] != cQuote) && ((u32Index + 1) < u32Length))
            {
                /*Search for the end quote.*/
                u32Index ++;
            }
            if (pstrCmd[u32Index + 1] != cQuote)
            {
                u32Ret = JF_ERR_MISSING_QUOTE;
            }
            else
            {
                u32Index ++;
                if (pstrCmd[u32Index + 1] == 0)
                {
                    /*End of comamnd.*/
                    argv[argc ++] = pstrCmd + u32Position;
                    u32Position = u32Index + 1;
                }
            }
        }
        else if (pstrCmd[u32Index] == cDelimiter)
        {
            /*Space.*/
            pstrCmd[u32Index] = '\0';
            argv[argc ++] = pstrCmd + u32Position;
            u32Position = u32Index + 1;

            while (pstrCmd[u32Index + 1] == cDelimiter)
            {
                /*Extra spaces.*/
                u32Position ++;
                u32Index ++;
            }
        }
        else if (pstrCmd[u32Index + 1] == '\0')
        {
            /*End of comamnd.*/
            argv[argc ++] = pstrCmd + u32Position;
            u32Position = u32Index + 1;
        }

        u32Index ++;
    } /*While loop.*/

    *psArgc = argc;

    return u32Ret;
}

u32 jf_process_switchToDaemon(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    pid_t pid;

#if defined(LINUX)
    pid = fork();
    if (pid == -1)
    {
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;
    }
    else if (pid != 0)
    {
        /*Parent process, quit.*/
        exit(0);
    }
    else
    {
        /*Child process, run it in a new session and close the stdin stdout and stderr.*/
        setsid();
        close(0);
        close(1);
        close(2);
        /*Attach file descriptors 0, 1 and 2 to "/dev/null". This is necessary, the 0, 1 and 2 may
          be assigned later for socket or file if they are not attached to "/dev/null". When the
          program is trying to print something into the stdout, the data is actually written to
          socket or file.*/
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }
#elif defined(WINDOWS)


#endif

    return u32Ret;
}

boolean_t jf_process_isAlreadyRunning(olchar_t * pstrDaemonName)
{
    boolean_t bRunning = FALSE;
#if defined(LINUX)
    olint_t fd = 0;
    olchar_t strPidFile[JF_LIMIT_MAX_PATH_LEN];
    olchar_t strBuf[128];

    /*Get the pid file name for the daemon.*/
    jf_process_getPidFilename(strPidFile, JF_LIMIT_MAX_PATH_LEN, pstrDaemonName);

    bRunning = _isProcessAlreadyRunning(strPidFile, pstrDaemonName);
    if (! bRunning)
    {
        /*Delete the PID file.*/
        ol_unlink(strPidFile);

        /*Open the PID file for write.*/
        fd = open(strPidFile, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
        if (fd > 0)
        {
            ol_sprintf(strBuf, "%d\n", jf_process_getCurrentId());
            ol_write(fd, strBuf, ol_strlen(strBuf));
            ol_close(fd);
        }
        else
        {
            ol_fprintf(stderr, "Failed to open file %s, %s\n", strPidFile, ol_strerror(errno));
        }
    }
#elif defined(WINDOWS)
    DWORD dwProcesses[1024], dwNeeded, dwNrOfProcess, dwPid;
    u32 u32Index;
    u8 u8ProcessName[MAX_PATH] = "<unknown>";
    HMODULE hMod;
    HANDLE hProcess;
    
    dwPid = GetCurrentProcessId();
    if (! EnumProcesses(dwProcesses, sizeof(dwProcesses), &dwNeeded))
        bRunning = FALSE;

    /*Calculate how many process identifiers were returned.*/
    dwNrOfProcess = dwNeeded / sizeof(DWORD);

    /*Print the name and process identifier for each process.*/
    for (u32Index = 0; u32Index < dwNrOfProcess; u32Index++)
        if (dwProcesses[u32Index] != 0)
        {
            /*Get a handle to the process.*/
            hProcess = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcesses[u32Index]);

            /*Get the process name.*/
            if (hProcess != NULL)
            {
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwNeeded))
                {
                    GetModuleBaseNameA(
                        hProcess, hMod, u8ProcessName, sizeof(u8ProcessName) / sizeof(u8));
                }
            }

            if ((ol_strcmp(u8ProcessName, pstrDaemonName) == 0) &&
                (dwProcesses[u32Index] != dwPid))
            {
                bRunning = TRUE;
            }

            CloseHandle(hProcess);
        }
#endif

    return bRunning;
}

void jf_process_initHandle(jf_process_handle_t * pHandle)
{
    ol_bzero(pHandle, sizeof(jf_process_handle_t));
}

boolean_t jf_process_isValidHandle(jf_process_handle_t * pHandle)
{
    boolean_t bRet = FALSE;

#if defined(LINUX)
    if (pHandle->jpi_pId > 0)
        bRet = TRUE;
#elif defined(WINDOWS)
    if (pHandle->jpi_hProcess != NULL)
        bRet = TRUE;
#endif

    return bRet;
}

u32 jf_process_create(
    jf_process_handle_t * pHandle, jf_process_attr_t * pAttr, olchar_t * pstrCommandLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    pid_t pid;

    pid = fork();
    if (pid == -1)
    {
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;
    }
    else if (pid > 0)
    {
        /*Parent process.*/
        pHandle->jpi_pId = pid;
    }
    else
    {
        /*Child process.*/
        u32Ret = _runCommandLine(pstrCommandLine);
    }
#elif defined(WINDOWS)
    boolean_t bRet;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ol_memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    ol_memset(&pi, 0, sizeof(pi));

    bRet = CreateProcess(NULL, pstrCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pHandle->jpi_hProcess = pi.hProcess;
    }
#endif

    return u32Ret;
}

u32 jf_process_kill(jf_process_handle_t * pHandle)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = kill(pHandle->jpi_pId, SIGKILL);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_TERMINATE_PROCESS;
    else
        jf_process_initHandle(pHandle);
#elif defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_process_terminate(jf_process_handle_t * pHandle)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = kill(pHandle->jpi_pId, SIGTERM);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_TERMINATE_PROCESS;
    else
        jf_process_initHandle(pHandle);
#elif defined(WINDOWS)
    boolean_t bRet;
    u32 u32ExitCode = 0;

    bRet = TerminateProcess(pHandle->jpi_hProcess, u32ExitCode);
    if (! bRet)
    {
        u32Ret = JF_ERR_FAIL_TERMINATE_PROCESS;
    }
    else
    {
        CloseHandle(pHandle->jpi_hProcess);

        jf_process_initHandle(pHandle);
    }
#endif

    return u32Ret;
}

u32 jf_process_waitForChildProcessTermination(
    jf_process_handle_t jphChild[], u32 u32Count, u32 u32BlockTime, u32 * pu32Index,
    u8 * pu8Reason)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Reason = JF_PROCESS_TERMINATION_REASON_UNKNOWN;
#if defined(LINUX)
    olint_t nStatus = 0;
    pid_t pid = 0;
    u32 u32Index = 0, u32Timeout = 0;

    assert((u32Count > 0) && (pu32Index != NULL));

    *pu32Index = U32_MAX;

    /*Convert the milli-second to second.*/
    if (u32BlockTime == 0)
        u32Timeout = 0;
    else if (u32BlockTime > 1000)
        u32Timeout = u32BlockTime / 1000;
    else
        u32Timeout = 1;

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Iterate through the process handle array.*/
        for (u32Index = 0; u32Index < u32Count; u32Index ++)
        {
            /*Test if the process is terminated.*/
            pid = waitpid(jphChild[u32Index].jpi_pId, &nStatus, WNOHANG);

            if (pid == -1)
            {
                /*Error.*/
                u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;
                break;
            }
            else if (jphChild[u32Index].jpi_pId == pid)
            {
                /*Success.*/
                *pu32Index = u32Index;
                _setProcessTerminationReason(nStatus, &u8Reason);
                break;
            }
        }

        /*Break the loop if timeout.*/
        if (u32Timeout == 0)
            break;

        /*Index is set, a child process is terminated.*/
        if (*pu32Index != U32_MAX)
            break;

        /*Sleep 1 second.*/
        sleep(1);

        /*Decrease timeout if not infinite.*/
        if (u32BlockTime != JF_TIME_INFINITE)
            u32Timeout --;
    }

    /*Return error if no child process is terminated.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (*pu32Index == U32_MAX)
            u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;
    }

#elif defined(WINDOWS)
    u32 u32Index = 0, u32Wait = 0;
    HANDLE hHandle[200];
    u32 u32HandleCount = 200;

    assert((u32Count > 0) && (pu32Index != NULL));

    *pu32Index = 0;
    u8Reason = JF_PROCESS_TERMINATION_REASON_UNKNOWN;

    if (u32HandleCount > u32Count)
        u32HandleCount = u32Count;

    for (u32Index = 0; u32Index < u32HandleCount; u32Index ++)
    {
        hHandle[u32Index] = jphChild[u32Index].jpi_hProcess;
    }

    /*Wait for termination of process.*/
    u32Wait = WaitForMultipleObjects(u32HandleCount, hHandle, FALSE, u32BlockTime);

    if ((u32Wait == WAIT_FAILED) || (u32Wait == WAIT_ABANDONED))
        u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;
    else if ((u32Wait >= WAIT_OBJECT_0) && (u32Wait < WAIT_OBJECT_0 + u32Count))
        *pu32Index = u32Wait - WAIT_OBJECT_0;

#endif

    if (pu8Reason != NULL)
        *pu8Reason = u8Reason;

    return u32Ret;
}

u32 jf_process_registerSignalHandlers(jf_process_fnSignalHandler_t fnSignalHandler)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nIndex;
#if defined(WINDOWS)
    olint_t nSignals[] = {SIGTERM, SIGINT};
    olint_t nSignalCount = 2;
#elif defined(LINUX)
    olint_t nSignals[] = {SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM,
        SIGTSTP, SIGINT};
    olint_t nRet, nSignalCount = 7;
    sigset_t ssSignalSet;
    struct sigaction act, oact;
#endif

    nIndex = 0;

#if defined(WINDOWS)
    while ((u32Ret == JF_ERR_NO_ERROR) && (nIndex < nSignalCount))
    {
        signal(nSignals[nIndex], fnSignalHandler);
        nIndex ++;
    }
#elif defined(LINUX)
    while ((u32Ret == JF_ERR_NO_ERROR) && (nIndex < nSignalCount))
    {
        nRet = sigemptyset(&ssSignalSet);
        if (nRet == -1)
        {
            u32Ret = JF_ERR_OPERATION_FAIL;
        }
        else
        {
            nRet = sigaddset(&ssSignalSet, nSignals[nIndex]);
            if (nRet == -1)
            {
                u32Ret = JF_ERR_OPERATION_FAIL;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*act.sa_sigaction = fnSignalHandler;*/
            if (nSignals[nIndex] == SIGPIPE)
                act.sa_handler = SIG_IGN;
            else
                act.sa_handler = fnSignalHandler;
            act.sa_mask = ssSignalSet;
            act.sa_flags = 0;

#ifdef SA_RESTART
            act.sa_flags |= SA_RESTART;
#endif

            nRet = sigaction(nSignals[nIndex], &act, &oact);
            if (nRet == -1)
            {
                u32Ret = JF_ERR_OPERATION_FAIL;
            }
        }
        nIndex ++;
    }
#endif
    return u32Ret;
}

u32 jf_process_ignoreSignal(olint_t sig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (signal(sig, SIG_IGN) == SIG_ERR)
        u32Ret = JF_ERR_OPERATION_FAIL;

    return u32Ret;
}

u32 jf_process_getCurrentWorkingDirectory(olchar_t * pstrDir, olsize_t sDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olchar_t * pstr;

    assert((pstrDir != NULL) && (sDir > 0));

    pstr = getcwd(pstrDir, sDir);
    if (pstr == NULL)
        u32Ret = JF_ERR_FAIL_GET_CWD;

#elif defined(WINDOWS)
    u32 u32RetLen;

    assert((pstrDir != NULL) && (sDir > 0));

    u32RetLen = GetCurrentDirectory(sDir, pstrDir);
    if (u32RetLen == 0)
        u32Ret = JF_ERR_FAIL_GET_CWD;
#endif
    return u32Ret;
}

u32 jf_process_setCurrentWorkingDirectory(const olchar_t * pstrDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    assert(pstrDir != NULL);

    nRet = chdir(pstrDir);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_SET_CWD;

#elif defined(WINDOWS)
    boolean_t bRet;

    assert(pstrDir != NULL);

    bRet = SetCurrentDirectory(pstrDir);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_SET_CWD;

#endif
    return u32Ret;
}

u32 jf_process_initSocket(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)

#elif defined(WINDOWS)
    WORD wVersionRequested;
    WSADATA wsaData;
    olint_t err;

    srand(time(NULL));

    wVersionRequested = MAKEWORD(2, 0);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        u32Ret = JF_ERR_FAIL_INIT_SOCKET;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Confirm that the WinSock DLL supports 2.0.*/
        /*Note that if the DLL supports versions greater than 2.0 in addition to 2.0, it will still
          return. 2.0 in wVersion since that is the version we requested.*/
        if (LOBYTE(wsaData.wVersion ) != 2 || HIBYTE(wsaData.wVersion ) != 0)
        {
            /*Tell the user that we could not find a usable WinSock DLL.*/
            WSACleanup();
            u32Ret = JF_ERR_FAIL_INIT_SOCKET;
        }
    }

#endif

    return u32Ret;
}

u32 jf_process_finiSocket(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)

#elif defined(WINDOWS)
    WSACleanup();
#endif

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
