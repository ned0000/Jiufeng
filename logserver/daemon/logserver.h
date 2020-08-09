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

#define LOG_SERVER_NAME                   "logserver"

/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for initializing the log server data type.
 */
typedef struct
{
    olchar_t * lsip_pstrCmdLine;

    olchar_t * lsip_pstrServerAddress;
    u16 lsip_u16ServerPort;
    u16 lsip_u16Reserved2[3];

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

u32 setDefaultLogServerParam(log_server_init_param_t * plsip);

u32 initLogServer(log_server_init_param_t * plsip);

u32 finiLogServer(void);

u32 startLogServer(void);

u32 stopLogServer(void);

#endif /*JIUFENG_LOG_SERVER_H*/

/*------------------------------------------------------------------------------------------------*/
