/**
 *  @file prng.c
 *
 *  @brief Implementation file for pseudo random number generator
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#if defined(LINUX)
    #include <sys/time.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "prng.h"
#include "cghash.h"
#include "errcode.h"
#include "xmalloc.h"
#include "process.h"
#include "xtime.h"
#include "process.h"
#include "prngcommon.h"
#include "seed.h"
#include "syncmutex.h"

/* --- private data/data structure section --------------------------------- */

#define PRNG_POOL_SIZE    (1024)

typedef struct
{
    boolean_t ip_bInitialized;
    boolean_t ip_bPoolStirred;
    u8 ip_u8Reserved[6];

    sync_mutex_t ip_smLock;

    u32 ip_u32PoolSize;
    u32 ip_u32PoolIndex;
    u8 ip_u8Pool[PRNG_POOL_SIZE];
    u8 ip_u8Reserved2;

    u8 ip_u8Md[MD5_DIGEST_LEN];
    u32 ip_u32NumOfGet;
    oldouble_t ip_dbEntropy;

} internal_prng_t;

static internal_prng_t ls_ipPrng;

/* --- private routine section---------------------------------------------- */

static u32 _getPrngData(internal_prng_t * pip, u8 * pu8Data, u32 u32Len)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, j, k;
    u32 u32Idx;
    u32 u32Count, u32PoolSize;
    boolean_t bEntropy = FALSE;
    u8 local_md[MD5_DIGEST_LEN];
    pid_t curr_pid = getCurrentProcessId();
    md5_t md5;

    assert((pu8Data != NULL) && (u32Len != 0));

    u32Count = ALIGN(u32Len, MD5_DIGEST_LEN / 2);

    if (pip->ip_dbEntropy >= ENTROPY_NEEDED)
        bEntropy = TRUE;

    if (! bEntropy)
    {
        /*entropy is not enough*/
        pip->ip_dbEntropy -= u32Len;
        if (pip->ip_dbEntropy < 0)
            pip->ip_dbEntropy = 0;
    }

    if (! pip->ip_bPoolStirred)
    {
        /* stir all data in pool */
        u32PoolSize = PRNG_POOL_SIZE;
        while ((u32PoolSize > 0) && (u32Ret == OLERR_NO_ERROR))
        {
            /* At least MD5_DIGEST_LEN */
#define DUMMY_SEED "...................."
            u32Ret = seedPrng((u8 *)DUMMY_SEED, MD5_DIGEST_LEN, 0.0);
            if (u32Ret == OLERR_NO_ERROR)
                u32PoolSize -= MD5_DIGEST_LEN;
        }
        if (bEntropy)
            pip->ip_bPoolStirred = TRUE;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Idx = pip->ip_u32PoolIndex;
        memcpy(local_md, pip->ip_u8Md, sizeof(pip->ip_u8Md));

        pip->ip_u32PoolIndex += u32Count;
        if (pip->ip_u32PoolIndex > pip->ip_u32PoolSize)
            pip->ip_u32PoolIndex %= pip->ip_u32PoolSize;

        /* increase the number of get operation */
        pip->ip_u32NumOfGet += 1;

        initMd5(&md5);
    }

    while ((u32Len > 0) && (u32Ret == OLERR_NO_ERROR))
    {
        j = (u32Len >= MD5_DIGEST_LEN / 2) ? MD5_DIGEST_LEN / 2 : u32Len;
        u32Len -= j;

        if (curr_pid)
        {
            /* just in the first iteration to save time */
            updateMd5(&md5, (u8 *)&curr_pid, sizeof(curr_pid));
            curr_pid = 0;
        }

        updateMd5(&md5, local_md, MD5_DIGEST_LEN);
        updateMd5(&md5, (u8 *)&pip->ip_u32NumOfGet, sizeof(pip->ip_u32NumOfGet));
        /* include the data in parameter from caller */
        updateMd5(&md5, pu8Data, j);

        k = (u32Idx + MD5_DIGEST_LEN / 2) - pip->ip_u32PoolSize;
        if (k > 0)
        {
            updateMd5(
                &md5, &(pip->ip_u8Pool[u32Idx]), MD5_DIGEST_LEN / 2 - k);
            updateMd5(&md5, &(pip->ip_u8Pool[0]), k);
        }
        else
        {
            updateMd5(&md5, &(pip->ip_u8Pool[u32Idx]), MD5_DIGEST_LEN / 2);
        }
        finalMd5(&md5, local_md);

        for (i = 0; i < MD5_DIGEST_LEN / 2; i ++)
        {
            /* put the first part of message digest into pool, return the last
               part of message digest to caller */
            pip->ip_u8Pool[u32Idx++] ^= local_md[i];
            if (u32Idx >= pip->ip_u32PoolSize)
                u32Idx=0;
            if (i < j)
                *(pu8Data++) = local_md[i + MD5_DIGEST_LEN / 2];
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        updateMd5(&md5, (u8 *)&pip->ip_u32NumOfGet, sizeof(pip->ip_u32NumOfGet));
        updateMd5(&md5, local_md, MD5_DIGEST_LEN);
        updateMd5(&md5, pip->ip_u8Md, MD5_DIGEST_LEN);
        finalMd5(&md5, pip->ip_u8Md);

        if (! bEntropy)
        {
            /* prng is not seeded and entropy is not enough, the random data
             * are pseudo
             */
            u32Ret = OLERR_PRNG_NOT_SEEDED;
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 initPrng(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_prng_t * pip = &ls_ipPrng;

    assert(! pip->ip_bInitialized);

    logInfoMsg("init prng");

    /** Get seed */
    u32Ret = getSeed();
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&(pip->ip_smLock));

    if (u32Ret == OLERR_NO_ERROR)
        pip->ip_bInitialized = TRUE;
    else if (pip != NULL)
        finiPrng();

    return u32Ret;
}

u32 finiPrng(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_prng_t * pip = &ls_ipPrng;

    finiSyncMutex(&(pip->ip_smLock));

    pip->ip_bInitialized = FALSE;

    return u32Ret;
}

u32 getPrngData(u8 * pu8Data, u32 u32Len)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_prng_t * pip = &ls_ipPrng;

    assert((pu8Data != NULL) && (u32Len > 0));

    acquireSyncMutex(&(pip->ip_smLock));

    u32Ret = _getPrngData(pip, pu8Data, u32Len);

    releaseSyncMutex(&(pip->ip_smLock));

    return u32Ret;
}

u32 getPrngPseudoData(u8 * pu8Data, u32 u32Len)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = getPrngData(pu8Data, u32Len);
    if (u32Ret == OLERR_PRNG_NOT_SEEDED)
        u32Ret = OLERR_NO_ERROR;

    return u32Ret;
}

u32 seedPrng(const u8 * pu8Data, olint_t u32Len, oldouble_t dbEntropy)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_prng_t * pip = &ls_ipPrng;
    olint_t i, j, k;
    u8 local_md[MD5_DIGEST_LEN];
    md5_t md5;
    u32 u32Index;

    u32Index = pip->ip_u32PoolIndex;
    memcpy(local_md, pip->ip_u8Md, sizeof(pip->ip_u8Md));

    pip->ip_u32PoolIndex += u32Len;
    if (pip->ip_u32PoolIndex >= PRNG_POOL_SIZE)
    {
        /* the pool index roll back from the start, set the pool size to the
           maximum pool size */
        pip->ip_u32PoolIndex %= PRNG_POOL_SIZE;
        pip->ip_u32PoolSize = PRNG_POOL_SIZE;
    }
    else if (pip->ip_u32PoolSize < PRNG_POOL_SIZE)
    {
        /* set the pool size to pool index */
        if (pip->ip_u32PoolIndex > pip->ip_u32PoolSize)
            pip->ip_u32PoolSize = pip->ip_u32PoolIndex;
    }
    
    initMd5(&md5);
    for (i = 0; i < u32Len; i += MD5_DIGEST_LEN)
    {
        j = (u32Len - i);
        j = (j > MD5_DIGEST_LEN) ? MD5_DIGEST_LEN : j;

        updateMd5(&md5, local_md, MD5_DIGEST_LEN);
        k = (u32Index + j) - PRNG_POOL_SIZE;
        if (k > 0)
        {
            updateMd5(&md5, &(pip->ip_u8Pool[u32Index]), j - k);
            updateMd5(&md5, &(pip->ip_u8Pool[0]), k);
        }
        else
        {
            updateMd5(&md5, &(pip->ip_u8Pool[u32Index]), j);
        }
        updateMd5(&md5, pu8Data, j);
        updateMd5(&md5, (u8 *)&pip->ip_u32NumOfGet, sizeof(pip->ip_u32NumOfGet));
        finalMd5(&md5, local_md);

        pu8Data += j;

        for (k = 0; k < j; k ++)
        {
            /* save the message digest to pool */
            pip->ip_u8Pool[u32Index ++] ^= local_md[k];
            if (u32Index >= PRNG_POOL_SIZE)
                u32Index = 0;
        }
    }

    for (k = 0; k < (olint_t)sizeof(pip->ip_u8Md); k++)
    {
        /* save the last message digest */
        pip->ip_u8Md[k] ^= local_md[k];
    }

    pip->ip_dbEntropy += dbEntropy;
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/
