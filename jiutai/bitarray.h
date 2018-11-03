/**
 *  @file bitarray.h
 *
 *  @brief bit array header file, arrays of arbitrary bit length
 *
 *  @author Min Zhang
 *
 *  @note
 *   Provides functions for creation and manipulation of arbitrary
 *   length arrays of bits.
 *   Bit arrays are implemented as arrays of unsigned chars.  Bit
 *   0 is the MSB of olchar_t 0, and the last bit is the least
 *   significant (non-spare) bit of the last unsigned char.
 *   Example: array of 20 bits (0 through 19) with 8 bit unsigned
 *            chars requires 3 unsigned chars (0 through 2) to
 *            store all the bits.
 *
 *            olchar_t       0       1         2
 *                   +--------+--------+--------+
 *                   |        |        |        |
 *                   +--------+--------+--------+
 *            bit     01234567 89111111 11112XXX
 *                               012345 67890
 *
 */

#ifndef JIUTAI_BITARRAY_H
#define JIUTAI_BITARRAY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
typedef u8  bit_array_t;


/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
/* number of bit in bit array unit */
#define BA_BITS_PER_BIT_ARRAY  BITS_PER_U8

/* number of olchar_t in bit array */
#define BA_CHARS(ba)  (sizeof(ba))

/* number of bit in bit array */
#define BA_BITS(ba)  (BA_CHARS(ba) * BITS_PER_U8)

/* position of bit within character */
#define BA_BIT_CHAR(bit)      ((bit) / BITS_PER_U8)

/* array index for character containing bit */
#define BA_BIT_IN_CHAR(bit)   (1 << (BITS_PER_U8 - 1 - ((bit) % BITS_PER_U8)))

/* number of characters required to contain number of bits */
#define BA_BITS_TO_CHARS(bits)   ((((bits) - 1) / BITS_PER_U8) + 1)

#define INIT_BIT_ARRAY(ba) \
    memset((ba), 0, BA_CHARS(ba))

/*dump bit array*/
#define DUMP_BIT_ARRAY(ba)  \
{                           \
    olint_t i, j;               \
                            \
    for (i = 0; i < BA_CHARS(ba); i ++)                  \
    {                                                    \
        for (j = 0; j < BITS_PER_U8; j ++)               \
        {                                                \
            if ((ba)[i] & (1 << (BITS_PER_U8 - j - 1)))  \
                printf("1");                             \
            else                                         \
                printf("0");                             \
        }                                                \
    }                                                    \
                                                         \
    printf("\n");                                        \
}

/*set bit array to all 1*/
#define SET_BIT_ARRAY(ba)      \
    memset((ba), 0xFF, BA_CHARS(ba))

/*left shift bit array, from */
#define LSHIFT_BIT_ARRAY(ba, shift)          \
{                                            \
    olint_t index;                               \
    olint_t chars = (shift) / BITS_PER_U8;  /* number of whole byte shift */ \
    olint_t remain;                              \
                                             \
    assert((shift) > 0);                     \
                                             \
    if ((shift) >= BA_BITS(ba))              \
    {                                        \
        /* all bits have been shifted off */ \
        INIT_BIT_ARRAY(ba);                  \
    }                                        \
    else                                     \
    {                                        \
        remain = (shift) % BITS_PER_U8;  /* number of bit remaining */ \
                                             \
        /* handle big jumps of bytes */      \
        if (chars > 0)                       \
        {                                    \
            for (index = 0; (index + chars) < BA_CHARS(ba); index ++)  \
            {                                                 \
                (ba)[index] = (ba)[index + chars];            \
            }                                                 \
                                                              \
            /* now zero out new bytes on the right */         \
            for (index = BA_CHARS(ba); chars > 0; chars --)   \
            {                                                 \
                (ba)[index - chars] = 0;                      \
            }                                                 \
        }                                                     \
                                                              \
        /*we have at most BITS_PER_U8 - 1 bit remain across the whole array*/\
        if (remain > 0)                                       \
        {                                                     \
            for (index = 0; index + 1 < BA_CHARS(ba); index ++)        \
            {                                                 \
                ba[index] = ((ba)[index] << remain) |         \
                    ((ba)[index + 1] >> (BITS_PER_U8 - remain));  \
            }                                                 \
                                                              \
            (ba)[index] <<= remain;                           \
        }                                                     \
    }                                                         \
}

/*right shift bit array*/
#define RSHIFT_BIT_ARRAY(ba, shift)              \
{                                                \
    olint_t index;                                   \
    olint_t chars = (shift) / BITS_PER_U8;  /* number of whole byte shift */ \
    olint_t remain;                                  \
                                                 \
    assert((shift) > 0);                         \
                                                 \
    if ((shift) >= BA_BITS(ba))                  \
    {                                            \
        /* all bits have been shifted off */     \
        INIT_BIT_ARRAY(ba);                      \
    }                                            \
    else                                         \
    {                                            \
        remain = (shift) % BITS_PER_U8;  /* number of bit remaining */  \
                                                 \
        /* first handle big jumps of bytes */    \
        if (chars > 0)                           \
        {                                        \
            for (index = BA_CHARS(ba) - 1; (index - chars) >= 0; index --)  \
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
        /* now we have at most BITS_PER_U8 - 1 bit across the whole array*/ \
        if (remain > 0)                          \
        {                                        \
            for (index = BA_CHARS(ba) - 1; index - 1 >= 0; index --)  \
            {                                                         \
                (ba)[index] = ((ba)[index] >> remain) |               \
                    ((ba)[index - 1] << (BITS_PER_U8 - remain));      \
            }                                                  \
                                                               \
            (ba)[0] >>= remain;                                \
        }                                                      \
    }                                                          \
}

#define AND_BIT_ARRAY(dest, src1, src2)                 \
{                                                       \
    olint_t index;                                          \
                                                        \
    assert(BA_CHARS(dest) == BA_CHARS(src1));           \
    assert(BA_CHARS(dest) == BA_CHARS(src2));           \
                                                        \
    /* AND array one u8 at a time */         \
    for (index = 0; index < BA_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] & (src2)[index];  \
    }                                                   \
}

#define OR_BIT_ARRAY(dest, src1, src2)                  \
{                                                       \
    olint_t index;                                          \
                                                        \
    assert(BA_CHARS(dest) == BA_CHARS(src1));           \
    assert(BA_CHARS(dest) == BA_CHARS(src2));           \
                                                        \
    /* OR array one u8 at a time */          \
    for (index = 0; index < BA_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] | (src2)[index];  \
    }                                                   \
}

#define XOR_BIT_ARRAY(dest, src1, src2)                 \
{                                                       \
    olint_t index;                                          \
                                                        \
    assert(BA_CHARS(dest) == BA_CHARS(src1));           \
    assert(BA_CHARS(dest) == BA_CHARS(src2));           \
                                                        \
    /* XOR array one u8 at a time */         \
    for (index = 0; index < BA_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = (src1)[index] ^ (src2)[index];  \
    }                                                   \
}

#define NOT_BIT_ARRAY(dest, src)                        \
{                                                       \
    olint_t index;                                          \
                                                        \
    assert(BA_CHARS(dest) == BA_CHARS(src));            \
                                                        \
    /* NOT array one u8 at a time */         \
    for (index = 0; index < BA_CHARS(dest); index++)    \
    {                                                   \
        (dest)[index] = ~((src)[index]);                \
    }                                                   \
}

/*duplicate bit array*/
#define COPY_BIT_ARRAY(dest, src)                       \
{                                                       \
    assert(BA_CHARS(dest) == BA_CHARS(src));            \
                                                        \
    memcpy((dest), (src), BA_CHARS(dest));              \
}

/*increment and decrement apply to the MSB bit*/
/*increment bit array*/
#define INCREMENT_BIT_ARRAY(ba)                         \
{                                                       \
    olint_t index;                                          \
                                                        \
    for (index = BA_CHARS(ba) - 1; index >= 0; index --)\
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
#define DECREMENT_BIT_ARRAY(ba)                         \
{                                                       \
    olint_t index;                                          \
                                                        \
    for (index = BA_CHARS(ba) - 1; index >= 0; index--) \
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

/* The following macros don't use BA_CHARS(), so the 'ba' can be
 * a parameter passed by a function as a pointer
 */

/*set bit specified by pos to 1*/
static inline void setBitArrayBit(bit_array_t * ba, u32 pos)
{
    (ba)[BA_BIT_CHAR(pos)] |= BA_BIT_IN_CHAR(pos);
}

/*clear bit specified by pos to 0*/
static inline void clearBitArrayBit(bit_array_t * ba, u32 pos)
{
    u8 mask;

    /* create a mask to zero out desired bit */
    mask = BA_BIT_IN_CHAR(pos);
    mask = ~mask;

    (ba)[BA_BIT_CHAR(pos)] &= mask;
}

/*test the bit specified by pos*/
static inline boolean_t testBitArrayBit(bit_array_t * ba, u32 pos)
{
    return (((ba)[BA_BIT_CHAR(pos)] & BA_BIT_IN_CHAR(pos)) != 0);
}

/*set a bit and return it's old value*/
static inline boolean_t testSetBitArrayBit(bit_array_t * ba, u32 pos)
{
    boolean_t oldbit = testBitArrayBit(ba, pos);

    setBitArrayBit(ba, pos);

    return oldbit;
}

/*clear a bit and return it's old value*/
static inline boolean_t testClearBitArrayBit(bit_array_t * ba, u32 pos)
{
    boolean_t oldbit = testBitArrayBit(ba, pos);

    clearBitArrayBit(ba, pos);

    return oldbit;
}

#endif /*JIUTAI_BITARRAY_H*/

/*---------------------------------------------------------------------------*/

