/**
 *  @file webclient/common.c
 *
 *  @brief Common routines shared by all other modules.
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

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
