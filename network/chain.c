/**
 *  @file chain.c
 *
 *  @brief Implementation for network chain.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_jiukun.h"
#include "jf_err.h"
#include "jf_network.h"
#include "jf_mutex.h"

#if defined(LINUX)
    #include "signal.h"   
#endif

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum wait time in second, 24 hours.
 */
#define BASIC_CHAIN_MAX_WAIT                     (86400)


/** Define the internal basic chain object data type.
 */
typedef struct internal_basic_chain_object
{
    /**Pointing to the chain object from user.*/
    jf_network_chain_object_t * ibco_pjncoObject;

    u32 ibco_u32Reserved[8];

    /**Next chain object.*/
    struct internal_basic_chain_object *ibco_pibcoNext;
} internal_basic_chain_object_t;

/** Define the internal basic chain data type.
 */
typedef struct internal_basic_chain
{
    /**TRUE means to stop the chain.*/
    boolean_t ibc_bToTerminate;
    u8 ibc_u8Reserved[7];

    /**pipe, to wakeup or stop the chain.*/
    jf_network_socket_t * ibc_pjnsWakeup[2];
    /**Mutex lock.*/
    jf_mutex_t ibc_jmLock;

    /**The first chain object in single linked list.*/
    internal_basic_chain_object_t * ibc_pibcoFirst;
    /**The last chain object in single linked list.*/
    internal_basic_chain_object_t * ibc_pibcoLast;
} internal_basic_chain_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _readWakeupSocket(internal_basic_chain_t * pibc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Buffer[100];
    olsize_t i = 0, u32Count = sizeof(u8Buffer);

    /*Receive data from the first socket in socket pair.*/
    u32Ret = jf_network_recv(pibc->ibc_pjnsWakeup[0], u8Buffer, &u32Count);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check the data.*/
        for (i = 0; i < u32Count; i ++)
            if (u8Buffer[i] == 'S')
            {
                pibc->ibc_bToTerminate = TRUE;
#if defined(DEBUG_CHAIN)
                JF_LOGGER_INFO("got terminate signal");
#endif
            }
    }

#if defined(DEBUG_CHAIN)
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("count: %u", u32Count);
    }
#endif
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_createChain(jf_network_chain_t ** ppChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = NULL;

    /*Allocate memory for the basic chain.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pibc, sizeof(internal_basic_chain_t));

    /*Create socket pair for wakeup and stop of the chain.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pibc, sizeof(internal_basic_chain_t));

        u32Ret = jf_network_createSocketPair(AF_INET, SOCK_STREAM, pibc->ibc_pjnsWakeup);
    }

    /*Initialize the Mutex.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pibc->ibc_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppChain = pibc;
    else if (pibc != NULL)
        jf_network_destroyChain((jf_network_chain_t **)&pibc);

    return u32Ret;
}

u32 jf_network_destroyChain(jf_network_chain_t ** ppChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = NULL;
    internal_basic_chain_object_t * pibco = NULL, * chainobject = NULL;

    assert((ppChain != NULL) && (*ppChain != NULL));

#if defined(DEBUG_CHAIN)
    JF_LOGGER_INFO("destroy chain");
#endif

    pibc = (internal_basic_chain_t *)*ppChain;
    *ppChain = NULL;

    /*Destroy the socket pair.*/
    if (pibc->ibc_pjnsWakeup[0] != NULL)
        u32Ret = jf_network_destroySocketPair(pibc->ibc_pjnsWakeup);

    /*Finalize the mutex.*/
    jf_mutex_fini(&(pibc->ibc_jmLock));

    /*Clean up all chain objects.*/
    pibco = pibc->ibc_pibcoFirst;
    while (pibco != NULL)
    {
        chainobject = pibco->ibco_pibcoNext;

        /*Free memory of basic chain object.*/
        jf_jiukun_freeMemory((void **)&pibco);

        pibco = chainobject;
    }

    /*Free memory of basic chain.*/
    jf_jiukun_freeMemory((void **)&pibc);

    return u32Ret;
}

u32 jf_network_appendToChain(
    jf_network_chain_t * pChain, jf_network_chain_object_t * pObject)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = (internal_basic_chain_t *) pChain;
    internal_basic_chain_object_t * pibco = NULL;

    /*Allocate memory for the chain object.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pibco, sizeof(*pibco));

    /*Initialize the chain object.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pibco, sizeof(*pibco));

        /*Save the object from user.*/
        pibco->ibco_pjncoObject = pObject;
    }

    /*Add the chain object to list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Test if the list is empty.*/
        if (pibc->ibc_pibcoFirst == NULL)
        {
            /*List is empty.*/
            pibc->ibc_pibcoFirst = pibc->ibc_pibcoLast = pibco;
        }
        else
        {
            /*List is not empty.*/
            pibc->ibc_pibcoLast->ibco_pibcoNext = pibco;
            pibc->ibc_pibcoLast = pibco;
        }
    }

    if ((u32Ret != JF_ERR_NO_ERROR) && (pibco != NULL))
        jf_jiukun_freeMemory((void **)&pibco);

    return u32Ret;
}

u32 jf_network_startChain(jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = NULL;
    internal_basic_chain_object_t * pibco = NULL;
    jf_network_chain_object_header_t * pjncoh = NULL;
    fd_set readset;
    fd_set errorset;
    fd_set writeset;
    struct timeval tv;
    olint_t slct = 0;
    u32 u32Time = 0;

    assert(pChain != NULL);

    pibc = (internal_basic_chain_t *)pChain;

    /*Keep looping until we are signaled to stop.*/
    while (! pibc->ibc_bToTerminate)
    {
        slct = 0;
        FD_ZERO(&readset);
        FD_ZERO(&errorset);
        FD_ZERO(&writeset);
        tv.tv_sec = BASIC_CHAIN_MAX_WAIT;
        tv.tv_usec = 0;

        /*Add the first socket in socket pair to read set.*/
        jf_network_setSocketToFdSet(pibc->ibc_pjnsWakeup[0], &readset);

        /*Iterate through all the pre_select function pointers in the chain.*/
        pibco = (internal_basic_chain_object_t *) pibc->ibc_pibcoFirst;
        while ((pibco != NULL) && (pibco->ibco_pjncoObject != NULL))
        {
            pjncoh = (jf_network_chain_object_header_t *)pibco->ibco_pjncoObject;
            if (pjncoh->jncoh_fnPreSelect != NULL)
            {
                u32Time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

                /*Call the callback function of chain object.*/
                pjncoh->jncoh_fnPreSelect(
                    pibco->ibco_pjncoObject, &readset, &writeset, &errorset, &u32Time);

                tv.tv_sec = u32Time / 1000;
                tv.tv_usec = 1000 * (u32Time % 1000);
            }
            pibco = pibco->ibco_pibcoNext;
        }

#if defined(DEBUG_CHAIN)
        JF_LOGGER_DEBUG("enter select, tv_sec: %ld, tv_usec: %ld", tv.tv_sec, tv.tv_usec);
#endif
        /*The actual select statement.*/
        slct = select(FD_SETSIZE, &readset, &writeset, &errorset, &tv);
#if defined(DEBUG_CHAIN)
        JF_LOGGER_DEBUG("exit select, return: %d", slct);
#endif
        if (slct == -1)
        {
            /*If the select error, clear these sets.*/
            FD_ZERO(&readset);
            FD_ZERO(&writeset);
            FD_ZERO(&errorset);
        }
        else
        {
            if (slct > 0)
            {
                /*Test if the wakeup socket is in read set.*/
                if (jf_network_isSocketSetInFdSet(pibc->ibc_pjnsWakeup[0], &readset) != 0)
                {
                    _readWakeupSocket(pibc);
                    slct --;
                }
            }

            /*Iterate through all of the post_select in the chain.*/
            pibco = (internal_basic_chain_object_t *) pibc->ibc_pibcoFirst;
            while ((pibco != NULL) && (pibco->ibco_pjncoObject != NULL))
            {
                pjncoh = (jf_network_chain_object_header_t *) pibco->ibco_pjncoObject;
                /*Call the callback function of chain object.*/
                if (pjncoh->jncoh_fnPostSelect != NULL)
                {
                    pjncoh->jncoh_fnPostSelect(
                        pibco->ibco_pjncoObject, slct, &readset, &writeset, &errorset);
                }
                pibco = pibco->ibco_pibcoNext;
            }
        }
    }

    JF_LOGGER_INFO("exit");

    return u32Ret;
}

u32 jf_network_stopChain(jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = (internal_basic_chain_t *) pChain;
    olsize_t u32Count = 0;

    JF_LOGGER_INFO("stop chain");

    /*Send 1 character to the second socket in socket pair.*/
    u32Count = 1;
    u32Ret = jf_network_send(pibc->ibc_pjnsWakeup[1], "S", &u32Count);
#if defined(DEBUG_CHAIN)
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_INFO("stop chain, done");
    }
#endif

    return u32Ret;
}

u32 jf_network_wakeupChain(jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_chain_t * pibc = (internal_basic_chain_t *) pChain;
    olsize_t u32Count = 0;

#if defined(DEBUG_CHAIN)
    JF_LOGGER_DEBUG("wakeup chain");
#endif

    /*Send 1 character to the second socket in socket pair.*/
    u32Count = 1;
    u32Ret = jf_network_send(pibc->ibc_pjnsWakeup[1], "W", &u32Count);
#if defined(DEBUG_CHAIN)
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("wakeup chain, done");
    }
#endif

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
