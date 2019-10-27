/**
 *  @file servmgmtcommon.c
 *
 *  @brief The service management common routine
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_sharedmemory.h"
#include "jf_filestream.h"
#include "jf_process.h"
#include "jf_attask.h"
#include "jf_time.h"
#include "jf_serv.h"

#include "servmgmtcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

const olchar_t * getStringServStatus(u8 u8Status)
{
    olchar_t * pstr = NULL;

    if (u8Status == JF_SERV_STATUS_STOPPED)
        pstr = "stopped";
    else if (u8Status == JF_SERV_STATUS_STARTING)
        pstr = "starting";
    else if (u8Status == JF_SERV_STATUS_RUNNING)
        pstr = "running";
    else if (u8Status == JF_SERV_STATUS_STOPPING)
        pstr = "stopping";
    else if (u8Status == JF_SERV_STATUS_ERROR)
        pstr = "error";
    else if (u8Status == JF_SERV_STATUS_TERMINATED)
        pstr = "terminated";
    else
        pstr = "unknown";

    return pstr;
}

const olchar_t * getStringServStartupType(u8 u8StartupType)
{
    olchar_t * pstr = NULL;

    if (u8StartupType == JF_SERV_STARTUP_TYPE_AUTOMATIC)
        pstr = "automatic";
    else if (u8StartupType == JF_SERV_STARTUP_TYPE_MANUAL)
        pstr = "manual";
    else
        pstr = "unknown";

    return pstr;
}

u32 getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (strcmp(pstrType, "automatic") == 0)
        *pu8StartupType = JF_SERV_STARTUP_TYPE_AUTOMATIC;
    else if (strcmp(pstrType, "manual") == 0)
        *pu8StartupType = JF_SERV_STARTUP_TYPE_MANUAL;
    else
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


