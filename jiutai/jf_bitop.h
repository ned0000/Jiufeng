/**
 *  @file jf_bitop.h
 *
 *  @brief Bitop header file. Provide some routines for bit operation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The word is u64 type.
 *  -# Bit positions are counted n...0, i.e. lowest bit is at position 0.
 *
 */

#ifndef JIUTAI_BITOP_H
#define JIUTAI_BITOP_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Generate a bitmask consisting of 1 bits from (and including) bit position 'left' to
 *  (and including) bit position 'right'.
 */
#define JF_BITOP_MASK(word, left, right)                \
    ((((u64)1 << (((left) - (right)) + 1)) - 1) << (right))

/** Extract value from 'word' at position 'left' to 'right' and return value.
 */
#define JF_BITOP_GET(word, left, right) \
    (((word) >> (right)) & JF_BITOP_MASK(word, (left) - (right), 0))

/** Extract 1 bit from 'word' at position 'pos' and return value.
 */
#define JF_BITOP_GET_BIT(word, pos)  JF_BITOP_GET(word, pos, pos)

/** Insert 'value' into 'word' at position 'left' to 'right'.
 */
#define JF_BITOP_SET(word, left, right, value) \
    ((word) = ((word & ~(JF_BITOP_MASK(word, left, right))) |                  \
     (((value) & JF_BITOP_MASK(word, (left) - (right), 0)) << (right))))

/** Set bit at position 'pos' to 1 of 'word'.
 */
#define JF_BITOP_SET_BIT(word, pos)  JF_BITOP_SET(word, pos, pos, 1)

/** Clear bits at position 'left' to 'right' to 0 of 'word'.
 */
#define JF_BITOP_CLEAR(word, left, right)   JF_BITOP_SET(word, left, right, 0)

/** Clear bit at position 'pos' to 0 of 'word'.
 */
#define JF_BITOP_CLEAR_BIT(word, pos)   JF_BITOP_CLEAR(word, pos, pos)

/** Generate a single 'bit' (0 or 1) at bit position `n'.
 */
#define JF_BITOP_BIT(pos, bit) ((bit) << (pos))

/** Generate a quad word octet of bits (a half byte, i.e. bit positions 3 to 0).
 */
#define JF_BITOP_QUAD(b3, b2, b1, b0) \
    (JF_BITOP_BIT(3, (b3)) | JF_BITOP_BIT(2, (b2)) | \
     JF_BITOP_BIT(1, (b1)) | JF_BITOP_BIT(0, (b0)))

/** Generate an octet word of bits (a byte, i.e. bit positions 7 to 0).
 */
#define JF_BITOP_OCTET(b7, b6, b5, b4, b3, b2, b1, b0) \
    ((JF_BITOP_QUAD(b7, b6, b5, b4) << 4) | JF_BITOP_QUAD(b3, b2, b1, b0))

/** Generate the value 2^n.
 */
#define JF_BITOP_POW2(n) JF_BITOP_BIT(n, 1)

/** Shift 'word' k bits to the left.
 */
#define JF_BITOP_SHL(word, k) ((word) << (k))

/** Shift 'word' k bits to the right.
 */
#define JF_BITOP_SHR(word, k) ((word) >> (k))

/** Rotate 'word' (of bits n..0) k bits to the left.
 */
#define JF_BITOP_ROL(word, n, k) \
    ((JF_BITOP_SHL((word), (k)) & JF_BITOP_MASK(word, n, 0)) | \
     JF_BITOP_SHR(((word) & JF_BITOP_MASK(word, n, 0)),(n) - (k)))

/** Rotate 'word' (of bits n..0) k bits to the right.
 */
#define JF_BITOP_ROR(word, n, k) \
    ((JF_BITOP_SHR(((word) & JF_BITOP_MASK(word, n, 0)), (k))) |   \
     JF_BITOP_SHL(((word), (n) - (k)) & JF_BITOP_MASK(word, n, 0)))

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */


#endif /*JIUTAI_BITOP_H*/

/*------------------------------------------------------------------------------------------------*/
