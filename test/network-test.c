/**
 *  @file network-test.c
 *
 *  @brief Test file for network function defined in jf_network library.
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
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define NETWORK_TEST  "NET-SERVER"

static boolean_t ls_bSocketPair = FALSE;
static boolean_t ls_bToTerminate = FALSE;
static olchar_t * ls_pstrServerIp = NULL;
static u16 ls_u16Port = 0;

/* --- private routine section ------------------------------------------------------------------ */

static void _printNetworkTestUsage(void)
{
    ol_printf("\
Usage: network-test [-o] [-s server ip] [-p port]\n\
    -o test socket pair.\n\
    -s specify the server ip to connect to.\n\
    -p specify the server port.\n");
    ol_printf("\n");

}

static u32 _parseNetworkTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "s:p:o?T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printNetworkTestUsage();
            exit(0);
            break;
        case 'o':
            ls_bSocketPair = TRUE;
            break;
        case 's':
            ls_pstrServerIp = optarg;
            break;
        case 'p':
            ol_sscanf(optarg, "%hu", &ls_u16Port); 
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
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

JF_THREAD_RETURN_VALUE _socketPairRead(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * psRead = (jf_network_socket_t *)pArg;
    u8 u8Buffer[100];
    olsize_t u32Count;

    ol_printf("read thread starts\n");

    while (! ls_bToTerminate)
    {
        sleep(5);
        memset(u8Buffer, 0, 100);
        u32Count = 100;
        u32Ret = jf_network_recv(psRead, u8Buffer, &u32Count);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("got message, %s\n", u8Buffer);
        else
            ol_printf("error getting message, 0x%X\n", u32Ret);
    }

    JF_THREAD_RETURN(u32Ret);
}

JF_THREAD_RETURN_VALUE _socketPairWrite(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * psWrite = (jf_network_socket_t *)pArg;
    olchar_t buffer[100];
    olsize_t u32Count;

    ol_printf("write thread starts\n");

    while (! ls_bToTerminate)
    {
        sleep(5);
        u32Count = ol_sprintf(buffer, "%s", "hello");
        u32Ret = jf_network_send(psWrite, buffer, &u32Count);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("send message\n");
        else
            ol_printf("error sending message, 0x%X\n", u32Ret);
    }

    JF_THREAD_RETURN(u32Ret);
}

static u32 _testSocketPair(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * psPair[2];
    jf_thread_id_t tiread, tiwrite;

    u32Ret = jf_network_createSocketPair(AF_INET, SOCK_STREAM, psPair);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("successfully create socket pair\n");

        jf_thread_initId(&tiread);
        jf_thread_initId(&tiwrite);

        u32Ret = jf_thread_create(&tiread, NULL, _socketPairRead, (void *)psPair[0]);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_thread_create(&tiwrite, NULL, _socketPairWrite, (void *)psPair[1]);

        sleep(100);

        ls_bToTerminate = TRUE;

        if (jf_thread_isValidId(&tiread))
            jf_thread_terminate(&tiread);

        if (jf_thread_isValidId(&tiwrite))
            jf_thread_terminate(&tiwrite);

        sleep(1);

        jf_network_destroySocketPair(psPair);
    }

    return u32Ret;
}

static void _terminate(olint_t signal)
{
    ls_bToTerminate = TRUE;
}

static u32 _testConnectServer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * client = NULL;
    jf_ipaddr_t ipaddr;

    jf_process_registerSignalHandlers(_terminate);

    u32Ret = jf_network_createSocket(AF_INET, SOCK_STREAM, 0, &client);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("connect to %s:%u\n", ls_pstrServerIp, ls_u16Port);

        jf_ipaddr_getIpAddrFromString(ls_pstrServerIp, JF_IPADDR_TYPE_V4, &ipaddr);
        u32Ret = jf_network_connect(client, &ipaddr, ls_u16Port);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("connected, press CTRL-x to return\n");
            while (! ls_bToTerminate)
                sleep(1);
        }

        jf_network_destroySocket(&client);
    }

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
    jlipParam.jlip_pstrCallerName = NETWORK_TEST;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseNetworkTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                if (ls_bSocketPair)
                {
                    u32Ret = _testSocketPair();
                }
                else if (ls_pstrServerIp != NULL && ls_u16Port != 0)
                {
                    u32Ret = _testConnectServer();
                }
                else
                {
                    ol_printf("No operation is specified !!!!\n\n");
                    _printNetworkTestUsage();
                }

                jf_process_finiSocket();
            }

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
