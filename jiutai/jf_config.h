/**
 *  @file jf_config.h
 *
 *  @brief Config header file which provides some functional routine for config.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_config library.
 *  -# All routines in this header file are synchronous, it will not return until the response is
 *   received or timeout.
 *
 *  @par Config Manager Daemon
 *  -# The daemon's name is "jf_configmgr", it's will read the setting file at default location
 *   "../config/configmgr.setting". The setting file can be changed by using option "-s".
 *  @code
 *  Run the daemon and read setting file at default location:
 *  jf_configmgr
 *  Run the daemon in background and read specified setting file:
 *  jf_configmgr -s setting-file
 *  Show the version information:
 *  jf_configmgr -V
 *  @endcode
 *
 *  @par Config Manager Setting File
 *  -# The setting are defined in the file.
 *  @code
 *  <configmgr>
 *    <name>zeus</name>
 *    <description>zeus service</description>
 *    <startupType>automatic</startupType>
 *    <cmdPath>olzeus</cmdPath>
 *    <cmdParam></cmdParam>
 *  </service>
 *  @endcode
 *
 *  @par Config Management Tool
 *  -# The tool's name is "jf_configctl". User can use the tool to list, start and stop config.
 *  -# The tool uses unix domain socket to communicate with daemon, so the daemon should be running
 *   when using the tool.
 *  @code
 *  List all config:
 *  jf_configctl -l
 *  List specified config:
 *  jf_configctl -l -n config-name
 *  Add config:
 *  jf_configctl -a -n config-name -v config-value
 *  Delete config:
 *  jf_configctl -d -n config-name -v config-value
 *  Show the version information:
 *  jf_configctl -V
 *  @endcode
 *
 */

#ifndef JIUFENG_CONFIG_H
#define JIUFENG_CONFIG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

#undef CONFIGAPI
#undef CONFIGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_CONFIG_DLL)
        #define CONFIGAPI  __declspec(dllexport)
        #define CONFIGCALL
    #else
        #define CONFIGAPI
        #define CONFIGCALL __cdecl
    #endif
#else
    #define CONFIGAPI
    #define CONFIGCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum config name length.
 */
#define JF_CONFIG_MAX_CONFIG_NAME_LEN              (512)

/** Maximum config value length.
 */
#define JF_CONFIG_MAX_CONFIG_VALUE_LEN             (512)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the config initialization parameter data type.
 */
typedef struct
{
    u8 jsip_u8Reserved[32];
} jf_config_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the config library.
 *
 *  @param pjsip [in] The parameter for initilizing the config library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CONFIGAPI u32 CONFIGCALL jf_config_init(jf_config_init_param_t * pjsip);

/** Finalize the config library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CONFIGAPI u32 CONFIGCALL jf_config_fini(void);

/** Get the config.
 *
 *  @param name [in] The name of config in string with NULL terminated.
 *  @param value [out] The buffer for the value.
 *  @param sValue [in] The size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CONFIGAPI u32 CONFIGCALL jf_config_get(const olchar_t * name, olchar_t * value, olsize_t sValue);

/** Set config.
 *
 *  @param name [in] The name of config in string with NULL terminated.
 *  @param value [in] The value of config in string with NULL terminated.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CONFIGAPI u32 CONFIGCALL jf_config_set(const olchar_t * name, const olchar_t * value);

/** Start transaction.
 *
 *  @return The error code.
 */
CONFIGAPI u32 CONFIGCALL jf_config_startTransaction(void);

/** Commit transaction.
 *
 *  @return The error code.
 */
CONFIGAPI u32 CONFIGCALL jf_config_commitTransaction(void);

/** Rollback transaction.
 *
 *  @return The error code.
 */
CONFIGAPI u32 CONFIGCALL jf_config_rollbackTransaction(void);

#endif /*JIUFENG_CONFIG_H*/

/*------------------------------------------------------------------------------------------------*/
