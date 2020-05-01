/**
 *  @file messagingclient.c
 *
 *  @brief Implementation file for dispatcher messaging client of messaging library.
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
#include "jf_process.h"
#include "jf_logger.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"
#include "jf_jiukun.h"
#include "jf_messaging.h"

#include "dispatchercommon.h"
#include "messagingclient.h"
#include "dispatcherxfer.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the dispather service client data type.
 */
typedef struct
{
    dispatcher_xfer_t * dmc_pdxXfer;

    u8 dmc_u8Reserved[8];

    /**The process id of the service.*/
    pid_t dmc_piServId;
} dispatcher_messaging_client_t;

/** The chain for service clients. 
 */
static jf_network_chain_t * ls_pjncMessagingClientChain = NULL;

/** The dispather messaging client.
 */
static dispatcher_messaging_client_t * ls_pdmcMessagingClient = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _createDispatcherMessagingClientXfer(
    dispatcher_messaging_client_t * pdmc, create_dispatcher_messaging_client_param_t * pcdmcp,
    jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_xfer_create_param_t dxcp;
    olchar_t sockfile[JF_LIMIT_MAX_PATH_LEN];
    jf_ipaddr_t jiRemote;

    ol_bzero(sockfile, sizeof(sockfile));
    ol_snprintf(
        sockfile, JF_LIMIT_MAX_PATH_LEN - 1, "%s/%s", pcdmcp->cdmcp_pstrSocketDir,
        pcdmcp->cdmcp_pstrMessagingOut);
    jf_ipaddr_setUdsAddr(&jiRemote, sockfile);

    ol_bzero(&dxcp, sizeof(dxcp));
    dxcp.dxcp_sMaxMsg = pcdmcp->cdmcp_sMaxMsg;
    dxcp.dxcp_u32MaxNumMsg = pcdmcp->cdmcp_u32MaxNumMsg;
    dxcp.dxcp_pjiRemote = &jiRemote;
    dxcp.dxcp_pstrName = pcdmcp->cdmcp_pstrName;

    u32Ret = dispatcher_xfer_create(pChain, &pdmc->dmc_pdxXfer, &dxcp);

    return u32Ret;
}

static u32 _destroyDispatcherMessagingClient(dispatcher_messaging_client_t ** ppClient)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_client_t * pdmc = *ppClient;

    /*Destroy xfer for messaging client.*/
    if (pdmc->dmc_pdxXfer != NULL)
        dispatcher_xfer_destroy(&pdmc->dmc_pdxXfer);

    jf_jiukun_freeMemory((void **)ppClient);

    return u32Ret;
}

static u32 _createDispatcherMessagingClient(
    dispatcher_messaging_client_t ** ppClient, create_dispatcher_messaging_client_param_t * pcdmcp,
    jf_network_chain_t * pChain)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_client_t * pdmc = NULL;

    jf_logger_logDebugMsg("create messaging client: %s", pcdmcp->cdmcp_pstrName);

    u32Ret = jf_jiukun_allocMemory((void **)&pdmc, sizeof(*pdmc));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdmc, sizeof(*pdmc));

    }

    /*Create xfer for messaging client.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherMessagingClientXfer(pdmc, pcdmcp, pChain);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppClient = pdmc;
    else if (pdmc != NULL)
        _destroyDispatcherMessagingClient(&pdmc);

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _messagingClientThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_chain_t * pChain = (jf_network_chain_t *)pArg;

    JF_LOGGER_DEBUG("enter");

    /*Start the client chain.*/
    u32Ret = jf_network_startChain(pChain);

    JF_LOGGER_DEBUG("quit");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _sendDispatcherServActiveMsg(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;
    dispatcher_serv_active_msg dsam;

    /*Initialize the message.*/
    initMessagingMsgHeader(
        (u8 *)&dsam, DISPATCHER_MSG_ID_SERV_ACTIVE, JF_MESSAGING_PRIO_MID,
        sizeof(dispatcher_serv_active_msg_payload));
    dsam.dsam_dsampPayload.dsamp_u32ServId = dsam.dsam_jmhHeader.jmh_u32SourceId;

    /*Create the dispatcher message.*/
    u32Ret = createDispatcherMsg(&pdm, (u8 *)&dsam, sizeof(dsam));

    /*Send the dispatcher message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = sendDispatcherMessagingMsg(pdm);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createDispatcherMessagingClient(create_dispatcher_messaging_client_param_t * pcdmcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pcdmcp->cdmcp_pstrSocketDir != NULL);
    assert(pcdmcp->cdmcp_pstrMessagingOut != NULL);

    JF_LOGGER_DEBUG("create");

    /*Create the network chain.*/
    u32Ret = jf_network_createChain(&ls_pjncMessagingClientChain);

    /*Create the messaging client.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createDispatcherMessagingClient(
            &ls_pdmcMessagingClient, pcdmcp, ls_pjncMessagingClientChain);
    }

    return u32Ret;
}

u32 destroyDispatcherMessagingClient(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("destroy");

    /*Destroy the messaging client.*/
    if (ls_pdmcMessagingClient != NULL)
        _destroyDispatcherMessagingClient(&ls_pdmcMessagingClient);

    /*Destroy the network chain.*/
    if (ls_pjncMessagingClientChain != NULL)
        u32Ret = jf_network_destroyChain(&ls_pjncMessagingClientChain);

    return u32Ret;
}

u32 startDispatcherMessagingClient(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to run the chain.*/
    u32Ret = jf_thread_create(NULL, NULL, _messagingClientThread, ls_pjncMessagingClientChain);

    /*Send service active message.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _sendDispatcherServActiveMsg();

    return u32Ret;
}

u32 stopDispatcherMessagingClient(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Stop the network chain.*/
    u32Ret = jf_network_stopChain(ls_pjncMessagingClientChain);

    return u32Ret;
}

u32 sendDispatcherMessagingMsg(dispatcher_msg_t * pdm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_messaging_client_t * pdmc = ls_pdmcMessagingClient;

    jf_logger_logDebugMsg("send dispatcher messaging msg");

    /*Send the message to dispatcher daemon.*/
    u32Ret = dispatcher_xfer_sendMsg(pdmc->dmc_pdxXfer, pdm);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


