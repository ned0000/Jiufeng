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
    u16 dm_u16Ref;
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

u32 createDispatcherMsg(dispatcher_msg_t ** ppMsg, u8 * pu8Msg, olsize_t sMsg);

void incDispatcherMsgRef(dispatcher_msg_t * pdm);

void decDispatcherMsgRef(dispatcher_msg_t * pdm);

u8 getDispatcherMsgRef(dispatcher_msg_t * pdm);

u32 freeDispatcherMsg(dispatcher_msg_t ** ppMsg);

u32 fnFreeDispatcherMsg(void ** ppData);

/*Functions for messaging message*/

u32 initMessagingMsgHeader(u8 * pu8Msg, u32 u32MsgId, u8 u8MsgPrio, u32 u32PayloadSize);

olsize_t getMessagingSize(u8 * pu8Msg);

u32 getMessagingMsgId(u8 * pu8Msg, olsize_t sMsg);

u32 setMessagingMsgId(u8 * pu8Msg, u32 u32MsgId);

u32 setMessagingMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize);

u32 setMessagingMsgSourceId(u8 * pu8Msg, pid_t sourceId);

u32 setMessagingMsgDestinationId(u8 * pu8Msg, pid_t destinationId);

#endif /*DISPATCHER_DISPATCHERCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/


