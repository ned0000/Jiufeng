/**
 *  @file configmgrmsg.h
 *
 *  @brief Message definition, the message is used between config library and daemon.
 *
 *  @author Min Zhang
 *
 */

#ifndef CONFIG_MGR_MSG_H
#define CONFIG_MGR_MSG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** magic number CFMR (0x43 0x46 0x4D 0x52)
 */
#define CONFIG_MGR_MSG_MAGIC_NUMBER                (0x43464D52)

/** Maximum message size. MUST be larger than sizoef(config_mgr_set_config_req_t) +
 *  JF_CONFIG_MAX_CONFIG_NAME_LEN + JF_CONFIG_MAX_CONFIG_VALUE_LEN
 */
#define CONFIG_MGR_MAX_MSG_SIZE                    (1500)

/* --- data structures -------------------------------------------------------------------------- */

/** Config manager message header
 */
typedef struct 
{
    /**Unique identifier of the message.*/
    u8 cmmh_u8MsgId;
    u8 cmmh_u8Reserved[3];
    /**Sequence number.*/
    u32 cmmh_u32SeqNum;
    /**Magic number for validation.*/
    u32 cmmh_u32MagicNumber;
    /**Payload size.*/
    u32 cmmh_u32PayloadSize;
    /**Result code for response message.*/
    u32 cmmh_u32Result;
    /**Transaction ID.*/
    u32 cmmh_u32TransactionId;
    u32 cmmh_u32Reserved[2];
} config_mgr_msg_header_t;

typedef enum
{
    CMMI_UNKNOWN = 0,
    CMMI_GET_CONFIG_REQ,
    CMMI_GET_CONFIG_RESP,
    CMMI_SET_CONFIG_REQ,
    CMMI_SET_CONFIG_RESP,
    CMMI_START_TRANSACTION_REQ,
    CMMI_START_TRANSACTION_RESP,
    CMMI_COMMIT_TRANSACTION_REQ,
    CMMI_COMMIT_TRANSACTION_RESP,
    CMMI_ROLLBACK_TRANSACTION_REQ,
    CMMI_ROLLBACK_TRANSACTION_RESP,
    CMMI_MAX,
} config_mgr_msg_id_t;

typedef struct
{
    config_mgr_msg_header_t cmgcr_cmmhHeader;

    u16 cmgcr_u16NameLen;
    u16 cmgcr_u16Reserved[3];
    olchar_t cmgcr_strName[0];
} config_mgr_get_config_req_t;

typedef struct
{
    config_mgr_msg_header_t cmgcr_cmmhHeader;

    u16 cmgcr_u16ValueLen;
    u16 cmgcr_u16Reserved[3];
    olchar_t cmgcr_strValue[0];
} config_mgr_get_config_resp_t;

typedef struct
{
    config_mgr_msg_header_t cmscr_cmmhHeader;

    u16 cmscr_u16NameLen;
    u16 cmscr_u16ValueLen;
    u16 cmscr_u16Reserved[2];
    olchar_t cmscr_strName[0];
    olchar_t cmscr_strValue[0];
} config_mgr_set_config_req_t;

typedef struct
{
    config_mgr_msg_header_t cmscr_cmmhHeader;

} config_mgr_set_config_resp_t;

typedef struct
{
    config_mgr_msg_header_t cmstr_cmmhHeader;

} config_mgr_start_transaction_req_t;

typedef struct
{
    config_mgr_msg_header_t cmstr_cmmhHeader;

} config_mgr_start_transaction_resp_t;

typedef struct
{
    config_mgr_msg_header_t cmctr_cmmhHeader;

} config_mgr_commit_transaction_req_t;

typedef struct
{
    config_mgr_msg_header_t cmctr_cmmhHeader;

} config_mgr_commit_transaction_resp_t;

typedef struct
{
    config_mgr_msg_header_t cmrtr_cmmhHeader;

} config_mgr_rollback_transaction_req_t;

typedef struct
{
    config_mgr_msg_header_t cmrtr_cmmhHeader;

} config_mgr_rollback_transaction_resp_t;

/* --- functional routines ---------------------------------------------------------------------- */



#endif /*CONFIG_MGR_MSG_H*/

/*------------------------------------------------------------------------------------------------*/


