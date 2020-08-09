/**
 *  @file configmgrcommon.h
 *
 *  @brief Service common header file
 *
 *  @author Min Zhang
 *
 */

#ifndef CONFIG_MGR_COMMON_H
#define CONFIG_MGR_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

#include "configmgrmsg.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define CONFIG_MGR_SERVER_ADDR  "/tmp/configmgr_server"

/** Define the config persistency type.
 */
typedef enum
{
    CMCPT_CONF_FILE = 0,
    CMCPT_SQLITE_DB,
    CMCPT_MAX,
} config_mgr_config_persistency_type_t;

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type);

const olchar_t * getStringConfigPersistencyType(u8 u8Type);

olsize_t getConfigMgrMsgSize(config_mgr_msg_header_t * pHeader);

#endif /*CONFIG_MGR_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


