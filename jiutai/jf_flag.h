/**
 *  @file jf_flag.h
 *
 *  @brief Header file for flags.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The flag operation is based on bit opration.
 *  -# Maximum 64 flags as u64 is used.
 */

#ifndef JIUTAI_FLAG_H
#define JIUTAI_FLAG_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_bitop.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Init the 'flag' with 'value'.
 */
#define JF_FLAG_INIT(flag)        (flag = 0)

/** Generate a bitmask consisting of 1 bits from (and including) bit position 'left' to
 *  (and including) bit position 'right'.
 */
#define JF_FLAG_MASK(pos)         (1ull << pos)

/** Extract 1 bit from 'flag' at position 'pos'.
 */
#define JF_FLAG_GET(flag, pos)    JF_BITOP_GET(flag, pos, pos)

/** Set bit at position 'pos' to 1 of 'flag'.
 */
#define JF_FLAG_SET(flag, pos)    JF_BITOP_SET(flag, pos, pos, 1)

/** Clear bit at position 'pos' to 0 of 'flag'.
 */
#define JF_FLAG_CLEAR(flag, pos)  JF_BITOP_CLEAR(flag, pos, pos)

/** Insert 'value' into 'flag' at position 'left' to 'right'.
 */
#define JF_FLAG_SET_VALUE(flag, left, right, value)  JF_BITOP_SET(flag, left, right, value)

/** Extract value from 'flag' at position 'left' to 'right' and return value.
 */
#define JF_FLAG_GET_VALUE(flag, left, right)  JF_BITOP_GET(flag, left, right)


/* --- data structures -------------------------------------------------------------------------- */

/** Define the flag data type.
 */
typedef u64    jf_flag_t;

/* --- functional routines ---------------------------------------------------------------------- */

#endif /*JIUTAI_FLAG_H*/

/*------------------------------------------------------------------------------------------------*/


