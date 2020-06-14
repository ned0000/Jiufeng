/**
 *  @file jf_messaging.h
 *
 *  @brief Header file defines the interface for messaging library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_messaging library.
 *  -# Link with jf_dispatcher_xfer library for message transfer.
 *
 *  @par Rules for Messaging
 *  -# The messaging library creates thread to send message so application will not block for
 *   sending message.
 *  -# The messaging library creates 2 threads for incoming message. One receives message and
 *   another processes message.
 *  -# Service main thread should not quit after starting messaging library.
 *  -# When the message queue is full, the oldest message is removed from queue and new message is
 *   added only when the priority of new message is not lower than all messages in queue. Otherwise
 *   an error is returned. Eg. the message in queue has middle priority, then high and middle
 *   message can be added to queue, low priority cannot be added.
 */

#ifndef JIUFENG_MESSAGING_H
#define JIUFENG_MESSAGING_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

#undef MESSAGINGAPI
#undef MESSAGINGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_MESSAGING_DLL)
        #define MESSAGINGAPI  __declspec(dllexport)
        #define MESSAGINGCALL
    #else
        #define MESSAGINGAPI
        #define MESSAGINGCALL __cdecl
    #endif
#else
    #define MESSAGINGAPI
    #define MESSAGINGCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum message size.
 */
#define JF_MESSAGING_MAX_MSG_SIZE               (128 * 1024)

/* --- data structures -------------------------------------------------------------------------- */

/** The callback function to process the incoming message.
 */
typedef u32 (* jf_messaging_fnProcessMsg_t)(u8 * pu8Msg, olsize_t sMsg);

/** The parameter for initializing messaging library.
 */
typedef struct
{
    /**Callback function to process message.*/
    jf_messaging_fnProcessMsg_t jmip_fnProcessMsg;
    /**The address to receive message from.*/
    olchar_t * jmip_pstrMessagingIn;
    /**The address to send message to.*/
    olchar_t * jmip_pstrMessagingOut;
    /**The name of the application.*/
    olchar_t * jmip_pstrName;
    /**Maximum message size.*/
    olsize_t jmip_sMaxMsg;
    /**Maximum number of message.*/
    u32 jmip_u32MaxNumMsg;
    u8 jmip_u8Reserved[32];
} jf_messaging_init_param_t;

/** Define the message priority level.
 */
typedef enum
{
    /**Low priority.*/
    JF_MESSAGING_PRIO_LOW = 0,
    /**Medium priority.*/
    JF_MESSAGING_PRIO_MID,
    /**High priority.*/
    JF_MESSAGING_PRIO_HIGH,
} jf_messaging_prio_t;

/** Define the messaging header data type.
 */
typedef struct
{
    /**Unique identifier of the message.*/    
    u16 jmh_u16MsgId;
    /**Message priority.*/
    u8 jmh_u8MsgPrio;
    u8 jmh_u8Reserved[5];
    /**Transaction identifier.*/
    u32 jmh_u32TransactionId;
    /**Payload size.*/
    u32 jmh_u32PayloadSize;
    /**Sender's identifier, suggest using process id.*/
    u32 jmh_u32SourceId;
    /**Receiver's identifier, suggest using process id.*/
    u32 jmh_u32DestinationId;
} jf_messaging_header_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the messaging library.
 *
 *  @param pjrip [in] The parameter for initilizing the messaging library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_init(jf_messaging_init_param_t * pjrip);

/** Finalize the messaging library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_fini(void);

/** Start messaging.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_THREAD Failed to create thread to start messaging.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_start(void);

/** Stop messaging.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_stop(void);

/** Send message.
 *
 *  @param pu8Msg [in] The message to send.
 *  @param sMsg [in] The message size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_sendMsg(u8 * pu8Msg, olsize_t sMsg);

/** Send internal message.
 *
 *  @note
 *  -# The message will not send to dispatcher but add to incoming message queue.
 *
 *  @param pu8Msg [in] The message to send.
 *  @param sMsg [in] The message size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_sendInternalMsg(u8 * pu8Msg, olsize_t sMsg);

/** Initialize message header.
 *
 *  @note
 *  -# The routine will set the source id of the header with current process id.
 *
 *  @param pu8Msg [in] The message to initialize.
 *  @param u16MsgId [in] The message ID.
 *  @param u8MsgPrio [in] The message priority.
 *  @param u32PayloadSize [in] The message payload size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_initMsgHeader(
    u8 * pu8Msg, u16 u16MsgId, u8 u8MsgPrio, u32 u32PayloadSize);

/** Get message ID.
 *
 *  @param pu8Msg [in] The message.
 *  @param sMsg [in] The message size.
 *
 *  @return The message ID.
 */
MESSAGINGAPI u16 MESSAGINGCALL jf_messaging_getMsgId(u8 * pu8Msg, olsize_t sMsg);

/** Set message payload size.
 *
 *  @param pu8Msg [in] The message to set.
 *  @param u32PayloadSize [in] The message payload size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_setMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize);

/** Set message destination ID.
 *
 *  @param pu8Msg [in] The message to set.
 *  @param destinationId [in] The message destination ID.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_setMsgDestinationId(u8 * pu8Msg, u32 destinationId);

/** Set message transaction ID.
 *
 *  @param pu8Msg [in] The message to set.
 *  @param transactionId [in] The message transaction ID.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_setMsgTransactionId(u8 * pu8Msg, u32 transactionId);

/** Get message transaction ID.
 *
 *  @param pu8Msg [in] The message.
 *  @param sMsg [in] The message size.
 *
 *  @return The message transaction ID.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_getMsgTransactionId(u8 * pu8Msg, olsize_t sMsg);

/** Check if full message is received.
 *
 *  @param pu8Msg [in] The message.
 *  @param sMsg [in] The message size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Full message is received.
 *  @retval JF_ERR_INCOMPLETE_DATA Partial message is received.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_isFullMsg(u8 * pu8Msg, olsize_t sMsg);

/** Get messaging message size.
 *
 *  @note
 *  -# Full message size including header and payload.
 *
 *  @return The message size.
 */
MESSAGINGAPI olsize_t MESSAGINGCALL jf_messaging_getMsgSize(u8 * pu8Msg);

#endif /*JIUFENG_MESSAGING_H*/

/*------------------------------------------------------------------------------------------------*/
