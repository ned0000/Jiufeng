/**
 *  @file logserver/daemon/logsave.h
 *
 *  @brief Log save header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_LOG_SERVER_LOG_SAVE_H
#define JIUFENG_LOG_SERVER_LOG_SAVE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
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

} log_save_init_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

u32 initLogSave(log_save_init_param_t * plsip);

u32 finiLogSave(void);

u32 startLogSave(void);

u32 stopLogSave(void);

u32 saveLogToQueue(u64 u64Time, const olchar_t * pstrBanner, const olchar_t * pstrLog);

#endif /*JIUFENG_LOG_SERVER_LOG_SAVE_H*/

/*------------------------------------------------------------------------------------------------*/


