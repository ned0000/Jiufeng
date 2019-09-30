/**
 *  @file network-test-client-chain.c
 *
 *  @brief Test file for network library. Network client with chain mode.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_network.h"
#include "jf_string.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_time.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_network_chain_t * ls_pjncNtccChain = NULL;

static jf_network_acsocket_t * ls_pjnaNtccAcsocket = NULL;

#define SERVER_PORT          (51200)

/* --- private routine section ------------------------------------------------------------------ */

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pjncNtccChain != NULL)
        jf_network_stopChain(ls_pjncNtccChain);
}

static u32 _ntccOnConnect(
    jf_network_acsocket_t * ls_pjnaNtccAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (u32Status == JF_ERR_NO_ERROR)
    {
        ol_printf("connected\n");
    }
    else
    {
        ol_printf("not connected\n");
    }

    return u32Ret;
}

static u32 _ntccOnDisconnect(
    jf_network_acsocket_t * ls_pjnaNtccAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static u32 _ntccOnData(
    jf_network_acsocket_t * ls_pjnaNtccAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t slen = sEndPointer - *psBeginPointer;

    ol_printf("on ntcc data, len: %d", slen);
    ol_printf("on ntcc data, content: %s", (olchar_t *) (pu8Buffer + *psBeginPointer));

    *psBeginPointer = sEndPointer;
    
    return u32Ret;
}


static u32 _ntccOnSendData(
    jf_network_acsocket_t * ls_pjnaNtccAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("on ntcc send data");

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _ntccThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;

    ol_printf("_ntccThread starts\n");

    u32Ret = jf_network_createChain(&ls_pjncNtccChain);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jnacp, sizeof(jnacp));
        jnacp.jnacp_sInitialBuf = 2048;
        jnacp.jnacp_u32MaxConn = 10;
        jnacp.jnacp_fnOnConnect = _ntccOnConnect;
        jnacp.jnacp_fnOnDisconnect = _ntccOnDisconnect;
        jnacp.jnacp_fnOnData = _ntccOnData;
        jnacp.jnacp_fnOnSendData = _ntccOnSendData;

        u32Ret = jf_network_createAcsocket(ls_pjncNtccChain, &ls_pjnaNtccAcsocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_startChain(ls_pjncNtccChain);
    }

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_ipaddr_t serveraddr;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    jf_logger_init(&jlipParam);

    u32Ret = jf_process_initSocket();
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_registerSignalHandlers(_terminate);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_thread_create(NULL, NULL, _ntccThread, NULL);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_time_sleep(3);
            jf_ipaddr_getIpAddrFromString("127.0.0.1", JF_IPADDR_TYPE_V4, &serveraddr);

            u32Ret = jf_network_connectAcsocketTo(ls_pjnaNtccAcsocket, &serveraddr, SERVER_PORT, NULL);
        }

        while (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_time_sleep(10);
        }

        jf_process_finiSocket();
    }

    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

