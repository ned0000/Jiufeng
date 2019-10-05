/**
 *  @file servmgmtcommon.h
 *
 *  @brief Service common header file
 *
 *  @author Min Zhang
 *
 */

#ifndef SERVMGMT_SERVMGMTCOMMON_H
#define SERVMGMT_SERVMGMTCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_sharedmemory.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define SERVMGMT_SERVER_ADDR  "/tmp/servmgmt_server"

/* --- data structures -------------------------------------------------------------------------- */

/** servmgmt message header
 */
typedef struct 
{
    /**unique identifier of the message*/    
    u8 smh_u8MsgId;
    /**message priority*/
    u8 smh_u8MsgPrio;
    u8 smh_u8Reserved[2];
    /**transaction identifier*/
    u32 smh_u32TransactionId;
    /**source identifier*/
    jf_process_id_t smh_jpiSourceId;
    /**destination identifier*/
    jf_process_id_t smh_jpiDestinationId;
} servmgmt_msg_header_t;

typedef enum
{
    SERVMGMT_MSG_ID_GET_UNKNOWN = 0,
    SERVMGMT_MSG_ID_GET_INFO,
    SERVMGMT_MSG_ID_START_SERV,
    SERVMGMT_MSG_ID_STOP_SERV,
    SERVMGMT_MSG_ID_SET_STARTUP_TYPE,
    SERVMGMT_MSG_ID_MAX,
} servmgmt_msg_id_t;

typedef struct
{
    servmgmt_msg_header_t smgi_smhHeader;

    u32 smgi_u32ShmLen;
    jf_sharedmemory_id_t smgi_jsiShmId[ JF_SHAREDMEMORY_ID_LEN];
} servmgmt_msg_get_info_t;

typedef struct
{
    servmgmt_msg_header_t smss_smhHeader;

    olchar_t smss_strName[JF_SERV_MAX_SERV_NAME_LEN];
} servmgmt_msg_start_serv_t;

typedef struct
{
    servmgmt_msg_header_t smss_smhHeader;

    olchar_t smss_strName[JF_SERV_MAX_SERV_NAME_LEN];
} servmgmt_msg_stop_serv_t;

typedef struct
{
    servmgmt_msg_header_t smsst_smhHeader;

    olchar_t smsst_strName[JF_SERV_MAX_SERV_NAME_LEN];
    u8 smsst_u8StartupType;
    u8 smsst_u8Reserved[7];
} servmgmt_msg_set_startup_type_t;

/* --- functional routines ---------------------------------------------------------------------- */

const olchar_t * getStringServStatus(u8 u8Status);

const olchar_t * getStringServStartupType(u8 u8StartupType);

u32 getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType);

#endif /*SERVMGMT_SERVMGMTCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


