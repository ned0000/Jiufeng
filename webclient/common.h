/**
 *  @file webclient/common.h
 *
 *  @brief Header file for common data structure and routines.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef WEBCLIENT_COMMON_H
#define WEBCLIENT_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_ipaddr.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Get hash key in string from address and port.
 *
 *  @param key [out] The key in string.
 *  @param addr [in] Address of remote server.
 *  @param port [in] Port of remote server.
 *
 *  @return Length the string.
 */
olint_t getStringHashKey(olchar_t * key, jf_ipaddr_t * addr, u16 port);

#endif /*WEBCLIENT_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/
