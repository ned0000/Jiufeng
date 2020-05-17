/**
 *  @file messagingserver.c
 *
 *  @brief Implementation file for dispatcher messaging server of messaging library.
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"
#include "jf_jiukun.h"
#include "jf_listhead.h"
#include "jf_sem.h"

#include "messagingserver.h"
#include "prioqueue.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather messaging server data type.
 */
typedef struct
{
    /**The network chain.*/
    jf_network_chain_t * dms_pjncChain;
    jf_network_assocket_t * dms_pjnaAssocket;

    /**Semaphore for worker thread.*/
    jf_sem_t dms_jsWorker;
    /**Dispatcher priority queue.*/
    dispatcher_prio_queue_t * dms_pdpqMsg;

    boolean_t dms_bToTerminateWorkerThread;
    u8 dms_u8Reserved[7];
    /**The callback function to process the message.*/
    jf_messaging_fnProcessMsg_t dms_fnProcessMsg;

} dispatcher_messaging_server_t;

/** The chain for service servers. 
 */
static jf_network_chain_t * ls_pjncMessagingServerChain = NULL;

/** The dispather messaging server.
 */
static dispatcher_messaging_server_t * ls_pdmsMessagingServer;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDispatcherMessagingServerConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG("new connection");

    return u32Ret;
}

static u32 _onDispatcherMessagingServerDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG("reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onDispatcherMessagingServerSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("no data is expected");

    return u32Ret;
}

static u32 _validateMessagingServerMsg(
    u8 * pu8Buffer, olsize_t sBeginPointer, olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBuf = sEndPointer - sBeginPointer;
    olsize_t sMsg = 0;

    if (sBuf < sizeof(jf_messaging_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*The full message size is header size plus payload size.*/
        sMsg = getMessagingSize(pu8Buffer + sBeginPointer);

        if (sBuf < sMsg)
            u32Ret = JF_ERR_INCOMPLETE_DATA;
    }

    return u32Ret;
}

static u32 _isMessagingServerMsgAllowed(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _addDispatcherMessagingMsg(dispatcher_messaging_server_t * pdms, dispatcher_msg_t * msg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = enqueueDispatcherPrioQueue(pdms->dms_pdpqMsg, msg);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Wakeup the worker thread.*/
        u32Ret = jf_sem_up(&pdms->dms_jsWorker);
    }
    
    return u32Ret;
}

static u32 _processMessagingServerMsg(
    dispatcher_messaging_server_t * pdms, jf_network_assocket_t * pAssocket,
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sBegin = *psBeginPointer;
    olsize_t sMsg = 0;
    dispatcher_msg_t * msg = NULL;

    u32Ret = _validateMessagingServerMsg(pu8Buffer, *psBeginPointer, sEndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Receive full message.*/
        sMsg = getMessagingSize(pu8Buffer + sBegin);

        u32Ret = _isMessagingServerMsgAllowed(pu8Buffer + sBegin, sMsg);
    }

    /*Create dispatcher message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = createDispatcherMsg(&msg, pu8Buffer + sBegin, sMsg);

    /*Add the message to queue.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _addDispatcherMessagingMsg(pdms, msg);
        if (u32Ret == JF_ERR_MSG_QUEUE_FULL)
        {
            JF_LOGGER_DEBUG("message queue full");
            freeDispatcherMsg(&msg);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        /*Message is processed, discard the message.*/
        *psBeginPointer = sMsg;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        /*Incomplete data.*/
        u32Ret = JF_ERR_NO_ERROR;
    else  /*For other errors, just discard the data.*/
        *psBeginPointer = sMsg;

    return u32Ret;
}

static u32 _onDispatcherMessagingServerData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = jf_network_getTagOfAssocket(pAssocket);

    JF_LOGGER_DEBUG(
        "begin: %d, end: %d", *psBeginPointer, sEndPointer);

    u32Ret = _processMessagingServerMsg(
        pdms, pAssocket, pAsocket, pu8Buffer, psBeginPointer, sEndPointer);

    return u32Ret;
}

static u32 _createDispatcherMessagingServerAssocket(
    dispatcher_messaging_server_t * pdms, create_dispatcher_messaging_server_param_t * pcdmsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];

    ol_bzero(&jnacp, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = pcdmsp->cdmsp_sMaxMsg;
    jnacp.jnacp_u32MaxConn = pcdmsp->cdmsp_u32MaxConnInServer;
    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdmsp->cdmsp_pstrSocketDir,
        pcdmsp->cdmsp_pstrMessagingIn);
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, sockfile);
    jnacp.jnacp_fnOnConnect = _onDispatcherMessagingServerConnect;
    jnacp.jnacp_fnOnDisconnect = _onDispatcherMessagingServerDisconnect;
    jnacp.jnacp_fnOnSendData = _onDispatcherMessagingServerSendData;
    jnacp.jnacp_fnOnData = _onDispatcherMessagingServerData;
    jnacp.jnacp_pstrName = pcdmsp->cdmsp_pstrName;

    u32Ret = jf_network_createAssocket(pdms->dms_pjncChain, &pdms->dms_pjnaAssocket, &jnacp);
    if (u32Ret == JF_ERR_NO_ERROR)
        jf_network_setTagOfAssocket(pdms->dms_pjnaAssocket, pdms);

    return u32Ret;
}

static u32 _destroyDispatcherMessagingServer(dispatcher_messaging_server_t ** ppServ)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = *ppServ;

    /*Destroy the async server socket.*/
    if (pdms->dms_pjnaAssocket != NULL)
        jf_network_destroyAssocket(&pdms->dms_pjnaAssocket);

    /*Finalize the semaphore.*/
    jf_sem_fini(&pdms->dms_jsWorker);

    /*Destroy the priority message queue.*/
    if (pdms->dms_pdpqMsg != NULL)
        destroyDispatcherPrioQueue(&pdms->dms_pdpqMsg);

    jf_jiukun_freeMemory((void **)ppServ);

    return u32Ret;
}

static u32 _createDispatcherMessagingServer(
    dispatcher_messaging_server_t ** ppServ, create_dispatcher_messaging_server_param_t * pcdmsp,
    jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = NULL;

    JF_LOGGER_INFO("name: %s", pcdmsp->cdmsp_pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&pdms, sizeof(*pdms));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdms, sizeof(*pdms));
        pdms->dms_pjncChain = pChain;
        pdms->dms_fnProcessMsg = pcdmsp->cdmsp_fnProcessMsg;

        u32Ret = jf_sem_init(&pdms->dms_jsWorker, 0, pcdmsp->cdmsp_u32MaxNumMsg);
    }

    /*Create dispatcher priority queue.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        create_dispatcher_prio_queue_param_t cdpqp;

        ol_bzero(&cdpqp, sizeof(cdpqp));
        cdpqp.cdpqp_u32MaxNumMsg = pcdmsp->cdmsp_u32MaxNumMsg;

        u32Ret = createDispatcherPrioQueue(&pdms->dms_pdpqMsg, &cdpqp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherMessagingServerAssocket(pdms, pcdmsp);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppServ = pdms;
    else if (pdms != NULL)
        _destroyDispatcherMessagingServer(&pdms);

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _messagingServerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    JF_LOGGER_DEBUG("enter");

    /*Start the server chain.*/
    u32Ret = jf_network_startChain(pChain);

    JF_LOGGER_DEBUG("quit");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _processMsgInMessagingServer(dispatcher_messaging_server_t * pdms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * msg = NULL;

    msg = dequeueDispatcherPrioQueue(pdms->dms_pdpqMsg);
    while (msg != NULL)
    {
        pdms->dms_fnProcessMsg(msg->dm_u8Msg, msg->dm_sMsg);

        freeDispatcherMsg(&msg);

        msg = dequeueDispatcherPrioQueue(pdms->dms_pdpqMsg);
    }

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _messagingServerWorkerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = pArg;

    JF_LOGGER_DEBUG("enter");

    /*Start the server chain.*/
    while (! pdms->dms_bToTerminateWorkerThread)
    {
        u32Ret = jf_sem_down(&pdms->dms_jsWorker);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _processMsgInMessagingServer(pdms);
        }
    }

    JF_LOGGER_DEBUG("quit");

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherMessagingServer(create_dispatcher_messaging_server_param_t * pcdmsp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO(
        "max conn: %u, sock dir: %s", pcdmsp->cdmsp_u32MaxConnInServer, pcdmsp->cdmsp_pstrSocketDir);

    /*Create the network chain.*/
    u32Ret = jf_network_createChain(&ls_pjncMessagingServerChain);

    /*Create the messaging server.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherMessagingServer(
            &ls_pdmsMessagingServer, pcdmsp, ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 destroyDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("destroy");

    if (ls_pdmsMessagingServer != NULL)
        _destroyDispatcherMessagingServer(&ls_pdmsMessagingServer);

    if (ls_pjncMessagingServerChain != NULL)
        jf_network_destroyChain(&ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 startDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to receive the message.*/
    u32Ret = jf_thread_create(NULL, NULL, _messagingServerThread, ls_pjncMessagingServerChain);

    /*Start a worker thread to process messages.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_create(NULL, NULL, _messagingServerWorkerThread, ls_pdmsMessagingServer);

    return u32Ret;
}

u32 stopDispatcherMessagingServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = ls_pdmsMessagingServer;

    pdms->dms_bToTerminateWorkerThread = TRUE;

    u32Ret = jf_network_stopChain(ls_pjncMessagingServerChain);

    return u32Ret;
}

u32 addDispatcherMessagingMsg(dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_server_t * pdms = ls_pdmsMessagingServer;

    u32Ret = _addDispatcherMessagingMsg(pdms, pdm);
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
