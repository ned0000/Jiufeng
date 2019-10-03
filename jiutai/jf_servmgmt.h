/**
 *  @file jf_servmgmt.h
 *
 *  @brief Service mamagement header file, provide some functional routine for
 *   service management
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_servmgmt library
 *  @note Service management library will create the setting and status files.
 *   The location of those files are in current working directory. The user
 *   should set the working directory accordingly before using this library.
 *  @note User libxml2-dev as the XML parser to parse setting file
 */

#ifndef JIUFENG_SERVMGMT_H
#define JIUFENG_SERVMGMT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

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

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_SERVMGMT_MAX_NUM_OF_SERVICE      (24)

#define JF_SERVMGMT_MAX_SERV_NAME_LEN       (24)

typedef enum
{
    JF_SERVMGMT_SERV_STATUS_UNKNOWN = 0,
    JF_SERVMGMT_SERV_STATUS_STOPPED,
    JF_SERVMGMT_SERV_STATUS_STARTING,
    JF_SERVMGMT_SERV_STATUS_RUNNING,
    JF_SERVMGMT_SERV_STATUS_STOPPING,
    JF_SERVMGMT_SERV_STATUS_ERROR,
    JF_SERVMGMT_SERV_STATUS_TERMINATED,
} jf_servmgmt_serv_status_t;

typedef enum
{
    JF_SERVMGMT_SERV_STARTUPTYPE_UNKNOWN = 0,
    JF_SERVMGMT_SERV_STARTUPTYPE_AUTOMATIC,
    JF_SERVMGMT_SERV_STARTUPTYPE_MANUAL,
} jf_servmgmt_serv_startuptype_t;

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t jssi_strName[JF_SERVMGMT_MAX_SERV_NAME_LEN];
    u8 jssi_u8Status;
    u8 jssi_u8StartupType;
    u8 jssi_u8Reserved[22];
} jf_servmgmt_serv_info_t;

typedef struct
{
    u8 jsi_u8Reserved[7];
    u8 jsi_u8NumOfService;
    jf_servmgmt_serv_info_t jsi_jssiService[JF_SERVMGMT_MAX_SERV_NAME_LEN];
    u8 jsi_u8Reserved2[16];
} jf_servmgmt_info_t;

typedef struct
{
    u8 jsip_u8Reserved[32];
} jf_servmgmt_init_param_t;

typedef struct
{
    olchar_t * jssp_pstrSettingFile;
    u32 jssp_u32Reserved[8];
} jf_servmgmt_start_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_init(jf_servmgmt_init_param_t * pjsip);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_fini(void);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_start(jf_servmgmt_start_param_t * pjssp);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_stop(void);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_getInfo(jf_servmgmt_info_t * pjsi);

SERVMGMTAPI const olchar_t * SERVMGMTCALL jf_servmgmt_getStringServStatus(u8 u8Status);

SERVMGMTAPI const olchar_t * SERVMGMTCALL jf_servmgmt_getStringServStartupType(u8 u8StartupType);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_stopServ(olchar_t * pstrName);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_startServ(olchar_t * pstrName);

SERVMGMTAPI u32 SERVMGMTCALL jf_servmgmt_setServStartupType(olchar_t * pstrName, u8 u8StartupType);

#endif /*JIUFENG_SERVMGMT_H*/

/*------------------------------------------------------------------------------------------------*/


