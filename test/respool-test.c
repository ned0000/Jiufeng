/**
 *  @file respool-test.c
 *
 *  @brief Test file for resource pool defined in jf_respool common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_logger.h"
#include "jf_err.h"
#include "jf_respool.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_mutex.h"
#include "jf_sem.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    jf_thread_id_t rtwd_jtiThreadId;
    boolean_t rtwd_bToTerminate;
    u8 swe_u8Reserved[7];

    jf_sem_t * rtwd_pjsSem;

    jf_mutex_t * rtwd_pjmLock;

} respool_test_worker_data_t;

#define RESPOOL_TEST_RESOURCE_POOL_NAME  "respool-test"
#define RESPOOL_TEST_MIN_RESOURCES     (2)
#define RESPOOL_TEST_MAX_RESOURCES     (5)

static jf_respool_t * ls_pjrRespoolTestWorkerPool;
static jf_mutex_t ls_smRespoolTestLock;
static jf_sem_t ls_jsRespoolTestSem;
static jf_thread_id_t ls_jtiRespoolTestProducer;
static boolean_t ls_bRespoolTestTerminateProducer;
static jf_respool_resource_t * ls_pjrrRespoolTestResource[RESPOOL_TEST_MAX_RESOURCES];


/* --- private routine section ------------------------------------------------------------------ */

JF_THREAD_RETURN_VALUE _respoolTestWorkerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = (respool_test_worker_data_t *)pArg;

    jf_logger_logDebugMsg("enter respool test worker thread, %p", prtwd);
    
    while (! prtwd->rtwd_bToTerminate)
    {
        u32Ret = jf_sem_downWithTimeout(prtwd->rtwd_pjsSem, 2000);
        if ((u32Ret == JF_ERR_NO_ERROR) && (! prtwd->rtwd_bToTerminate))
        {
            jf_logger_logDebugMsg("respool test worker thread, got work");

            jf_mutex_acquire(prtwd->rtwd_pjmLock);
            /*do the work*/
            jf_mutex_release(prtwd->rtwd_pjmLock);
        }
    }

    jf_logger_logDebugMsg("quit respool test worker thread, %p", prtwd);
    jf_jiukun_freeMemory((void **)&prtwd);
    
    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE _respoolTestProducerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("enter respool test producer thread");
    ls_bRespoolTestTerminateProducer = FALSE;

    while (! ls_bRespoolTestTerminateProducer)
    {
        jf_logger_logInfoMsg("respool test producer thread, produce work");

        jf_mutex_acquire(&ls_smRespoolTestLock);
        /*produce the work*/
        jf_mutex_release(&ls_smRespoolTestLock);

        jf_sem_up(&ls_jsRespoolTestSem);

        ol_sleep(2);
    }

    jf_logger_logDebugMsg("quit respool test producer thread");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _createRespoolTestWorker(
    jf_respool_resource_t * pr, jf_respool_resource_data_t ** pprd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = NULL;

    jf_logger_logInfoMsg("create respool test worker");

    u32Ret = jf_jiukun_allocMemory((void **)&prtwd, sizeof(respool_test_worker_data_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(prtwd, sizeof(respool_test_worker_data_t));

        prtwd->rtwd_pjsSem = &ls_jsRespoolTestSem;
        prtwd->rtwd_pjmLock = &ls_smRespoolTestLock;
        prtwd->rtwd_bToTerminate = FALSE;

        /* create a thread for the worker, using default thread attributes */
        u32Ret = jf_thread_create(
            &(prtwd->rtwd_jtiThreadId), NULL, _respoolTestWorkerThread, prtwd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *pprd = prtwd;
    else if (prtwd != NULL)
        jf_jiukun_freeMemory((void **)&prtwd);

    return u32Ret;
}

static u32 _destroyRespoolTestWorker(
    jf_respool_resource_t * pr, jf_respool_resource_data_t ** pprd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    respool_test_worker_data_t * prtwd = (respool_test_worker_data_t *) *pprd;

    jf_logger_logInfoMsg("destroy respool test worker, %p", prtwd);

    prtwd->rtwd_bToTerminate = TRUE;
    jf_sem_up(prtwd->rtwd_pjsSem);

    *pprd = NULL;

    return u32Ret;
}

static u32 _createTestRespool(jf_respool_t ** ppjr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_respool_create_param_t jrcp;

    ol_bzero(&jrcp, sizeof(jf_respool_create_param_t));

    jrcp.jrcp_pstrName = RESPOOL_TEST_RESOURCE_POOL_NAME;
    jrcp.jrcp_u32MinResources = RESPOOL_TEST_MIN_RESOURCES;
    jrcp.jrcp_u32MaxResources = RESPOOL_TEST_MAX_RESOURCES;

    jrcp.jrcp_fnCreateResource = _createRespoolTestWorker;
    jrcp.jrcp_fnDestroyResource = _destroyRespoolTestWorker;

    u32Ret = jf_respool_create(ppjr, &jrcp);
    
    return u32Ret;
}

static u32 _testRespool(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;

    u32Ret = jf_sem_init(&ls_jsRespoolTestSem, 0, RESPOOL_TEST_MAX_RESOURCES);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&ls_smRespoolTestLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createTestRespool(&ls_pjrRespoolTestWorkerPool);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0; u32Index < RESPOOL_TEST_MAX_RESOURCES; u32Index ++)
        {
            u32Ret = jf_respool_getResource(
                ls_pjrRespoolTestWorkerPool, &ls_pjrrRespoolTestResource[u32Index]);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* create a thread for the worker, using default thread attributes */
        u32Ret = jf_thread_create(
            &ls_jtiRespoolTestProducer, NULL, _respoolTestProducerThread, NULL);
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
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;

    for (u32Index = 0; u32Index < RESPOOL_TEST_MAX_RESOURCES; u32Index ++)
    {
        u32Ret = jf_respool_putResource(
            ls_pjrRespoolTestWorkerPool, &ls_pjrrRespoolTestResource[u32Index]);
    }

    u32Ret = jf_respool_destroy(&ls_pjrRespoolTestWorkerPool);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "RESPOOL-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);
    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_registerSignalHandlers(_terminate);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _testRespool();

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            while (! ls_bRespoolTestTerminateProducer)
            {
                ol_sleep(1);
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _releaseRespoolTestResource();
            ol_sleep(3);
        }

        jf_jiukun_fini();
    }
    
    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

