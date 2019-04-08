/**
 *  @file hash.c
 *
 *  @brief Hash table implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "hash.h"
#include "jf_mem.h"

/* --- private data/data structure section --------------------------------- */
typedef struct hash_table_bucket
{
    void * htb_pEntry;
    struct hash_table_bucket *htb_htbNext;
} hash_table_bucket_t;

typedef struct _hash_table
{
    u32 iht_u32Count;
    u32 iht_u32Size;
    u32 iht_u32Threshold;
    u32 iht_u32Resizes;
    u32 iht_u32PrimesIndex;
    u32 iht_u32Reserved[3];

    hash_table_bucket_t **iht_htbBucket;

    jf_hashtable_fnCmpKeys_t iht_fnCmpKeys;
    jf_hashtable_fnHashKey_t iht_fnHashKey;
    jf_hashtable_fnGetKeyFromEntry_t iht_fnGetKeyFromEntry;
    jf_hashtable_fnFreeEntry_t iht_fnFreeEntry;
    jf_hashtable_fnError_t iht_fnError;
} internal_hash_table_t;

/* let s be the current size and s' the desired next size
 * we request a larger size iff n = 80% s where n is the number of valid nodes.
 * Now we want to have n = 50% s' for not wasting too much space. 
 * This gives the formula
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
static olint_t primes[] = {
    3, 7, 13, 31, 61,
    127, 251, 509, 1021, 2039,
    4093, 8191, 16381, 26209, 41941,
    67121, 107441, 171917, 275083, 440159,
    704269, 1126831, 1802989, 2884787, 4615661,
    7385057, 11816111, 18905813, 30249367, 48399019,
    77438437, 123901501, 198242441, 317187907, 507500657,
    812001067, 1299201731, 2078722769
};

/* --- private routine section---------------------------------------------- */
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

static void _default_fnHtError(u8 * errmsg)
{
    fprintf(stderr, "ERROR: %s\n", errmsg);
    fflush(stderr);
//    kill(0, SIGSEGV);
}

static hash_table_bucket_t ** _getPositionOfKey(
    internal_hash_table_t * piht, void * pKey)
{
    olint_t h = piht->iht_fnHashKey(pKey);
    hash_table_bucket_t **p;
    jf_hashtable_fnCmpKeys_t fnCmpKeys = piht->iht_fnCmpKeys;
    jf_hashtable_fnGetKeyFromEntry_t fnGetKeyFromEntry = piht->iht_fnGetKeyFromEntry;

    if (h < 0)
        h = -h;
    h %= piht->iht_u32Size;
    p = &(piht->iht_htbBucket[h]);

    while (*p != NULL)
    {
        if (fnCmpKeys(pKey, fnGetKeyFromEntry((*p)->htb_pEntry)) == 0)
        {
            return p;
        }
        else
            p = &((*p)->htb_htbNext);
    }

    return p;
}

static u32 _resizeHashTable(internal_hash_table_t ** ppiht)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t *newIht;
    olint_t i;
    internal_hash_table_t * piht = *ppiht;
    jf_hashtable_fnGetKeyFromEntry_t fnGetKeyFromEntry =
        piht->iht_fnGetKeyFromEntry;
    jf_hashtable_create_param_t jhcp;

    memset(&jhcp, 0, sizeof(jf_hashtable_create_param_t));

    jhcp.jhcp_u32MinSize = primes[piht->iht_u32PrimesIndex + 1];
    jhcp.jhcp_fnCmpKeys = piht->iht_fnCmpKeys;
    jhcp.jhcp_fnHashKey = piht->iht_fnHashKey;
    jhcp.jhcp_fnGetKeyFromEntry = piht->iht_fnGetKeyFromEntry;
    jhcp.jhcp_fnFreeEntry = piht->iht_fnFreeEntry;
    jhcp.jhcp_fnError = piht->iht_fnError;

    u32Ret = jf_hashtable_create((jf_hashtable_t **)&newIht, &jhcp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < piht->iht_u32Size; i++)
        {
            hash_table_bucket_t *p, *tmp;

            for (p = piht->iht_htbBucket[i]; p; p = tmp)
            {
                hash_table_bucket_t **position = (hash_table_bucket_t **)
                    _getPositionOfKey(newIht, fnGetKeyFromEntry(p->htb_pEntry));
                tmp = p->htb_htbNext;
                p->htb_htbNext = *position;
                *position = p;
                newIht->iht_u32Count++;
            }
        }

        free(piht->iht_htbBucket);
        free(*ppiht);
        newIht->iht_u32Resizes = piht->iht_u32Resizes + 1;
        **ppiht = *newIht;
    }

    return u32Ret;
}

static u32 _insertAtPosition(
    internal_hash_table_t * piht,
    hash_table_bucket_t ** ppPosition, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_table_bucket_t * phtb, ** position = ppPosition;

    assert(ppPosition != NULL);

    if (piht->iht_u32Count >= piht->iht_u32Threshold)
    {
        _resizeHashTable(&piht);
        position = (hash_table_bucket_t **)
            _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));
    }

    u32Ret = jf_mem_alloc((void **)&phtb, sizeof(hash_table_bucket_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(phtb, 0, sizeof(hash_table_bucket_t));

        phtb->htb_pEntry = pEntry;
        phtb->htb_htbNext = *position;
        *position = phtb;
        piht->iht_u32Count++;
    }

    return u32Ret;
}

static u32 _overwriteAtPosition(internal_hash_table_t * piht,
    hash_table_bucket_t ** ppPosition, void * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hash_table_bucket_t * tmp;

    assert(ppPosition != NULL);

    tmp = *ppPosition;

    (piht->iht_fnFreeEntry) (tmp->htb_pEntry);
    tmp->htb_pEntry = pEntry;

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_hashtable_create(
    jf_hashtable_t ** ppht, jf_hashtable_create_param_t * pjhcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hash_table_t *piht;
    olint_t u32PrimesIndex = 0;

    assert((ppht != NULL) && (pjhcp != NULL));

    u32Ret = jf_mem_alloc((void **)&piht, sizeof(internal_hash_table_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(piht, 0, sizeof(internal_hash_table_t));

        while (pjhcp->jhcp_u32MinSize > primes[u32PrimesIndex])
            u32PrimesIndex++;

        piht->iht_u32PrimesIndex = u32PrimesIndex;
        piht->iht_u32Size = primes[u32PrimesIndex];
        piht->iht_u32Count = 0;

        piht->iht_fnCmpKeys = pjhcp->jhcp_fnCmpKeys ? pjhcp->jhcp_fnCmpKeys :
            _default_fnHtCmpKeys;
        piht->iht_fnHashKey = pjhcp->jhcp_fnHashKey ? pjhcp->jhcp_fnHashKey :
            _default_fnHtHashKey;
        piht->iht_fnGetKeyFromEntry = pjhcp->jhcp_fnGetKeyFromEntry ?
            pjhcp->jhcp_fnGetKeyFromEntry : _default_fnHtGetKeyFromEntry;
        piht->iht_fnFreeEntry = pjhcp->jhcp_fnFreeEntry ?
            pjhcp->jhcp_fnFreeEntry : _default_fnHtFreeEntry;
        piht->iht_fnError = pjhcp->jhcp_fnError ? pjhcp->jhcp_fnError :
            _default_fnHtError;

        /* ceil(0.8 * piht -> iht_u32Size) */
        piht->iht_u32Threshold = (((piht->iht_u32Size) << 2) + 4) / 5;
        piht->iht_u32Resizes = 0;

        u32Ret = jf_mem_alloc((void **)&(piht->iht_htbBucket),
            piht->iht_u32Size * sizeof(hash_table_bucket_t *));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(piht->iht_htbBucket, 0,
            piht->iht_u32Size * sizeof(hash_table_bucket_t *));

        *ppht = piht;
    }
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

    if (piht->iht_htbBucket != NULL)
    {
        for (i = 0; i < piht->iht_u32Size; i++)
        {
            hash_table_bucket_t * phtb, * tmp;

            for (phtb = piht->iht_htbBucket[i]; phtb != NULL; phtb = tmp)
            {
                tmp = phtb->htb_htbNext;
                fnFreeEntry(&(phtb->htb_pEntry));
                jf_mem_free((void **)&phtb);
            }
        }

        jf_mem_free((void **)&(piht->iht_htbBucket));
    }

    jf_mem_free((void **)ppht);

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

    if ((tmp = *position))
    {
        *position = tmp->htb_htbNext;
        piht->iht_fnFreeEntry(tmp->htb_pEntry);
        jf_mem_free((void **)&tmp);
    }
    else
        u32Ret = JF_ERR_HASH_ENTRY_NOT_FOUND;

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
    hash_table_bucket_t **position = _getPositionOfKey(piht, pKey);

    if (*position != NULL)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_hashtable_isEntryInTable(jf_hashtable_t * pht, void * pEntry)
{
    boolean_t bRet = FALSE;
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    hash_table_bucket_t **position =
        _getPositionOfKey(piht, (piht->iht_fnGetKeyFromEntry) (pEntry));

    if (*position != NULL)
        bRet = TRUE;

    return bRet;
}

u32 jf_hashtable_getSize(jf_hashtable_t * pht)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;

    return piht->iht_u32Count;  /* do not mess up with piht->iht_u32Size ! */
}

void jf_hashtable_showStatistics(jf_hashtable_t * pht)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;
    olchar_t statistics[200];
    olint_t collisions = 0, maxchain = 0, i;

    for (i = 0; i < piht->iht_u32Size; i++)
    {
        hash_table_bucket_t *p = piht->iht_htbBucket[i];

        if (p)
        {
            olint_t j = 1;

            for (p = p->htb_htbNext; p != NULL; p = p->htb_htbNext)
                j++;

            if (maxchain < j)
                maxchain = j;
            if (j > 1)
                collisions += j - 1;
        }
    }

    ol_sprintf(statistics,
            "elements=%d size=%d resizes=%d collisions=%d maxchain=%d",
            piht->iht_u32Count, piht->iht_u32Size, piht->iht_u32Resizes,
            collisions, maxchain);

    ol_printf("statistics: %s\n", statistics);
}

/**
 *  iterator routines
 */

void jf_hashtable_setupIterator(
    jf_hashtable_t * pht, jf_hashtable_iterator_t * pIterator)
{
    internal_hash_table_t * piht = (internal_hash_table_t *)pht;

    pIterator->jhi_htTable = piht;
    pIterator->jhi_nPos = -1;   /* have a look at ht_increment_iterator */
    pIterator->jhi_pCursor = 0;
    jf_hashtable_incrementIterator(pIterator);
}

void jf_hashtable_incrementIterator(jf_hashtable_iterator_t * pIterator)
{
    hash_table_bucket_t * current =
        (hash_table_bucket_t *)pIterator->jhi_pCursor;

    if (current && current->htb_htbNext)
    {
        pIterator->jhi_pCursor = current->htb_htbNext;
    }
    else
    {
        olint_t i = pIterator->jhi_nPos + 1;
        u32 sz =
            ((internal_hash_table_t *)(pIterator->jhi_htTable))->iht_u32Size;
        hash_table_bucket_t ** table =
            ((internal_hash_table_t *)(pIterator->jhi_htTable))->iht_htbBucket;

        while (i < sz)
        {
            if (table[i] != NULL)
            {
                pIterator->jhi_nPos = i;
                pIterator->jhi_pCursor = table[i];
                return;
            }
            else
                i++;
        }

        pIterator->jhi_pCursor = 0;
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
    hash_table_bucket_t * bucket =
        (hash_table_bucket_t *)pIterator->jhi_pCursor;

    if (bucket == NULL)
        return NULL;
    else
        return bucket->htb_pEntry;
}

/**
 *  the following function is the famous hash function of P.J. Weinberger as 
 *  desribed in the dragon book of A.V. Aho, R. Sethi and J.D. Ullman.
 */

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


/*--------------------------------------------------------------------------*/

