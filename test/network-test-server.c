/**
 *  @file network-test-server.c
 *
 *  @brief Test file containing network server for network function defined in jf_network library.
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
#include "jf_jiukun.h"
#include "jf_thread.h"
#include "jf_time.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define NETWORK_TEST_SERVER  "NT-SERVER"
#define SERVER_PORT          (51200)

typedef struct server_data
{
    u8 sd_u8Id[24];
} server_data_t;

static jf_network_chain_t * ls_pjncNtsChain = NULL;

static boolean_t ls_bToTerminateNts = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printNetworkTestServerUsage(void)
{
    ol_printf("\
Usage: network-test-server [-h] [logger options] \n\
    -h print the usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");

    exit(0);
}

static u32 _parseNetworkTestServerCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printNetworkTestServerUsage();
            exit(0);
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = optarg;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(optarg, &pjlip->jlip_sLogFile);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pjncNtsChain != NULL)
        jf_network_stopChain(ls_pjncNtsChain);

    ls_bToTerminateNts = TRUE;
}

static u32 _onNtsConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    server_data_t * psd = NULL;

    ol_printf("on nts connect, new connection\n");

    u32Ret = jf_jiukun_allocMemory((void **)&psd, sizeof(server_data_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(psd, sizeof(server_data_t));
        ol_strcpy((olchar_t *)psd->sd_u8Id, "network-test-server");

        *ppUser = psd;
    }

    return u32Ret;
}

static u32 _onNtsDisconnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    server_data_t * psd = (server_data_t *)pUser;

    ol_printf(
        "on nts disconnect, id: %s, reason: %s\n", psd->sd_u8Id, jf_err_getDescription(u32Status));

    jf_jiukun_freeMemory((void **)&psd);

    return u32Ret;
}

static u32 _onNtsSendData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("on nts send data, len: %d\n", sBuf);
    ol_printf("on nts send data, content: %s\n", (olchar_t *)pu8Buffer);

    return u32Ret;
}

static u32 _onNtsData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer, void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    server_data_t * psd = (server_data_t *)pUser;
    u32 u32Begin = *pu32BeginPointer;
    u8 u8Buffer[100];

    ol_printf("on nts data, receive ok, id: %s\n", psd->sd_u8Id);
    ol_printf("on nts data, begin: %d, end: %d\n", u32Begin, u32EndPointer);
    ol_memcpy(u8Buffer, pu8Buffer + u32Begin, u32EndPointer - u32Begin);
    u8Buffer[u32EndPointer - u32Begin] = '\0';
    ol_printf("on nts data, buffer: %s\n", u8Buffer);

    *pu32BeginPointer = u32EndPointer;

    ol_strcpy((olchar_t *)u8Buffer, "hello everybody");
    u32Ret = jf_network_sendAssocketData(
        pAssocket, pAsocket, u8Buffer, ol_strlen((olchar_t *)u8Buffer));

    return u32Ret;
}

JF_THREAD_RETURN_VALUE _networkTestServerThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_assocket_create_param_t jnacp;
    jf_network_assocket_t * pjnaNtsAssocket = NULL;

    u32Ret = jf_network_createChain(&ls_pjncNtsChain);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&jnacp, 0, sizeof(jnacp));

        jnacp.jnacp_sInitialBuf = 2048;
        jnacp.jnacp_u32MaxConn = 10;
        jnacp.jnacp_u16ServerPort = SERVER_PORT;
        jnacp.jnacp_fnOnConnect = _onNtsConnect;
        jnacp.jnacp_fnOnDisconnect = _onNtsDisconnect;
        jnacp.jnacp_fnOnSendData = _onNtsSendData;
        jnacp.jnacp_fnOnData = _onNtsData;
        jnacp.jnacp_pstrName = NETWORK_TEST_SERVER;

        u32Ret = jf_network_createAssocket(ls_pjncNtsChain, &pjnaNtsAssocket, &jnacp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_startChain(ls_pjncNtsChain);
    }

    if (pjnaNtsAssocket != NULL)
        jf_network_destroyAssocket(&pjnaNtsAssocket);
    if (ls_pjncNtsChain != NULL)
        jf_network_destroyChain(&ls_pjncNtsChain);
    
    JF_THREAD_RETURN(u32Ret);
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_thread_id_t threadid;
    u32 u32RetCode = 0;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = NETWORK_TEST_SERVER;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseNetworkTestServerCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_terminate);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);
        JF_LOGGER_DEBUG("%s starts", NETWORK_TEST_SERVER);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_thread_create(&threadid, NULL, _networkTestServerThread, NULL);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    while (! ls_bToTerminateNts)
                    {
                        jf_time_sleep(3);
                    }
                }

                jf_process_finiSocket();
                jf_thread_waitForThreadTermination(threadid, &u32RetCode);
            }

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
