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
static olchar_t * ls_pstrConfigPersistencyType[CMCPT_MAX] =
{
    "unknown",
    "conf_file",
    "sqlite_db",
};

/* --- public routine section ------------------------------------------------------------------- */

u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Index = 0;

    *pu8Type = CMCPT_UNKNOWN;

    for (u8Index = 0; u8Index < CMCPT_MAX; u8Index ++)
    {
        if (ol_strcmp(pstrType, ls_pstrConfigPersistencyType[u8Index]) == 0)
        {
            *pu8Type = u8Index;
            break;
        }
    }

    if (u8Index == CMCPT_MAX)
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

const olchar_t * getStringConfigPersistencyType(u8 u8Type)
{
    if (u8Type >= CMCPT_MAX)
        u8Type = CMCPT_UNKNOWN;

    return ls_pstrConfigPersistencyType[u8Type];
}    

olsize_t getConfigMgrMsgSize(config_mgr_msg_header_t * pHeader)
{
    return sizeof(*pHeader) + (olsize_t)pHeader->cmmh_u32PayloadSize;
}

/*------------------------------------------------------------------------------------------------*/
