/**
 *  @file network-test-client.c
 *
 *  @brief Test file containing network client for network function defined in jf_network library.
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
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define NETWORK_TEST_CLIENT  "NT-CLIENT"
#define SERVER_IP            "127.0.0.1"
#define SERVER_PORT          (51200)

/* --- private routine section ------------------------------------------------------------------ */

static void _printNetworkTestClientUsage(void)
{
    ol_printf("\
Usage: network-test-client [-h] [logger options] \n\
  -h: print the usage.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");

    exit(0);
}

static u32 _parseNetworkTestClientCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "T:F:OS:h")) != -1))
           
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printNetworkTestClientUsage();
            exit(0);
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            u32Ret = jf_option_getS32FromString(jf_option_getArg(), &pjlip->jlip_sLogFile);
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _networkTestClient(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_network_socket_t * pSocket = NULL;
    jf_ipaddr_t ipaddr;
    olchar_t * pstrBuffer = "Hello world!";
    u8 u8Buffer[100];
    olsize_t u32Len;

    u32Ret = jf_network_createSocket(AF_INET, SOCK_STREAM, 0, &pSocket);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("socket created\n");

        jf_ipaddr_setIpV4Addr(&ipaddr, inet_addr(SERVER_IP));

        u32Ret = jf_network_connect(pSocket, &ipaddr, SERVER_PORT);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("connected\n");
        u32Len = ol_sprintf((olchar_t *)u8Buffer, "%s", pstrBuffer);
        ol_printf("send data\n");
        u32Ret = jf_network_send(pSocket, u8Buffer, &u32Len);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("recv data\n");
        memset(u8Buffer, 0, sizeof(u8Buffer));
        u32Len = 100;
        u32Ret = jf_network_recv(pSocket, u8Buffer, &u32Len);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("%s\n", u8Buffer);
    }

    if (pSocket != NULL)
    {
        jf_network_destroySocket(&pSocket);
        ol_printf("socket closed\n");
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
    jlipParam.jlip_pstrCallerName = NETWORK_TEST_CLIENT;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseNetworkTestClientCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _networkTestClient();

                jf_process_finiSocket();
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
