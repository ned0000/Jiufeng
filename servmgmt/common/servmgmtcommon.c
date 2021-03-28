/**
 *  @file servmgmtcommon.c
 *
 *  @brief Implementation file for common routines of service management.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_serv.h"

#include "servmgmtcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrServStatus[JF_SERV_STATUS_MAX] =
{
    "unknown",
    "stopped",
    "starting",
    "running",
    "stopping",
    "error",
    "terminated",
};

static olchar_t * ls_pstrServStartupType[JF_SERV_STARTUP_TYPE_MAX] =
{
    "unknown",
    "automatic",
    "manual",
};

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

const olchar_t * getStringServStatus(u8 u8Status)
{
    olchar_t * pstr = NULL;

    if ((u8Status > JF_SERV_STATUS_UNKNOWN) && (u8Status < JF_SERV_STATUS_MAX))
        pstr = ls_pstrServStatus[u8Status];
    else
        pstr = ls_pstrServStatus[JF_SERV_STATUS_UNKNOWN];

    return pstr;
}

const olchar_t * getStringServStartupType(u8 u8StartupType)
{
    olchar_t * pstr = NULL;

    if ((u8StartupType > JF_SERV_STARTUP_TYPE_UNKNOWN) &&
        (u8StartupType < JF_SERV_STARTUP_TYPE_MAX))
        pstr = ls_pstrServStartupType[u8StartupType];
    else
        pstr = ls_pstrServStartupType[JF_SERV_STARTUP_TYPE_UNKNOWN];

    return pstr;
}

u32 getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Index = 0;

    for (u8Index = 0; u8Index < JF_SERV_STARTUP_TYPE_MAX; u8Index ++)
    {
        if (strcmp(pstrType, ls_pstrServStartupType[u8Index]) == 0)
        {
            *pu8StartupType = u8Index;
            break;
        }
    }

    if (u8Index == JF_SERV_STARTUP_TYPE_MAX)
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
