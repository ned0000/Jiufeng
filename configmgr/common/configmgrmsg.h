/**
 *  @file configmgrmsg.h
 *
 *  @brief Header file for message definition, the message is used between config library and
 *   daemon.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Config name and value are string which are null-terminated.
 */

#ifndef CONFIG_MGR_MSG_H
#define CONFIG_MGR_MSG_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Magic number for message, CFMR (0x43 0x46 0x4D 0x52)
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

/** Define the config manager message ID.
 */
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

/** Define the get config request.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmgcr_cmmhHeader;

    /**Length of config name.*/
    u16 cmgcr_u16NameLen;
    u16 cmgcr_u16Reserved[3];
    /**Config name.*/
    olchar_t cmgcr_strName[0];
} config_mgr_get_config_req_t;

/** Define the get config response.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmgcr_cmmhHeader;

    /**Length of config value.*/
    u16 cmgcr_u16ValueLen;
    u16 cmgcr_u16Reserved[3];
    /**Config value.*/
    olchar_t cmgcr_strValue[0];
} config_mgr_get_config_resp_t;

/** Define the set config request.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmscr_cmmhHeader;

    /**Length of config name.*/
    u16 cmscr_u16NameLen;
    /**Length of config value.*/
    u16 cmscr_u16ValueLen;
    u16 cmscr_u16Reserved[2];
    /**Config name.*/
    olchar_t cmscr_strName[0];
    /**Config value.*/
    olchar_t cmscr_strValue[0];
} config_mgr_set_config_req_t;

/** Define the set config response.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmscr_cmmhHeader;

} config_mgr_set_config_resp_t;

/** Define the start transaction request.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmstr_cmmhHeader;

} config_mgr_start_transaction_req_t;

/** Define the start transaction response.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmstr_cmmhHeader;

} config_mgr_start_transaction_resp_t;

/** Define the commit transaction request.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmctr_cmmhHeader;

} config_mgr_commit_transaction_req_t;

/** Define the commit transaction response.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmctr_cmmhHeader;

} config_mgr_commit_transaction_resp_t;

/** Define the rollback transaction request.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmrtr_cmmhHeader;

} config_mgr_rollback_transaction_req_t;

/** Define the rollback transaction response.
 */
typedef struct
{
    /**Message header.*/
    config_mgr_msg_header_t cmrtr_cmmhHeader;

} config_mgr_rollback_transaction_resp_t;

/* --- functional routines ---------------------------------------------------------------------- */



#endif /*CONFIG_MGR_MSG_H*/

/*------------------------------------------------------------------------------------------------*/
