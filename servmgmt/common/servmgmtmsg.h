/**
 *  @file servmgmtmsg.h
 *
 *  @brief Message definition, the message is used between serv library and dongyuan
 *
 *  @author Min Zhang
 *
 */

#ifndef SERVMGMT_SERVMGMTMSG_H
#define SERVMGMT_SERVMGMTMSG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_sharedmemory.h"
#include "jf_serv.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Magic number, SEMG (0x53 0x45 0x4D 0x47)
 */
#define SERVMGMT_MSG_MAGIC_NUMBER  (0x53454D47)

/* --- data structures -------------------------------------------------------------------------- */

/** service management message header
 */
typedef struct 
{
    /**Unique identifier of the message.*/
    u8 smh_u8MsgId;
    u8 smh_u8Reserved[3];
    /**Sequence number.*/
    u32 smh_u32SeqNum;
    /**Magic number for validation.*/
    u32 smh_u32MagicNumber;
    /**Payload size.*/
    u32 smh_u32PayloadSize;
    /**Result code for response message.*/
    u32 smh_u32Result;
    u32 smh_u32Reserved[3];
} servmgmt_msg_header_t;

typedef enum
{
    SERVMGMT_MSG_ID_UNKNOWN = 0,
    SERVMGMT_MSG_ID_GET_INFO_REQ,
    SERVMGMT_MSG_ID_GET_INFO_RESP,
    SERVMGMT_MSG_ID_GET_INFO_LIST_REQ,
    SERVMGMT_MSG_ID_GET_INFO_LIST_RESP,
    SERVMGMT_MSG_ID_START_SERV_REQ,
    SERVMGMT_MSG_ID_START_SERV_RESP,
    SERVMGMT_MSG_ID_STOP_SERV_REQ,
    SERVMGMT_MSG_ID_STOP_SERV_RESP,
    SERVMGMT_MSG_ID_SET_STARTUP_TYPE_REQ,
    SERVMGMT_MSG_ID_SET_STARTUP_TYPE_RESP,
    SERVMGMT_MSG_ID_MAX,
} servmgmt_msg_id_t;

typedef struct
{
    servmgmt_msg_header_t sgilr_smhHeader;

} servmgmt_get_info_list_req_t;

typedef struct
{
    servmgmt_msg_header_t sgilr_smhHeader;

    jf_serv_info_list_t sgilr_jsilList;
} servmgmt_get_info_list_resp_t;

typedef struct
{
    servmgmt_msg_header_t sgir_smhHeader;

    olchar_t sgir_strName[JF_SERV_MAX_SERV_NAME_LEN];
} servmgmt_get_info_req_t;

typedef struct
{
    servmgmt_msg_header_t sgir_smhHeader;

    jf_serv_info_t sgir_jsiInfo;
} servmgmt_get_info_resp_t;

typedef struct
{
    servmgmt_msg_header_t sssr_smhHeader;

    olchar_t sssr_strName[JF_SERV_MAX_SERV_NAME_LEN];
} servmgmt_start_serv_req_t;

typedef struct
{
    servmgmt_msg_header_t sssr_smhHeader;
} servmgmt_start_serv_resp_t;

typedef struct
{
    servmgmt_msg_header_t sssr_smhHeader;

    olchar_t sssr_strName[JF_SERV_MAX_SERV_NAME_LEN];
} servmgmt_stop_serv_req_t;

typedef struct
{
    servmgmt_msg_header_t sssr_smhHeader;

} servmgmt_stop_serv_resp_t;

typedef struct
{
    servmgmt_msg_header_t ssstr_smhHeader;

    olchar_t ssstr_strName[JF_SERV_MAX_SERV_NAME_LEN];
    u8 ssstr_u8StartupType;
    u8 ssstr_u8Reserved[7];
} servmgmt_set_startup_type_req_t;

typedef struct
{
    servmgmt_msg_header_t ssstr_smhHeader;

} servmgmt_set_startup_type_resp_t;

/* --- functional routines ---------------------------------------------------------------------- */



#endif /*SERVMGMT_SERVMGMTMSG_H*/

/*------------------------------------------------------------------------------------------------*/


