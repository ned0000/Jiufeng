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

/* --- constant definitions --------------------------------------------------------------------- */

#define CONFIG_MGR_SERVER_ADDR  "/tmp/configmgr_server"

/** Define the config persistency type.
 */
typedef enum
{
    CMCPT_CONF_FILE = 0,
    CMCPT_SQLITE_DB,
} config_mgr_config_persistency_type_t;

/* --- data structures -------------------------------------------------------------------------- */

u32 getConfigPersistencyTypeFromString(const olchar_t * pstrType, u8 * pu8Type);

/* --- functional routines ---------------------------------------------------------------------- */



#endif /*CONFIG_MGR_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/


