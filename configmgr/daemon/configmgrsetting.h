/**
 *  @file configmgrsetting.h
 *
 *  @brief Setting header file, provide some functional routine to read config manager setting.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef CONFIG_MGR_SETTING_H
#define CONFIG_MGR_SETTING_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_ptree.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the internal config manager setting data type.
 */
typedef struct
{
    /**Setting file.*/
    olchar_t icms_strSettingFile[JF_LIMIT_MAX_PATH_LEN];
    /**Setting version.*/
    olchar_t * icms_pstrVersion;

    /**Maximum number of transaction.*/
    u16 icms_u16MaxNumOfTransaction;
    u8 icms_u8Reserved[5];
    /**Config persistency type, refer to config_mgr_config_persistency_type_t.*/
    u8 icms_u8ConfigPersistencyType;
    /**Config persistency location, it's the file path.*/
    olchar_t * icms_pstrConfigPersistencyLocation;

    /**The property tree representing the setting file.*/
    jf_ptree_t * icms_pjpSetting;

    u32 icms_u32Reserved[8];
} internal_config_mgr_setting_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Read service management setting file.
 *
 *  @param picms [in] The internal setting object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 readConfigMgrSetting(internal_config_mgr_setting_t * picms);

/** Free the resource in setting.
 *
 *  @param picms [in] The internal setting object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 freeConfigMgrSetting(internal_config_mgr_setting_t * picms);

#endif /*CONFIG_MGR_SETTING_H*/

/*------------------------------------------------------------------------------------------------*/
