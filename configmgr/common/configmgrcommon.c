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
#include "jf_process.h"
#include "jf_config.h"

#include "configmgrcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (strcmp(pstrType, "conf_file") == 0)
        *pu8Type = CMCPT_CONF_FILE;
    else if (strcmp(pstrType, "sqlite_db") == 0)
        *pu8Type = CMCPT_SQLITE_DB;
    else
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


