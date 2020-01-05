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

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

olint_t getStringHashKey(olchar_t * key, jf_ipaddr_t * addr, u16 port);

#endif /*WEBCLIENT_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


