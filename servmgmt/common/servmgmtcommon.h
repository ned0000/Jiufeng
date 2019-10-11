/**
 *  @file servmgmtcommon.h
 *
 *  @brief Service common header file
 *
 *  @author Min Zhang
 *
 */

#ifndef SERVMGMT_SERVMGMTCOMMON_H
#define SERVMGMT_SERVMGMTCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_sharedmemory.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define SERVMGMT_SERVER_ADDR  "/tmp/servmgmt_server"

/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

const olchar_t * getStringServStatus(u8 u8Status);

const olchar_t * getStringServStartupType(u8 u8StartupType);

u32 getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*SERVMGMT_SERVMGMTCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


