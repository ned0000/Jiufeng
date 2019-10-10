/**
 *  @file servmgmtsetting.h
 *
 *  @brief Setting header file, provide some functional routine to read service setting
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

/* --- constant definitions --------------------------------------------------------------------- */

#define MAX_SERVICE_CMD_PARAM_LEN    (512)

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
    olchar_t isi_strName[JF_SERV_MAX_SERV_NAME_LEN];
    u8 isi_u8Status;
    u8 isi_u8StartupType;
    u8 isi_u8Reserved[5];
    olchar_t isi_strCmdPath[JF_LIMIT_MAX_PATH_LEN];
    olchar_t isi_strCmdParam[MAX_SERVICE_CMD_PARAM_LEN];

    u8 isi_u8Reserved2[32];

    jf_process_id_t isi_jpiProcessId;
    u8 isi_u8RestartCount;
    u8 isi_u8Reserved3[7];
} internal_service_info_t;

typedef struct
{
    olchar_t isms_strSettingFile[JF_LIMIT_MAX_PATH_LEN];
    olchar_t isms_strVersion[8];
    u16 isms_u16NumOfService;
    u16 isms_u16Reserved[3];
    u8 isms_u8FailureRetryCount;
    u8 isms_u8Reserved2[15];

    internal_service_info_t isms_isiService[JF_SERV_MAX_NUM_OF_SERV];

} internal_serv_mgmt_setting_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

u32 writeServMgmtSetting(internal_serv_mgmt_setting_t * pisms);

u32 modifyServiceStartupType(
    const olchar_t * pu8SettingFile, olchar_t * pstrServName, jf_serv_startup_type_t startupType);

#endif /*DONGYUAN_SERVMGMT_SETTING_H*/

/*------------------------------------------------------------------------------------------------*/


