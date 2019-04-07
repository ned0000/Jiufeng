/**
 *  @file uuid/common.h
 *
 *  @brief common header file for uuid library, provide some common definition
 *   and data structure
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef UUID_COMMON_H
#define UUID_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "ifmgmt.h"

/* --- constant definitions ------------------------------------------------ */


typedef s8   uuid_int8_t;
typedef u8   uuid_uint8_t;
typedef s16  uuid_int16_t;
typedef u16  uuid_uint16_t;
typedef s32  uuid_int32_t;
typedef u32  uuid_uint32_t;

/* --- data structures ----------------------------------------------------- */

/** UUID binary representation according to UUID standards
 */
typedef struct
{
    /** bits 0-31: time field */
    uuid_uint32_t uo_u32TimeLow;
    /** bits 32-47: time field */
    uuid_uint16_t uo_u16TimeMid;
    /** bits 48-59: time field plus 4 bit version */
    uuid_uint16_t uo_u16TimeHiAndVersion;
    /** bits 8-13: clock sequence field plus 2 bit variant */
    uuid_uint8_t uo_u8ClockSeqHiAndReserved;
    /** bits 0-7: clock sequence field */
    uuid_uint8_t uo_u8ClockSeqLow;
    /** bits 0-47: node MAC address */
    uuid_uint8_t uo_u8Node[JF_LIMIT_MAC_LEN];
} uuid_obj_t;

/* --- functional routines ------------------------------------------------- */


#endif /*UUID_COMMON_H*/

/*---------------------------------------------------------------------------*/


