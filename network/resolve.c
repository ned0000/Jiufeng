/**
 *  @file resolve.c
 *
 *  @brief Implementation file for routines to resolve host name etc.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_network.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_getHostByName(
    const olchar_t * pstrName, struct hostent ** ppHostent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct hostent * pHost = NULL;

    pHost = gethostbyname(pstrName);
    if (pHost == NULL)
    {
#if defined(LINUX)
        switch (h_errno)
        {
        case HOST_NOT_FOUND:
            u32Ret = JF_ERR_HOST_NOT_FOUND;
            break;
        case NO_ADDRESS:
            u32Ret = JF_ERR_HOST_NO_ADDRESS;
            break;
        case NO_RECOVERY:
            u32Ret = JF_ERR_NAME_SERVER_NO_RECOVERY;
            break;
        case TRY_AGAIN:
            u32Ret = JF_ERR_RESOLVE_TRY_AGAIN;
            break;
        default:
            u32Ret = JF_ERR_FAIL_RESOLVE_HOST;
            break;
        }
#elif defined(WINDOWS)
        switch (WSAGetLastError())
        {
        case WSAHOST_NOT_FOUND:
            break;
        default:
            u32Ret = JF_ERR_FAIL_RESOLVE_HOST;
            break;
        }
#endif
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppHostent = pHost;
    else
        *ppHostent = NULL;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
