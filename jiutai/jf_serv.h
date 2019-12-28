/**
 *  @file jf_serv.h
 *
 *  @brief Service header file which provides some functional routine for service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_serv library.
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

/** Maximum number of service the service supported.
 */
#define JF_SERV_MAX_NUM_OF_SERV         (50)

/** Maximum service name length.
 */
#define JF_SERV_MAX_SERV_NAME_LEN       (24)

/** Define the service status data type.
 */
typedef enum
{
    /**Unknown status.*/
    JF_SERV_STATUS_UNKNOWN = 0,
    /**Service is stopped.*/
    JF_SERV_STATUS_STOPPED,
    /**Service is starting but not fully started.*/
    JF_SERV_STATUS_STARTING,
    /**Service is started.*/
    JF_SERV_STATUS_RUNNING,
    /**Service is stopping but not fully stopped.*/
    JF_SERV_STATUS_STOPPING,
    /**Service is error.*/
    JF_SERV_STATUS_ERROR,
    /**Service is stopped.*/
    JF_SERV_STATUS_TERMINATED,
} jf_serv_status_t;

/** Define the service startup type data type.
 */
typedef enum
{
    /**Unknown startup type.*/
    JF_SERV_STARTUP_TYPE_UNKNOWN = 0,
    /**Automatic startup type, the service is started automaticlly.*/
    JF_SERV_STARTUP_TYPE_AUTOMATIC,
    /**Manual startup type, the service is not started automaticlly.*/
    JF_SERV_STARTUP_TYPE_MANUAL,
} jf_serv_startup_type_t;

/* --- data structures -------------------------------------------------------------------------- */

/** Define the service information data type.
 */
typedef struct
{
    /**The name of the service.*/
    olchar_t jsi_strName[JF_SERV_MAX_SERV_NAME_LEN];
    /**The status of the service, refer to jf_serv_status_t.*/
    u8 jsi_u8Status;
    /**The startup type of the service.*/
    u8 jsi_u8StartupType;
    u8 jsi_u8Reserved[14];
} jf_serv_info_t;

/** Define the service information list data type.
 */
typedef struct
{
    /**Number of service in the list.*/
    u16 jsil_u16NumOfService;
    u16 jsil_u8Reserved[3];
    /**The service list.*/
    jf_serv_info_t jsil_jsiService[0];
} jf_serv_info_list_t;

/** Define the service initialization parameter data type.
 */
typedef struct
{
    u8 jsip_u8Reserved[32];
} jf_serv_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the service library.
 */
SERVAPI u32 SERVCALL jf_serv_init(jf_serv_init_param_t * pjsip);

/** Finalize the service library.
 */
SERVAPI u32 SERVCALL jf_serv_fini(void);

/** Get the service information list.
 */
SERVAPI u32 SERVCALL jf_serv_getInfoList(jf_serv_info_list_t * pjsil);

/** Get information of a service.
 */
SERVAPI u32 SERVCALL jf_serv_getInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi);

/** Stop a service.
 */
SERVAPI u32 SERVCALL jf_serv_stopServ(const olchar_t * pstrName);

/** Start a service.
 */
SERVAPI u32 SERVCALL jf_serv_startServ(const olchar_t * pstrName);

/** Change the startup type of a service.
 */
SERVAPI u32 SERVCALL jf_serv_setServStartupType(const olchar_t * pstrName, const u8 u8StartupType);

/** Get the service status in string format.
 */
SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStatus(const u8 u8Status);

/** Get the service startup type in string format.
 */
SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStartupType(const u8 u8StartupType);

/** Get the service startup type from string.
 */
SERVAPI u32 SERVCALL jf_serv_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*JIUFENG_SERV_H*/

/*------------------------------------------------------------------------------------------------*/


