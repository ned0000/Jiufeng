/**
 *  @file dispatcher.c
 *
 *  @brief software management implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_time.h"
#include "jf_file.h"
#include "jf_messaging.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"
#include "jf_jiukun.h"
#include "jf_sem.h"
#include "jf_mutex.h"
#include "jf_queue.h"

#include "dispatcher.h"
#include "dispatchercommon.h"
#include "serviceconfig.h"
#include "serviceserver.h"
#include "serviceclient.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Default configuration directory for dispatcher.
 */
#define DEFAULT_DISPATCHER_CONFIG_DIR     "../config/dispatcher"

/** The buffer size for dispatcher async server socket.
 */
#define MAX_DISPATCHER_ASSOCKET_BUF_SIZE  (2048)

/** Maximum concurrent dispatcher message.
 */
#define MAX_CONCURRENT_DISPATCHER_MSG     (100)


/** Define the internal dispather data type.
 */
typedef struct
{
    boolean_t id_bInitialized;
    boolean_t id_bToTerminate;
    u8 id_u8Reserved[6];

    olchar_t * id_pstrConfigDir;
    u32 id_u32Reserved[8];


    jf_mutex_t id_jmMsgLock;
    jf_sem_t id_jsMsgSem;
    jf_queue_t id_jqMsgQueue;
    
} internal_dispatcher_t;

/** The internal dispatcher.
 */
static internal_dispatcher_t ls_idDispatcher;

/** Number of service config found in config directory.
 */
static u16 ls_u16NumOfServiceConfig = 0;

/** Service config list.
 */
static jf_linklist_t ls_jlServiceConfig;


/* --- private routine section ------------------------------------------------------------------ */

static dispatcher_msg_t * _dequeueMsgInDispatcherMsgQueue(internal_dispatcher_t * pid)
{
    dispatcher_msg_t * pdm = NULL;

    jf_mutex_acquire(&pid->id_jmMsgLock);
    pdm = jf_queue_dequeue(&pid->id_jqMsgQueue);
    jf_mutex_release(&pid->id_jmMsgLock);

    return pdm;
}

static u32 _enqueueMsgInDispatcherMsgQueue(internal_dispatcher_t * pid, dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mutex_acquire(&pid->id_jmMsgLock);
    u32Ret = jf_queue_enqueue(&pid->id_jqMsgQueue, pdm);
    jf_mutex_release(&pid->id_jmMsgLock);

    return u32Ret;
}

/** Queue dispatcher message.
 */
static u32 _fnDispatcherQueueServiceServerMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;
    dispatcher_msg_t * pdm = NULL;

    JF_LOGGER_DEBUG("msg id: %u", getMessagingMsgId(pu8Msg, sMsg));

    /*Create the message and copy the data. This is necessary as the original message buffer is not
      allocated.*/
    u32Ret = createDispatcherMsg(&pdm, pu8Msg, sMsg);

    /*Add the message to the queue.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _enqueueMsgInDispatcherMsgQueue(pid, pdm);

    /*Up the semaphore to wakeup the dispatcher thread.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_sem_up(&pid->id_jsMsgSem);
    
    return u32Ret;
}

static u32 _dispatchMsg(internal_dispatcher_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;

    pdm = _dequeueMsgInDispatcherMsgQueue(pid);

    while ((pdm != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /*Send the message to destination service.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = dispatchMsgToServiceClients(pdm);

        /*Free the message*/
        freeDispatcherMsg(&pdm);

        /*Get the next message.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            pdm = _dequeueMsgInDispatcherMsgQueue(pid);
    }

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _dispatcherMsgThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = (internal_dispatcher_t *)pArg;

    JF_LOGGER_INFO("enter");

    /*Start the loop.*/
    while (! pid->id_bToTerminate)
    {
        u32Ret = jf_sem_down(&pid->id_jsMsgSem);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _dispatchMsg(pid);
        }
    }

    JF_LOGGER_INFO("quit");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _startMsgDispatcherThread(internal_dispatcher_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to dispatch the message.*/
    u32Ret = jf_thread_create(NULL, NULL, _dispatcherMsgThread, pid);

    return u32Ret;
}

static u32 _stopMsgDispatcherThread(internal_dispatcher_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pid->id_bToTerminate = TRUE;

    u32Ret = jf_sem_up(&pid->id_jsMsgSem);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initDispatcher(dispatcher_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];
    scan_dispatcher_config_dir_param_t sdcdp;

    assert(pdp != NULL);
    assert(! pid->id_bInitialized);
    
    JF_LOGGER_DEBUG("config dir: %s", pdp->dp_pstrConfigDir);

    ol_bzero(pid, sizeof(internal_dispatcher_t));

    pid->id_pstrConfigDir = pdp->dp_pstrConfigDir;
    jf_linklist_init(&ls_jlServiceConfig);

    /*Change the working directory.*/
    jf_file_getDirectoryName(
        strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pid->id_jmMsgLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_sem_init(&pid->id_jsMsgSem, 0, MAX_CONCURRENT_DISPATCHER_MSG);

    /*Scan the config directory and parse the config file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&sdcdp, sizeof(sdcdp));
        sdcdp.sdcdp_pstrConfigDir = pid->id_pstrConfigDir;
        sdcdp.sdcdp_pjlServiceConfig = &ls_jlServiceConfig;

        u32Ret = scanDispatcherConfigDir(&sdcdp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ls_u16NumOfServiceConfig = sdcdp.sdcdp_u16NumOfServiceConfig;
    }

    /*Create the directory for unix domain socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createUdsDir();

    /*Create the service clients.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_service_client_param_t cdscp;

        ol_bzero(&cdscp, sizeof(cdscp));
        cdscp.cdscp_pstrSocketDir = DISPATCHER_UDS_DIR;

        u32Ret = createDispatcherServiceClients(&ls_jlServiceConfig, &cdscp);
    }

    /*Create the service servers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_service_server_param_t cdssp;

        ol_bzero(&cdssp, sizeof(cdssp));
        cdssp.cdssp_u32MaxConnInServer = DISPATCHER_MAX_CONN_IN_SERVICE_SERVER;
        cdssp.cdssp_pstrSocketDir = DISPATCHER_UDS_DIR;
        cdssp.cdssp_fnQueueMsg = _fnDispatcherQueueServiceServerMsg;

        u32Ret = createDispatcherServiceServers(&ls_jlServiceConfig, &cdssp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pid->id_bInitialized = TRUE;
    else
        finiDispatcher();

    return u32Ret;
}

u32 finiDispatcher(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;

    JF_LOGGER_DEBUG("fini dispatcher");

    destroyDispatcherServiceServers();

    destroyDispatcherServiceClients();

    destroyDispatcherServiceConfigList(&ls_jlServiceConfig);

    jf_time_sleep(3);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_fini(&pid->id_jmMsgLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_sem_fini(&pid->id_jsMsgSem);

    pid->id_bInitialized = FALSE;

    return u32Ret;
}

u32 startDispatcher(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;

    JF_LOGGER_DEBUG("start dispatcher");
    
    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    /*Start the dispather thread*/
    u32Ret = _startMsgDispatcherThread(pid);

    /*Start the service client.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherServiceClients();

    /*Start the service server.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherServiceServers();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Main thread will loop until quit*/
        while (! pid->id_bToTerminate)
        {
            jf_time_sleep(2);
        }
        JF_LOGGER_INFO("dispatcher main thread stop");
    }

    return u32Ret;
}

u32 stopDispatcher(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;

    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    /*Stop service server.*/
    stopDispatcherServiceServers();

    /*Stop service client.*/
    stopDispatcherServiceClients();

    /*Stop dispatcher thread.*/
    _stopMsgDispatcherThread(pid);

    pid->id_bToTerminate = TRUE;

    return u32Ret;
}

u32 setDefaultDispatcherParam(dispatcher_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pdp, sizeof(*pdp));

    pdp->dp_pstrConfigDir = DEFAULT_DISPATCHER_CONFIG_DIR;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
