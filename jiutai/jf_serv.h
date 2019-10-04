/**
 *  @file jf_serv.h
 *
 *  @brief Service header file, provide some functional routine for service
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_serv library
 *  @note Service management library will create the setting and status files.
 *   The location of those files are in current working directory. The user
 *   should set the working directory accordingly before using this library.
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

#define JF_SERV_MAX_NUM_OF_SERV         (100)

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
    JF_SERV_STARTUPTYPE_UNKNOWN = 0,
    JF_SERV_STARTUPTYPE_AUTOMATIC,
    JF_SERV_STARTUPTYPE_MANUAL,
} jf_serv_startuptype_t;

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t jsi_strName[JF_SERV_MAX_SERV_NAME_LEN];
    u8 jsi_u8Status;
    u8 jsi_u8StartupType;
    u8 jsi_u8Reserved[22];
} jf_serv_info_t;

typedef struct
{
    u16 js_u16MaxService;
    u16 js_u16NumOfService;
    u16 js_u8Reserved[2];
    /*variable length for the service info array*/
    jf_serv_info_t js_jsiService[1];
} jf_serv_t;

typedef struct
{
    u8 jsip_u8Reserved[32];
} jf_serv_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

SERVAPI u32 SERVCALL jf_serv_init(jf_serv_init_param_t * pjsip);

SERVAPI u32 SERVCALL jf_serv_fini(void);

SERVAPI u32 SERVCALL jf_serv_getInfo(jf_serv_t * pjs);

SERVAPI u32 SERVCALL jf_serv_stopServ(olchar_t * pstrName);

SERVAPI u32 SERVCALL jf_serv_startServ(olchar_t * pstrName);

SERVAPI u32 SERVCALL jf_serv_setServStartupType(olchar_t * pstrName, u8 u8StartupType);

SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStatus(u8 u8Status);

SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStartupType(u8 u8StartupType);

SERVAPI u32 SERVCALL jf_serv_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*JIUFENG_SERV_H*/

/*------------------------------------------------------------------------------------------------*/

