/**
 *  @file crc32c.h
 *
 *  @brief crc32c header file
 *
 *  @author Min Zhang
 *
 *  @note
 *   Implement CRC32C: 32 bit CRC of the polynomial represented by 0x11EDC6F41.
 *   cyclic redundancy check (CRC) is a type of function that takes as
 *   input a data stream of any length, and produces as output a value
 *   of a certain space, commonly a 32-bit integer.
 *
 */

#ifndef JIUTAI_CRC32C_H
#define JIUTAI_CRC32C_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
/*init the result before doing crc*/
#define CRC32C_FLAG_INIT_RESULT  0x1
/*return the result with network byte order*/
#define CRC32C_FLAG_NETWORK_BYTE_ORDER  0x2

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u8 * cv_pu8Buffer;
    u32 cv_u32Len;
} crc32c_vec_t;

/* --- functional routines ------------------------------------------------- */
void crc32c(u8 * pu8Data, u32 u32Len, u32 u32Flags, u32 * pu32Result);

void crc32cVec(crc32c_vec_t * pcv, u32 u32Count,
    u32 u32Flags, u32 * pu32Result);

#endif /*JIUTAI_CRC32C_H*/

/*---------------------------------------------------------------------------*/


