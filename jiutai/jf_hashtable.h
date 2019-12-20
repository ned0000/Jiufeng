/**
 *  @file jf_hashtable.h
 *
 *  @brief Header file for hash table common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_hashtable object
 *  -# This is an implementation of a general hash table module. It assumes that keys for entry
 *   stored in the hash table can be extracted from the entry.
 *  -# Link with jf_jiukun library for memory allocation.
 */

/*------------------------------------------------------------------------------------------------*/
#ifndef JIUTAI_HASH_H
#define JIUTAI_HASH_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

typedef void  jf_hashtable_t;

typedef olint_t (* jf_hashtable_fnCmpKeys_t) (void * pKey1, void * pKey2);
typedef olint_t (* jf_hashtable_fnHashKey_t) (void * pKey);
typedef void * (* jf_hashtable_fnGetKeyFromEntry_t) (void * pEntry);
typedef u32 (* jf_hashtable_fnFreeEntry_t) (void ** ppEntry);

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter data type for creating hash table.
 */
typedef struct
{
    /**Minimal number of entry by estimation.*/
    u32 jhcp_u32MinSize;
    u32 jhcp_u32Reserved;
    /**Callback function to compare key.*/
    jf_hashtable_fnCmpKeys_t jhcp_fnCmpKeys;
    /**Callback function to hash key.*/
    jf_hashtable_fnHashKey_t jhcp_fnHashKey;
    /**Callback function to get key from entry.*/
    jf_hashtable_fnGetKeyFromEntry_t jhcp_fnGetKeyFromEntry;
    /**Callback function to free the entry.*/
    jf_hashtable_fnFreeEntry_t jhcp_fnFreeEntry;
    u32 jhcp_u32Reserved2[4];
} jf_hashtable_create_param_t;

/** Define the hash table statistic data type.
 */
typedef struct
{
    /**Number of entry in hash table.*/
    u32 jhs_u32NumOfEntry;
    /**Size of hash table.*/
    u32 jhs_u32Size;
    /**Number of collisions.*/
    u32 jhs_u32Collisions;
    /**The bucket index with maximum entries.*/
    u32 jhs_u32BucketIndexWithMaxEntries;
    /**Count of resize operation.*/
    u32 jhs_u32CountOfResizeOp;
} jf_hashtable_stat_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_hashtable_create(
    jf_hashtable_t ** ppjh, jf_hashtable_create_param_t * pjhcp);

/** Destructor of hash table. Free all stored entry with the user provided deallocation function.
 */
u32 jf_hashtable_destroy(jf_hashtable_t ** pjh);

/** Insert a entry into the hash table but do not overwrite existing entry with the same key.
 *
 *  @note
 *  -# The return value is JF_ERR_NO_ERROR if some entry with the same key is already stored in the
 *     hash table. In this case the user generally has to deallocate entry on his own.
 */
u32 jf_hashtable_insertEntry(jf_hashtable_t * pjh, void * pEntry);

/** Remove correponding entry from the hash table and deallocate it using the user provided
 *  deallocator.
 */
u32 jf_hashtable_removeEntry(jf_hashtable_t * pjh, void * pEntry);

/** Overwrite an existing entry in the hash table with the same key as the key of entry or insert a
 *  new entry. In the first case also deallocate the overwritten entry.
 */
u32 jf_hashtable_overwriteEntry(jf_hashtable_t * pjh, void * pEntry);

/** Search for entry with this key. Return error code if not found.
 */
u32 jf_hashtable_getEntry(jf_hashtable_t * pjh, void * pKey, void ** ppEntry);

boolean_t jf_hashtable_isEntryInTable(jf_hashtable_t * pjh, void * pEntry);

boolean_t jf_hashtable_isKeyInTable(jf_hashtable_t * pjh, void * pKey);

/** Return the number of stored key/entry pairs.
 */
u32 jf_hashtable_getSize(jf_hashtable_t * pjh);

/** Returns an statically allocated string with useful statistical information. This string is
 *  overwritten by subsequent calls to this function.
 */
void jf_hashtable_getStat(jf_hashtable_t * pjh, jf_hashtable_stat_t * stat);

/** The definition of this structure is placed here because it should be possible to allocate an
 *  iterator as an automatic variable. This is also more convenient for the user.
 */
typedef struct
{
    jf_hashtable_t * jhi_htTable;
    olint_t jhi_nPos;
    void * jhi_pCursor;
} jf_hashtable_iterator_t;

/** Setup an iterator. It is possible to have multiple iterators for the same hash table and to
 *  traverse the hash table in parallel.
 *
 *  @note
 *  -# These iterators are not safe with respect to 'remove' operations on the traversed hash table.
 */
void jf_hashtable_setupIterator(
    jf_hashtable_t * pjh, jf_hashtable_iterator_t * pIterator);

void jf_hashtable_incrementIterator(jf_hashtable_iterator_t * pIterator);

void * jf_hashtable_getEntryFromIterator(jf_hashtable_iterator_t * pIterator);

boolean_t jf_hashtable_isEndOfIterator(jf_hashtable_iterator_t * pIterator);

/** Hash function for zero terminated entry (f.e. strings).
 *  It is from the `dragon book' and should work very well especially for strings as keys.
 */
olint_t jf_hashtable_hashPJW(void * pKey);


#ifndef BITS_PER_LONG
    #if defined(JIUFENG_64BIT)
        #define BITS_PER_LONG  64
    #else
        #define BITS_PER_LONG  32
    #endif
#endif

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define JF_HASHTABLE_GOLDEN_RATIO_PRIME_32 0x9e370001UL
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define JF_HASHTABLE_GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

#if BITS_PER_LONG == 32
    #define JF_HASHTABLE_GOLDEN_RATIO_PRIME  JF_HASHTABLE_GOLDEN_RATIO_PRIME_32
    #define jf_hashtable_hashLong(val, bits) jf_hashtable_hashU32(val, bits)
#elif BITS_PER_LONG == 64
    #define jf_hashtable_hashLong(val, bits) jf_hashtable_hashU64(val, bits)
    #define JF_HASHTABLE_GOLDEN_RATIO_PRIME  JF_HASHTABLE_GOLDEN_RATIO_PRIME_64
#else
    #error Wordsize not 32 or 64
#endif


static inline u64 jf_hashtable_hashU64(u64 val, u32 bits)
{
    u64 hash = val;

    /*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
    u64 n = hash;
    n <<= 18;
    hash -= n;
    n <<= 33;
    hash -= n;
    n <<= 3;
    hash += n;
    n <<= 3;
    hash -= n;
    n <<= 4;
    hash += n;
    n <<= 2;
    hash += n;

    /* High bits are more random, so use them. */
    return hash >> (64 - bits);
}

static inline u32 jf_hashtable_hashU32(u32 val, u32 bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    u32 hash = val * JF_HASHTABLE_GOLDEN_RATIO_PRIME_32;

    /* High bits are more random, so use them. */
    return hash >> (32 - bits);
}

static inline unsigned long jf_hashtable_hashPtr(void *ptr, u32 bits)
{
    return jf_hashtable_hashLong((unsigned long)ptr, bits);
}


#endif /*JIUTAI_HASH_H*/

/*------------------------------------------------------------------------------------------------*/

