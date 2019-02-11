/**
 *  @file network-test-client.c
 *
 *  @brief Test file for network library, network client
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "comminit.h"
#include "network.h"
#include "stringparse.h"
#include "process.h"

/* --- private data/data structure section --------------------------------- */

#define NETWORK_TEST_CLIENT  "NT-CLIENT"
#define SERVER_IP            "127.0.0.1"
#define SERVER_PORT          (51200)

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: network-test-client [-h] [logger options] \n\
    -h print the usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "T:F:S:h")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case 'T':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                plp->lp_u8TraceLevel = (u8)u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                plp->lp_sLogFile = u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _networkTestClient(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    socket_t * pSocket = NULL;
    ip_addr_t ipaddr;
    olchar_t * pstrBuffer = "Hello world!";
    u8 u8Buffer[100];
    olsize_t u32Len;

    u32Ret = createSocket(AF_INET, SOCK_STREAM, 0, &pSocket);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("socket created\n");

        setIpV4Addr(&ipaddr, inet_addr(SERVER_IP));

        u32Ret = sConnect(pSocket, &ipaddr, SERVER_PORT);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("connected\n");
        u32Len = ol_sprintf((olchar_t *)u8Buffer, "%s", pstrBuffer);
        ol_printf("send data\n");
        u32Ret = sSend(pSocket, u8Buffer, &u32Len);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("recv data\n");
        memset(u8Buffer, 0, sizeof(u8Buffer));
        u32Len = 100;
        u32Ret = sRecv(pSocket, u8Buffer, &u32Len);
    }

    sleep(20);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("%s\n", u8Buffer);
    }

    if (pSocket != NULL)
    {
        destroySocket(&pSocket);
        ol_printf("socket closed\n");
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    logger_param_t lpParam;

    memset(&lpParam, 0, sizeof(logger_param_t));
    lpParam.lp_pstrCallerName = NETWORK_TEST_CLIENT;
//    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = 3;

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);

        u32Ret = initNetLib();
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = _networkTestClient();

            finiNetLib();
        }

        finiLogger();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

