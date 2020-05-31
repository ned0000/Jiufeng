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
#include "jf_err.h"
#include "jf_process.h"
#include "jf_limit.h"
#include "jf_serv.h"
#include "jf_ptree.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the internal config manager setting data type.
 */
typedef struct
{
    olchar_t icms_strSettingFile[JF_LIMIT_MAX_PATH_LEN];
    olchar_t * icms_pstrVersion;

    u16 icms_u16MaxNumTransaction;
    u8 icms_u8Reserved[5];
    u8 icms_u8ConfigPersistencyType;
    olchar_t * icms_pstrConfigPersistencyLocation;

    /**The property tree representing the setting file.*/
    jf_ptree_t * icms_pjpSetting;

    u32 icms_u32Reserved[8];
} internal_config_mgr_setting_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Read service management setting file.
 */
u32 readConfigMgrSetting(internal_config_mgr_setting_t * pisms);

/** Free the resource in setting.
 */
u32 freeConfigMgrSetting(internal_config_mgr_setting_t * picms);

#endif /*CONFIG_MGR_SETTING_H*/

/*------------------------------------------------------------------------------------------------*/
