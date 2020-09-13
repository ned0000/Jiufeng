/**
 *  @file logger/log2servermsg.h
 *
 *  @brief Header file of message definition for logging to server.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_LOGGER_LOG2SERVER_MSG_H
#define JIUFENG_LOGGER_LOG2SERVER_MSG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_logger.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the message signature, LS.
 */
#define LOG_2_SERVER_MSG_SIGNATURE                   (0x4C53)

/** Maximum message source length, banner format is "[caller-name:thread-id]",
 *  length: JF_LOGGER_MAX_CALLER_NAME_LEN + thread id
 */
#define LOG_2_SERVER_MAX_MSG_SOURCE_SIZE             (64)

/** Maximum message size.
 */
#define LOG_2_SERVER_MAX_MSG_SIZE                    (2 * JF_LOGGER_MAX_MSG_SIZE)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the log to server message ID.
 */
typedef enum
{
    /**Unknown message ID.*/
    L2SMI_UNKNOWN = 0,
    /**Save log service message ID.*/
    L2SMI_SAVE_LOG_SVC,
    /**Maximum message ID.*/
    L2SMI_MAX,
} log_2_server_msg_id_t;

/** Define the message header data type used between logger library and log server.
 */
typedef struct
{
    /**Unique identifier of the message.*/
    u8 l2smh_u8MsgId;
    u8 l2smh_u8Reserved[3];
    /**Message signature, must be LOG_2_SERVER_MSG_SIGNATURE.*/
    u16 l2smh_u16Signature;
    /**Size of the payload.*/
    u16 l2smh_u16PayloadSize;
    /**Sequence number.*/
    u32 l2smh_u32SeqNum;
    /**Result code for response message.*/
    u32 l2smh_u32Result;
} log_2_server_msg_header_t;

/** Define the message to save log, no response for this message.
 */
typedef struct
{
    /**Message header.*/
    log_2_server_msg_header_t l2ssls_l2smhHeader;
    /**Time in micro-second.*/
    u64 l2ssls_u64Time;
    /**Message source with null-terminated.*/
    olchar_t l2ssls_strSource[LOG_2_SERVER_MAX_MSG_SOURCE_SIZE];
    /**Log with null-terminated.*/
    olchar_t l2ssls_strLog[JF_LOGGER_MAX_MSG_SIZE];
} log_2_server_save_log_svc_t;

/* --- functional routines ---------------------------------------------------------------------- */


#endif /*JIUFENG_LOGGER_LOG2SERVER_MSG_H*/

/*------------------------------------------------------------------------------------------------*/
