/**
 *  @file serviceconfig.h
 *
 *  @brief Dispatcher service config header file, provide some functional routine to read service
 *   config.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef DISPATCHER_SERVICE_CONFIG_H
#define DISPATCHER_SERVICE_CONFIG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_process.h"
#include "jf_limit.h"
#include "jf_messaging.h"
#include "jf_user.h"
#include "jf_linklist.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define MAX_DISPATCHER_SERVICE_VERSION_LEN           (8)

#define MAX_DISPATCHER_SERVICE_NAME_LEN              (24)

#define MAX_DISPATCHER_MESSAGING_NAME_LEN            (48)

#define MAX_DISPATCHER_MSG_DESC_LEN                  (48)

/** Define the invalid service ID.
 */
#define DISPATCHER_INVALID_SERVICE_ID                (U32_MAX)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the service config data type.
 */
typedef struct
{
    /**The service config ID, the ID is unique for each service in this daemon scope.*/
    u16 dsc_u16ServiceConfigId;
    u16 dsc_u16Reserved[3];

    /*Data from configuration file.*/
    /**Version.*/
    olchar_t dsc_strVersion[MAX_DISPATCHER_SERVICE_VERSION_LEN];
    /**Service name.*/
    olchar_t dsc_strName[MAX_DISPATCHER_SERVICE_NAME_LEN];
    /**User ID who starts the service.*/
    uid_t dsc_uiUser;
    /**Group ID who starts the service.*/
    gid_t dsc_giGroup;
    /**Messaing in address of the service. For dispatcher, it's the address to send message.*/
    olchar_t dsc_strMessagingIn[MAX_DISPATCHER_MESSAGING_NAME_LEN];
    /**Messaing out address of the service. For dispatcher, it's the address to receive message.*/
    olchar_t dsc_strMessagingOut[MAX_DISPATCHER_MESSAGING_NAME_LEN];
    /**Maximum number of message in dispatcher message queue.*/
    u32 dsc_u32MaxNumMsg;
    /**Maximum message size in dispatcher message queue.*/
    u32 dsc_u32MaxMsgSize;
    /**Number of published message.*/
    u16 dsc_u16NumOfPublishedMsg;
    /**Number of subscribed message.*/
    u16 dsc_u16NumOfSubscribedMsg;
    u16 dsc_u16Reserved2[2];
    /**The linked list for published message config (dispatcher_msg_config_t).*/
    jf_linklist_t dsc_jlPublishedMsg;
    /**The linked list for subscribed message config (dispatcher_msg_config_t).*/
    jf_linklist_t dsc_jlSubscribedMsg;

    /*Data for running time.*/
    /**The service ID of the service connected to the dispatcher.*/
    u32 dsc_u32ServiceId;
} dispatcher_service_config_t;

typedef struct
{
    u16 dmc_u16MsgId;
    u16 dmc_u16Reserved[3];
    olchar_t dmc_strMsgDesc[MAX_DISPATCHER_MSG_DESC_LEN];

    u32 dmc_u32Reserved[4];

} dispatcher_msg_config_t;

typedef struct
{
    olchar_t * sdcdp_pstrConfigDir;

    jf_linklist_t * sdcdp_pjlServiceConfig;
    u16 sdcdp_u16NumOfServiceConfig;
    u16 sdcdp_u16Reserved[3];

} scan_dispatcher_config_dir_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

u32 scanDispatcherConfigDir(scan_dispatcher_config_dir_param_t * pParam);

u32 destroyDispatcherServiceConfigList(jf_linklist_t * pjlServiceConfig);

#endif /*DISPATCHER_SERVICE_CONFIG_H*/

/*------------------------------------------------------------------------------------------------*/


