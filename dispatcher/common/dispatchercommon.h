/**
 *  @file dispatchercommon.h
 *
 *  @brief Header file for common definitions and routines in dispatcher.
 *
 *  @author Min Zhang
 *
 */

#ifndef DISPATCHER_DISPATCHERCOMMON_H
#define DISPATCHER_DISPATCHERCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_messaging.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** The directory containing the uds socket file.
 */
#define DISPATCHER_UDS_DIR  "/tmp/jf_dispatcher"

typedef jf_messaging_header_t  jf_messaging_msg_t;

/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

olsize_t getDispatcherMsgSize(u8 * pu8Msg);



#endif /*DISPATCHER_DISPATCHERCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


