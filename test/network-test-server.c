/**
 *  @file network-test-server.c
 *
 *  @brief Test file for network library, network server.
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
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
#define NETWORK_TEST_SERVER  "NT-SERVER"
#define SERVER_PORT          (51200)

typedef struct server_data
{
    u8 sd_u8Id[24];
} server_data_t;

static basic_chain_t * ls_pbcChain = NULL;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: network-test-server [-h] [logger options] \n\
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

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pbcChain != NULL)
        stopBasicChain(ls_pbcChain);
}

static u32 _onNtsConnect(
    assocket_t * pAssocket, asocket_t * pAsocket, void ** ppUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    server_data_t * psd = NULL;

    ol_printf("New connection\n");

    u32Ret = xcalloc((void **)&psd, sizeof(server_data_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strcpy((olchar_t *)psd->sd_u8Id, "network-test-server");

        *ppUser = psd;
    }

    return u32Ret;
}

static u32 _onNtsDisConnect(
    assocket_t * pAssocket, void * pAsocket, u32 u32Status, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;
    server_data_t * psd = (server_data_t *)pUser;

    ol_printf("connection closed, id: %s\n", psd->sd_u8Id);

    xfree((void **)&psd);

    return u32Ret;
}

static u32 _onNtsSendOK(assocket_t * pAssocket, asocket_t * connection, void * pUser)
{
    u32 u32Ret = OLERR_NO_ERROR;

    ol_printf("send ok\n");

    return u32Ret;
}

static u32 _onNtsData(assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * pu32BeginPointer, olsize_t u32EndPointer,
    void * pUser, boolean_t * bPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    server_data_t * psd = (server_data_t *)pUser;
    u32 u32Begin = *pu32BeginPointer;
    u8 u8Buffer[100];

    ol_printf("receive ok, id: %s\n", psd->sd_u8Id);
    ol_printf("begin: %d, end: %d\n", u32Begin, u32EndPointer);
    memcpy(u8Buffer, pu8Buffer + u32Begin, u32EndPointer - u32Begin);
    u8Buffer[u32EndPointer - u32Begin] = '\0';
    ol_printf("%s\n", u8Buffer);
    *pu32BeginPointer = u32EndPointer;

    ol_strcpy((olchar_t *)u8Buffer, "hello everybody");
    u32Ret = assSendData(
        pAssocket, pAsocket, u8Buffer,
        ol_strlen((olchar_t *)u8Buffer), AS_MEM_OWNER_USER);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    assocket_t * pAssocket = NULL;
    assocket_param_t ap;
    logger_param_t lpParam;

    memset(&lpParam, 0, sizeof(logger_param_t));
    lpParam.lp_pstrCallerName = NETWORK_TEST_SERVER;
//    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = 3;

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = registerSignalHandlers(_terminate);

    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);

        u32Ret = initNetLib();
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = createBasicChain(&ls_pbcChain);

            if (u32Ret == OLERR_NO_ERROR)
            {
                memset(&ap, 0, sizeof(assocket_param_t));

                ap.ap_sInitialBuf = 2048;
                ap.ap_u32MaxConn = 10;
                ap.ap_u16PortNumber = SERVER_PORT;
                ap.ap_fnOnConnect = _onNtsConnect;
                ap.ap_fnOnDisconnect = _onNtsDisConnect;
                ap.ap_fnOnSendOK = _onNtsSendOK;
                ap.ap_fnOnData = _onNtsData;

                u32Ret = createAssocket(ls_pbcChain, &pAssocket, &ap);
            }

            if (u32Ret == OLERR_NO_ERROR)
            {
                u32Ret = startBasicChain(ls_pbcChain);
            }

            finiNetLib();
        }

        finiLogger();
    }

    if (ls_pbcChain != NULL)
        destroyBasicChain(&ls_pbcChain);

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

