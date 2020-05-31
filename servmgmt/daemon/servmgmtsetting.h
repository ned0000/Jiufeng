/**
 *  @file servmgmtsetting.h
 *
 *  @brief Setting header file, provide some functional routine to read service setting.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef DONGYUAN_SERVMGMT_SETTING_H
#define DONGYUAN_SERVMGMT_SETTING_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_process.h"
#include "jf_limit.h"
#include "jf_serv.h"
#include "jf_ptree.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum service command parameter length.
 */
#define MAX_SERVICE_CMD_PARAM_LEN    (512)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the internal service information data type.
 */
typedef struct
{
    olchar_t * isi_pstrName;
    olchar_t * isi_pstrDescription;
    olchar_t * isi_pstrCmdPath;
    olchar_t * isi_pstrCmdParam;
    u8 isi_u8StartupType;
    u8 isi_u8Reserved[7];

    /**The property tree node for startup type.*/
    jf_ptree_node_t * isi_pjpnStartupType;

    u8 isi_u8Reserved2[32];

    u8 isi_u8Status;
    u8 isi_u8RestartCount;
    u8 isi_u8Reserved3[6];

    jf_process_handle_t isi_jphHandle;

} internal_service_info_t;

/** Define the internal service management setting data type.
 */
typedef struct
{
    olchar_t isms_strSettingFile[JF_LIMIT_MAX_PATH_LEN];
    u16 isms_u16NumOfService;
    u16 isms_u16Reserved[3];
    u8 isms_u8FailureRetryCount;
    u8 isms_u8Reserved2[15];
    /**The property tree representing the setting file.*/
    jf_ptree_t * isms_pjpService;
    olchar_t * isms_pstrVersion;

    internal_service_info_t isms_isiService[JF_SERV_MAX_NUM_OF_SERV];

} internal_serv_mgmt_setting_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Read service management setting file.
 */
u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

/** Free the resource in setting.
 */
u32 freeServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

/** Modify the startup type of the service and save the new setting.
 */
u32 modifyServiceStartupType(internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi);

#endif /*DONGYUAN_SERVMGMT_SETTING_H*/

/*------------------------------------------------------------------------------------------------*/


