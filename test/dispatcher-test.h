/**
 *  @file dispatcher-test.h
 *
 *  @brief Header file defines the interface for testing dispatcher.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef JIUFENG_TEST_DISPATCHER_TEST_H
#define JIUFENG_TEST_DISPATCHER_TEST_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_messaging.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define BGAD_MSG_ID_ACTIVITY_INFO        (2000)
#define BGAD_MSG_ID_ACTIVITY_STATUS      (2001)
#define BGAD_MSG_ID_START_ACTIVITY       (2002)
#define BGAD_MSG_ID_STOP_ACTIVITY        (2003)

#define SYSCTLD_MSG_ID_SYSTEM_INFO       (3000)

/* --- data structures -------------------------------------------------------------------------- */

/*bgad message definition.*/

typedef struct
{
    u8 baimp_u8Payload[64];
} bgad_activity_info_msg_payload_t;

typedef struct
{
    jf_messaging_header_t baim_jmhHeader;
    bgad_activity_info_msg_payload_t baim_baimpPayload;
} bgad_activity_info_msg;

typedef struct
{
    u8 basmp_u8Payload[64];
} bgad_activity_status_msg_payload_t;

typedef struct
{
    jf_messaging_header_t basm_jmhHeader;
    bgad_activity_status_msg_payload_t basm_basmpPayload;
} bgad_activity_status_msg;

typedef struct
{
    u8 bsamp_u8Payload[64];
} bgad_start_activity_msg_payload_t;

typedef struct
{
    jf_messaging_header_t bsam_jmhHeader;
    bgad_start_activity_msg_payload_t bsam_bsampPayload;
} bgad_start_activity_msg;

typedef struct
{
    u8 bsamp_u8Payload[64];
} bgad_stop_activity_msg_payload_t;

typedef struct
{
    jf_messaging_header_t bsam_jmhHeader;
    bgad_start_activity_msg_payload_t bsam_bsampPayload;
} bgad_stop_activity_msg;

/*Sysctld message definition.*/

typedef struct
{
    u8 ssimp_u8Payload[64];
} sysctld_system_info_msg_payload_t;

typedef struct
{
    jf_messaging_header_t ssim_jmhHeader;
    sysctld_system_info_msg_payload_t ssim_ssimpPayload;

} sysctld_system_info_msg;

/* --- functional routines ---------------------------------------------------------------------- */


#endif /*JIUFENG_TEST_DISPATCHER_TEST_H*/

/*------------------------------------------------------------------------------------------------*/


