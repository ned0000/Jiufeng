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

static jf_network_chain_t * ls_pjncChain = NULL;

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
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'T':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                pjlip->jlip_sLogFile = u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_PARAM;
            }
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

    if (ls_pjncChain != NULL)
        jf_network_stopChain(ls_pjncChain);
}

static u32 _onNtsConnect(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    void ** ppUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    server_data_t * psd = NULL;

    ol_printf("New connection\n");

    u32Ret = jf_mem_calloc((void **)&psd, sizeof(server_data_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy((olchar_t *)psd->sd_u8Id, "network-test-server");

        *ppUser = psd;
    }

    return u32Ret;
}

static u32 _onNtsDisConnect(
    jf_network_assocket_t * pAssocket, void * pAsocket, u32 u32Status,
    void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    server_data_t * psd = (server_data_t *)pUser;

    ol_printf("connection closed, id: %s\n", psd->sd_u8Id);

    jf_mem_free((void **)&psd);

    return u32Ret;
}

static u32 _onNtsSendOK(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * connection,
    void * pUser)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("send ok\n");

    return u32Ret;
}

static u32 _onNtsData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * pu32BeginPointer, olsize_t u32EndPointer,
    void * pUser, boolean_t * bPause)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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
    u32Ret = jf_network_sendAssocketData(
        pAssocket, pAsocket, u8Buffer,
        ol_strlen((olchar_t *)u8Buffer), JF_NETWORK_MEM_OWNER_USER);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_network_assocket_t * pAssocket = NULL;
    jf_network_assocket_create_param_t jnacp;
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = NETWORK_TEST_SERVER;
//    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = registerSignalHandlers(_terminate);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_network_initLib();
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_network_createChain(&ls_pjncChain);

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                ol_memset(&jnacp, 0, sizeof(jnacp));

                jnacp.jnacp_sInitialBuf = 2048;
                jnacp.jnacp_u32MaxConn = 10;
                jnacp.jnacp_u16PortNumber = SERVER_PORT;
                jnacp.jnacp_fnOnConnect = _onNtsConnect;
                jnacp.jnacp_fnOnDisconnect = _onNtsDisConnect;
                jnacp.jnacp_fnOnSendOK = _onNtsSendOK;
                jnacp.jnacp_fnOnData = _onNtsData;

                u32Ret = jf_network_createAssocket(ls_pjncChain, &pAssocket, &jnacp);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_network_startChain(ls_pjncChain);
            }

            jf_network_finiLib();
        }

        jf_logger_fini();
    }

    if (ls_pjncChain != NULL)
        jf_network_destroyChain(&ls_pjncChain);

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

