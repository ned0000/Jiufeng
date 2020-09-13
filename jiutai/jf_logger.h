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
 *  -# The logger can be used to print logs to the stdout, syslog, log file, specified TTY and log
 *   server.
 *  -# Log to stdout is NOT thread safe.
 *
 *  @par Log Server Daemon
 *  -# The daemon's name is "jf_logserver", it's will wait for the logs from other modules and save
 *   the logs to specified log location. The log location can be stdout, log file and specified TTY.
 *  -# The daemon's log can only be saved to log file, it's a different log location for the logs
 *   from other modules.
 *  -# The default listening address is set to accept any incoming messages. The default listening
 *   port is 23456.
 *  @code
 *  Run the daemon with a specified listening port and save the logs to log file:
 *  jf_logserver -p 12345 -f all.log
 *  Run the daemon with loopback network interface and save the logs to stdout:
 *  jf_logserver -a 127.0.0.1 -o
 *  Run the daemon and save the log to TTY:
 *  jf_logserver -t /dev/pts/3
 *  Show the version information:
 *  jf_logserver -V
 *  @endcode
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

/** Maximum caller name length.
 */
#define JF_LOGGER_MAX_CALLER_NAME_LEN                    (16)

/* Maximum message size.
 */
#define JF_LOGGER_MAX_MSG_SIZE                           (512)

/** Default log server address, local loopback interface.
 */
#define JF_LOGGER_DEFAULT_SERVER_ADDRESS                 "127.0.0.1"

/** Default log server port.
 */
#define JF_LOGGER_DEFAULT_SERVER_PORT                    (23456)

/** Define the logger trace level.
 */
typedef enum
{
    /**No trace level.*/
    JF_LOGGER_TRACE_LEVEL_NONE = 0,
    /**Error trace level.*/
    JF_LOGGER_TRACE_LEVEL_ERROR,
    /**Warning trace level.*/
    JF_LOGGER_TRACE_LEVEL_WARN,
    /**Information trace level.*/
    JF_LOGGER_TRACE_LEVEL_INFO,
    /**Debug trace level.*/
    JF_LOGGER_TRACE_LEVEL_DEBUG,
    /**Data trace level.*/
    JF_LOGGER_TRACE_LEVEL_DATA,
} jf_logger_trace_level_t;

/** The helper function to output error message with function name and line number.
 */
#define JF_LOGGER_ERR(u32ErrCode, fmt, ...)  \
    jf_logger_logErrMsg(u32ErrCode, "%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** The helper function to output warning message with function name and line number.
 */
#define JF_LOGGER_WARN(fmt, ...)  \
    jf_logger_logWarnMsg("%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** The helper function to output information message with function name and line number.
 */
#define JF_LOGGER_INFO(fmt, ...)  \
    jf_logger_logInfoMsg("%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** The helper function to output debug message with function name and line number.
 */
#define JF_LOGGER_DEBUG(fmt, ...)  \
    jf_logger_logDebugMsg("%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** The helper function to output data message with function name and line number.
 */
#define JF_LOGGER_DATA(data, len, fmt, ...)  \
    jf_logger_logDataMsg(data, len, "%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/** The helper function to output data message with ASCII with function name and line number.
 */
#define JF_LOGGER_DATAA(data, len, fmt, ...)  \
    jf_logger_logDataMsgWithAscii(data, len, "%s:%d, "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameters for initializing logger.
 */
typedef struct
{
    /**Log to the stdout.*/
    boolean_t jlip_bLogToStdout;
    /**Log to the system log, for Linux only now.*/
    boolean_t jlip_bLogToSystemLog;
    /**Log to a file.*/
    boolean_t jlip_bLogToFile;
    /**Log to the specified TTY, for Linux only. If TRUE, tty path must be specified.*/
    boolean_t jlip_bLogToTty;
    /**Log to the server. If TRUE, server address and port must be specified. Other log location
       will be ignored when logging to server is enabled.*/
    boolean_t jlip_bLogToServer;
    u8 jlip_u8Reserved[3];

    /**Trace level.*/
    u8 jlip_u8TraceLevel;
    u8 jlip_u8Reserved2[7];

    /**The name of the caller. The length should not exceed JF_LOGGER_MAX_CALLER_NAME_LEN.*/
    olchar_t * jlip_pstrCallerName;

    /**The log file name, the log file will be "callername.log" if it's not specified.*/
    olchar_t * jlip_pstrLogFile;
    /**The size of the log file in byte. If 0, no limit.*/
    olsize_t jlip_sLogFile;

    /**The TTY file name.*/
    olchar_t * jlip_pstrTtyFile;

    /**The address of the log server, only IPv4 is supported now.*/
    olchar_t * jlip_pstrServerAddress;
    /**The port of the log server.*/
    u16 jlip_u16ServerPort;
    u16 jlip_u16Reserved[3];

} jf_logger_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the logger according to the specified parameters.
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

/** Log an info type message.
 *
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logInfoMsg(const olchar_t * fmt, ...);

/** Log an debug type message.
 *
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDebugMsg(const olchar_t * fmt, ...);

/** Log an warning type message.
 *
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logWarnMsg(const olchar_t * fmt, ...);

/** Log an error type message.
 *
 *  @param u32ErrCode [in] The error code.
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logErrMsg(u32 u32ErrCode, const olchar_t * fmt, ...);

/** Log a data message. The system error code is in errno.
 *
 *  @param pu8Data [in] The data to be logged.
 *  @param u32DataLen [in] The length of the data in bytes.
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDataMsg(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

/** Log a data message with ascii string. The system error code is in errno.
 *
 *  @param pu8Data [in] The data to be logged.
 *  @param u32DataLen [in] The length of the data in bytes.
 *  @param fmt [in] The message format.
 *  @param ... [in] The input to the message format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
LOGGERAPI u32 LOGGERCALL jf_logger_logDataMsgWithAscii(
    u8 * pu8Data, u32 u32DataLen, const olchar_t * fmt, ...);

#endif /*JIUFENG_LOGGER_H*/

/*------------------------------------------------------------------------------------------------*/
