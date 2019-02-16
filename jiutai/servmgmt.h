/**
 *  @file servmgmt.h
 *
 *  @brief Service mamagement header file, provide some functional routine for
 *   service management
 *
 *  @author Min Zhang
 *
 *  @note Service management library will create the setting and status files.
 *   The location of those files are in current working directory. The user
 *   should set the working directory accordingly before using this library.
 *  
 */

#ifndef JIUFENG_SERVMGMT_H
#define JIUFENG_SERVMGMT_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

#undef SERVMGMTAPI
#undef SERVMGMTCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_SERVMGMT_DLL)
        #define SERVMGMTAPI  __declspec(dllexport)
        #define SERVMGMTCALL
    #else
        #define SERVMGMTAPI
        #define SERVMGMTCALL __cdecl
    #endif
#else
    #define SERVMGMTAPI
    #define SERVMGMTCALL
#endif

/* --- constant definitions ------------------------------------------------ */
#define MAX_NUM_OF_SERVICE      (24)
#define MAX_SERVICE_NAME_LEN    (24)

typedef enum
{
    SERVICE_STATUS_UNKNOWN = 0,
    SERVICE_STATUS_STOPPED,
    SERVICE_STATUS_STARTING,
    SERVICE_STATUS_RUNNING,
    SERVICE_STATUS_STOPPING,
    SERVICE_STATUS_ERROR,
    SERVICE_STATUS_TERMINATED,
} service_status_t;

typedef enum
{
    SERVICE_STARTUPTYPE_UNKNOWN = 0,
    SERVICE_STARTUPTYPE_AUTOMATIC,
    SERVICE_STARTUPTYPE_MANUAL,
} service_startuptype_t;

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    olchar_t si_strName[MAX_SERVICE_NAME_LEN];
    u8 si_u8Status;
    u8 si_u8StartupType;
    u8 si_u8Reserved[22];
} service_info_t;

typedef struct
{
    u8 smi_u8Reserved[7];
    u8 smi_u8NumOfService;
    service_info_t smi_siService[MAX_NUM_OF_SERVICE];
    u8 smi_u8Reserved2[16];
} serv_mgmt_info_t;

typedef struct
{
    u8 smp_u8Reserved[32];
} serv_mgmt_param_t;

typedef struct
{
    olchar_t * ssmp_pstrSettingFile;
    u32 ssmp_u32Reserved[8];
} start_serv_mgmt_param_t;

/* --- functional routines ------------------------------------------------- */
SERVMGMTAPI u32 SERVMGMTCALL initServMgmt(serv_mgmt_param_t * psmp);

SERVMGMTAPI u32 SERVMGMTCALL finiServMgmt(void);

SERVMGMTAPI u32 SERVMGMTCALL startServMgmt(start_serv_mgmt_param_t * pssmp);

SERVMGMTAPI u32 SERVMGMTCALL stopServMgmt(void);

SERVMGMTAPI u32 SERVMGMTCALL getServMgmtInfo(serv_mgmt_info_t * psmi);

SERVMGMTAPI const olchar_t * SERVMGMTCALL getStringServMgmtServStatus(
    u8 u8Status);

SERVMGMTAPI const olchar_t * SERVMGMTCALL getStringServMgmtServStartupType(
    u8 u8StartupType);

SERVMGMTAPI u32 SERVMGMTCALL getServMgmtServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

SERVMGMTAPI u32 SERVMGMTCALL stopServMgmtServ(olchar_t * pstrName);

SERVMGMTAPI u32 SERVMGMTCALL startServMgmtServ(olchar_t * pstrName);

SERVMGMTAPI u32 SERVMGMTCALL setServMgmtServStartupType(
    olchar_t * pstrName, u8 u8StartupType);

#endif /*JIUFENG_SERVMGMT_H*/

/*---------------------------------------------------------------------------*/


