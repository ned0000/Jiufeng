/**
 *  @file bitop.h
 *
 *  @brief Bitop header file. Provide some routines for bit operation
 *
 *  @author Min Zhang
 *
 *  @note The word is u64 type
 *  @note Bit positions are counted n...0, i.e. lowest bit is at position 0
 *
 */

#ifndef JIUTAI_BITOP_H
#define JIUTAI_BITOP_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
/** generate a bitmask consisting of 1 bits from (and including)
 *  bit position 'left' to (and including) bit position 'right'
 */
#define BITOP_MASK(word, left, right)                \
    ((((u64)1 << (((left) - (right)) + 1)) - 1) << (right))

/** extract value from 'word' at position 'left' to 'right' and return value
 */
#define BITOP_GET(word, left, right) \
    (((word) >> (right)) & BITOP_MASK(word, (left) - (right), 0))

/** extract 1 bit from 'word' at position 'pos' and return value
 */
#define BITOP_GET_BIT(word, pos)  BITOP_GET(word, pos, pos)

/** insert 'value' into 'word' at position 'left' to 'right'
 */
#define BITOP_SET(word, left, right, value) \
    ((word) = ((word & ~(BITOP_MASK(word, left, right))) |                  \
     (((value) & BITOP_MASK(word, (left) - (right), 0)) << (right))))

/** set bit at position 'pos' to 1 of 'word'
 */
#define BITOP_SET_BIT(word, pos)  BITOP_SET(word, pos, pos, 1)

/** clear bits at position 'left' to 'right' to 0 of 'word'
 */
#define BITOP_CLEAR(word, left, right)   BITOP_SET(word, left, right, 0)

/** clear bit at position 'pos' to 0 of 'word' 
 */
#define BITOP_CLEAR_BIT(word, pos)   BITOP_CLEAR(word, pos, pos)

/** generate a single 'bit' (0 or 1) at bit position `n'
 */
#define BITOP_BIT(pos, bit) ((bit) << (pos))

/** generate a quad word octet of bits (a half byte, i.e. bit positions 3 to 0)
 */
#define BITOP_QUAD(b3, b2, b1, b0) \
    (BITOP_BIT(3, (b3)) | BITOP_BIT(2, (b2)) | \
     BITOP_BIT(1, (b1)) | BITOP_BIT(0, (b0)))

/** generate an octet word of bits (a byte, i.e. bit positions 7 to 0) 
 */
#define BITOP_OCTET(b7, b6, b5, b4, b3, b2, b1, b0) \
    ((BITOP_QUAD(b7, b6, b5, b4) << 4) | BITOP_QUAD(b3, b2, b1, b0))

/** generate the value 2^n 
 */
#define BITOP_POW2(n) BITOP_BIT(n, 1)

/** shift 'word' k bits to the left
 */
#define BITOP_SHL(word, k) ((word) << (k))

/** shift 'word' k bits to the right
 */
#define BITOP_SHR(word, k) ((word) >> (k))

/** rotate 'word' (of bits n..0) k bits to the left
 */
#define BITOP_ROL(word, n, k) \
    ((BITOP_SHL((word), (k)) & BITOP_MASK(word, n, 0)) | \
     BITOP_SHR(((word) & BITOP_MASK(word, n, 0)),(n) - (k)))

/** rotate 'word' (of bits n..0) k bits to the right
 */
#define BITOP_ROR(word, n, k) \
    ((BITOP_SHR(((word) & BITOP_MASK(word, n, 0)), (k))) |   \
     BITOP_SHL(((word), (n) - (k)) & BITOP_MASK(word, n, 0)))

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

#endif /*JIUTAI_BITOP_H*/

/*---------------------------------------------------------------------------*/


