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
#include "servconfig.h"
#include "servserver.h"
#include "servclient.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Default configuration directory for dispatcher.
 */
#define DEFAULT_DISPATCHER_CONFIG_DIR     "../config/dispatcher"

/** The buffer size for dispatcher async server socket.
 */
#define MAX_DISPATCHER_ASSOCKET_BUF_SIZE  (2048)

/** Maximum connection in the async server socket for a service, one for the receiving message,
 *  another is for backup.
 */
#define MAX_CONN_IN_SERV_SERVER           (2)

/** Maximum connection in the async client socket for a service, one for the sending message,
 *  another is for backup.
 */
#define MAX_CONN_IN_SERV_CLIENT           (2)

/** Maximum concurrent dispatcher message.
 */
#define MAX_CONCURRENT_DISPATCHER_MSG     (100)


/** Define the internal dispather data type.
 */
typedef struct
{
    boolean_t id_bInitialized;
    boolean_t id_bToTerminate;
    u8 id_u8Reserved[7];

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
static u16 ls_u16NumOfServConfig = 0;

/** Service config list.
 */
static jf_linklist_t ls_jlServConfig;


/* --- private routine section ------------------------------------------------------------------ */

static u32 _fnDispatcherQueueServServerMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;
    dispatcher_msg_t * pdm = NULL;

    u32Ret = createDispatcherMsg(&pdm, pu8Msg, sMsg);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_mutex_acquire(&pid->id_jmMsgLock);
        u32Ret = jf_queue_enqueue(&pid->id_jqMsgQueue, pdm);
        jf_mutex_release(&pid->id_jmMsgLock);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_sem_up(&pid->id_jsMsgSem);
    
    return u32Ret;
}

static u32 _dispatchMsg(internal_dispatcher_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;

    jf_mutex_acquire(&pid->id_jmMsgLock);
    pdm = jf_queue_dequeue(&pid->id_jqMsgQueue);
    jf_mutex_release(&pid->id_jmMsgLock);

    while ((pdm != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /*Send the message to destination service.*/
        u32Ret = dispatchMsgToServ(pdm);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_mutex_acquire(&pid->id_jmMsgLock);
            pdm = jf_queue_dequeue(&pid->id_jqMsgQueue);
            jf_mutex_release(&pid->id_jmMsgLock);
        }
    }

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _dispatcherMsgThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = (internal_dispatcher_t *)pArg;

    jf_logger_logInfoMsg("enter dispatcher msg thread");

    /*Start the loop.*/
    while (! pid->id_bToTerminate)
    {
        u32Ret = jf_sem_down(&pid->id_jsMsgSem);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _dispatchMsg(pid);
        }
    }

    jf_logger_logInfoMsg("quit dispatcher msg thread");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _startMsgDispatcherThread(internal_dispatcher_t * pid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to run the chain.*/
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
    
    jf_logger_logDebugMsg("init dispatcher");

    ol_bzero(pid, sizeof(internal_dispatcher_t));

    pid->id_pstrConfigDir = pdp->dp_pstrConfigDir;
    jf_linklist_init(&ls_jlServConfig);

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
        sdcdp.sdcdp_pjlServConfig = &ls_jlServConfig;

        u32Ret = scanDispatcherConfigDir(&sdcdp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ls_u16NumOfServConfig = sdcdp.sdcdp_u16NumOfServConfig;
    }

    /*Create the directory for unix domain socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createUdsDir();

    /*Create the service clients.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_serv_client_param_t cdscp;

        ol_bzero(&cdscp, sizeof(cdscp));
        cdscp.cdscp_u32MaxConnInClient = MAX_CONN_IN_SERV_CLIENT;
        cdscp.cdscp_pstrSocketDir = DISPATCHER_UDS_DIR;

        u32Ret = createDispatcherServClients(&ls_jlServConfig, &cdscp);
    }

    /*Create the service servers.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_serv_server_param_t cdssp;

        ol_bzero(&cdssp, sizeof(cdssp));
        cdssp.cdssp_u32MaxConnInServer = MAX_CONN_IN_SERV_SERVER;
        cdssp.cdssp_pstrSocketDir = DISPATCHER_UDS_DIR;
        cdssp.cdssp_fnQueueMsg = _fnDispatcherQueueServServerMsg;

        u32Ret = createDispatcherServServers(&ls_jlServConfig, &cdssp);
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

    jf_logger_logDebugMsg("fini dispatcher");

    destroyDispatcherServServers();

    destroyDispatcherServClients();

    destroyDispatcherServConfigList(&ls_jlServConfig);

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

    jf_logger_logDebugMsg("start dispatcher");
    
    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    /*Start the dispather thread*/
    u32Ret = _startMsgDispatcherThread(pid);

    /*Start the service client.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherServClients();

    /*Start the service server.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = startDispatcherServServers();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Main thread will loop until quit*/
        while (! pid->id_bToTerminate)
        {
            jf_time_sleep(2);
        }
        jf_logger_logInfoMsg("dispatcher main thread quit.");
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
    stopDispatcherServServers();

    /*Stop service client.*/

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


