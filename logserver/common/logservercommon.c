/**
 *  @file logservercommon.c
 *
 *  @brief Implementation file for log server common routine.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_process.h"
#include "jf_logger.h"

#include "logservercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

olsize_t getLogServerMsgSize(log_2_server_msg_header_t * pHeader)
{
    return sizeof(*pHeader) + (olsize_t)pHeader->l2smh_u16PayloadSize;
}

/*------------------------------------------------------------------------------------------------*/
