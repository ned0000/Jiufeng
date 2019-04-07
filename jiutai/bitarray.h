/**
 *  @file bitarray.h
 *
 *  @brief bit array header file, arrays of arbitrary bit length
 *
 *  @author Min Zhang
 *
 *  @note provides functions for creation and manipulation of arbitrary
 *   length arrays of bits.
 *  @note bit arrays are implemented as arrays of unsigned chars.  Bit 0 is the
 *   MSB of char 0, and the last bit is the least significant (non-spare)
 *   bit of the last unsigned char.
 *  @note example: array of 20 bits (0 through 19) with 8 bit unsigned chars
 *   requires 3 unsigned chars (0 through 2) to store all the bits.
 *           u8         0       1         2          \n
 *                  +--------+--------+--------+     \n
 *                  |        |        |        |     \n
 *                  +--------+--------+--------+     \n
 *           bit     01234567 89111111 11112XXX      \n
 *                              012345 67890         \n
 *
 */

#ifndef JIUTAI_BITARRAY_H
#define JIUTAI_BITARRAY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */
typedef u8  jf_bitarray_t;


/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

/* number of bit in bit array unit */
#define JF_BITARRAY_BITS_PER_UNIT  (BITS_PER_U8)

/* number of u8 in bit array */
#define JF_BITARRAY_CHARS(ba)  (sizeof(ba))

/* number of bit in bit array */
#define JF_BITARRAY_BITS(ba)  (JF_BITARRAY_CHARS(ba) * JF_BITARRAY_BITS_PER_UNIT)

/* position of bit within character */
#define JF_BITARRAY_BIT_CHAR(bit)  ((bit) / JF_BITARRAY_BITS_PER_UNIT)

/* array index for character containing bit */
#define JF_BITARRAY_BIT_IN_CHAR(bit)  \
    (1 << (JF_BITARRAY_BITS_PER_UNIT - 1 - ((bit) % JF_BITARRAY_BITS_PER_UNIT)))

/* number of characters required to contain number of bits */
#define JF_BITARRAY_BITS_TO_CHARS(bits)   ((((bits) - 1) / JF_BITARRAY_BITS_PER_UNIT) + 1)

#define JF_BITARRAY_INIT(ba)  \
    memset((ba), 0, JF_BITARRAY_CHARS(ba))

/*dump bit array*/
#define JF_BITARRAY_DUMP(ba)  \
{                           \
    olint_t i, j;           \
                            \
    for (i = 0; i < JF_BITARRAY_CHARS(ba); i ++)         \
    {                                                    \
        for (j = 0; j < JF_BITARRAY_BITS_PER_UNIT; j ++) \
        {                                                \
            if ((ba)[i] & (1 << (JF_BITARRAY_BITS_PER_UNIT - j - 1)))  \
                printf("1");                             \
            else                                         \
                printf("0");                             \
        }                                                \
    }                                                    \
                                                         \
    printf("\n");                                        \
}

/*set bit array to all 1*/
#define JF_BITARRAY_SET(ba)      \
    memset((ba), 0xFF, JF_BITARRAY_CHARS(ba))

/*left shift bit array*/
#define JF_BITARRAY_LSHIFT(ba, shift)        \
{                                            \
    olint_t index;                           \
    olint_t chars = (shift) / JF_BITARRAY_BITS_PER_UNIT;  /* number of whole byte shift */ \
    olint_t remain;                          \
                                             \
    assert((shift) > 0);                     \
                                             \
    if ((shift) >= JF_BITARRAY_BITS(ba))     \
    {                                        \
        /* all bits have been shifted off */ \
        JF_BITARRAY_INIT(ba);                \
    }                                        \
    else                                     \
    {                                        \
        remain = (shift) % JF_BITARRAY_BITS_PER_UNIT;  /* number of bit remaining */ \
                                             \
        /* handle big jumps of bytes */      \
        if (chars > 0)                       \
        {                                    \
            for (index = 0; (index + chars) < JF_BITARRAY_CHARS(ba); index ++)  \
            {                                                 \
                (ba)[index] = (ba)[index + chars];            \
            }                                                 \
                                                              \
            /* now zero out new bytes on the right */         \
            for (index = JF_BITARRAY_CHARS(ba); chars > 0; chars --)   \
            {                                                 \
                (ba)[index - chars] = 0;                      \
            }                                                 \
        }                                                     \
                                                              \
        /*we have at most JF_BITARRAY_BITS_PER_UNIT - 1 bit remain across the whole array*/\
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

/*right shift bit array*/
#define JF_BITARRAY_RSHIFT(ba, shift)            \
{                                                \
    olint_t index;                               \
    olint_t chars = (shift) / JF_BITARRAY_BITS_PER_UNIT;  /* number of whole byte shift */ \
    olint_t remain;                              \
                                                 \
    assert((shift) > 0);                         \
                                                 \
    if ((shift) >= JF_BITARRAY_BITS(ba))                  \
    {                                            \
        /* all bits have been shifted off */     \
        JF_BITARRAY_INIT(ba);                    \
    }                                            \
    else                                         \
    {                                            \
        remain = (shift) % JF_BITARRAY_BITS_PER_UNIT;  /* number of bit remaining */  \
                                                 \
        /* first handle big jumps of bytes */    \
        if (chars > 0)                           \
        {                                        \
            for (index = JF_BITARRAY_CHARS(ba) - 1; (index - chars) >= 0; index --)  \
            {                                    \
                (ba)[index] = (ba)[index - chars];      \
            }                                    \
                                                 \
            /* now zero out new bytes on the right */   \
            for (; chars > 0; chars --)          \
            {                                    \
                (ba)[chars - 1] = 0;             \
            }                                    \
        }                                        \
                                                 \
        /* now we have at most JF_BITARRAY_BITS_PER_UNIT - 1 bit across the whole array*/ \
        if (remain > 0)                          \
        {                                        \
            for (index = JF_BITARRAY_CHARS(ba) - 1; index - 1 >= 0; index --)  \
            {                                                  \
                (ba)[index] = ((ba)[index] >> remain) |        \
                    ((ba)[index - 1] << (JF_BITARRAY_BITS_PER_UNIT - remain));      \
            }                                                  \
                                                               \
            (ba)[0] >>= remain;                                \
        }                                                      \
    }                                                          \
}

#define JF_BITARRAY_AND(dest, src1, src2)               \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /* AND array one u8 at a time */                    \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] & (src2)[index];  \
    }                                                   \
}

#define JF_BITARRAY_OR(dest, src1, src2)                \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /* OR array one u8 at a time */                     \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] | (src2)[index];  \
    }                                                   \
}

#define JF_BITARRAY_XOR(dest, src1, src2)               \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src1));  \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src2));  \
                                                        \
    /* XOR array one u8 at a time */                    \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] ^ (src2)[index];  \
    }                                                   \
}

#define JF_BITARRAY_NOT(dest, src)                      \
{                                                       \
    olint_t index;                                      \
                                                        \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src));   \
                                                        \
    /* NOT array one u8 at a time */                    \
    for (index = 0; index < JF_BITARRAY_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = ~((src)[index]);                \
    }                                                   \
}

/*duplicate bit array*/
#define JF_BITARRAY_COPY(dest, src)                     \
{                                                       \
    assert(JF_BITARRAY_CHARS(dest) == JF_BITARRAY_CHARS(src));   \
                                                        \
    memcpy((dest), (src), JF_BITARRAY_CHARS(dest));     \
}

/*increment and decrement apply to the MSB bit*/
/*increment bit array*/
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
            /* need to carry to next byte */            \
            (ba)[index] = 0;                            \
        }                                               \
    }                                                   \
}

/*decrement bit array*/
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

/* The following macros don't use JF_BITARRAY_CHARS(), so the 'ba' can be
 * a parameter passed by a function as a pointer
 */

/*set bit specified by pos to 1*/
static inline void jf_bitarray_setBit(jf_bitarray_t * ba, u32 pos)
{
    (ba)[JF_BITARRAY_BIT_CHAR(pos)] |= JF_BITARRAY_BIT_IN_CHAR(pos);
}

/*clear bit specified by pos to 0*/
static inline void jf_bitarray_clearBit(jf_bitarray_t * ba, u32 pos)
{
    u8 mask;

    /* create a mask to zero out desired bit */
    mask = JF_BITARRAY_BIT_IN_CHAR(pos);
    mask = ~mask;

    (ba)[JF_BITARRAY_BIT_CHAR(pos)] &= mask;
}

/*test the bit specified by pos*/
static inline boolean_t jf_bitarray_testBit(jf_bitarray_t * ba, u32 pos)
{
    return (((ba)[JF_BITARRAY_BIT_CHAR(pos)] & JF_BITARRAY_BIT_IN_CHAR(pos)) != 0);
}

/*set a bit and return it's old value*/
static inline boolean_t jf_bitarray_testSetBit(jf_bitarray_t * ba, u32 pos)
{
    boolean_t oldbit = jf_bitarray_testBit(ba, pos);

    jf_bitarray_setBit(ba, pos);

    return oldbit;
}

/*clear a bit and return it's old value*/
static inline boolean_t jf_bitarray_testClearBit(jf_bitarray_t * ba, u32 pos)
{
    boolean_t oldbit = jf_bitarray_testBit(ba, pos);

    jf_bitarray_clearBit(ba, pos);

    return oldbit;
}

#endif /*JIUTAI_BITARRAY_H*/

/*---------------------------------------------------------------------------*/

