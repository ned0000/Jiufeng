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
#define DISPATCHER_UDS_DIR  "/tmp/jf_dispatcher"

typedef jf_messaging_header_t  jf_messaging_msg_t;

/** The internal message id, service active.
 */
#define DISPATCHER_MSG_ID_SERV_ACTIVE (JF_MESSAGING_RESERVED_MSG_ID + 0x10)

/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    jf_messaging_header_t dsam_jmhHeader;
} dispatcher_serv_active_msg;

/** Define the dispatcher message data type.
 */
typedef struct
{
    /**Reference number*/
    olint_t dm_nRef;
    u16 dm_u16Reserved[3];
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

/** Check if the message is reserved message.
 */
boolean_t isReservedDispatcherMsg(dispatcher_msg_t * pdm);

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
pid_t getDispatcherMsgSourceId(dispatcher_msg_t * pdm);

/** Get destination ID of dispatcher message.
 */
pid_t getDispatcherMsgDestinationId(dispatcher_msg_t * pdm);

/** Get dispatcher message id.
 */
u32 getDispatcherMsgId(dispatcher_msg_t * pdm);

/** Free dispatcher message.
 */
u32 fnFreeDispatcherMsg(void ** ppData);

/*Functions for messaging message*/

/** Initialize the messaging message header.
 */
u32 initMessagingMsgHeader(u8 * pu8Msg, u32 u32MsgId, u8 u8MsgPrio, u32 u32PayloadSize);

/** Get messaging message size.
 */
olsize_t getMessagingSize(u8 * pu8Msg);

/** Get messaging message ID.
 */
u32 getMessagingMsgId(u8 * pu8Msg, olsize_t sMsg);

/** Set messaging message ID.
 */
u32 setMessagingMsgId(u8 * pu8Msg, u32 u32MsgId);

/** Set messaging message payload size.
 */
u32 setMessagingMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize);

/** Set messaging message source ID.
 */
u32 setMessagingMsgSourceId(u8 * pu8Msg, pid_t sourceId);

/** Get messaging message source ID.
 */
pid_t getMessagingMsgSourceId(u8 * pu8Msg, olsize_t sMsg);

/** Set messaging message destination ID.
 */
u32 setMessagingMsgDestinationId(u8 * pu8Msg, pid_t destinationId);

/** Get messaging message destination ID.
 */
pid_t getMessagingMsgDestinationId(u8 * pu8Msg, olsize_t sMsg);

#endif /*DISPATCHER_DISPATCHERCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


