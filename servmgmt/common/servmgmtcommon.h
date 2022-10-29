/**
 *  @file servmgmtcommon.h
 *
 *  @brief Header file for common things of sevice management.
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

/* --- constant definitions --------------------------------------------------------------------- */

/** The service management server address.
 */
#define SERVMGMT_SERVER_ADDR     "/tmp/servmgmt_server"

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Get string of service status.
 *
 *  @param u8Status [in] The status.
 *
 *  @return The string of service status.
 */
const olchar_t * getStringServStatus(u8 u8Status);

/** Get string of service startup type.
 *
 *  @param u8StartupType [in] The startup type.
 *
 *  @return The string of service startup type.
 */
const olchar_t * getStringServStartupType(u8 u8StartupType);

/** Get service startup type from string.
 *
 *  @param pstrType [in] The string of service startup type.
 *  @param pu8StartupType [out] The startup type.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*SERVMGMT_SERVMGMTCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/
