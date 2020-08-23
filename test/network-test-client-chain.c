/**
 *  @file network-test-client-chain.c
 *
 *  @brief Test file containing network chain server for network function defined in jf_network
     library.
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
#include "jf_err.h"
#include "jf_network.h"
#include "jf_string.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_time.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_network_chain_t * ls_pjncNtccChain = NULL;

static jf_network_acsocket_t * ls_pjnaNtccAcsocket = NULL;

static boolean_t ls_bToTerminateNtcc = FALSE;

#define NETWORK_TEST_CLIENT_CHAIN  "NT-CLIENT-CHAIN"

#define SERVER_PORT          (51200)

/* --- private routine section ------------------------------------------------------------------ */

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pjncNtccChain != NULL)
        jf_network_stopChain(ls_pjncNtccChain);

    ls_bToTerminateNtcc = TRUE;
}

static u32 _ntccOnConnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strBuffer[32];

    if (u32Status == JF_ERR_NO_ERROR)
    {
        ol_printf("on ntcc connect, connected\n");
        ol_strcpy(strBuffer, "hello");
        u32Ret = jf_network_sendAcsocketData(pAcsocket, pAsocket, (u8 *)strBuffer, 5);
    }
    else
    {
        ol_printf("on ntcc connect, not connected\n");
    }

    return u32Ret;
}

static u32 _ntccOnDisconnect(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("on ntcc disconnect, reason: %s\n", jf_err_getDescription(u32Status));

    return u32Ret;
}

static u32 _ntccOnData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t slen = sEndPointer - *psBeginPointer;

    ol_printf("on ntcc data, len: %d\n", slen);
    ol_printf("on ntcc data, content: %s\n", (olchar_t *) (pu8Buffer + *psBeginPointer));

    *psBeginPointer = sEndPointer;

    u32Ret = jf_network_disconnectAcsocket(pAcsocket, pAsocket);
    
    return u32Ret;
}

static u32 _ntccOnSendData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf(
        "on ntcc send data, status: %s, data: %s\n",
        jf_err_getDescription(u32Status), (olchar_t *)pu8Buffer);

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _networkTestClientChainThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_acsocket_create_param_t jnacp;

    ol_printf("_networkTestClientChainThread starts\n");

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
        jnacp.jnacp_pstrName = NETWORK_TEST_CLIENT_CHAIN;

        u32Ret = jf_network_createAcsocket(ls_pjncNtccChain, &ls_pjnaNtccAcsocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_startChain(ls_pjncNtccChain);
    }

    if (ls_pjnaNtccAcsocket != NULL)
        jf_network_destroyAcsocket(&ls_pjnaNtccAcsocket);
    if (ls_pjncNtccChain != NULL)
        jf_network_destroyChain(&ls_pjncNtccChain);

    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_ipaddr_t serveraddr;
    jf_thread_id_t threadid;
    u32 u32RetCode = 0;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = NETWORK_TEST_CLIENT_CHAIN;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_initSocket();
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_registerSignalHandlers(_terminate);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_thread_create(&threadid, NULL, _networkTestClientChainThread, NULL);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_sleep(3);
                jf_ipaddr_getIpAddrFromString("127.0.0.1", JF_IPADDR_TYPE_V4, &serveraddr);

                u32Ret = jf_network_connectAcsocketTo(
                    ls_pjnaNtccAcsocket, &serveraddr, SERVER_PORT, NULL);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                while (! ls_bToTerminateNtcc)
                {
                    ol_sleep(3);
                }
            }

            jf_process_finiSocket();
            jf_thread_waitForThreadTermination(threadid, &u32RetCode);
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
