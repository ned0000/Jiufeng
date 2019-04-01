/**
 *  @file webclient-test.c
 *
 *  @brief test file for webclient library
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
#include "webclient.h"
#include "xmalloc.h"
#include "files.h"

/* --- private data/data structure section --------------------------------- */
static const olchar_t * ls_pstrProgramName = "webclient-test";
static const olchar_t * ls_pstrVersion = "1.0.0";

static jf_network_chain_t * ls_pjncChain = NULL;
static jf_webclient_t * ls_pwWebclient = NULL;
static jf_network_utimer_t * ls_pjnuUtimer = NULL;

static olchar_t * ls_pstrQuotationServer = "hq.sinajs.cn";
static jf_ipaddr_t ls_jiServerAddr;
static olchar_t * ls_pstrStocks = "sh000001,sh600000";

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: %s [-V] [logger options] [-h]\n\
    -h show this usage.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrProgramName);

    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "VT:F:S:Oh")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'O':
            plp->lp_bLogToStdout = TRUE;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_sLogFile = u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
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

    if (ls_pjncChain != NULL)
        jf_network_stopChain(ls_pjncChain);
}

static u32 _wcTestOnResponse(
    jf_network_asocket_t * pAsocket, olint_t nEvent,
    jf_httpparser_packet_header_t * header, void * user, boolean_t * pbPause)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * buf = NULL;
    olsize_t size;
    file_t fd = INVALID_FILE_VALUE;

    logInfoMsg("wc test response, nEvent %d", nEvent);

    if (nEvent == JF_WEBCLIENT_EVENT_DATAOBJECT_DESTROYED)
    {
        logInfoMsg("wc test response, web data obj is destroyed");
        return u32Ret;
    }

    jf_httpparser_getRawPacket(header, &buf, &size);
    logDataMsgWithAscii(
        (u8 *)buf, size, "Wc test on response, body %d", header->jhph_sBody);
    xfree((void **)&buf);

    if (header->jhph_sBody > 0)
    {
        u32Ret = openFile2(
            "webclient-http-data.xls", O_WRONLY | O_CREAT | O_TRUNC,
            DEFAULT_CREATE_FILE_MODE, &fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = writen(fd, header->jhph_pu8Body, header->jhph_sBody);

            closeFile(&fd);
        }
    }

    return u32Ret;
}

static u32 _getSinaQuotation(void * object)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t buffer[2048];
    olsize_t len;
#if 1
    len = ol_snprintf(
        buffer, 2048,
        "GET /list=%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:42.0) Gecko/20100101 Firefox/42.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: identity\r\n" //gzip,deflate\r\n"
        "Connection: keep-alive\r\n"
        "\r\n", ls_pstrStocks, ls_pstrQuotationServer);
#else
    len = ol_snprintf(
        buffer, 2048,
        "GET /service/chddata.html?code=0600000&start=20100701&end=20151225&"
        "fields=TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER;TCAP;MCAP HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:12.0) Gecko/20100101 Firefox/12.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: close\r\n"
        "\r\n", ls_pstrQuotationServer);

#endif
    u32Ret = jf_webclient_pipelineWebRequestEx(
        ls_pwWebclient, &ls_jiServerAddr, 80, buffer, len, FALSE, NULL, 0,
        FALSE, _wcTestOnResponse, NULL);

//    jf_network_addUtimerItem(ls_pjnuUtimer, NULL, 30, _getSinaQuotation, NULL);

    return u32Ret;
}

static u32 _testWebclient(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    jf_webclient_create_param_t jwcp;
    struct hostent * servp;

    u32Ret = jf_network_getHostByName(ls_pstrQuotationServer, &servp);

    if (u32Ret == OLERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&ls_jiServerAddr, *(long *)(servp->h_addr));

        u32Ret = jf_network_createChain(&ls_pjncChain);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = registerSignalHandlers(_terminate);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(ls_pjncChain, &ls_pjnuUtimer);

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&jwcp, 0, sizeof(jwcp));
        jwcp.jwcp_nPoolSize = 5;
        jwcp.jwcp_sBuffer = 1024 * 8;

        u32Ret = jf_webclient_create(ls_pjncChain, &ls_pwWebclient, &jwcp);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        jf_network_addUtimerItem(ls_pjnuUtimer, NULL, 5, _getSinaQuotation, NULL);

        jf_network_startChain(ls_pjncChain);

        sleep(3);
        jf_network_destroyChain(&ls_pjncChain);
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
    lpParam.lp_pstrCallerName = "wc-test";
    lpParam.lp_u8TraceLevel = LOGGER_TRACE_DATA;
    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_bLogToFile = TRUE;
    lpParam.lp_pstrLogFilePath = "webclient-test.log";

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);

        u32Ret = jf_network_initLib();
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = _testWebclient(argc, argv);

            jf_network_finiLib();
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

