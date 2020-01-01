/**
 *  @file jf_messaging.h
 *
 *  @brief Header file defines the interface for messaging library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_messaging library.
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



/* --- data structures -------------------------------------------------------------------------- */

/** The parameter for initializing messaging library.
 */
typedef struct
{
    u8 jmip_u8Reserved[32];
} jf_messaging_init_param_t;

/** Define the message priority level.
 */
typedef enum
{
    JF_MESSAGING_PRIO_LOW = 0,
    JF_MESSAGING_PRIO_MID,
    JF_MESSAGING_PRIO_HIGH,
} jf_messaging_prio_t;

/** Define the messaging header data type.
 */
typedef struct
{
    /**Unique identifier of the message.*/    
    u32 jmh_u32MsgId;
    /**Message priority.*/
    u8 jmh_u8MsgPrio;
    u8 jmh_u8Reserved[3];
    /**Transaction identifier.*/
    u32 jmh_u32TransactionId;
    /**Payload size.*/
    u32 jmh_u32PayloadSize;
    /**Sender's identifier, it's the process id.*/
    pid_t jmh_piSourceId;
    /**Receiver's identifier, it's the process id.*/
    pid_t jmh_piDestinationId;
} jf_messaging_header_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initializing the messaging library.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_init(jf_messaging_init_param_t * pjrip);

/** Finalizing the messaging library.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_fini(void);

/** Send message.
 */
MESSAGINGAPI u32 MESSAGINGCALL jf_messaging_sendMsg(u8 * pu8Msg, olsize_t sMsg);

#endif /*JIUFENG_MESSAGING_H*/

/*------------------------------------------------------------------------------------------------*/


