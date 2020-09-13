/**
 *  @file jf_user.c
 *
 *  @brief Implementation file for routine related to user.
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_user.h"
#include "jf_err.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_user_getUidGid(const olchar_t * pstrName, uid_t * pUid, gid_t * pGid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(WINDOWS)
    u32Ret = JF_ERR_NOT_IMPLEMENTED;
#elif defined(LINUX)
    struct passwd * pUserinfo = NULL;

    pUserinfo = getpwnam(pstrName);
    if (pUserinfo == NULL)
    {
        u32Ret = JF_ERR_FAIL_GET_USER_INFO;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *pUid = pUserinfo->pw_uid;
        *pGid = pUserinfo->pw_gid;
    }
#endif
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
