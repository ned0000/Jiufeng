/**
 *  @file jf_serv.h
 *
 *  @brief Service header file, provide some functional routine for service
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_serv library
 */

#ifndef JIUFENG_SERV_H
#define JIUFENG_SERV_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

#undef SERVAPI
#undef SERVCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_SERV_DLL)
        #define SERVAPI  __declspec(dllexport)
        #define SERVCALL
    #else
        #define SERVAPI
        #define SERVCALL __cdecl
    #endif
#else
    #define SERVAPI
    #define SERVCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_SERV_MAX_NUM_OF_SERV         (30)

#define JF_SERV_MAX_SERV_NAME_LEN       (24)

typedef enum
{
    JF_SERV_STATUS_UNKNOWN = 0,
    JF_SERV_STATUS_STOPPED,
    JF_SERV_STATUS_STARTING,
    JF_SERV_STATUS_RUNNING,
    JF_SERV_STATUS_STOPPING,
    JF_SERV_STATUS_ERROR,
    JF_SERV_STATUS_TERMINATED,
} jf_serv_status_t;

typedef enum
{
    JF_SERV_STARTUP_TYPE_UNKNOWN = 0,
    JF_SERV_STARTUP_TYPE_AUTOMATIC,
    JF_SERV_STARTUP_TYPE_MANUAL,
} jf_serv_startup_type_t;

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t jsi_strName[JF_SERV_MAX_SERV_NAME_LEN];
    u8 jsi_u8Status;
    u8 jsi_u8StartupType;
    u8 jsi_u8Reserved[14];
} jf_serv_info_t;

typedef struct
{
    u16 jsil_u16NumOfService;
    u16 jsil_u8Reserved[3];
    jf_serv_info_t jsil_jsiService[0];
} jf_serv_info_list_t;

typedef struct
{
    u8 jsip_u8Reserved[32];
} jf_serv_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

SERVAPI u32 SERVCALL jf_serv_init(jf_serv_init_param_t * pjsip);

SERVAPI u32 SERVCALL jf_serv_fini(void);

SERVAPI u32 SERVCALL jf_serv_getInfoList(jf_serv_info_list_t * pjsil);

SERVAPI u32 SERVCALL jf_serv_getInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi);

SERVAPI u32 SERVCALL jf_serv_stopServ(const olchar_t * pstrName);

SERVAPI u32 SERVCALL jf_serv_startServ(const olchar_t * pstrName);

SERVAPI u32 SERVCALL jf_serv_setServStartupType(const olchar_t * pstrName, const u8 u8StartupType);

SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStatus(const u8 u8Status);

SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStartupType(const u8 u8StartupType);

SERVAPI u32 SERVCALL jf_serv_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*JIUFENG_SERV_H*/

/*------------------------------------------------------------------------------------------------*/


