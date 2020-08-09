/**
 *  @file logger.c
 *
 *  @brief Implementation file for logger library.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)
    #include <sys/errno.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h" 
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_hex.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_data.h"
#include "jf_time.h"

#include "common.h"
#include "log2stdout.h"
#include "log2systemlog.h"
#include "log2tty.h"
#include "log2file.h"
#include "log2server.h"
#include "log2servermsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Log location mask.
 */
#define JF_LOGGER_MASK_NONE                  (0x0)

/** Log to stdout.
 */
#define JF_LOGGER_MASK_STDOUT                (0x01)

/** Log to systemlog.
 */
#define JF_LOGGER_MASK_SYSTEMLOG             (0x02)

/** Log to file
 */
#define JF_LOGGER_MASK_LOGFILE               (0x04)

/** Log to tty.
 */
#define JF_LOGGER_MASK_TTY                   (0x08)

/** Log to server.
 */
#define JF_LOGGER_MASK_SERVER                (0x10)

/** The prefix before log message with error level.
 */
#define JF_LOGGER_ERROR_MSG_PREFIX           "ERR "

/** The size of the prefix before log message with error level.
 */
#define JF_LOGGER_ERROR_MSG_PREFIX_SIZE      ol_strlen(JF_LOGGER_ERROR_MSG_PREFIX)


/** Define the internal logger data type.
 */
typedef struct
{
    /**Logger library has been initialized if it's TRUE.*/
    boolean_t il_bInitialized;
    /**Log mask.*/
    u8 il_u8LogMask;
    /**Trade level, defined in jf_logger_trace_level_t.*/
    u8 il_u8TraceLevel;
    u8 il_u8Reserved[5];
    /**The name of the calling module.*/
    olchar_t il_strCallerName[JF_LOGGER_MAX_CALLER_NAME_LEN];

    /**File log location.*/
    jf_logger_log_location_t * il_pjlllFile;
    /**Stdout log location.*/
    jf_logger_log_location_t * il_pjlllStdout;
    /**System log log location.*/
    jf_logger_log_location_t * il_pjlllSystemlog;
    /**Tty log location.*/
    jf_logger_log_location_t * il_pjlllTty;
    /**Server log location.*/
    jf_logger_log_location_t * il_pjlllServer;


} internal_logger_t;

static internal_logger_t ls_ilLogger;

/* --- private routine section ------------------------------------------------------------------ */

/** Ouput the log message to the specified location.
 *
 *  @param pil [in] The pointer to the logger.
 *  @param u8LogLevel [in] The log level.
 *  @param bBanner [in] Include banner if it's TRUE.
 *  @param pstrLog [in] The log message.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _logToLocation(
    internal_logger_t * pil, u8 u8LogLevel, boolean_t bBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pil->il_u8LogMask & JF_LOGGER_MASK_STDOUT)
        logToStdout(pil->il_pjlllStdout, bBanner, pstrLog);

    if (pil->il_u8LogMask & JF_LOGGER_MASK_SYSTEMLOG)
        logToSystemlog(pil->il_pjlllSystemlog, u8LogLevel, bBanner, pstrLog);

#if defined(LINUX)
    if (pil->il_u8LogMask & JF_LOGGER_MASK_TTY)
        logToTty(pil->il_pjlllTty, bBanner, pstrLog);
#endif

    if (pil->il_u8LogMask & JF_LOGGER_MASK_LOGFILE)
        logToFile(pil->il_pjlllFile, bBanner, pstrLog);

    if (pil->il_u8LogMask & JF_LOGGER_MASK_SERVER)
        logToServer(pil->il_pjlllServer, pstrLog, ol_strlen(pstrLog));

    return u32Ret;
}

static u32 _logSysErrMsg(
    internal_logger_t * pil, u32 u32ErrCode, const olchar_t * pstrMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t bufMsg[128];
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

    bufMsg[0] = '\0';
#if defined(WINDOWS)
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, bufMsg, sizeof(bufMsg), NULL);
#elif defined(LINUX)
    ol_strncpy(bufMsg, strerror(errno_save), sizeof(bufMsg));
#endif
    bufMsg[sizeof(bufMsg) - 1] = '\0';

#if defined(LINUX)
    ol_snprintf(
        buf, JF_LOGGER_MAX_MSG_SIZE, "%s - (0x%X) %s\n    %d, %s", pstrMsg, u32ErrCode,
        jf_err_getDescription(u32ErrCode), errno_save, bufMsg);
#elif defined(WINDOWS)
    ol_snprintf(
        buf, JF_LOGGER_MAX_MSG_SIZE, "%s - (0x%X) %s\n    %d, %s", pstrMsg, u32ErrCode,
        jf_err_getDescription(u32ErrCode), dwErrorCode, bufMsg);
#endif
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';

    u32Ret = _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_ERROR, TRUE, buf);

    return u32Ret;    
}

static u32 _logErrMsg(internal_logger_t * pil, u32 u32ErrCode, const olchar_t * pstrMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    ol_snprintf(
        buf, JF_LOGGER_MAX_MSG_SIZE, "%s - (0x%X) %s", pstrMsg, u32ErrCode,
        jf_err_getDescription(u32ErrCode));
    buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';

    u32Ret = _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_ERROR, TRUE, buf);

    return u32Ret;
}

static u32 _createOtherLogLocation(internal_logger_t * pil, jf_logger_init_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pParam->jlip_bLogToFile)
    {
        create_file_log_location_param_t cfllp;

        pil->il_u8LogMask |= JF_LOGGER_MASK_LOGFILE;

        ol_bzero(&cfllp, sizeof(cfllp));
        cfllp.cfllp_pstrCallerName = pParam->jlip_pstrCallerName;
        cfllp.cfllp_pstrLogFile = pParam->jlip_pstrLogFile;
        cfllp.cfllp_sLogFile = pParam->jlip_sLogFile;

        u32Ret = createFileLogLocation(&cfllp, &pil->il_pjlllFile);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (pParam->jlip_bLogToStdout))
    {
        create_stdout_log_location_param_t csllp;

        pil->il_u8LogMask |= JF_LOGGER_MASK_STDOUT;

        ol_bzero(&csllp, sizeof(csllp));
        csllp.csllp_pstrCallerName = pParam->jlip_pstrCallerName;

        u32Ret = createStdoutLogLocation(&csllp, &pil->il_pjlllStdout);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (pParam->jlip_bLogToSystemLog))
    {
        create_systemlog_log_location_param_t csllp;

        pil->il_u8LogMask |= JF_LOGGER_MASK_SYSTEMLOG;

        ol_bzero(&csllp, sizeof(csllp));

        u32Ret = createSystemlogLogLocation(&csllp, &pil->il_pjlllSystemlog);
    }

#if defined(LINUX)
    if ((u32Ret == JF_ERR_NO_ERROR) && (pParam->jlip_bLogToTty))
    {
        create_tty_log_location_param_t ctllp;

        pil->il_u8LogMask |= JF_LOGGER_MASK_TTY;

        ol_bzero(&ctllp, sizeof(ctllp));
        ctllp.ctllp_pstrCallerName = pParam->jlip_pstrCallerName;
        ctllp.ctllp_pstrTtyFile = pParam->jlip_pstrTtyFile;

        u32Ret = createTtyLogLocation(&ctllp, &pil->il_pjlllTty);
    }
#endif

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_logger_init(jf_logger_init_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;

    if (pil->il_bInitialized)
        return u32Ret;

    assert(pParam != NULL);

    ol_bzero(pil, sizeof(internal_logger_t));
    pil->il_u8TraceLevel = pParam->jlip_u8TraceLevel;

    if (pParam->jlip_pstrCallerName != NULL)
        ol_strncpy(
            pil->il_strCallerName, pParam->jlip_pstrCallerName, sizeof(pil->il_strCallerName) - 1);
    else
        ol_strcpy(pil->il_strCallerName, JF_LOGGER_DEF_CALLER_NAME);

    if (pParam->jlip_bLogToServer)
    {
        create_server_log_location_param_t csllp;

        pil->il_u8LogMask |= JF_LOGGER_MASK_SERVER;

        ol_bzero(&csllp, sizeof(csllp));
        csllp.csllp_pstrCallerName = pParam->jlip_pstrCallerName;
        csllp.csllp_pstrServerAddress = pParam->jlip_pstrServerAddress;
        csllp.csllp_u16ServerPort = pParam->jlip_u16ServerPort;

        u32Ret = createServerLogLocation(&csllp, &pil->il_pjlllServer);
    }
    else
    {
        u32Ret = _createOtherLogLocation(pil, pParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pil->il_bInitialized = TRUE;

        jf_logger_logInfoMsg("Logger started");
    }
    else
    {
        jf_logger_fini();
    }

    return u32Ret;
}

u32 jf_logger_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;

    jf_logger_logInfoMsg("Logger stopped");

    if (pil->il_pjlllFile != NULL)
        destroyFileLogLocation(&pil->il_pjlllFile);

    if (pil->il_pjlllStdout != NULL)
        destroyStdoutLogLocation(&pil->il_pjlllStdout);

#if defined(LINUX)
    if (pil->il_pjlllTty != NULL)
        destroyTtyLogLocation(&pil->il_pjlllTty);
#endif

    if (pil->il_pjlllServer != NULL)
        destroyServerLogLocation(&pil->il_pjlllServer);

    if (pil->il_pjlllSystemlog != NULL)
        destroySystemlogLogLocation(&pil->il_pjlllSystemlog);

    pil->il_bInitialized = FALSE;

    return u32Ret; 
}

u32 jf_logger_logInfoMsg(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_INFO))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);
    
        _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_INFO, TRUE, buf);
    }

    return u32Ret;    
}

u32 jf_logger_logDebugMsg(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_DEBUG))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);
    
        _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_DEBUG, TRUE, buf);
    }

    return u32Ret;    
}

u32 jf_logger_logWarnMsg(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_WARN))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);
    
        _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_WARN, TRUE, buf);
    }

    return u32Ret;    
}

u32 jf_logger_logErrMsg(u32 u32ErrCode, const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    va_list ap; 
    internal_logger_t * pil = &ls_ilLogger;
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_ERROR))
    {
        /*Add prefix before the log message.*/
        ol_strcpy(buf, JF_LOGGER_ERROR_MSG_PREFIX);

        va_start(ap, fmt);
        ol_vsnprintf(
            buf + JF_LOGGER_ERROR_MSG_PREFIX_SIZE,
            JF_LOGGER_MAX_MSG_SIZE - JF_LOGGER_ERROR_MSG_PREFIX_SIZE, fmt, ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = 0;
        va_end(ap);

        if (isSysErrorCode(u32ErrCode))
            u32Ret = _logSysErrMsg(pil, u32ErrCode, buf);
        else
            u32Ret = _logErrMsg(pil, u32ErrCode, buf);
    }

    return u32Ret;    
}

u32 jf_logger_logDataMsg(u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;
    va_list ap; 
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t strLength[64];
    olsize_t sLength = 0;
    u32 u32Index, u32Logged;

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_DATA))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE, fmt, ap);
        va_end(ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';
        
        sLength = ol_snprintf(strLength, sizeof(strLength), " (DATA length %d)", u32DataLen);
        /*Should include the null-terminated character.*/
        jf_data_copyToBuffer(
            buf, JF_LOGGER_MAX_MSG_SIZE - 1, ol_strlen(buf), strLength, sLength + 1);

        _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_DATA, TRUE, buf);
            
        u32Index = 0;
        u32Logged = 1;
        while((u32Index < u32DataLen) && (u32Logged != 0))
        {
            u32Logged = jf_hex_convertByteDataToString(
                pu8Data, u32DataLen, u32Index, buf, JF_LOGGER_MAX_MSG_SIZE - 1);
            if (u32Logged != 0)
            {
                _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_DATA, FALSE, buf);
                u32Index += u32Logged;
            }
        }
    }

    return u32Ret;    
}

u32 jf_logger_logDataMsgWithAscii(u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_logger_t * pil = &ls_ilLogger;
    va_list ap; 
    olchar_t buf[JF_LOGGER_MAX_MSG_SIZE];
    olchar_t strLength[64];
    olsize_t sLength = 0;
    u32 u32Index, u32Logged;

    if (! pil->il_bInitialized)
        return u32Ret;

    if ((pil->il_u8LogMask != JF_LOGGER_MASK_NONE) &&
        (pil->il_u8TraceLevel >= JF_LOGGER_TRACE_LEVEL_DATA))
    {
        va_start(ap, fmt);
        ol_vsnprintf(buf, JF_LOGGER_MAX_MSG_SIZE, fmt, ap);
        va_end(ap);
        buf[JF_LOGGER_MAX_MSG_SIZE - 1] = '\0';

        sLength = ol_snprintf(strLength, sizeof(strLength), " (DATA length %d)", u32DataLen);
        /*Should include the null-terminated character.*/
        jf_data_copyToBuffer(
            buf, JF_LOGGER_MAX_MSG_SIZE - 1, ol_strlen(buf), strLength, sLength + 1);

        _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_DATA, TRUE, buf);
            
        u32Index = 0;
        u32Logged = 1;
        while((u32Index < u32DataLen) && (u32Logged != 0))
        {
            u32Logged = jf_hex_convertByteDataToStringWithAscii(
                pu8Data, u32DataLen, u32Index, buf, JF_LOGGER_MAX_MSG_SIZE - 1);
            if (u32Logged != 0)
            {
                _logToLocation(pil, JF_LOGGER_TRACE_LEVEL_DATA, FALSE, buf);
                u32Index += u32Logged;
            }
        }
    }

    return u32Ret;    
}

/*------------------------------------------------------------------------------------------------*/
