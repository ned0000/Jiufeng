/**
 *  @file logger.h
 *
 *  @brief Logger header file. It provides the external definition of the
 *   logger and the related routines and data structures.
 *
 *  @author Min Zhang
 *
 *  @note The logger can be used to print logs to the stdout, syslog, a log file
 *   and a specified TTY.
 *  @note Log to stdout is NOT thread safe
 */

#ifndef JIUFENG_LOGGER_H
#define JIUFENG_LOGGER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

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

/* --- constant definitions ------------------------------------------------ */
#define LOGGER_TRACE_NONE   (0)
#define LOGGER_TRACE_ERROR  (1)
#define LOGGER_TRACE_INFO   (2)
#define LOGGER_TRACE_DEBUG  (3)
#define LOGGER_TRACE_DATA   (4)

#define MAX_MSG_SIZE 256

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    /** log to the stdout */
    boolean_t lp_bLogToStdout;
    /** log to the system log */
    boolean_t lp_bLogToSysLog;
    /** log to a file. If yes, lp_pstrLogFilePath must be specified */
    boolean_t lp_bLogToFile;
    /** log to the specified TTY. If yes, lp_pstrTTY must be specified.
        Not supported for now */
    boolean_t lp_bLogToTTY;
    /** trace level */
    u8 lp_u8TraceLevel;
    u8 lp_u8Reserved[3];
    /** the size of the log file in byte. If 0, no limit */
    olsize_t lp_sLogFile;
    /** the IP address of the remote machine. Not supported for now */
    u8 * lp_pu8RemoteMachineIP;
    /** the path to the log file */
    olchar_t * lp_pstrLogFilePath;
    /** the path to the TTY. Not supported for now */
    olchar_t * lp_pstrTTY;
    /** the name of the caller. The length should not exceed 15 characters */
    olchar_t * lp_pstrCallerName;
} logger_param_t;

/* --- functional routines ------------------------------------------------- */

/** Initialize the looger according to the specified parameters.
 *
 *  @param plp [in] the pointer to the logger parameter
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 */
LOGGERAPI u32 LOGGERCALL initLogger(logger_param_t * plp);

/** Finalize the logger
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL finiLogger(void);

/** Log an info type msg.
 *
 *  @param fmt [in] the msg format
 *  @param ... [in] the input to the msg format
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL logInfoMsg(const olchar_t * fmt, ...);

/** Log an debug type msg.
 *
 *  @param fmt [in] the msg format
 *  @param ... [in] the input to the msg format
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL logDebugMsg(const olchar_t * fmt, ...);

/** Log an error type msg.
 *
 *  @param u32ErrCode [in] the error code
 *  @param fmt [in] the msg format
 *  @param ... [in] the input to the msg format
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL logErrMsg(u32 u32ErrCode, const olchar_t * fmt, ...);

/** Log a data msg. The system error code is in errno.
 *
 *  @param pu8Data [in] the data to be logged
 *  @param u32DataLen [in] the length of the data in bytes
 *  @param fmt [in] the msg format
 *  @param ... [in] the input to the msg format
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL logDataMsg(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

/** Log a data msg with ascii string. The system error code is in errno.
 *
 *  @param pu8Data [in] the data to be logged
 *  @param u32DataLen [in] the length of the data in bytes
 *  @param fmt [in] the msg format
 *  @param ... [in] the input to the msg format
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
LOGGERAPI u32 LOGGERCALL logDataMsgWithAscii(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

#endif /*JIUFENG_LOGGER_H*/

/*---------------------------------------------------------------------------*/


