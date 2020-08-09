/**
 *  @file logservercommon.h
 *
 *  @brief Service common header file
 *
 *  @author Min Zhang
 *
 */

#ifndef LOG_SERVER_COMMON_H
#define LOG_SERVER_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

#include "logservermsg.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

olsize_t getLogServerMsgSize(log_2_server_msg_header_t * pHeader);

#endif /*LOG_SERVER_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


