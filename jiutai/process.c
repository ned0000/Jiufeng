/**
 *  @file process.c
 *
 *  @brief process implementation file
 *   provide some functional routine for managing process
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
#include "olbasic.h"
#include "ollimit.h"
#include "process.h"
#include "errcode.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
#if defined(LINUX)
static u32 _setThreadAttr(thread_attr_t * pta, pthread_attr_t * ppa)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pthread_attr_init(ppa);
    if (pta->ta_bDetached)
        pthread_attr_setdetachstate(ppa, PTHREAD_CREATE_DETACHED);
    else
        pthread_attr_setdetachstate(ppa, PTHREAD_CREATE_JOINABLE);

    return u32Ret;
}

static u32 _runCommandLine(olchar_t * pstrCommandLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t argc = 80;
    olchar_t * argv[80];
    olint_t ret;

    u32Ret = formCmdLineArguments(pstrCommandLine, &argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ret = execv(argv[0], (olchar_t **)argv);

        exit(ret);
    }

    return u32Ret;
}

static void setProcessTerminationReason(olint_t nStatus, u32 * pu32Reason)
{
    olint_t nSignal;

    if (WIFEXITED(nStatus))
        *pu32Reason = PROCESS_TERMINATION_REASON_EXITED;
    else if (WIFSIGNALED(nStatus))
    {
        nSignal = WTERMSIG(nStatus);
        if (nSignal == SIGKILL)
            *pu32Reason = PROCESS_TERMINATION_REASON_KILLED;
        else if (nSignal == SIGTERM)
            *pu32Reason = PROCESS_TERMINATION_REASON_TERMINATED;
        else if (nSignal == SIGSEGV)
            *pu32Reason = PROCESS_TERMINATION_REASON_ACCESS_VIOLATION;
        else
            *pu32Reason = PROCESS_TERMINATION_REASON_SIGNALED;
    }
    else
        *pu32Reason = PROCESS_TERMINATION_REASON_UNKNOWN;
}
#endif

/* --- public routine section ---------------------------------------------- */

#if defined(LINUX)
void getPidFilename(olchar_t * pstrPidFilename, olsize_t sFile, olchar_t * pstrDaemonName)
{
    ol_snprintf(pstrPidFilename, sFile - 1, "/var/run/%s.pid", pstrDaemonName);
    pstrPidFilename[sFile - 1] = '\0';
}
#endif

u32 formCmdLineArguments(olchar_t * pstrCmd, olsize_t * psArgc, olchar_t * argv[])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t cDelimiter = ' ';
    olchar_t cQuote = '"';
    u32 u32Length = 0, u32Index = 0, u32Position = 0;
    olsize_t argc = 0;

    memset(argv, 0, sizeof(olchar_t **) * (*psArgc));

    u32Length = (u32)ol_strlen(pstrCmd);

    while ((u32Index < u32Length) && (argc < *psArgc))
    {
        /* the start quote */
        if (pstrCmd[u32Index] == cQuote)
        {
            while ((pstrCmd[u32Index + 1] != cQuote) &&
                   ((u32Index + 1) < u32Length))
            {
                /* search for the end quote */
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
                    /* end of comamnd */
                    argv[argc ++] = pstrCmd + u32Position;
                    u32Position = u32Index + 1;
                }
            }
        }
        else if (pstrCmd[u32Index] == cDelimiter)
        {
            /* space */
            pstrCmd[u32Index] = '\0';
            argv[argc ++] = pstrCmd + u32Position;
            u32Position = u32Index + 1;

            while (pstrCmd[u32Index + 1] == cDelimiter)
            {
                /* extra spaces */
                u32Position ++;
                u32Index ++;
            }
        }
        else if (pstrCmd[u32Index + 1] == '\0')
        {
            /* end of comamnd */
            argv[argc ++] = pstrCmd + u32Position;
            u32Position = u32Index + 1;
        }

        u32Index ++;
    } /* while loop */

    *psArgc = argc;

    return u32Ret;
}

u32 switchToDaemon(olchar_t * pstrDaemonName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    pid_t pid;
    olchar_t strPidFile[MAX_PATH_LEN];
    FILE * fp;

    assert(pstrDaemonName != NULL);

#if defined(LINUX)
    pid = fork();
    if (pid == -1)
    {
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;
    }
    else if (pid != 0)
    {
        /*parent process*/
        getPidFilename(strPidFile, MAX_PATH_LEN, pstrDaemonName); 
        fp = fopen(strPidFile, "w");
        if (fp == NULL)
        {
            u32Ret = JF_ERR_FILE_NOT_FOUND;
        }
        else
        {
            u32Ret = JF_ERR_PROCESS_CREATED;
            fprintf(fp, "%d\n", pid);
            fclose(fp);
        }
    }
    else
    {
        /*child process*/
        setsid();
        close(0);
        close(1);
        close(2);
    }
#elif defined(WINDOWS)


#endif

    return u32Ret;
}

boolean_t bAlreadyRunning(olchar_t * pstrDaemonName)
{
    boolean_t bRunning = FALSE;
#if defined(LINUX)
    olint_t nRet = 0;
    olint_t fd = 0;
    pid_t pid = 0;
    olchar_t strPidFile[MAX_PATH_LEN];
    olchar_t strBuf[128];
    olchar_t strBuf2[128];
    olint_t fdCmdLine = 0;

    getPidFilename(strPidFile, MAX_PATH_LEN, pstrDaemonName);
    fd = open(strPidFile, O_RDONLY);
    if (fd > 0)
    {
        nRet = read(fd, strBuf, 31);
        if (nRet > 0)
        {
            strBuf[nRet] = 0;
            sscanf(strBuf, "%d\n", &pid);

            if (pid != getpid())
            {
                ol_sprintf(strBuf2, "/proc/%d/status", pid);
                fdCmdLine = open(strBuf2, O_RDONLY);

                if (fdCmdLine > 0)
                {
                    memset(strBuf2, 0, 128);
                    nRet = read(fdCmdLine, strBuf2, 127);
                    close(fdCmdLine);
                    if (nRet > 0)
                    {
                        strBuf2[nRet] = 0;

                        nRet = sscanf(strBuf2, "Name:\t%s", strBuf);
                        if (nRet == 1)
                        {
                            if (strcmp(strBuf, pstrDaemonName) == 0)
                                bRunning = TRUE;
                        }
                    }
                }
            }
        }
        close(fd);

        if (! bRunning)
        {
            unlink(strPidFile);
        }
    }

    if (! bRunning)
    {
        fd = open(strPidFile, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
        if (fd > 0)
        {
            ol_sprintf(strBuf, "%d\n", getpid());
            write(fd, strBuf, ol_strlen(strBuf));
            close(fd);
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

    /* Calculate how many process identifiers were returned */
    dwNrOfProcess = dwNeeded / sizeof(DWORD);

    /* Print the name and process identifier for each process */
    for (u32Index = 0; u32Index < dwNrOfProcess; u32Index++)
        if (dwProcesses[u32Index] != 0)
        {
            /* Get a handle to the process */
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
               PROCESS_VM_READ, FALSE, dwProcesses[u32Index]);
            /* Get the process name */
            if (hProcess != NULL)
            {
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), 
                    &dwNeeded))
                {
                    GetModuleBaseName(hProcess, hMod, u8ProcessName, 
                        sizeof(u8ProcessName)/sizeof(u8));
                }
            }

            if ((strcmp(u8ProcessName, pstrDaemonName) == 0) &&
                (dwProcesses[u32Index] != dwPid))
            {
                bRunning = TRUE;
            }

            CloseHandle(hProcess);
        }
#endif

    return bRunning;
}

void initProcessId(process_id_t * pProcessId)
{
    memset(pProcessId, 0, sizeof(process_id_t));
}

boolean_t isValidProcessId(process_id_t * pProcessId)
{
    boolean_t bRet = FALSE;

#if defined(LINUX)
    if (pProcessId->pi_pId > 0)
        bRet = TRUE;
#elif defined(WINDOWS)
    if (pProcessId->pi_hProcess != NULL)
        bRet = TRUE;
#endif

    return bRet;
}

u32 createProcess(process_id_t * pProcessId, process_attr_t * pAttr,
    olchar_t * pstrCommandLine)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    pid_t pid;

    pid = fork();
    if (pid == -1)
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;
    else if (pid > 0)
    {
        /* parent process */
        pProcessId->pi_pId = pid;
    }
    else
    {
        /* child process */
        u32Ret = _runCommandLine(pstrCommandLine);
    }
#elif defined(WINDOWS)
    boolean_t bRet;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    bRet = CreateProcess(NULL, pstrCommandLine, NULL,
        NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_CREATE_PROCESS;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pProcessId->pi_hProcess = pi.hProcess;
    }
#endif

    return u32Ret;
}

u32 terminateProcess(process_id_t * pProcessId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = kill(pProcessId->pi_pId, SIGTERM);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_TERMINATE_PROCESS;
    else
        initProcessId(pProcessId);
#elif defined(WINDOWS)
    boolean_t bRet;
    u32 u32ExitCode = 0;

    bRet = TerminateProcess(pProcessId->pi_hProcess, u32ExitCode);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_TERMINATE_PROCESS;
    else
    {
        CloseHandle(pProcessId->pi_hProcess);

        initProcessId(pProcessId);
    }
#endif

    return u32Ret;
}

/*it returns if a child process terminates*/
/*u32BlockTime is in millisecond*/
u32 waitForChildProcessTermination(process_id_t pidChild[], u32 u32Count,
    u32 u32BlockTime, u32 * pu32Index, u32 * pu32Reason)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nStatus;
    pid_t pid = 0;
    u32 u32Index, u32Reason, u32Timeout;

    assert((u32Count > 0) && (pu32Index != NULL));

    *pu32Index = 0;
    u32Reason = PROCESS_TERMINATION_REASON_UNKNOWN;

    if (u32BlockTime == INFINITE)
    {
        pid = wait(&nStatus);
    }
    else
    {
        if (u32BlockTime > 1000)
            u32Timeout = u32BlockTime / 1000;
        else
            u32Timeout = 1;

        while (u32Timeout > 0)
        {
            pid = waitpid(-1, &nStatus, WNOHANG);
            if (pid > 0)
                break;

            sleep(1);

            u32Timeout --;
        }
    }

    if (pid > 0)
    {
        for (u32Index = 0; u32Index < u32Count; u32Index ++)
        {
            if (pidChild[u32Index].pi_pId == pid)
            {
                *pu32Index = u32Index;
                break;
            }
        }

        if (u32Index == u32Count)
            u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;
        else
            setProcessTerminationReason(nStatus, &u32Reason);
    }
    else
        u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;

    if (pu32Reason != NULL)
        *pu32Reason = u32Reason;


#elif defined(WINDOWS)
    u32 u32Index, u32Wait, u32Reason;
    HANDLE hHandle[200];
    u32 u32HandleCount = 200;

    assert((u32Count > 0) && (pu32Index != NULL));

    *pu32Index = 0;
    u32Reason = PROCESS_TERMINATION_REASON_UNKNOWN;

    if (u32HandleCount > u32Count)
        u32HandleCount = u32Count;

    for (u32Index = 0; u32Index < u32HandleCount; u32Index ++)
    {
        hHandle[u32Index] = pidChild[u32Index].pi_hProcess;
    }

    u32Wait = WaitForMultipleObjects(u32HandleCount, hHandle,
        FALSE, u32BlockTime);
    if ((u32Wait == WAIT_FAILED) || (u32Wait == WAIT_ABANDONED))
        u32Ret = JF_ERR_FAIL_WAIT_PROCESS_TERMINATION;
    else if ((u32Wait >= WAIT_OBJECT_0) && (u32Wait < WAIT_OBJECT_0 + u32Count))
        *pu32Index = u32Wait - WAIT_OBJECT_0;

#endif
    return u32Ret;
}

void initThreadId(thread_id_t * pThreadId)
{
    memset(pThreadId, 0, sizeof(thread_id_t));
}

boolean_t isValidThreadId(thread_id_t * pThreadId)
{
    boolean_t bRet = TRUE;

#if defined(LINUX)
    if (pThreadId->ti_ptThreadId == 0)
        bRet = FALSE;
#elif defined(WINDOWS)
    if (pThreadId->ti_hThread == NULL)
        bRet = FALSE;
#endif

    return bRet;
}

u32 createThread(thread_id_t * pThreadId, thread_attr_t * pAttr,
    fnThreadRoutine_t fnThreadRoutine, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    thread_id_t ti;
#if defined(LINUX)
    pthread_attr_t attr;
    olint_t ret;

    pthread_attr_init(&attr);
    if (pAttr != NULL)
        u32Ret = _setThreadAttr(pAttr, &attr);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ret = pthread_create(&(ti.ti_ptThreadId), &attr, fnThreadRoutine, pArg);
        if (ret != 0)
            u32Ret = JF_ERR_FAIL_CREATE_THREAD;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pThreadId != NULL)
            memcpy(pThreadId, &ti, sizeof(thread_id_t));
    }
#elif defined(WINDOWS)
    ti.ti_hThread = CreateThread(NULL, 0, fnThreadRoutine, pArg, 0, NULL);
    if (ti.ti_hThread == NULL) 
        u32Ret = JF_ERR_FAIL_CREATE_THREAD;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pThreadId != NULL)
            memcpy(pThreadId, &ti, sizeof(thread_id_t));
    }
#endif

    return u32Ret;
}

u32 terminateThread(thread_id_t * pThreadId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = pthread_cancel(pThreadId->ti_ptThreadId);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_TERMINATE_THREAD;
    else
        initThreadId(pThreadId);
#elif defined(WINDOWS)
    boolean_t bRet;
    u32 u32ExitCode = 0;

    bRet = TerminateThread(pThreadId->ti_hThread, u32ExitCode);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_TERMINATE_THREAD;
    else
        initThreadId(pThreadId);
#endif

    return u32Ret;
}

u32 waitForThreadTermination(thread_id_t threadId, u32 * pu32RetCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t ret;

    ret = pthread_join(threadId.ti_ptThreadId, NULL);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_WAIT_THREAD_TERMINATION;
#elif defined(WINDOWS)
    u32 u32Wait;

    u32Wait = WaitForSingleObject(threadId.ti_hThread, INFINITE);
    if (u32Wait != WAIT_OBJECT_0)
        u32Ret = JF_ERR_FAIL_WAIT_THREAD_TERMINATION;
#endif

    return u32Ret;
}

u32 registerSignalHandlers(fnSignalHandler_t fnSignalHandler)
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
            act.sa_flags = SA_SIGINFO;

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

u32 getCurrentWorkingDirectory(olchar_t * pstrDir, olsize_t sDir)
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

u32 setCurrentWorkingDirectory(const olchar_t * pstrDir)
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

/*---------------------------------------------------------------------------*/


