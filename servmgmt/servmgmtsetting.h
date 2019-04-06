/**
 *  @file servmgmtsetting.h
 *
 *  @brief read dongyuan setting header file, provide some functional routine to
 *   read gaea setting
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef SERVMGMT_SETTING_H
#define SERVMGMT_SETTING_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "process.h"

/* --- constant definitions ------------------------------------------------ */
#define MAX_SERVICE_CMD_PARAM_LEN    (512)

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    olchar_t isi_strName[JF_SERVMGMT_MAX_SERV_NAME_LEN];
    u8 isi_u8Status;
    u8 isi_u8StartupType;
#define SERVICE_ROLE_INTERNAL   0
#define SERVICE_ROLE_WAKEUP     1
#define SERVICE_ROLE_EXTERNAL   2
    u8 isi_u8Role;
    u8 isi_u8Reserved[5];
    olchar_t isi_strCmdPath[MAX_PATH_LEN];
    olchar_t isi_strCmdParam[MAX_SERVICE_CMD_PARAM_LEN];
    /**the delay to start a service when the termination of the service is
       detected, in second*/
    u8 isi_u8RestartDelay;

    u8 isi_u8RestartCount;
    u8 isi_u8Reserved2[30];

    jf_process_id_t isi_jpiProcessId;
} internal_service_info_t;

typedef struct
{
    olchar_t isms_strSettingFile[MAX_PATH_LEN];
    olchar_t isms_strVersion[8];
    u32 isms_u32Reserved;
    u32 isms_u32NumOfService;
    internal_service_info_t isms_isiService[JF_SERVMGMT_MAX_NUM_OF_SERVICE];
    u8 isms_u8Reserved2[16];
} internal_serv_mgmt_setting_t;

/* --- functional routines ------------------------------------------------- */
u32 readServMgmtSetting(
    internal_serv_mgmt_setting_t * pisms);

u32 writeServMgmtSetting(
    internal_serv_mgmt_setting_t * pisms);

u32 modifyServiceStartupType(
    const olchar_t * pu8SettingFile, olchar_t * pstrServiceName,
    u8 u8StartupType);

#endif /*SERVMGMT_SETTING_H*/

/*---------------------------------------------------------------------------*/


