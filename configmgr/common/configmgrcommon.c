/**
 *  @file configmgrcommon.c
 *
 *  @brief Implementation file for config manager common routine.
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

/** Define the string of config persistency type.
 */
static olchar_t * ls_pstrConfigPersistencyType[CMCPT_MAX + 1] =
{
    "conf_file",
    "sqlite_db",
    "unknown",
};

/* --- public routine section ------------------------------------------------------------------- */

u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ol_strcmp(pstrType, ls_pstrConfigPersistencyType[CMCPT_CONF_FILE]) == 0)
        *pu8Type = CMCPT_CONF_FILE;
    else if (ol_strcmp(pstrType, ls_pstrConfigPersistencyType[CMCPT_SQLITE_DB]) == 0)
        *pu8Type = CMCPT_SQLITE_DB;
    else
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

const olchar_t * getStringConfigPersistencyType(u8 u8Type)
{
    if (u8Type > CMCPT_MAX)
        u8Type = CMCPT_MAX;

    return ls_pstrConfigPersistencyType[u8Type];
}    

olsize_t getConfigMgrMsgSize(config_mgr_msg_header_t * pHeader)
{
    return sizeof(*pHeader) + (olsize_t)pHeader->cmmh_u32PayloadSize;
}

/*------------------------------------------------------------------------------------------------*/


