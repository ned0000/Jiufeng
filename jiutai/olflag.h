/**
 *  @file olflag.h
 *
 *  @brief Flag header file.
 *
 *  @author Min Zhang
 *
 *  @note The flag operation is based on bit opration
 *  @note Maximum 64 flags as u64 is used
 *
 */

#ifndef JIUTAI_OLFLAG_H
#define JIUTAI_OLFLAG_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "bitop.h"

/* --- constant definitions ------------------------------------------------ */

/** init the 'flag' with 'value'
 */
#define INIT_FLAG(flag)        (flag = 0)

/** generate a bitmask consisting of 1 bits from (and including)
 *  bit position 'left' to (and including) bit position 'right'
 */
#define FLAG_MASK(pos)         (1ull << pos)

/** extract 1 bit from 'flag' at position 'pos'
 */
#define GET_FLAG(flag, pos)    BITOP_GET(flag, pos, pos)

/** set bit at position 'pos' to 1 of 'flag'
 */
#define SET_FLAG(flag, pos)    BITOP_SET(flag, pos, pos, 1)

/** clear bit at position 'pos' to 0 of 'flag'
 */
#define CLEAR_FLAG(flag, pos)  BITOP_CLEAR(flag, pos, pos)

/** insert 'value' into 'flag' at position 'left' to 'right'
 */
#define SET_FLAG_VALUE(flag, left, right, value)  \
    BITOP_SET(flag, left, right, value)

/** extract value from 'flag' at position 'left' to 'right' and return value
 */
#define GET_FLAG_VALUE(flag, left, right)  BITOP_GET(flag, left, right)


/* --- data structures ----------------------------------------------------- */
typedef u64    olflag_t;

/* --- functional routines ------------------------------------------------- */

#endif /*JIUTAI_OLFLAG_H*/

/*---------------------------------------------------------------------------*/


