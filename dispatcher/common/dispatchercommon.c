/**
 *  @file dispatchercommon.c
 *
 *  @brief The common routine in dispatcher.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_messaging.h"

#include "dispatchercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

olsize_t getDispatcherMsgSize(u8 * pu8Msg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return sizeof(*pHeader) + pHeader->jmh_u32PayloadSize;
}



/*------------------------------------------------------------------------------------------------*/


