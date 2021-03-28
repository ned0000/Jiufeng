/**
 *  @file logserver/daemon/logserver.h
 *
 *  @brief Log server header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_LOG_SERVER_H
#define JIUFENG_LOG_SERVER_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Log server name.
 */
#define LOG_SERVER_NAME                   "logserver"

/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for initializing the log server data type.
 */
typedef struct
{
    /**Command line for the service.*/
    olchar_t * lsip_pstrCmdLine;

    /**Server address. All available network interfaces are binded if it's not specified.*/
    olchar_t * lsip_pstrServerAddress;
    /**Server port.*/
    u16 lsip_u16ServerPort;
    u16 lsip_u16Reserved2[3];

    /**Maximum log client supported.*/
    u16 lsip_u16MaxLogClient;
    u16 lsip_u16Reserved[3];

    /**Log to the stdout.*/
    boolean_t lsip_bLogToStdout;
    /**Log to a file.*/
    boolean_t lsip_bLogToFile;
    /**Log to the specified TTY, for Linux only. If TRUE, tty path must be specified.*/
    boolean_t lsip_bLogToTty;
    u8 lsip_u8Reserved[5];

    /**The log file name, the log file will be "jiufeng.log" if it's not specified.*/
    olchar_t * lsip_pstrLogFile;
    /**The size of the log file in byte. If 0, no limit.*/
    olsize_t lsip_sLogFile;

    /**The TTY file name.*/
    olchar_t * lsip_pstrTtyFile;

} log_server_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Set default parameter of log server.
 *
 *  @param plsip [in] The pointer to the log server parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 setDefaultLogServerParam(log_server_init_param_t * plsip);

/** Initialize the log server according to the specified parameters.
 *
 *  @param plsip [in] The pointer to the log server parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 */
u32 initLogServer(log_server_init_param_t * plsip);

/** Finalize the log server.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 finiLogServer(void);

/** Start the log server.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 startLogServer(void);

/** Stop the log server.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 stopLogServer(void);

#endif /*JIUFENG_LOG_SERVER_H*/

/*------------------------------------------------------------------------------------------------*/
