/**
 *  @file jf_hashtable.h
 *
 *  @brief Header file for hash table common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_hashtable object
 *  -# This is an implementation of a general hash table. It assumes that keys for entry stored in
 *   the hash table can be extracted from the entry.
 *  -# There is only one entry with the same key in hash table. In the case the new entry has the
 *   same key, for insert operation, the old entry is kept; for overwrite operation, the old entry
 *   is replaced.
 *  -# Link with jf_jiukun library for memory allocation.
 */

#ifndef JIUTAI_HASH_H
#define JIUTAI_HASH_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the hash table data type.
 */
typedef void  jf_hashtable_t;

/** Callback function for comparing keys.
 *
 *  @param pKey1 [in] The first key.
 *  @param pKey2 [in] The second key.
 *
 *  @return The result of comparision.
 *  @retval 0 If key1 == key2.
 *  @retval other If key1 != key2.
 */
typedef olint_t (* jf_hashtable_fnCmpKeys_t) (void * pKey1, void * pKey2);

/** Callback function for hashing key to integer.
 *
 *  @param pKey [in] The key.
 *
 *  @return The integer from key.
 */
typedef olint_t (* jf_hashtable_fnHashKey_t) (void * pKey);

/** Callback function for getting key from entry.
 *
 *  @param pEntry [in] The entry.
 *
 *  @return The key.
 */
typedef void * (* jf_hashtable_fnGetKeyFromEntry_t) (void * pEntry);

/** Callback function for freeing entry.
 *
 *  @param ppEntry [in/out] The entry to free.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
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
    /**Callback function of deallocator to free the entry.*/
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
    /**Number of collisions, collision means there are at least 2 entries in the bucket.*/
    u32 jhs_u32Collisions;
    /**Maximum entries per one hash array entry.*/
    u32 jhs_u32MaxEntries;
    /**Count of resize operation.*/
    u32 jhs_u32CountOfResizeOp;
} jf_hashtable_stat_t;

/** Define the hash table iterator.
 *
 *  @note
 *  -# The definition of this structure is placed here because it should be possible to allocate an
 *   iterator as an automatic variable. This is also more convenient for the user.
 */
typedef struct
{
    /**The hash table this iterator attached to.*/
    jf_hashtable_t * jhi_htTable;
    /**The index of the hash array.*/
    olint_t jhi_nPos;
    /**The cursor to the bucket.*/
    void * jhi_pCursor;
} jf_hashtable_iterator_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create hash table.
 *
 *  @param ppjh [out] The hash table to be created and returned.
 *  @param pjhcp [in] The parameter for creating hash table.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hashtable_create(
    jf_hashtable_t ** ppjh, jf_hashtable_create_param_t * pjhcp);

/** Destructor of hash table. Free all stored entry with the user provided deallocation function.
 *
 *  @param ppjh [out] The hash table to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hashtable_destroy(jf_hashtable_t ** ppjh);

/** Insert a entry into the hash table but do not overwrite existing entry with the same key.
 *
 *  @note
 *  -# If the entry with the same key is not existing, the entry is inserted.
 *  -# If the entry with the same key is existing, the old entry is kept and JF_ERR_NO_ERROR is
 *   returned. In this case the user generally has to deallocate the new entry on his own.
 *
 *  @param pjh [in] The hash table.
 *  @param pEntry [in] The entry to be inserted.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hashtable_insertEntry(jf_hashtable_t * pjh, void * pEntry);

/** Remove correponding entry from the hash table and deallocate it using the user provided
 *  deallocator.
 *
 *  @param pjh [in] The hash table.
 *  @param pEntry [in] The entry to be removed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hashtable_removeEntry(jf_hashtable_t * pjh, void * pEntry);

/** Overwrite an existing entry in the hash table with the same key.
 *
 *  @note
 *  -# If the entry with the same key is not existing, the entry is inserted.
 *  -# If the entry with the same key is existing and deallocator is available, the routine will
 *   deallocate the old entry and new entry is inserted.
 *  -# If the entry with the same key is existing and deallocator is not available, the old entry
 *   is replaced by new entry. In this case the user generally has to deallocate the old entry on
 *  his own.
 *
 *  @param pjh [in] The hash table.
 *  @param pEntry [in] The entry to be overwritten.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hashtable_overwriteEntry(jf_hashtable_t * pjh, void * pEntry);

/** Search for entry with this key.
 *
 *  @param pjh [in] The hash table.
 *  @param pKey [in] The key.
 *  @param ppEntry [out] The entry found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_HASH_ENTRY_NOT_FOUND Hash entry is not found.
 */
u32 jf_hashtable_getEntry(jf_hashtable_t * pjh, void * pKey, void ** ppEntry);

/** Check if the entry is in hash table.
 *
 *  @param pjh [in] The hash table.
 *  @param pEntry [in] The entry to be checked.
 *
 *  @return The status.
 *  @retval TRUE The entry is in hash table.
 *  @retval FALSE The entry is not in hash table.
 */
boolean_t jf_hashtable_isEntryInTable(jf_hashtable_t * pjh, void * pEntry);

/** Check if the key is in hash table.
 *
 *  @param pjh [in] The hash table.
 *  @param pKey [in] The key to be checked.
 *
 *  @return The status.
 *  @retval TRUE The entry is in hash table.
 *  @retval FALSE The entry is not in hash table.
 */
boolean_t jf_hashtable_isKeyInTable(jf_hashtable_t * pjh, void * pKey);

/** Return the number of stored key/entry pairs.
 *
 *  @param pjh [in] The hash table.
 *
 *  @return The size of the hash table.
 */
u32 jf_hashtable_getSize(jf_hashtable_t * pjh);

/** Returns an statically allocated string with useful statistical information. This string is
 *  overwritten by subsequent calls to this function.
 *
 *  @param pjh [in] The hash table.
 *  @param stat [out] The statistical information.
 *
 *  @return The error code.
 */
void jf_hashtable_getStat(jf_hashtable_t * pjh, jf_hashtable_stat_t * stat);

/** Setup an iterator. It is possible to have multiple iterators for the same hash table and to
 *  traverse the hash table in parallel.
 *
 *  @note
 *  -# These iterators are not safe with respect to 'remove' operations on the traversed hash table.
 *
 *  @param pjh [in] The hash table.
 *  @param pIterator [out] The iterator to be setup.
 *
 *  @return Void.
 */
void jf_hashtable_setupIterator(jf_hashtable_t * pjh, jf_hashtable_iterator_t * pIterator);

/** Increment an iterator.
 *
 *  @note
 *  -# The cursor is set to NULL if the end of hash table is reached.
 *
 *  @param pIterator [in] The hash table iterator.
 *
 *  @return Void.
 */
void jf_hashtable_incrementIterator(jf_hashtable_iterator_t * pIterator);

/** Get entry from hash table iterator.
 *
 *  @param pIterator [out] The hash table iterator.
 *
 *  @return The entry from iterator.
 *  @retval Entry The cursor in iterator is not NULL.
 *  @retval NULL The cursor in iterator is NULL.
 */
void * jf_hashtable_getEntryFromIterator(jf_hashtable_iterator_t * pIterator);

/** Check if end of iterator is reached.
 *
 *  @param pIterator [out] The hash table iterator.
 *
 *  @return The end status.
 *  @retval TRUE The end of iterator is reached.
 *  @retval FALSE The end of iterator is not reached.
 */
boolean_t jf_hashtable_isEndOfIterator(jf_hashtable_iterator_t * pIterator);

/** Hash function for zero terminated entry (f.e. strings).
 *
 *  @note
 *  -# It is from the 'dragon book' and should work very well especially for strings as keys.
 *
 *  @param pKey [in] The key to hash.
 *
 *  @return The hashed key.
 */
olint_t jf_hashtable_hashPJW(void * pKey);

/** Define bits per long for different architecture.
 */
#ifndef BITS_PER_LONG
    #if defined(JIUFENG_64BIT)
        #define BITS_PER_LONG  64
    #else
        #define BITS_PER_LONG  32
    #endif
#endif

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define JF_HASHTABLE_GOLDEN_RATIO_PRIME_32         (0x9e370001UL)
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define JF_HASHTABLE_GOLDEN_RATIO_PRIME_64         (0x9e37fffffffc0001UL)

/** Define the function to hash long integer.
 */
#if BITS_PER_LONG == 32
    #define jf_hashtable_hashLong(val, bits)       jf_hashtable_hashU32((u32)val, bits)
    #define JF_HASHTABLE_GOLDEN_RATIO_PRIME        JF_HASHTABLE_GOLDEN_RATIO_PRIME_32
#elif BITS_PER_LONG == 64
    #define jf_hashtable_hashLong(val, bits)       jf_hashtable_hashU64((u64)val, bits)
    #define JF_HASHTABLE_GOLDEN_RATIO_PRIME        JF_HASHTABLE_GOLDEN_RATIO_PRIME_64
#else
    #error Wordsize not 32 or 64
#endif

/** Hash function for unsigned long long integer.
 *
 *  @param val [in] The unsigned long long integer.
 *  @param bits [in] The shift bits.
 *
 *  @return The hashed key.
 */
static inline u64 jf_hashtable_hashU64(u64 val, u32 bits)
{
    u64 hash = val;

    /*Sigh, gcc can't optimise this alone like it does for 32 bits.*/
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

    /*High bits are more random, so use them.*/
    return hash >> (64 - bits);
}

/** Hash function for unsigned integer.
 *
 *  @param val [in] The unsigned integer.
 *  @param bits [in] The shift bits.
 *
 *  @return The hashed key.
 */
static inline u32 jf_hashtable_hashU32(u32 val, u32 bits)
{
    /*On some cpus multiply is faster, on others gcc will do shifts.*/
    u32 hash = val * JF_HASHTABLE_GOLDEN_RATIO_PRIME_32;

    /*High bits are more random, so use them.*/
    return hash >> (32 - bits);
}

/** Hash function for pointer.
 *
 *  @param ptr [in] The pointer.
 *  @param bits [in] The shift bits.
 *
 *  @return The hashed key.
 */
static inline unsigned long jf_hashtable_hashPtr(void * ptr, u32 bits)
{
    return jf_hashtable_hashLong(ptr, bits);
}


#endif /*JIUTAI_HASH_H*/

/*------------------------------------------------------------------------------------------------*/
