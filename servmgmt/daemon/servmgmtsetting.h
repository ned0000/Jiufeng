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
#define MAX_SERVICE_CMD_PARAM_LEN               (512)

/** Maximum service pause time in second.
 */
#define MAX_SERVICE_PAUSE_TIME                  (10)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the internal service information data type.
 */
typedef struct
{
    /**Service name.*/
    olchar_t * isi_pstrName;
    /**Service description.*/
    olchar_t * isi_pstrDescription;
    /**Service command path.*/
    olchar_t * isi_pstrCmdPath;
    /**Service command parameter.*/
    olchar_t * isi_pstrCmdParam;
    /**Service startup type.*/
    u8 isi_u8StartupType;
    /**Pause in second after start this service.*/
    u8 isi_u8PauseTime;
    u8 isi_u8Reserved[6];

    /**The property tree node for startup type.*/
    jf_ptree_node_t * isi_pjpnStartupType;

    u8 isi_u8Reserved2[32];

    /**Service status.*/
    u8 isi_u8Status;
    /**Restart count of this Service.*/
    u8 isi_u8RestartCount;
    u8 isi_u8Reserved3[6];

    /**Process handle of this service.*/
    jf_process_handle_t isi_jphHandle;

} internal_service_info_t;

/** Define the internal service management setting data type.
 */
typedef struct
{
    /**Service management setting file path.*/
    olchar_t isms_strSettingFile[JF_LIMIT_MAX_PATH_LEN];
    /**Number of service in the array.*/
    u16 isms_u16NumOfService;
    u16 isms_u16Reserved[3];
    /**Failure retry count for a service.*/
    u8 isms_u8FailureRetryCount;
    u8 isms_u8Reserved2[15];
    /**The property tree representing the setting file.*/
    jf_ptree_t * isms_pjpService;
    /**SerService management setting version.*/
    olchar_t * isms_pstrVersion;

    /**Service array.*/
    internal_service_info_t isms_isiService[JF_SERV_MAX_NUM_OF_SERV];

} internal_serv_mgmt_setting_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Read service management setting file.
 *
 *  @param pisms [out] The service management setting object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

/** Free the resource in setting.
 *
 *  @param pisms [out] The service management setting object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 freeServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

/** Modify the startup type of the service and save the new setting.
 *
 *  @param pisms [in] The service management setting object.
 *  @param pisi [in] The service information to modify startup type.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 modifyServiceStartupType(internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi);

#endif /*DONGYUAN_SERVMGMT_SETTING_H*/

/*------------------------------------------------------------------------------------------------*/
