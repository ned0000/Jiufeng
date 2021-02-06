/**
 *  @file jf_hashtable.c
 *
 *  @brief Implementation files for hash table.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The position in the hash array is not the key. Different keys may at the same position.
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_hashtable.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the hash table bucket data type.
 */
typedef struct hash_table_bucket
{
    /**Entry from user.*/
    void * htb_pEntry;
    /**Next bucket in the list.*/
    struct hash_table_bucket * htb_phtbNext;
} hash_table_bucket_t;

/** Define the internal hash table data type.
 */
typedef struct _hash_table
{
    /**Number of entry in the table.*/
    u32 iht_u32NumOfEntry;
    /**Size of the table.*/
    u32 iht_u32Size;
    /**Threshold for resizing table.*/
    u32 iht_u32Threshold;
    /**Number of resize.*/
    u32 iht_u32Resizes;
    /**Prime index in the prime array.*/
    u32 iht_u32PrimesIndex;
    u32 iht_u32Reserved[3];

    /*The hash array for buckets.*/
    hash_table_bucket_t ** iht_phtbBucket;

    /**Callback function for comparing keys.*/
    jf_hashtable_fnCmpKeys_t iht_fnCmpKeys;
    /**Callback function for hashing key to integer.*/
    jf_hashtable_fnHashKey_t iht_fnHashKey;
    /**Callback function for getting key from entry.*/
    jf_hashtable_fnGetKeyFromEntry_t iht_fnGetKeyFromEntry;
    /**Callback function for freeing entry.*/
    jf_hashtable_fnFreeEntry_t iht_fnFreeEntry;
} internal_hash_table_t;

/* Let s be the current size and s' is the desired next size.
 * We request a larger size if n = 80% * s where n is the number of valid nodes.
 * Now we want to have n = 50% * s' for not wasting too much space, this gives the formula:
 *
 *    s' = (8/5) s
 *
 * the primes can be generated with the following awk program:
 *
 *    BEGIN {
 *      P = 3; M = 2147483647   # 2**31
 *      while(P<M) {
 *        print P ",";
 *        P = int(8*P/5); "primes " P | getline; P = $1
 *      }
 *    }
 */
static olint_t ls_nHashTablePrimes[] = {
    3, 7, 13, 31, 61,
    127, 251, 509, 1021, 2039,
    4093, 8191, 16381, 26209, 41941,
    67121, 107441, 171917, 275083, 440159,
    704269, 1126831, 1802989, 2884787, 4615661,
    7385057, 11816111, 18905813, 30249367, 48399019,
    77438437, 123901501, 198242441, 317187907, 507500657,
    812001067, 1299201731, 2078722769
};

/* --- private routine section ------------------------------------------------------------------ */

static olint_t _default_fnHtCmpKeys(void * pKey1, void * pKey2)
{
    if (pKey1 == pKey2)
        return 0;
    else if (pKey1 < pKey2)
        return 1;
    else
        return -1;
}

static olint_t _default_fnHtHashKey(void * pKey)
{
#if defined(JIUFENG_64BIT)
    return (int)(u64)pKey;
#else
    return (int) pKey;
#endif
}

static void * _default_fnHtGetKeyFromEntry(void * pEntry)
{
    return pEntry;
}

static u32 _default_fnHtFreeEntry(void ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static hash_table_bucket_t ** _getPositionOfKey(internal_hash_table_t * piht, void * pKey)
{
    olint_t h = piht->iht_fnHashKey(pKey);
    hash_table_bucket_t ** p = NULL;
    jf_hashtable_fnCmpKeys_t fnCmpKeys = piht->iht_fnCmpKeys;
    jf_hashtable_fnGetKeyFromEntry_t fnGetKeyFromEntry = piht->iht_fnGetKeyFromEntry;

    /*Locate hash array entry.*/
    if (h < 0)
        h = -h;
    h %= piht->iht_u32Size;
    p = &(piht->iht_phtbBucket[h]);

    /*Iterate through the buckets.*/
    while (*p != NULL)
    {
        if (fnCmpKeys(pKey, fnGetKeyFromEntry((*p)->htb_pEntry)) == 0)
        {
            return p;
        }
        else
        {
            p = &((*p)->htb_phtbNext);
        }
    }

    return p;
}

static u32 _createHashTableArray(internal_hash_table_t * piht)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_jiukun_allocMemory(
        (void **)&piht->iht_phtbBucket, piht->iht_u32Size * sizeof(hash_table_bucket_t *));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(piht->iht_phtbBucket, piht->iht_u32Size * sizeof(hash_table_bucket_t *));
    }

    return u32Ret;
}

static void _setHashTableThreshold(internal_hash_table_t * piht)
{
    /* ceil(0.8 * piht->iht_u32Size) */
    piht->iht_u32Threshold = (((piht->iht_u32Size) << 2) + 4) / 5;
}

static u32 _resizeHashTable(internal_hash_table_t * piht)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i = 0;
    jf_hashtable_fnGetKeyFromEntry_t fnGetKeyFromEntry = piht->iht_fnGetKeyFromEntry;
    /*Save the hash array and size.*/
    u32 u32Size = piht->iht_u32Size;
    hash_table_bucket_t ** phtbBucket = piht->iht_phtbBucket;

    /*Set the new size.*/
    piht->iht_u32Size = ls_nHashTablePrimes[piht->iht_u32PrimesIndex + 1];

    /*Create a new hash array with new size from prime array.*/
    u32Ret = _createHashTableArray(piht);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Add all buckets in the saved hash array to new hash table.*/
        for (i = 0; i < u32Size; i++)
        {
            hash_table_bucket_t * p = NULL, * tmp = NULL;

            for (p = phtbBucket[i]; p != NULL; p = tmp)
            {
                hash_table_bucket_t **position = (hash_table_bucket_t **)
                    _getPositionOfKey(piht, fnGetKeyFromEntry(p->htb_pEntry));

                tmp = p->htb_phtbNext;

                p->htb_phtbNext = NULL;
                *position = p;
            }
        }

        /*Update hash table.*/
        piht->iht_u32PrimesIndex ++;
        _setHashTableThreshold(piht);

        /*Collect statistics.*/
        piht->iht_u32Resizes ++;

        /*Free the old hash array.*/
        jf_jiukun_freeMemory((void **)&phtbBucket);
    }
    else
    {
        /*Restore the saved values if error.*/
        piht->iht_u32Size = u32Size;
        piht->iht_phtbBucket = phtbBucket;
    }

    return u32Ret;
}

static u32 _insertAtPosition(
    internal_hash_table_t * piht, hash_table_bucket_t ** ppPosition, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_table_bucket_t * phtb = NULL, ** position = ppPosition;

    assert(ppPosition != NULL);

    /*Check if resize is required.*/
    if (piht->iht_u32NumOfEntry >= piht->iht_u32Threshold)
    {
        u32Ret = _resizeHashTable(piht);

        /*Need to get position again after resize.*/
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            position = _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));
        }
    }

    /*Allocate memory for the new entry.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_jiukun_allocMemory((void **)&phtb, sizeof(hash_table_bucket_t));
    }

    /*Initialize and insert the new entry.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(phtb, sizeof(hash_table_bucket_t));

        phtb->htb_pEntry = pEntry;
        phtb->htb_phtbNext = *position;
        *position = phtb;
        piht->iht_u32NumOfEntry ++;
    }

    return u32Ret;
}

static u32 _overwriteAtPosition(
    internal_hash_table_t * piht, hash_table_bucket_t ** ppPosition, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_table_bucket_t * tmp = NULL;

    assert(ppPosition != NULL);

    tmp = *ppPosition;

    (piht->iht_fnFreeEntry) (tmp->htb_pEntry);
    tmp->htb_pEntry = pEntry;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_hashtable_create(jf_hashtable_t ** ppht, jf_hashtable_create_param_t * pjhcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht = NULL;
    olint_t u32PrimesIndex = 0;

    assert((ppht != NULL) && (pjhcp != NULL));

    /*Allocate memory for internal hash table.*/
    u32Ret = jf_jiukun_allocMemory((void **)&piht, sizeof(internal_hash_table_t));

    /*Initialize the internal hash table.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(piht, 0, sizeof(internal_hash_table_t));

        /*Get the prime index.*/
        while (pjhcp->jhcp_u32MinSize > ls_nHashTablePrimes[u32PrimesIndex])
            u32PrimesIndex++;

        piht->iht_u32PrimesIndex = u32PrimesIndex;
        piht->iht_u32Size = ls_nHashTablePrimes[u32PrimesIndex];

        piht->iht_fnCmpKeys = pjhcp->jhcp_fnCmpKeys ? pjhcp->jhcp_fnCmpKeys : _default_fnHtCmpKeys;
        piht->iht_fnHashKey = pjhcp->jhcp_fnHashKey ? pjhcp->jhcp_fnHashKey : _default_fnHtHashKey;
        piht->iht_fnGetKeyFromEntry = pjhcp->jhcp_fnGetKeyFromEntry ?
            pjhcp->jhcp_fnGetKeyFromEntry : _default_fnHtGetKeyFromEntry;
        piht->iht_fnFreeEntry =
            pjhcp->jhcp_fnFreeEntry ? pjhcp->jhcp_fnFreeEntry : _default_fnHtFreeEntry;

        _setHashTableThreshold(piht);

        u32Ret = _createHashTableArray(piht);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppht = piht;
    else if (piht != NULL)
        jf_hashtable_destroy((jf_hashtable_t **)&piht);

    return u32Ret;
}

u32 jf_hashtable_destroy(jf_hashtable_t ** ppht)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht;
    olint_t i;
    jf_hashtable_fnFreeEntry_t fnFreeEntry;

    assert((ppht != NULL) && (*ppht != NULL));

    piht = (internal_hash_table_t *)*ppht;
    fnFreeEntry = piht->iht_fnFreeEntry;

    if (piht->iht_phtbBucket != NULL)
    {
        for (i = 0; i < piht->iht_u32Size; i++)
        {
            hash_table_bucket_t * phtb, * tmp;

            for (phtb = piht->iht_phtbBucket[i]; phtb != NULL; phtb = tmp)
            {
                tmp = phtb->htb_phtbNext;
                fnFreeEntry(&(phtb->htb_pEntry));
                jf_jiukun_freeMemory((void **)&phtb);
            }
        }

        jf_jiukun_freeMemory((void **)&(piht->iht_phtbBucket));
    }

    jf_jiukun_freeMemory((void **)ppht);

    return u32Ret;
}

u32 jf_hashtable_insertEntry(jf_hashtable_t * pht, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t **position =
        _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));

    if (*position == NULL)
        u32Ret = _insertAtPosition(pht, position, pEntry);

    return u32Ret;
}

u32 jf_hashtable_removeEntry(jf_hashtable_t * pht, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t * tmp;
    hash_table_bucket_t ** position =
        _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));

    tmp = *position;
    if (tmp != NULL)
    {
        *position = tmp->htb_phtbNext;
        piht->iht_fnFreeEntry(tmp->htb_pEntry);
        jf_jiukun_freeMemory((void **)&tmp);
        piht->iht_u32NumOfEntry --;
    }
    else
    {
        u32Ret = JF_ERR_HASH_ENTRY_NOT_FOUND;
    }

    return u32Ret;
}

u32 jf_hashtable_overwriteEntry(jf_hashtable_t * pht, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t ** position =
        _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));

    if (*position)
    {
        u32Ret = _overwriteAtPosition(piht, position, pEntry);
    }
    else
    {
        u32Ret = _insertAtPosition(piht, position, pEntry);
    }

    return u32Ret;
}

u32 jf_hashtable_getEntry(jf_hashtable_t * pht, void * pKey, void ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t ** bucket = _getPositionOfKey(piht, pKey);

    if ((*bucket) == NULL)
        u32Ret = JF_ERR_HASH_ENTRY_NOT_FOUND;
    else
        *ppEntry = (*bucket)->htb_pEntry;

    return u32Ret;
}

boolean_t jf_hashtable_isKeyInTable(jf_hashtable_t * pht, void * pKey)
{
    boolean_t bRet = FALSE;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t ** position = _getPositionOfKey(piht, pKey);

    if (*position != NULL)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_hashtable_isEntryInTable(jf_hashtable_t * pht, void * pEntry)
{
    boolean_t bRet = FALSE;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t ** position =
        _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));

    if (*position != NULL)
        bRet = TRUE;

    return bRet;
}

u32 jf_hashtable_getSize(jf_hashtable_t * pht)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;

    return piht->iht_u32NumOfEntry;  /*Do not mess up with piht->iht_u32Size !*/
}

void jf_hashtable_getStat(jf_hashtable_t * pht, jf_hashtable_stat_t * stat)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    olint_t collisions = 0, maxentries = 0, i = 0;

    for (i = 0; i < piht->iht_u32Size; i++)
    {
        hash_table_bucket_t *p = piht->iht_phtbBucket[i];

        if (p)
        {
            olint_t j = 1;

            /*Count number of entry in hash array entry, from the second entry.*/
            for (p = p->htb_phtbNext; p != NULL; p = p->htb_phtbNext)
                j++;

            /*Save maximum entries per one hash array entry.*/
            if (maxentries < j)
                maxentries = j;

            /*Suppose only 1 bucket per one hash array entry, others are collisions.*/
            if (j > 1)
                collisions += j - 1;
        }
    }

    stat->jhs_u32NumOfEntry = piht->iht_u32NumOfEntry;
    stat->jhs_u32Size = piht->iht_u32Size;
    stat->jhs_u32Collisions = collisions;
    stat->jhs_u32MaxEntries = maxentries;
    stat->jhs_u32CountOfResizeOp = piht->iht_u32Resizes;
}

/* iterator routines */

void jf_hashtable_setupIterator(
    jf_hashtable_t * pht, jf_hashtable_iterator_t * pIterator)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;

    pIterator->jhi_htTable = piht;
    pIterator->jhi_nPos = -1;
    pIterator->jhi_pCursor = NULL;
    jf_hashtable_incrementIterator(pIterator);
}

void jf_hashtable_incrementIterator(jf_hashtable_iterator_t * pIterator)
{
    hash_table_bucket_t * current = (hash_table_bucket_t *)pIterator->jhi_pCursor;

    /*Check the next bucket.*/
    if ((current != NULL) && (current->htb_phtbNext != NULL))
    {
        /*Next bucket is available.*/
        pIterator->jhi_pCursor = current->htb_phtbNext;
    }
    else
    {
        /*Next bucket is not available. Let's move to the next bucket in the array.*/
        olint_t i = pIterator->jhi_nPos + 1;
        u32 sz = ((internal_hash_table_t *)(pIterator->jhi_htTable))->iht_u32Size;
        hash_table_bucket_t ** table =
            ((internal_hash_table_t *)(pIterator->jhi_htTable))->iht_phtbBucket;

        while (i < sz)
        {
            if (table[i] != NULL)
            {
                /*Find a bucket.*/
                pIterator->jhi_nPos = i;
                pIterator->jhi_pCursor = table[i];
                return;
            }
            else
            {
                i++;
            }
        }

        /*No more bucket.*/
        pIterator->jhi_pCursor = NULL;
        pIterator->jhi_nPos = sz;
    }

    return;
}

boolean_t jf_hashtable_isEndOfIterator(jf_hashtable_iterator_t * pIterator)
{
    boolean_t bRet = FALSE;

    if (pIterator->jhi_pCursor == NULL)
        bRet = TRUE;

    return bRet;
}

void * jf_hashtable_getEntryFromIterator(jf_hashtable_iterator_t * pIterator)
{
    hash_table_bucket_t * bucket = (hash_table_bucket_t *)pIterator->jhi_pCursor;

    if (bucket == NULL)
        return NULL;
    else
        return bucket->htb_pEntry;
}

#define HASHPJWSHIFT  ((sizeof(int)) * 8 - 4)

olint_t jf_hashtable_hashPJW(void * pKey)
{
    const olchar_t *s = (olchar_t *) pKey, *p;
    olint_t h = 0, g;

    for (p = s; *p != '\0'; p++)
    {
        h = (h << 4) + *p;
        if ((g = h & (0xf << HASHPJWSHIFT)))
        {
            h = h ^ (g >> HASHPJWSHIFT);
            h = h ^ g;
        }
    }

    return h > 0 ? h : -h;  // no modulo but positive
}

/*------------------------------------------------------------------------------------------------*/
