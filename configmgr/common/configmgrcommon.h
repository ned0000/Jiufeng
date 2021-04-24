/**
 *  @file configmgrcommon.h
 *
 *  @brief Header file for common routines of config manager.
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

/** Server address of config manager.
 */
#define CONFIG_MGR_SERVER_ADDR  "/tmp/configmgr_server"

/** Define the config persistency type.
 */
typedef enum
{
    /**Unknown persistency type.*/
    CMCPT_UNKNOWN = 0,
    /**File persistency type.*/
    CMCPT_CONF_FILE,
    /**Sqlite DB persistency type.*/
    CMCPT_SQLITE_DB,
    /**Maximum persistency type.*/
    CMCPT_MAX,
} config_mgr_config_persistency_type_t;

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Get persistency type from string.
 *
 *  @param pstrType [in] The string of persistency type.
 *  @param pu8Type [out] The persistency type.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type);

/** Get string of persistency type.
 *
 *  @param u8Type [in] The persistency type.
 *
 *  @return The string of persistency type.
 */
const olchar_t * getStringConfigPersistencyType(u8 u8Type);

/** Get full config manager message size.
 *
 *  @param pHeader [in] The message header.
 *
 *  @return The full message size.
 */
olsize_t getConfigMgrMsgSize(config_mgr_msg_header_t * pHeader);

#endif /*CONFIG_MGR_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/
