/**
 *  @file user.h
 *
 *  @brief Header file defines the routines for user operation.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef JIUTAI_USER_H
#define JIUTAI_USER_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <sys/types.h>
#include <pwd.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"


/* --- constant definitions --------------------------------------------------------------------- */

/** The maximum user name length.
 */
#define JF_USER_MAX_NAME_LEN    (32)

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Get user ID and group ID for the specified user.
 *
 *  @param pstrName [in] The name of the user.
 *  @param pUid [out] The user ID.
 *  @param pGid [out] The group ID.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_GET_USER_INFO Failed to get user info.
 */
u32 jf_user_getUidGid(const olchar_t * pstrName, uid_t * pUid, gid_t * pGid);

#endif /*JIUTAI_USER_H*/

/*------------------------------------------------------------------------------------------------*/


