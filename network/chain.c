/**
 *  @file chain.c
 *
 *  @brief The chain structure
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "xmalloc.h"
#include "errcode.h"
#include "network.h"
#include "syncmutex.h"

#if defined(LINUX)
    #include "signal.h"   
#endif

/* --- private data/data structure section --------------------------------- */

/** Maximum wait time in second, 24 hours
 */
#define BASIC_CHAIN_MAX_WAIT  (86400)

/** Base chain
 */
typedef struct internal_basic_chain
{
    /** TRUE means to stop the chain */
    boolean_t ibc_bToTerminate;
    u8 ibc_u8Reserved[7];
    /** Pointing to an object */
    basic_chain_object_t * ibc_pbcoObject;
    /** pipe, to wakeup or stop the chain*/
    socket_t * ibc_psWakeup[2];
    sync_mutex_t ibc_smLock;
    /** Next chain */
    struct internal_basic_chain *ibc_pibcNext;
} internal_basic_chain_t;


/* --- private routine section---------------------------------------------- */
static u32 _readWakeupSocket(internal_basic_chain_t * pibc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 u8Buffer[100];
    olsize_t i, u32Count = 100;

    u32Ret = sRecv(pibc->ibc_psWakeup[0], u8Buffer, &u32Count);
    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < u32Count; i ++)
            if (u8Buffer[i] == 'S')
            {
                pibc->ibc_bToTerminate = TRUE;
#if defined(DEBUG_CHAIN)
                logInfoMsg("read wakeup socket, got terminate signal");
#endif
            }
    }

#if defined(DEBUG_CHAIN)
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("read wakeup socket, %u", u32Count);
    }
#endif
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 createBasicChain(basic_chain_t ** ppChain)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc;

    u32Ret = xmalloc((void **)&pibc, sizeof(internal_basic_chain_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(pibc, 0, sizeof(internal_basic_chain_t));

        u32Ret = createSocketPair(AF_INET, SOCK_STREAM, pibc->ibc_psWakeup);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&pibc->ibc_smLock);

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppChain = pibc;
    }
    else if (pibc != NULL)
    {
        destroyBasicChain((basic_chain_t **)&pibc);
    }

    return u32Ret;
}

u32 destroyBasicChain(basic_chain_t ** ppChain)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc, * chain;

    assert((ppChain != NULL) && (*ppChain != NULL));

#if defined(DEBUG_CHAIN)
    logInfoMsg("destroy chain");
#endif

    pibc = (internal_basic_chain_t *)*ppChain;
    *ppChain = NULL;

    if (pibc->ibc_psWakeup[0] != NULL)
        u32Ret = destroySocketPair(pibc->ibc_psWakeup);

    finiSyncMutex(&(pibc->ibc_smLock));

    /* Clean up the chain by iterating through all the destroy. */
    while (pibc != NULL)
    {
        chain = pibc->ibc_pibcNext;

        xfree((void **)&pibc);

        pibc = chain;
    }

    return u32Ret;
}

u32 appendToBasicChain(basic_chain_t * pChain, basic_chain_object_t * pObject)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc;

    pibc = (internal_basic_chain_t *) pChain;
    /* Add link to the end of the chain (Linked List) */
    while (pibc->ibc_pibcNext != NULL)
    {
        pibc = pibc->ibc_pibcNext;
    }

    if (pibc->ibc_pbcoObject != NULL)
    {
        u32Ret = xcalloc(
            (void **)&pibc->ibc_pibcNext, sizeof(internal_basic_chain_t));
        if (u32Ret == OLERR_NO_ERROR)
        {
            pibc = pibc->ibc_pibcNext;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pibc->ibc_pbcoObject = pObject;
    }

    return u32Ret;
}

u32 startBasicChain(basic_chain_t * pChain)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc, * pBasicChain;
    basic_chain_object_header_t * pbcoh;
    fd_set readset;
    fd_set errorset;
    fd_set writeset;
    struct timeval tv;
    olint_t slct;
    u32 u32Time;

    assert(pChain != NULL);

    pibc = (internal_basic_chain_t *)pChain;

    /* Use this thread as if it's our own. Keep looping until we are signaled to
       stop*/
    while (! pibc->ibc_bToTerminate)
    {
        slct = 0;
        FD_ZERO(&readset);
        FD_ZERO(&errorset);
        FD_ZERO(&writeset);
        tv.tv_sec = BASIC_CHAIN_MAX_WAIT;
        tv.tv_usec = 0;

        setSocketToFdSet(pibc->ibc_psWakeup[0], &readset);

        /*Iterate through all the pre_select function pointers in the chain*/
        pBasicChain = (internal_basic_chain_t *) pibc;
        while ((pBasicChain != NULL) && (pBasicChain->ibc_pbcoObject != NULL))
        {
            pbcoh = (basic_chain_object_header_t *)pBasicChain->ibc_pbcoObject;
            if (pbcoh->bcoh_fnPreSelect != NULL)
            {
                u32Time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
                pbcoh->bcoh_fnPreSelect(
                    pBasicChain->ibc_pbcoObject, &readset, &writeset, &errorset,
                    &u32Time);
                tv.tv_sec = u32Time / 1000;
                tv.tv_usec = 1000 * (u32Time % 1000);
            }
            pBasicChain = pBasicChain->ibc_pibcNext;
        }

#if defined(DEBUG_CHAIN)
        logInfoMsg("enter select, %ld, %ld", tv.tv_sec, tv.tv_usec);
#endif
        /*The actual select statement*/
        slct = select(FD_SETSIZE, &readset, &writeset, &errorset, &tv);
#if defined(DEBUG_CHAIN)
        logInfoMsg("exit select, %d", slct);
#endif
        if (slct == -1)
        {
            /*If the select error, clear these sets*/
            FD_ZERO(&readset);
            FD_ZERO(&writeset);
            FD_ZERO(&errorset);
        }
        else
        {
            if (slct > 0)
            {
                if (isSocketSetInFdSet(pibc->ibc_psWakeup[0], &readset) != 0)
                {
                    _readWakeupSocket(pibc);
                    slct --;
                }
            }

            /*Iterate through all of the post_select in the chain*/
            pBasicChain = pibc;
            while ((pBasicChain != NULL) && (pBasicChain->ibc_pbcoObject != NULL))
            {
                pbcoh = (basic_chain_object_header_t *)pBasicChain->ibc_pbcoObject;
                if (pbcoh->bcoh_fnPostSelect != NULL)
                {
                    pbcoh->bcoh_fnPostSelect(
                        pBasicChain->ibc_pbcoObject, slct, &readset, &writeset,
                        &errorset);
                }
                pBasicChain = pBasicChain->ibc_pibcNext;
            }
        }
    }

    return u32Ret;
}

u32 stopBasicChain(basic_chain_t * pChain)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc = (internal_basic_chain_t *) pChain;
    olsize_t u32Count;

#if defined(DEBUG_CHAIN)
    logInfoMsg("stop chain");
#endif

    u32Count = 1;
    u32Ret = sSend(pibc->ibc_psWakeup[1], "S", &u32Count);
#if defined(DEBUG_CHAIN)
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("stop chain, done");
    }
#endif

    return u32Ret;
}

u32 wakeupBasicChain(basic_chain_t * pChain)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_basic_chain_t * pibc = (internal_basic_chain_t *) pChain;
    olsize_t u32Count;

#if defined(DEBUG_CHAIN)
    logInfoMsg("wakeup chain");
#endif

    u32Count = 1;
    u32Ret = sSend(pibc->ibc_psWakeup[1], "W", &u32Count);
#if defined(DEBUG_CHAIN)
    if (u32Ret == OLERR_NO_ERROR)
    {
        logInfoMsg("wakeup chain, done");
    }
#endif

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


