/**
 *  @file dispatchercommon.h
 *
 *  @brief Header file for common definitions and routines in dispatcher.
 *
 *  @author Min Zhang
 *
 */

#ifndef DISPATCHER_DISPATCHERCOMMON_H
#define DISPATCHER_DISPATCHERCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_messaging.h"
#include "jf_hlisthead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** The directory containing the uds socket file.
 */
#define DISPATCHER_UDS_DIR                        "/tmp/jf_dispatcher"

/** Maximum connection for a service.
 */
#define DISPATCHER_MAX_CONN_IN_SERVICE_SERVER     (2)


/* --- data structures -------------------------------------------------------------------------- */

/** Define the messaging message data type.
 */
typedef jf_messaging_header_t  jf_messaging_msg_t;

/** Define the dispatcher message data type.
 */
typedef struct
{
    /**Reference number*/
    olint_t dm_nRef;
    u16 dm_u16Reserved[4];
    /**The message size*/
    olsize_t dm_sMsg;
    /**The start of the message*/
    u8 dm_u8Msg[0];
} dispatcher_msg_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the directory for the unix domain socket.
 */
u32 createUdsDir(void);

/*Functions for internal dispatcher message*/

/** Create dispatcher message.
 */
u32 createDispatcherMsg(dispatcher_msg_t ** ppMsg, u8 * pu8Msg, olsize_t sMsg);

/** Destroy dispatcher message.
 */
u32 freeDispatcherMsg(dispatcher_msg_t ** ppMsg);

/** Increase the reference of dispatcher message.
 */
void incDispatcherMsgRef(dispatcher_msg_t * pdm);

/** Decrease the reference of dispatcher message.
 */
void decDispatcherMsgRef(dispatcher_msg_t * pdm);

/** Get reference of dispatcher message.
 */
olint_t getDispatcherMsgRef(dispatcher_msg_t * pdm);

/** Get source ID of dispatcher message.
 */
u32 getDispatcherMsgSourceId(dispatcher_msg_t * pdm);

/** Get destination ID of dispatcher message.
 */
u32 getDispatcherMsgDestinationId(dispatcher_msg_t * pdm);

/** Get dispatcher message id.
 */
u16 getDispatcherMsgId(dispatcher_msg_t * pdm);

/** Get dispatcher message priority.
 */
u8 getDispatcherMsgPrio(dispatcher_msg_t * pdm);

/** Free dispatcher message.
 */
u32 fnFreeDispatcherMsg(void ** ppData);

/*Functions for messaging message*/

/** Initialize the messaging message header.
 *
 *  @return The error code.
 */
u32 initMessagingMsgHeader(u8 * pu8Msg, u16 u16MsgId, u8 u8MsgPrio, u32 u32PayloadSize);

/** Get messaging message size.
 *
 *  @note
 *  -# Full message size including header and payload.
 *
 *  @return The message size.
 */
olsize_t getMessagingMsgSize(u8 * pu8Msg);

/** Get messaging message priority.
 *
 *  @return The message priority.
 */
u8 getMessagingMsgPrio(u8 * pu8Msg, olsize_t sMsg);

/** Get messaging message ID.
 *
 *  @return The message ID.
 */
u16 getMessagingMsgId(u8 * pu8Msg, olsize_t sMsg);

/** Set messaging message ID.
 *
 *  @return The error code.
 */
u32 setMessagingMsgId(u8 * pu8Msg, u16 u16MsgId);

/** Set messaging message payload size.
 */
u32 setMessagingMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize);

/** Set messaging message source ID.
 *
 *  @return The error code.
 */
u32 setMessagingMsgSourceId(u8 * pu8Msg, u32 sourceId);

/** Get messaging message source ID.
 *
 *  @return The source ID.
 */
u32 getMessagingMsgSourceId(u8 * pu8Msg, olsize_t sMsg);

/** Set messaging message destination ID.
 *
 *  @return The error code.
 */
u32 setMessagingMsgDestinationId(u8 * pu8Msg, u32 destinationId);

/** Get messaging message destination ID.
 *
 *  @return The destination ID.
 */
u32 getMessagingMsgDestinationId(u8 * pu8Msg, olsize_t sMsg);

/** Get messaging message transaction ID.
 *
 *  @return The transaction ID.
 */
u32 getMessagingMsgTransactionId(u8 * pu8Msg, olsize_t sMsg);

/** Set messaging message transaction ID.
 *
 *  @return The error code.
 */
u32 setMessagingMsgTransactionId(u8 * pu8Msg, u32 transactionId);

/** Check if full message is received.
 *
 *  @return The error code.
 */
u32 isMessagingFullMsg(u8 * pu8Msg, olsize_t sMsg);


#endif /*DISPATCHER_DISPATCHERCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/
