/**
 *  @file hash.h
 *
 *  @brief Header file for hash common object.
 *
 *  @author Min Zhang
 *
 *  @note This is an implementation of a general hash table module. It assumes
 *   that keys for entry stored in the hash table can be extracted from the
 *   entry.
 *  @note Link with xmalloc object file
 */

/*--------------------------------------------------------------------------*/
#ifndef JIUTAI_HASH_H
#define JIUTAI_HASH_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */
typedef void  hash_table_t;

typedef olint_t (* fnHtCmpKeys_t) (void * pKey1, void * pKey2);
typedef olint_t (* fnHtHashKey_t) (void * pKey);
typedef void * (* fnHtGetKeyFromEntry_t) (void * pEntry);
typedef u32 (* fnHtFreeEntry_t) (void ** ppEntry);
typedef void (* fnHtError_t) (u8 * pu8Errmsg);

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u32 htp_u32MinSize; /*minimal number of entry by estimation*/
    u32 htp_u32Reserved;
    fnHtCmpKeys_t htp_fnHtCmpKeys;
    fnHtHashKey_t htp_fnHtHashKey;
    fnHtGetKeyFromEntry_t htp_fnHtGetKeyFromEntry;
    fnHtFreeEntry_t htp_fnHtFreeEntry;
    fnHtError_t htp_fnHtError;
    u32 htp_u32Reserved2[4];
} hash_table_param_t;

/* --- functional routines ------------------------------------------------- */

u32 createHashTable(hash_table_t ** ppht, hash_table_param_t * phtp);

/** Destructor of HashTables. Free all stored entry with the user provided
 *  deallocation function.
 */
u32 destroyHashTable(hash_table_t ** pht);

/** Insert a entry into the HashTable but do not overwrite existing entry with
 *  the same key. The return value is JF_ERR_NO_ERROR if some entry with the same
 *  key is already stored in the hash table. In this case the user generally has
 *  to deallocate entry on his own.
 */
u32 insertHashTableEntry(hash_table_t * pht, void * pEntry);

/** Remove correponding entry from the hash table and deallocate it using the
 *  user provided deallocator.
 */
u32 removeHashTableEntry(hash_table_t * pht, void * pEntry);

/** Overwrite an existing entry in the hash table with the same key as the key
 *  of entry or insert a new entry.
 *  In the first case also deallocate the overwritten entry.
 */
u32 overwriteHashTableEntry(hash_table_t * pht, void * pEntry);

/** Search for entry with this key. Return error code if not found.
 */
u32 getHashTableEntry(hash_table_t * pht, void * pKey, void ** ppEntry);

boolean_t isEntryInHashTable(hash_table_t * pht, void * pEntry);

boolean_t isKeyInHashTable(hash_table_t * pht, void * pKey);

/** Return the number of stored key/entry pairs
 */
u32 getHashTableSize(hash_table_t * pht);

/** Returns an statically allocated string with useful statistical information.
 *  This string is overwritten by subsequent calls to this function.
 */
void showHashTableStatistics(hash_table_t * pht);

/** The definition of this structure is placed here because it should be
 *  possible to allocate an iterator as an automatic variable. This is also more
 *  convenient for the user.
 */
typedef struct
{
    hash_table_t * hti_htTable;
    olint_t hti_nPos;
    void * hti_pCursor;
} hash_table_iterator_t;

/** Setup an iterator. It is possible to have multiple iterators for the same
 *  hash table and to traverse the hash table in parallel.
 *  ATTENTION: These iterators are not save with respect to 'remove' operations
 * on the traversed hash table.
 */
void setupHashTableIterator(hash_table_t * pht,
    hash_table_iterator_t * pIterator);

void incrementHashTableIterator(hash_table_iterator_t * pIterator);

void * getEntryFromHashTableIterator(hash_table_iterator_t * pIterator);

boolean_t isDoneHashTableIterator(hash_table_iterator_t * pIterator);

/** Hash function for zero terminated entry (f.e. strings).
 *  It is from the `dragon book' and should work very well especially for strings
 *  as keys.
 */
olint_t hashPJW(void * pKey);


#ifndef BITS_PER_LONG
#define BITS_PER_LONG  64
#endif

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

#if BITS_PER_LONG == 32
    #define GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_32
    #define hashLong(val, bits) hashU32(val, bits)
#elif BITS_PER_LONG == 64
    #define hashLong(val, bits) hashU64(val, bits)
    #define GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_64
#else
    #error Wordsize not 32 or 64
#endif


static inline u64 hashU64(u64 val, u32 bits)
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

static inline u32 hashU32(u32 val, u32 bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    u32 hash = val * GOLDEN_RATIO_PRIME_32;

    /* High bits are more random, so use them. */
    return hash >> (32 - bits);
}

static inline unsigned long hashPtr(void *ptr, u32 bits)
{
    return hashLong((unsigned long)ptr, bits);
}


#endif /*JIUTAI_HASH_H*/

/*--------------------------------------------------------------------------*/

