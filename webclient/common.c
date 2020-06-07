/**
 *  @file webclient/common.c
 *
 *  @brief Common routines shared by all other object.
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_ipaddr.h"
#include "jf_mutex.h"
#include "jf_queue.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

olint_t getStringHashKey(olchar_t * key, jf_ipaddr_t * addr, u16 port)
{
    olint_t keyLength;
    olchar_t strIpAddr[50];

    jf_ipaddr_getStringIpAddr(strIpAddr, sizeof(strIpAddr), addr);
    keyLength = ol_sprintf(key, "%s:%d", strIpAddr, port);

    return keyLength;
}


/*------------------------------------------------------------------------------------------------*/

