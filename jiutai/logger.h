/**
 *  @file logger.h
 *
 *  @brief logger header file
 *    It provides the external definition of the logger and the related
 *    routines and data structures.
 *    The logger can be used to print logs to the stdout, syslog, a log file
 *    and a specified TTY.
 *
 *  @author Min Zhang
 *
 *  @note log to stdout is NOT thread safe
 *
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
    /* whether log to the stdout */
    boolean_t lp_bLogToStdout;
    /* whether log to the system log */
    boolean_t lp_bLogToSysLog;
    /* whether log to a file. If yes, lp_pu8RemoteMachineIP and
       lp_pstrLogFilePath must be specified */
    boolean_t lp_bLogToFile;
    /* whether log to the specified TTY. If yes, 
       lp_pu8TTY must be specified - not supported for now */
    boolean_t lp_bLogToTTY;
    /* 0: no trace; 1: error; 2: info; 3: data*/
    u8 lp_u8TraceLevel;
    u8 lp_u8Reserved[3];
    /*the size of the log file in byte. If 0, no limit*/
    olsize_t lp_sLogFile;
    /* the IP address of the remote machine the IP address of the remote
       machine resides on. If null, the file is local - not supported
       for now */
    u8 * lp_pu8RemoteMachineIP;
    /* the path to the log file */
    olchar_t * lp_pstrLogFilePath;
    /* the path to the TTY - not supported for now */
    olchar_t * lp_pstrTTY;
    /*the name of the calling module. The name not exceed 15 characters */
    olchar_t * lp_pstrCallerName;
} logger_param_t;

/* --- functional routines ------------------------------------------------- */

/** initialize the looger according to the specified parameters.
 *
 *  @param pParam : logger_param_t * <BR>
 *     @b [in/out] the pointer to the logger parameter
 *
 *  Return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL initLogger(logger_param_t * plp);

/** finalize the logger
 *
 *  Return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL finiLogger(void);

/** modify the logger according to the specified parameters.
 *
 *  - Notes
 *    -# Not implemented now
 *
 *  @param pParam : logger_param_t <BR>
 *     @b [in] the pointer to the parameters to alter the behavior of 
 *          the logger
 *
 *  Return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL modifyLogger(logger_param_t *pParam);

/** log an info type msg.
 *
 *  @param fmt : const olchar_t <BR>
 *     @b [in] the msg format
 *  @param ... : ... <BR>
 *     @b [in] the input to the msg format
 *
 *  @return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL logInfoMsg(const olchar_t * fmt, ...);

/** log an debug type msg.
 *
 *  @param fmt : const olchar_t <BR>
 *     @b [in] the msg format
 *  @param ... : ... <BR>
 *     @b [in] the input to the msg format
 *
 *  @return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL logDebugMsg(const olchar_t * fmt, ...);

/** log an error type msg.
 *
 *  @param u32ErrCode : u32 <BR>
 *     @b [in] the error code
 *  @param fmt : const olchar_t <BR>
 *     @b [in] the msg format
 *  @param ... : ... <BR>
 *     @b [in] the input to the msg format
 *
 *  @return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL logErrMsg(u32 errCode, const olchar_t * fmt, ...);

/** log a data msg. The system error code is in errno.
 *
 *  @param pu8Data : u8 * <BR>    
 *     @b [in] the data to be logged
 *  @param u32DataLen : u32 <BR>
 *     @b [in] the length of the data in bytes
 *  @param fmt : const olchar_t <BR>
 *     @b [in] the msg format
 *  @param ... : ... <BR>
 *     @b [in] the input to the msg format
 *
 *  @return: return OLERR_NO_ERROR on success, otherwise the error code
 */
LOGGERAPI u32 LOGGERCALL logDataMsg(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

LOGGERAPI u32 LOGGERCALL logDataMsgWithAscii(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

#endif /*JIUFENG_LOGGER_H*/

/*---------------------------------------------------------------------------*/


