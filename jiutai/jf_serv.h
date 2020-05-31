/**
 *  @file jf_serv.h
 *
 *  @brief Service header file which provides some functional routine for service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_serv library.
 *  -# All service control routines are synchronous, it will not return until the response is
 *   received or timeout.
 *
 *  @par Service Management Daemon
 *  -# The daemon's name is "jf_dongyuan", it's will read the setting file at default location
 *   "../config/servmgmt.setting". The setting file can be changed by using option "-s".
 *  -# By default, the daemon is running in background.
 *  @code
 *  Run the daemon in background and read setting file at default location:
 *  jf_dongyuan
 *  Run the daemon in background and read specified setting file:
 *  jf_dongyuan -s setting-file
 *  Run the daemon in foreground:
 *  jf_dongyuan -f
 *  Show the version information:
 *  jf_dongyuan -V
 *  @endcode
 *
 *  @par Service Management Setting File
 *  -# The service which is supposed to be managed by daemon should provide following configuration
 *   in setting file.
 *  -# The startupType is how to start the service. The service management daemon will start the
 *   service if the startup type is "automatic". The daemon will not start the service if the
 *   startup type is "manual".
 *  -# If the service is terminated abnormally, the daemon will restart the service. The maximum
 *   retry count is 3 by default. Daemon will not restart the service after 3 times. User can change
 *   the configuration in setting file, the configuration name is "maxFailureRetryCount".
 *  -# The "cmdPath" is the path to the executable file of the service. The "cmdParam" is the
 *   parameter for starting the service.
 *  @code
 *  <service>
 *    <name>zeus</name>
 *    <description>zeus service</description>
 *    <startupType>automatic</startupType>
 *    <cmdPath>olzeus</cmdPath>
 *    <cmdParam></cmdParam>
 *  </service>
 *  @endcode
 *
 *  @par Service Management Tool
 *  -# The tool's name is "jf_servctl". User can use the tool to list, start and stop service, and
 *   also change the setting of service.
 *  -# The tool uses unix domain socket to communicate with daemon, so the daemon should be running
 *   when using the tool.
 *  @code
 *  List all service:
 *  jf_servctl -l
 *  List specified service:
 *  jf_servctl -l -n service-name
 *  Start a service:
 *  jf_servctl -t -n service-name
 *  Stop a service:
 *  jf_servctl -s -n service-name
 *  Change the startup type of a service:
 *  jf_servctl -u manual -n service-name
 *  Show the version information:
 *  jf_servctl -V
 *  @endcode
 *
 *  @par Rules for Service
 *  -# Service may not care about the background and daemonize, as all services are started by
 *   service management daemon.
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
 *
 *  @param pjsip [in] The parameter for initilizing the service library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_init(jf_serv_init_param_t * pjsip);

/** Finalize the service library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_fini(void);

/** Get the service information list.
 *
 *  @param pjsil [out] The service information list.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_getInfoList(jf_serv_info_list_t * pjsil);

/** Get information of a service.
 *
 *  @param pstrName [in] The name of the service.
 *  @param pjsi [out] The service information.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_getInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi);

/** Stop a service.
 *
 *  @param pstrName [in] The name of the service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_stopServ(const olchar_t * pstrName);

/** Start a service.
 *
 *  @param pstrName [in] The name of the service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_startServ(const olchar_t * pstrName);

/** Change the startup type of a service.
 *
 *  @param pstrName [in] The name of the service.
 *  @param u8StartupType [in] The startup type of the service, refer to jf_serv_startup_type_t.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
SERVAPI u32 SERVCALL jf_serv_setServStartupType(const olchar_t * pstrName, const u8 u8StartupType);

/** Get the service status in string format.
 *
 *  @param u8Status [in] The service status, refer to jf_serv_status_t.
 *
 *  @return The status string.
 */
SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStatus(const u8 u8Status);

/** Get the service startup type in string format.
 *
 *  @param u8StartupType [in] The service startup type, refer to jf_serv_startup_type_t.
 *
 *  @return The startup type string.
 */
SERVAPI const olchar_t * SERVCALL jf_serv_getStringServStartupType(const u8 u8StartupType);

/** Get the service startup type from string.
 *
 *  @param pstrType [in] The service startup type string.
 *  @param pu8StartupType [in] The service startup type parsed from string.
 *
 *  @return The startup type, one of value in jf_serv_startup_type_t.
 */
SERVAPI u32 SERVCALL jf_serv_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*JIUFENG_SERV_H*/

/*------------------------------------------------------------------------------------------------*/


