/**
 *  @file respool-test.c
 *
 *  @brief test file for testing respool common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "logger.h"
#include "errcode.h"
#include "respool.h"
#include "process.h"
#include "syncmutex.h"
#include "syncsem.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

typedef struct
{
    thread_id_t rtwd_tiThreadId;
    boolean_t rtwd_bToTerminate;
    u8 swe_u8Reserved[7];

    sync_sem_t * rtwd_pssSem;

    /*The SCSI command queue and the lock*/
    sync_mutex_t * rtwd_psmLock;
//    list_head_t * rtwd_plhHermesCmd;

} respool_test_worker_data_t;

#define RESPOOL_TEST_RESOURCE_POOL_NAME  "respool-test"
#define RESPOOL_TEST_MIN_RESOURCES     (2)
#define RESPOOL_TEST_MAX_RESOURCES     (5)

static resource_pool_t * ls_prpRespoolTestWorkerPool;
static sync_mutex_t ls_smRespoolTestLock;
static sync_sem_t ls_ssRespoolTestSem;
static thread_id_t ls_tiRespoolTestProducer;
static boolean_t ls_bRespoolTestTerminateProducer;
static resource_t * ls_prRespoolTestResource[RESPOOL_TEST_MAX_RESOURCES];


/* --- private routine section---------------------------------------------- */

THREAD_RETURN_VALUE _respoolTestWorkerThread(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = (respool_test_worker_data_t *)pArg;

    logDebugMsg("enter respool test worker thread");
    
    while (! prtwd->rtwd_bToTerminate)
    {
        u32Ret = downSyncSem(prtwd->rtwd_pssSem);
        if ((u32Ret == OLERR_NO_ERROR) && (! prtwd->rtwd_bToTerminate))
        {
            logInfoMsg("respool test worker thread, got work");

            acquireSyncMutex(prtwd->rtwd_psmLock);
            /*do the work*/
            releaseSyncMutex(prtwd->rtwd_psmLock);
        }
    }

    logDebugMsg("quit respool test worker thread");
    xfree((void **)&prtwd);
    
    THREAD_RETURN(u32Ret);
}

THREAD_RETURN_VALUE _respoolTestProducerThread(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logDebugMsg("enter respool test producer thread");
    ls_bRespoolTestTerminateProducer = FALSE;

    while (! ls_bRespoolTestTerminateProducer)
    {
        logInfoMsg("respool test producer thread, produce work");

        acquireSyncMutex(&ls_smRespoolTestLock);
        /*produce the work*/
        releaseSyncMutex(&ls_smRespoolTestLock);

        upSyncSem(&ls_ssRespoolTestSem);

        sleep(2);
    }

    logDebugMsg("quit respool test producer thread");

    THREAD_RETURN(u32Ret);
}

static u32 _createRespoolTestWorker(resource_t * pr, resource_data_t ** pprd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = NULL;

    logInfoMsg("create respool test worker");

    u32Ret = xmalloc((void **)&prtwd, sizeof(respool_test_worker_data_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memset(prtwd, 0, sizeof(respool_test_worker_data_t));

        prtwd->rtwd_pssSem = &ls_ssRespoolTestSem;
        prtwd->rtwd_psmLock = &ls_smRespoolTestLock;
        prtwd->rtwd_bToTerminate = FALSE;

        /* create a thread for the worker, using default thread attributes */
        u32Ret = createThread(
            &(prtwd->rtwd_tiThreadId), NULL, _respoolTestWorkerThread, prtwd);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *pprd = prtwd;
    else if (prtwd != NULL)
        xfree((void **)&prtwd);

    return u32Ret;
}

static u32 _destroyRespoolTestWorker(resource_t * pr, resource_data_t ** pprd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = (respool_test_worker_data_t *) *pprd;

    logInfoMsg("destroy respool test worker");

    prtwd->rtwd_bToTerminate = TRUE;
//    upSyncSem(prtwd->rtwd_pssSem);

    *pprd = NULL;

    return u32Ret;
}

static u32 _createTestRespool(resource_pool_t ** pprp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    resource_pool_param_t rpp, * prpp = &rpp;

    ol_memset(prpp, 0, sizeof(resource_pool_param_t));

    prpp->rpp_bImmediateRelease = FALSE;
    prpp->rpp_pstrName = RESPOOL_TEST_RESOURCE_POOL_NAME;
    prpp->rpp_u32MinResources = RESPOOL_TEST_MIN_RESOURCES;
    prpp->rpp_u32MaxResources = RESPOOL_TEST_MAX_RESOURCES;

    prpp->rpp_fnCreateResource = _createRespoolTestWorker;
    prpp->rpp_fnDestroyResource = _destroyRespoolTestWorker;

    u32Ret = createResourcePool(pprp, &rpp);
    
    return u32Ret;
}

static u32 _testRespool(void)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32 u32Index;

    u32Ret = initSyncSem(&ls_ssRespoolTestSem, 0, RESPOOL_TEST_MAX_RESOURCES);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = initSyncMutex(&ls_smRespoolTestLock);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _createTestRespool(&ls_prpRespoolTestWorkerPool);

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (u32Index = 0; u32Index < RESPOOL_TEST_MAX_RESOURCES; u32Index ++)
        {
            u32Ret = getResourceFromPool(
                ls_prpRespoolTestWorkerPool, &ls_prRespoolTestResource[u32Index]);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* create a thread for the worker, using default thread attributes */
        u32Ret = createThread(
            &ls_tiRespoolTestProducer, NULL, _respoolTestProducerThread, NULL);
    }

    return u32Ret;
}

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");
    
    ls_bRespoolTestTerminateProducer = TRUE;
}

static u32 _releaseRespoolTestResource(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Index;

    for (u32Index = 0; u32Index < RESPOOL_TEST_MAX_RESOURCES; u32Index ++)
    {
        u32Ret = putResourceInPool(
            ls_prpRespoolTestWorkerPool, &ls_prRespoolTestResource[u32Index]);
    }

    u32Ret = destroyResourcePool(&ls_prpRespoolTestWorkerPool);
    
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "RESPOOL-TEST";
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;

    jf_logger_init(&jlipParam);

    u32Ret = registerSignalHandlers(_terminate);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testRespool();

    if (u32Ret == OLERR_NO_ERROR)
    {
        while (! ls_bRespoolTestTerminateProducer)
        {
            sleep(1);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        _releaseRespoolTestResource();
        sleep(3);
        
    }
    
    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    jf_logger_fini();

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

