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
#include "jf_process.h"
#include "jf_file.h"
#include "jf_messaging.h"
#include "jf_mem.h"
#include "jf_network.h"
#include "jf_ipaddr.h"
#include "jf_thread.h"
#include "jf_jiukun.h"

#include "dispatcher.h"
#include "dispatchercommon.h"
#include "servconfig.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** default configuration directory for dispatcher
 *
 */
#define DEFAULT_DISPATCHER_CONFIG_DIR  "../config/dispatcher"

/** the buffer should be large enough to hold all sevice information
 *  sizeof(jf_serv_info_t) * JF_SERV_MAX_NUM_OF_SERV
 */
#define MAX_DISPATCHER_ASSOCKET_BUF_SIZE  (2048)

#define MAX_DISPATCHER_ASSOCKET_CONN      (3)

#define DISPATCHER_MSG_CACHE   "dispatcher_msg_config"

typedef struct
{
    boolean_t id_bInitialized;
    u8 id_u8Reserved[7];

    olchar_t * id_pstrConfigDir;
    u32 id_u32Reserved[8];

    jf_network_chain_t * id_pjncDispatcherChain;
    jf_network_assocket_t * id_pjnaDispatcherAssocket;

    jf_listhead_t id_jlServConfig;

    u16 id_u16NumOfServ;
    u16 id_u16Reserved[3];

    jf_queue_t id_jqServConfig;
    
    jf_jiukun_cache_t * id_pjjcMsgConfig;

} internal_dispatcher_t;

static internal_dispatcher_t ls_idDispatcher;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _onDispatcherConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("on dispatcher connect, new connection");

    return u32Ret;
}

static u32 _onDispatcherDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg(
        "on dispatcher disconnect, reason: %s", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _onDispatcherSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("on dispatcher send data, len: %d", sBuf);

    return u32Ret;
}

static u32 _dispatcherValidateMsg(
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;
    u32 u32Size = u32EndPointer - u32Begin;
//    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)(pu8Buffer + u32Begin);

    if (u32Size < sizeof(jf_messaging_header_t))
        u32Ret = JF_ERR_INCOMPLETE_DATA;

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _dispatchMsg(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    u32 u32Begin = 0, u32Size = 0;
//    jf_messaging_header_t * pHeader = NULL;

    u32Ret = _dispatcherValidateMsg(pu8Buffer, pu32BeginPointer, u32EndPointer);
    if (u32Ret == JF_ERR_NO_ERROR)
    {


    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *pu32BeginPointer = u32EndPointer;
    else if (u32Ret == JF_ERR_INCOMPLETE_DATA)
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _onDispatcherData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Begin = *pu32BeginPointer;

    jf_logger_logDebugMsg("on dispatcher data, begin: %d, end: %d", u32Begin, u32EndPointer);

    u32Ret = _dispatchMsg(pAssocket, pAsocket, pu8Buffer, pu32BeginPointer, u32EndPointer);

    return u32Ret;
}

static u32 _createDispatcherAssocket(internal_dispatcher_t * pid, dispatcher_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;

    ol_memset(&jnacp, 0, sizeof(jnacp));

    jnacp.jnacp_sInitialBuf = MAX_DISPATCHER_ASSOCKET_BUF_SIZE;
    jnacp.jnacp_u32MaxConn = MAX_DISPATCHER_ASSOCKET_CONN;
    jf_ipaddr_setUdsAddr(&jnacp.jnacp_jiServer, DISPATCHER_SERVER_ADDR);
    jnacp.jnacp_fnOnConnect = _onDispatcherConnect;
    jnacp.jnacp_fnOnDisconnect = _onDispatcherDisconnect;
    jnacp.jnacp_fnOnSendData = _onDispatcherSendData;
    jnacp.jnacp_fnOnData = _onDispatcherData;

    u32Ret = jf_network_createAssocket(
        pid->id_pjncDispatcherChain, &pid->id_pjnaDispatcherAssocket, &jnacp);

    return u32Ret;
}

static void _dispatcherSignalHandler(olint_t signal)
{
    ol_printf("get signal %d\n", signal);

    stopDispatcher();
}

static u32 _fnFreeServConfig(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initDispatcher(dispatcher_param_t * pdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];
    scan_dispatcher_config_dir_param_t sdcdp;
    jf_jiukun_cache_create_param_t jjccp;

    assert(pdp != NULL);
    assert(! pid->id_bInitialized);
    
    jf_logger_logDebugMsg("init dispatcher");

    ol_bzero(pid, sizeof(internal_dispatcher_t));

    pid->id_pstrConfigDir = pdp->dp_pstrConfigDir;
    jf_listhead_init(&pid->id_jlServConfig);
    jf_queue_init(&pid->id_jqServConfig);

    /*Change the working directory.*/
    jf_file_getDirectoryName(
        strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pdp->dp_pstrCmdLine);
    if (strlen(strExecutablePath) > 0)
        u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jjccp, sizeof(jjccp));
        jjccp.jjccp_pstrName = DISPATCHER_MSG_CACHE;
        jjccp.jjccp_sObj = sizeof(dispatcher_msg_config_t);
        JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

        u32Ret = jf_jiukun_createCache(&pid->id_pjjcMsgConfig, &jjccp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&sdcdp, sizeof(sdcdp));
        sdcdp.sdcdp_pstrConfigDir = pid->id_pstrConfigDir;
        sdcdp.sdcdp_pjqServConfig = &pid->id_jqServConfig;
        sdcdp.sdcdp_pjjcMsgConfig = pid->id_pjjcMsgConfig;

        u32Ret = scanDispatcherConfigDir(&sdcdp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pid->id_u16NumOfServ = sdcdp.sdcdp_u16NumOfServ;


    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createChain(&pid->id_pjncDispatcherChain);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_dispatcherSignalHandler);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createDispatcherAssocket(pid, pdp);
    
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

    jf_queue_finiQueueAndData(&pid->id_jqServConfig, _fnFreeServConfig);

    if (pid->id_pjnaDispatcherAssocket != NULL)
        u32Ret = jf_network_destroyAssocket(&pid->id_pjnaDispatcherAssocket);

    if (pid->id_pjncDispatcherChain != NULL)
        u32Ret = jf_network_destroyChain(&pid->id_pjncDispatcherChain);

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

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_startChain(pid->id_pjncDispatcherChain);

    return u32Ret;
}

u32 stopDispatcher(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dispatcher_t * pid = &ls_idDispatcher;

    if (! pid->id_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_stopChain(pid->id_pjncDispatcherChain);
    }

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


