/**
 *  @file servconfig.h
 *
 *  @brief Dispatcher service config header file, provide some functional routine to read service
 *   config.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef DISPATCHER_SERV_CONFIG_H
#define DISPATCHER_SERV_CONFIG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_process.h"
#include "jf_limit.h"
#include "jf_messaging.h"
#include "jf_user.h"
#include "jf_queue.h"
#include "jf_hlisthead.h"
#include "jf_jiukun.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define MAX_DISPATCHER_SERV_VERSION_LEN        (8)

#define MAX_DISPATCHER_SERV_NAME_LEN           (24)

#define MAX_DISPATCHER_MESSAGING_NAME_LEN      (48)

#define MAX_DISPATCHER_MSG_DESC_LEN            (48)

/* --- data structures -------------------------------------------------------------------------- */
typedef struct
{
    /*data from configuration file*/
    /**Version.*/
    olchar_t dsc_strVersion[MAX_DISPATCHER_SERV_VERSION_LEN];
    /**Service name.*/
    olchar_t dsc_strName[MAX_DISPATCHER_SERV_NAME_LEN];

    uid_t dsc_uiUser;
    gid_t dsc_giGroup;

    olchar_t dsc_strMessagingIn[MAX_DISPATCHER_MESSAGING_NAME_LEN];
    olchar_t dsc_strMessagingOut[MAX_DISPATCHER_MESSAGING_NAME_LEN];

    u32 dsc_u32MaxNumMsg;
    u32 dsc_u32MaxMsgSize;

    u16 dsc_u16NumOfPublishedMsg;
    u16 dsc_u16NumOfSubscribedMsg;
    u16 dsc_u16Reserved[2];
    jf_queue_t dsc_jqPublishedMsg;
    jf_queue_t dsc_jqSubscribedMsg;
    
} dispatcher_serv_config_t;

typedef struct
{
    u32 dmc_u32MsgId;
    olchar_t dmc_strMsgDesc[MAX_DISPATCHER_MSG_DESC_LEN];

    u32 dmc_u32Reserved[4];

} dispatcher_msg_config_t;

typedef struct
{
    olchar_t * sdcdp_pstrConfigDir;

    jf_queue_t * sdcdp_pjqServConfig;
    u16 sdcdp_u16NumOfServConfig;
    u16 sdcdp_u16Reserved[3];

    jf_jiukun_cache_t * sdcdp_pjjcMsgConfig;

} scan_dispatcher_config_dir_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

u32 scanDispatcherConfigDir(scan_dispatcher_config_dir_param_t * pParam);


#endif /*DISPATCHER_SERV_CONFIG_H*/

/*------------------------------------------------------------------------------------------------*/


