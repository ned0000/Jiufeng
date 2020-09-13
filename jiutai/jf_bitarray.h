/**
 *  @file jf_bitarray.h
 *
 *  @brief Header file for bit array. Provides functions for creation and manipulation of arbitrary
 *  length arrays of bits.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Bit arrays are implemented as arrays of unsigned chars.  Bit 0 is the MSB of char 0,
 *   and the last bit is the least significant (non-spare) bit of the last unsigned char.
 *
 *  <HR>
 *
 *  @par Example
 *  -# Array of 20 bits (0 through 19) with 8 bit unsigned chars requires 3 unsigned chars
 *  (0 through 2) to store all the bits.
 *  @code
 *           u8         0       1         2
 *                  +--------+--------+--------+
 *                  |        |        |        |
 *                  +--------+--------+--------+
 *           bit     01234567 89111111 11112XXX
 *                              012345 67890
 *  @endcode
 *
 *  <HR>
 */

#ifndef JIUTAI_BITARRAY_H
#define JIUTAI_BITARRAY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the bitarray data type.
 */
typedef u8  jf_bitarray_t;


/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

/** Return number of bits in bit array unit.
 */
#define JF_BITARRAY_BITS_PER_UNIT  (BITS_PER_U8)

/** Return number of char in bit array.
 */
#define JF_BITARRAY_CHARS(ba)  (sizeof(ba))

/** Return number of bits in bit array.
 */
#define JF_BITARRAY_BITS(ba)  (JF_BITARRAY_CHARS(ba) * JF_BITARRAY_BITS_PER_UNIT)

/** Return position of bit within char.
 */
#define JF_BITARRAY_BIT_CHAR(bit)  ((bit) / JF_BITARRAY_BITS_PER_UNIT)

/** Return array index for char containing bit.
 */
#define JF_BITARRAY_BIT_IN_CHAR(bit)  \
    (1 << (JF_BITARRAY_BITS_PER_UNIT - 1 - ((bit) % JF_BITARRAY_BITS_PER_UNIT)))

/** Return number of char required to contain number of bits.
 */
#define JF_BITARRAY_BITS_TO_CHARS(bits)   ((((bits) - 1) / JF_BITARRAY_BITS_PER_UNIT) + 1)

/** Initialize the bit array.
 */
#define JF_BITARRAY_INIT(ba)  ol_memset((ba), 0, JF_BITARRAY_CHARS(ba))

/** Dump bit array. If the bit is set, print "1", otherwise print "0".
 */
#define JF_BITARRAY_DUMP(ba)  \
{                             \
    olint_t i, j;             \
                              \
    for (i = 0; i < JF_BITARRAY_CHARS(ba); i ++)         \
    {                                                    \
        for (j = 0; j < JF_BITARRAY_BITS_PER_UNIT; j ++) \
        {                                                \
            if ((ba)[i] & (1 << (JF_BITARRAY_BITS_PER_UNIT - j - 1)))  \
                ol_printf("1");                          \
            else                                         \
                ol_printf("0");                          \
        }                                                \
    }                                                    \
                                                         \
    ol_printf("\n");                                     \
}

/** Set bit array to all 1.
 */
#define JF_BITARRAY_SET(ba)  ol_memset((ba), 0xFF, JF_BITARRAY_CHARS(ba))

/** Left shift bit array "ba" with "shift" bits.
 */
#define JF_BITARRAY_LSHIFT(ba, shift)        \
{                                            \
    olint_t index;                           \
    /*Number of whole byte shift.*/          \
    olint_t chars = (shift) / JF_BITARRAY_BITS_PER_UNIT; \
    olint_t remain;                          \
                                             \
    assert((shift) > 0);                     \
                                             \
    if ((shift) >= JF_BITARRAY_BITS(ba))     \
    {                                        \
        /*All bits have been shifted off.*/  \
        JF_BITARRAY_INIT(ba);                \
    }                                        \
    else                                     \
    {                                        \
        /*Number of bit remaining.*/         \
        remain = (shift) % JF_BITARRAY_BITS_PER_UNIT;    \
                                             \
        /*Handle big jumps of bytes.*/       \
        if (chars > 0)                       \
        {                                    \
            for (index = 0; (index + chars) < JF_BITARRAY_CHARS(ba); index ++)  \
            {                                                 \
                (ba)[index] = (ba)[index + chars];            \
            }                                                 \
                                                              \
            /*Now zero out new bytes on the right.*/          \
            for (index = JF_BITARRAY_CHARS(ba); chars > 0; chars --)   \
            {                                                 \
                (ba)[index - chars] = 0;                      \
            }                                                 \
        }                                                     \
                                                              \
        /*We have at most JF_BITARRAY_BITS_PER_UNIT - 1 bit remain across the whole array.*/\
        if (remain > 0)                                       \
        {                                                     \
            for (index = 0; index + 1 < JF_BITARRAY_CHARS(ba); index ++)        \
            {                                                 \
                ba[index] = ((ba)[index] << remain) |         \
                    ((ba)[index + 1] >> (JF_BITARRAY_BITS_PER_UNIT - remain));  \
            }                                                 \
                                                              \
            (ba)[index] <<= remain;                           \
        }                                                     \
    }                                                         \
}

/** Right shift bit array "ba" with "shift" bits.
 */
#define JF_BITARRAY_RSHIFT(ba, shift)            \
{                                                \
    olint_t index;                               \
    /*Number of whole byte shift.*/              \
    olint_t chars = (shift) / JF_BITARRAY_BITS_PER_UNIT;  \
    olint_t remain;                              \
                                                 \
    assert((shift) > 0);                         \
                                                 \
    if ((shift) >= JF_BITARRAY_BITS(ba))         \
    {                                            \
        /*All bits have been shifted off.*/      \
        JF_BITARRAY_INIT(ba);                    \
    }                                            \
    else                                         \
    {                                            \
        /*Number of bit remaining.*/             \
        remain = (shift) % JF_BITARRAY_BITS_PER_UNIT;   \
                                                 \
        /*First handle big jumps of bytes.*/     \
        if (chars > 0)                           \
        {                                        \
            for (index = JF_BITARRAY_CHARS(ba) - 1; (index - chars) >= 0; index --)  \
            {                                    \
                (ba)[index] = (ba)[index - chars];      \
            }                                    \
                                                 \
            /*Now zero out new bytes on the right.*/    \
            for (; chars > 0; chars --)          \
            {                                    \
                (ba)[chars - 1] = 0;             \
            }                                    \
        }                                        \
                                                 \
        /*Now we have at most JF_BITARRAY_BITS_PER_UNIT - 1 bit across the whole array.*/ \
        if (remain > 0)                          \
        {                                        \
            for (index = JF_BITARRAY_CHARS(ba) - 1; index - 1 >= 0; index --)  \
            {                                                  \
                (ba)[index] = ((ba)[index] >> remain) |        \
                    ((ba)[index - 1] << (JF_BITARRAY_BITS_PER_UNIT - remain)); \
            }                                                  \
                                                               \
            (ba)[0] >>= remain;                                \
        }                                                      \
    }                                                          \
}

/** And the bitarray "src1" with "src2", the result saves to "dest".
 */
#define JF_BITARRAY_AND(dest, src1, src2)               \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /*AND array one u8 at a time.*/                     \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] & (src2)[index];  \
    }                                                   \
}

/** Or the bitarray "src1" with "src2", the result saves to "dest".
 */
#define JF_BITARRAY_OR(dest, src1, src2)                \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /*OR array one char at a time.*/                    \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] | (src2)[index];  \
    }                                                   \
}

/** XOR the bitarray "src1" with "src2", the result saves to "dest".
 */
#define JF_BITARRAY_XOR(dest, src1, src2)               \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /*XOR array one char at a time.*/                   \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] ^ (src2)[index];  \
    }                                                   \
}

/** Complement the bitarray "src", the result saves to "dest".
 */
#define JF_BITARRAY_NOT(dest, src)                      \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src));   \
                                                        \
    /*NOT array one char at a time.*/                   \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = ~((src)[index]);                \
    }                                                   \
}

/** Duplicate bit array "src" to "dest".
 */
#define JF_BITARRAY_COPY(dest, src)                     \
{                                                       \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src));   \
                                                        \
    memcpy((dest), (src), JF_BITARRAY_CHARS(dest));     \
}

/** Increment bit array, apply to the MSB bit.
 */
#define JF_BITARRAY_INCREMENT(ba)                       \
{                                                       \
    olint_t index;                                      \
                                                        \
    for (index = JF_BITARRAY_CHARS(ba) - 1; index >= 0; index --)\
    {                                                   \
        if ((ba)[index] != U8_MAX)                      \
        {                                               \
            (ba)[index] ++;                             \
            break;                                      \
        }                                               \
        else                                            \
        {                                               \
            /*Need to carry to next byte.*/             \
            (ba)[index] = 0;                            \
        }                                               \
    }                                                   \
}

/** Decrement bit array, apply to the MSB bit.
 */
#define JF_BITARRAY_DECREMENT(ba)                       \
{                                                       \
    olint_t index;                                      \
                                                        \
    for (index = JF_BITARRAY_CHARS(ba) - 1; index >= 0; index--) \
    {                                                   \
        if ((ba)[index] >= 1)                           \
        {                                               \
            (ba)[index] --;                             \
            break;                                      \
        }                                               \
        else                                            \
        {                                               \
            (ba)[index] = U8_MAX;                       \
        }                                               \
    }                                                   \
}

/** Set bit specified by "pos" to 1.
 *  
 *  @param ba [in] The bit array.
 *  @param pos [in] The position.
 *
 *  @return Void.
 */
static inline void jf_bitarray_setBit(jf_bitarray_t * ba, u32 pos)
{
    assert(JF_BITARRAY_BITS(ba) > pos);

    (ba)[JF_BITARRAY_BIT_CHAR(pos)] |= JF_BITARRAY_BIT_IN_CHAR(pos);
}

/** Clear bit specified by "pos" to 0.
 *
 *  @param ba [in] The bit array.
 *  @param pos [in] The position.
 *
 *  @return Void.
 */
static inline void jf_bitarray_clearBit(jf_bitarray_t * ba, u32 pos)
{
    u8 mask = 0;

    assert(JF_BITARRAY_BITS(ba) > pos);

    /*Create a mask to zero out desired bit.*/
    mask = JF_BITARRAY_BIT_IN_CHAR(pos);
    mask = ~mask;

    (ba)[JF_BITARRAY_BIT_CHAR(pos)] &= mask;
}

/** Test the bit specified by "pos".
 *
 *  @param ba [in] The bit array.
 *  @param pos [in] The position.
 *
 *  @return The bit status.
 *  @retval TRUE The bit is set.
 *  @retval FALSE The bit is cleared.
 */
static inline boolean_t jf_bitarray_testBit(jf_bitarray_t * ba, u32 pos)
{
    assert(JF_BITARRAY_BITS(ba) > pos);

    return (((ba)[JF_BITARRAY_BIT_CHAR(pos)] & JF_BITARRAY_BIT_IN_CHAR(pos)) != 0);
}

/** Set a bit specified by "pos" and return it's old value.
 *
 *  @param ba [in] The bit array.
 *  @param pos [in] The position.
 *
 *  @return The bit status.
 *  @retval TRUE The bit is set.
 *  @retval FALSE The bit is cleared.
 */
static inline boolean_t jf_bitarray_testSetBit(jf_bitarray_t * ba, u32 pos)
{
    boolean_t oldbit = jf_bitarray_testBit(ba, pos);

    jf_bitarray_setBit(ba, pos);

    return oldbit;
}

/** Clear a bit specified by "pos" and return it's old value.
 *
 *  @param ba [in] The bit array.
 *  @param pos [in] The position.
 *
 *  @return The bit status.
 *  @retval TRUE The bit is set.
 *  @retval FALSE The bit is cleared.
 */
static inline boolean_t jf_bitarray_testClearBit(jf_bitarray_t * ba, u32 pos)
{
    boolean_t oldbit = jf_bitarray_testBit(ba, pos);

    jf_bitarray_clearBit(ba, pos);

    return oldbit;
}

#endif /*JIUTAI_BITARRAY_H*/

/*------------------------------------------------------------------------------------------------*/
