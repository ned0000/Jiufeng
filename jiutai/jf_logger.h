/**
 *  @file jf_logger.h
 *
 *  @brief Logger header file which provides the external definition of the logger and the related
 *   routines and data structures.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_logger library.
 *  -# The logger can be used to print logs to the stdout, syslog, a log file and a specified TTY.
 *  -# Log to stdout is NOT thread safe.
 */

#ifndef JIUFENG_LOGGER_H
#define JIUFENG_LOGGER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

#undef LOGGERAPI
#undef LOGGERCALL
#ifdef WINDOWS
    #if defined(JIUFENG_LOGGER_DLL)
        #define LOGGERAPI  __declspec(dllexport)
        #define LOGGERCALL
    #else
        #define LOGGERAPI
        #define LOGGERCALL __cdecl
    #endif
#else
    #define LOGGERAPI
    #define LOGGERCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_LOGGER_TRACE_NONE      (0)
#define JF_LOGGER_TRACE_ERROR     (1)
#define JF_LOGGER_TRACE_INFO      (2)
#define JF_LOGGER_TRACE_DEBUG     (3)
#define JF_LOGGER_TRACE_DATA      (4)

#define JF_LOGGER_MAX_MSG_SIZE    (256)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter data type for creating logger.
 */
typedef struct
{
    /**Log to the stdout.*/
    boolean_t jlip_bLogToStdout;
    /**Log to the system log.*/
    boolean_t jlip_bLogToSysLog;
    /**Log to a file. If yes, jlip_pstrLogFilePath must be specified.*/
    boolean_t jlip_bLogToFile;
    /**Log to the specified TTY. If yes, jlip_pstrTTY must be specified. NOT SUPPORTED for now.*/
    boolean_t jlip_bLogToTTY;
    /**Trace level.*/
    u8 jlip_u8TraceLevel;
    u8 jlip_u8Reserved[3];
    /**The size of the log file in byte. If 0, no limit.*/
    olsize_t jlip_sLogFile;
    /**The IP address of the remote machine. Not supported for now.*/
    u8 * jlip_pu8RemoteMachineIP;
    /**The path to the log file.*/
    olchar_t * jlip_pstrLogFilePath;
    /**The path to the TTY. Not supported for now.*/
    olchar_t * jlip_pstrTTY;
    /**The name of the caller. The length should not exceed 15 characters.*/
    olchar_t * jlip_pstrCallerName;
} jf_logger_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the looger according to the specified parameters.
 *
 *  @param pjlip [in] The pointer to the logger parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_init(jf_logger_init_param_t * pjlip);

/** Finalize the logger.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_fini(void);

/** Log an info type msg.
 *
 *  @param fmt [in] The msg format.
 *  @param ... [in] The input to the msg format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logInfoMsg(const olchar_t * fmt, ...);

/** Log an debug type msg.
 *
 *  @param fmt [in] The msg format.
 *  @param ... [in] The input to the msg format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDebugMsg(const olchar_t * fmt, ...);

/** Log an error type msg.
 *
 *  @param u32ErrCode [in] The error code.
 *  @param fmt [in] The msg format.
 *  @param ... [in] The input to the msg format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logErrMsg(u32 u32ErrCode, const olchar_t * fmt, ...);

/** Log a data msg. The system error code is in errno.
 *
 *  @param pu8Data [in] The data to be logged.
 *  @param u32DataLen [in] The length of the data in bytes.
 *  @param fmt [in] The msg format.
 *  @param ... [in] The input to the msg format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDataMsg(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

/** Log a data msg with ascii string. The system error code is in errno.
 *
 *  @param pu8Data [in] The data to be logged.
 *  @param u32DataLen [in] The length of the data in bytes.
 *  @param fmt [in] The msg format.
 *  @param ... [in] The input to the msg format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDataMsgWithAscii(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

#endif /*JIUFENG_LOGGER_H*/

/*------------------------------------------------------------------------------------------------*/


