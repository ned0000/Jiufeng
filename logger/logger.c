/**
 *  @file logger.c
 *
 *  @brief logger implementation
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#if defined(LINUX)
    #include <sys/errno.h>
    #include <syslog.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h" 
#include "errcode.h"
#include "logger.h"
#include "hexstr.h"
#include "ollimit.h"
#include "process.h"
#include "common.h"

/* --- private data/data structure section --------------------------------- */
#define IL_LOG_MASK_NONE     0x0
#define IL_LOG_MASK_STDOUT   0x01
#define IL_LOG_MASK_SYSLOG   0x02
#define IL_LOG_MASK_LOGFILE  0x04
#define IL_LOG_MASK_TTY      0x08

#ifndef LINUX
/* LOG_INFO and LOG_ERR is not used on Windows. Simply to clear the
   compiling errors */
#define LOG_INFO 0  
#define LOG_ERR 1
#endif

#define MAX_CALLER_NAME  16

typedef struct
{
    /*if the logger has been initialized*/
    boolean_t il_bInitialized;
    /* mask whether to log the messages to specific devices */
    u8 il_u8LogMask;
    /* 0: no trace; 1: info; 2: error */
    u8 il_u8TraceLevel;
    u8 il_u8Reserved[5];
    /* the name of the calling module. The name should
        not exceed MAX_CALLER_NAME characters */
    olchar_t il_strCallerName[MAX_CALLER_NAME];
    olchar_t il_strLogFilename[MAX_PATH_LEN];
    /* the maximum lines of the log file. Zero (0) means no limit. */
    u32 il_u32LogFileLines;
    /* the next line to print log to */
    u32 il_u32NextLine;
#ifdef LINUX
    /* the file descriptor to the TTY - not supported for now */
    olint_t il_nTTY;
    olint_t il_nReserved1;
#endif
} internal_logger_t;

static internal_logger_t ls_ilLogger;

/* --- private routine section ------------------------------------------------ */
/** Get max lines of the log file according to the file size.
 *
 *  @param u32Size [in] the file size
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _getLogFileLines(u32 u32Size)
{
    u32 u32lines;
    u32 u32MaxCharPerLine = 80;

    u32lines = u32Size / u32MaxCharPerLine;

    return u32lines;
}

/** Get the log time stamp of the current time. The time stamp is in the format
 *  of "mm/dd/yyyy hh:mm:ss".
 *
 *  @param pstrStamp [out] the string buffer where the time stamp will be
 *   returned
 *
 *  @return none
 */
static void _getLogTimeStamp(olchar_t * pstrStamp)
{
    time_t tCurrent;
    struct tm * tmLocal;

    time(&tCurrent);
    tmLocal = localtime(&tCurrent);
    if (tmLocal != NULL)
    {
        ol_sprintf(pstrStamp, "%02d/%02d/%04d %02d:%02d:%02d", 
            tmLocal->tm_mon+1, tmLocal->tm_mday, (tmLocal->tm_year + 1900),
            tmLocal->tm_hour, tmLocal->tm_min, tmLocal->tm_sec);
    }
    else
    {
        ol_sprintf(pstrStamp, "00/00/0000 00:00:00");
    }
}

/** Log the message log to the specified output of the logger
 *
 *  @param pil [in] the pointer to the logger
 *  @param nLevel [in] the log level to be passed to syslog
 *  @param pstrHeader [in] the header
 *  @param pstrLog [in] the log message
 *
 *  @return none
 */
static void _log(
    internal_logger_t * pil, olint_t nLevel, olchar_t * pstrHeader, olchar_t * pstrLog)
{
    if ((pil->il_u8LogMask & IL_LOG_MASK_STDOUT) != 0)
    {
        ol_printf("%s%s\n", pstrHeader, pstrLog);
        fflush(stdout);
    }

#ifdef LINUX
    if ((pil->il_u8LogMask & IL_LOG_MASK_SYSLOG) != 0)
    {
        syslog(nLevel, "%s", pstrHeader);
        syslog(nLevel, "%s", pstrLog);
    }
#endif

    if ((pil->il_u8LogMask & IL_LOG_MASK_LOGFILE) != 0)
    {
        /* to be done with file sizing */
        FILE * fd = fopen(pil->il_strLogFilename, "a");
        if (fd != NULL)
        {
            fprintf(fd, "%s%s\n", pstrHeader, pstrLog);
            fclose(fd);
        }
    }

    if ((pil->il_u8LogMask & IL_LOG_MASK_TTY) != 0)
    {
        /* to be done */
    }
}

/** Log the message to various log devices. It also add message header to the
 *  raw message. The message header format is:  \n
 *     "<time stamp> [<caller name>:<pid>:<threadid>] "
 *
 *  @param pil [in] the pointer to the logger
 *  @param nLevel [in] the log level to be passed to syslog
 *  @param pstrMsg [in] the log message
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _logMsg(internal_logger_t * pil, olint_t nLevel, olchar_t * pstrMsg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strHeader[256];
    olint_t nOffset;

    _getLogTimeStamp(strHeader);
    nOffset = (int)strlen(strHeader);

    ol_sprintf(strHeader + nOffset, " [%s:%d:%lu] ", pil->il_strCallerName, 
        getCurrentProcessId(), getCurrentThreadId());
    nOffset = (int)strlen(strHeader);

    _log(pil, nLevel, strHeader, pstrMsg);

    return u32Ret;
}

/** Set the default parameter of logger
 *
 *  @param pjlip [in] the pointer to logger parameter where the default
 *   values are returned.
 *
 *  @return: none.
 */
static void _setDefaultParam(jf_logger_init_param_t * pjlip)
{
    memset(pjlip, 0, sizeof(jf_logger_init_param_t));
    pjlip->jlip_bLogToFile = FALSE;
    pjlip->jlip_bLogToStdout = FALSE;
    pjlip->jlip_bLogToSysLog = FALSE;
    pjlip->jlip_bLogToTTY = FALSE;
    pjlip->jlip_pstrCallerName = NULL;
    pjlip->jlip_pstrLogFilePath = NULL;
    pjlip->jlip_pu8RemoteMachineIP = NULL;
    pjlip->jlip_pstrTTY = NULL;
    pjlip->jlip_sLogFile = 0;
    pjlip->jlip_u8TraceLevel = JF_LOGGER_TRACE_NONE;
}

static u32 _logSysErrMsg(internal_logger_t * pil, u32 u32ErrCode,
    const olchar_t * fmt, va_list ap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nPrint;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t bufMsg[128 + 1];
#if defined(WINDOWS)
    DWORD dwErrorCode = 0, dwRet = 0;
#elif defined(LINUX)
    olint_t errno_save = 0;
#endif

#if defined(WINDOWS)
    dwErrorCode = GetLastError();
#elif defined(LINUX)
    errno_save = errno;
#endif

    ol_strcpy(buf, "ERR ");
    nPrint = ol_vsnprintf(buf + 4, (JF_LOGGER_MAX_MSG_SIZE - 5), fmt, ap);
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;

    bufMsg[0] = '\0';
#if defined(WINDOWS)
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, 
        bufMsg, 128, NULL);
    bufMsg[128] = 0;
#elif defined(LINUX)
    ol_strncpy(bufMsg, strerror(errno_save), 128);
    bufMsg[128] = 0;
#endif

    if (nPrint == -1)
    {
        u32Ret = _logMsg(pil, LOG_ERR, buf);
        buf[0] = 0;
        nPrint = 0;
    }
    else
    {
        nPrint += 4;
    }

#if defined(LINUX)
    nPrint = ol_snprintf(buf + nPrint, JF_LOGGER_MAX_MSG_SIZE - nPrint - 1,
        " - (0x%X) %s\n      %d, %s", 
        u32ErrCode, getErrorDescription(u32ErrCode), errno_save, bufMsg);
#elif defined(WINDOWS)
    nPrint = ol_snprintf(buf + nPrint, JF_LOGGER_MAX_MSG_SIZE - nPrint - 1,
        " - (0x%X) %s\n      %d, %s", 
        u32ErrCode, getErrorDescription(u32ErrCode), dwErrorCode, bufMsg);
#endif
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
    u32Ret = _logMsg(pil, LOG_ERR, buf);

    return u32Ret;    
}

static u32 _logErrMsg(internal_logger_t * pil, u32 u32ErrCode,
    const olchar_t * fmt, va_list ap)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nPrint;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    ol_strcpy(buf, "ERR ");
    nPrint = ol_vsnprintf(buf + 4, (JF_LOGGER_MAX_MSG_SIZE - 5), fmt, ap);
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;

    if (nPrint == -1)
    {
        u32Ret = _logMsg(pil, LOG_ERR, buf);
        buf[0] = 0;
        nPrint = 0;
    }
    else
    {
        nPrint += 4;
    }

    ol_snprintf(buf + nPrint, JF_LOGGER_MAX_MSG_SIZE - nPrint - 1, " - (0x%X) %s",
        u32ErrCode, getErrorDescription(u32ErrCode));
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
    u32Ret = _logMsg(pil, LOG_ERR, buf);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_logger_init(jf_logger_init_param_t * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    FILE * fd = NULL;
    internal_logger_t * pil = &ls_ilLogger;
    jf_logger_init_param_t param, * pjlip;

    if (pil->il_bInitialized)
        return u32Ret;

    memset(pil, 0, sizeof(internal_logger_t));

    if (pParam != NULL)
    {
        pjlip = pParam;
    }
    else
    {
        /* create default parameters */
        pjlip = &param;
        _setDefaultParam(pjlip);
    }

    if (pjlip->jlip_pstrCallerName != NULL)
    {
        ol_strncpy(
            pil->il_strCallerName, pjlip->jlip_pstrCallerName, MAX_CALLER_NAME - 1);
    }
    else
    {
        ol_strcpy(pil->il_strCallerName, "NONAME");
    }

    if (pjlip->jlip_bLogToFile)
    {
        pil->il_u8LogMask |= IL_LOG_MASK_LOGFILE;

        if (pjlip->jlip_pstrLogFilePath != NULL)
            ol_strncpy(
                pil->il_strLogFilename, pjlip->jlip_pstrLogFilePath,
                MAX_PATH_LEN - 1);
        else
            ol_snprintf(
                pil->il_strLogFilename, MAX_PATH_LEN - 1,
                "%s.log", pil->il_strCallerName);

        fd = fopen(pil->il_strLogFilename, "w");
        if (fd == NULL)
        {
#if defined(WINDOWS)
            ol_printf("Init logger failed - (%d)\n", GetLastError());
#elif defined(LINUX)
            ol_printf(
                "Init logger failed - (%d) - %s\n", errno,
                strerror(errno));
#endif
            fflush(stdout);
            u32Ret = OLERR_OPERATION_FAIL;
        }
        else
        {
            fclose(fd);
        }
    }
    else
    {
        pil->il_u8LogMask &= ~IL_LOG_MASK_LOGFILE;
        pil->il_strLogFilename[0] = 0;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pjlip->jlip_bLogToStdout)
        {
            pil->il_u8LogMask |= IL_LOG_MASK_STDOUT;
        }
        if (pjlip->jlip_bLogToSysLog)
        {
            pil->il_u8LogMask |= IL_LOG_MASK_SYSLOG;
        }
        if (pjlip->jlip_bLogToTTY)
        {
            pil->il_u8LogMask |= IL_LOG_MASK_TTY;
        }

        /*pil->il_pfLogFile = fd;*/
#ifdef LINUX
        pil->il_nTTY = -1;
#endif
        pil->il_u8TraceLevel = pjlip->jlip_u8TraceLevel;
        pil->il_u32LogFileLines = _getLogFileLines(pjlip->jlip_sLogFile);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pil->il_bInitialized = TRUE;

        logInfoMsg("Logger started");
    }
    else
    {
        jf_logger_fini();
    }

    return u32Ret;
}

u32 jf_logger_fini(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;

    logInfoMsg("Logger stopped");
#ifdef LINUX
    if (pil->il_nTTY != -1)
    {
        close(pil->il_nTTY);
    }
#endif

    return u32Ret; 
}

u32 logInfoMsg(const olchar_t * fmt, ...)
{
    u32 u32Ret = OLERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != IL_LOG_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_INFO))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE - 1, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);
    
        _logMsg(pil, LOG_INFO, buf);
    }

    return u32Ret;    
}

u32 logDebugMsg(const olchar_t * fmt, ...)
{
    u32 u32Ret = OLERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != IL_LOG_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_DEBUG))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE - 1, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);
    
        _logMsg(pil, LOG_INFO, buf);
    }

    return u32Ret;    
}

u32 logErrMsg(u32 u32ErrCode, const olchar_t * fmt, ...)
{
    u32 u32Ret = OLERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != IL_LOG_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_ERROR))
    {
        va_start(ap, fmt);

        if (isSysErrorCode(u32ErrCode))
            u32Ret = _logSysErrMsg(pil, u32ErrCode, fmt, ap);
        else
            u32Ret = _logErrMsg(pil, u32ErrCode, fmt, ap);

        va_end(ap);
    }

    return u32Ret;    
}

u32 logDataMsg(u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;
    va_list ap; 
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t strLength[64];
    u32 u32Index, u32Logged;

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != IL_LOG_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_DATA))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE - 1, fmt, ap);
        va_end(ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';
        
        ol_snprintf(strLength, 63, " (DATA length %d)", u32DataLen);
        ol_strcat(buf, strLength);
        u32Ret = _logMsg(pil, LOG_INFO, buf);
            
        u32Index = 0;
        u32Logged = 1;
        while((u32Index < u32DataLen) && (u32Logged != 0))
        {
            u32Logged = getByteHexString(pu8Data, u32DataLen, u32Index,
                 buf, JF_LOGGER_MAX_MSG_SIZE - 1);
            if (u32Logged != 0)
            {
                _log(pil, LOG_INFO, "", buf);
                u32Index += u32Logged;
            }
        }
    }

    return u32Ret;    
}

u32 logDataMsgWithAscii(u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;
    va_list ap; 
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t strLength[64];
    u32 u32Index, u32Logged;

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != IL_LOG_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_DATA))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE - 1, fmt, ap);
        va_end(ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';

        ol_snprintf(strLength, 63, " (DATA length %d)", u32DataLen);
        ol_strcat(buf, strLength);
        u32Ret = _logMsg(pil, LOG_INFO, buf);
            
        u32Index = 0;
        u32Logged = 1;
        while((u32Index < u32DataLen) && (u32Logged != 0))
        {
            u32Logged = getByteHexStringWithAscii(pu8Data, u32DataLen, u32Index,
                 buf, JF_LOGGER_MAX_MSG_SIZE - 1);
            if (u32Logged != 0)
            {
                _log(pil, LOG_INFO, "", buf);
                u32Index += u32Logged;
            }
        }
    }

    return u32Ret;    
}

/*---------------------------------------------------------------------------*/


