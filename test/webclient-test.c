/**
 *  @file webclient-test.c
 *
 *  @brief Test file for WEB client function defined in jf_webclient library.
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
#include "jf_webclient.h"
#include "jf_jiukun.h"
#include "jf_file.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static const olchar_t * ls_pstrProgramName = "webclient-test";
static const olchar_t * ls_pstrVersion = "1.0.0";

static jf_network_chain_t * ls_pjncChain = NULL;
static jf_webclient_t * ls_pwWebclient = NULL;
static jf_network_utimer_t * ls_pjnuUtimer = NULL;

static olchar_t * ls_pstrQuotationServer = "hq.sinajs.cn";
static jf_ipaddr_t ls_jiServerAddr;
static olchar_t * ls_pstrStocks = "sh000001,sh600000";

/* --- private routine section ------------------------------------------------------------------ */

static void _printWebclientTestUsage(void)
{
    ol_printf("\
Usage: %s [-V] [logger options] [-h]\n\
    -h show this usage.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
           ls_pstrProgramName);

    ol_printf("\n");
}

static u32 _parseWebclientTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "VT:F:S:Oh")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printWebclientTestUsage();
            exit(0);
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
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

    if (ls_pjncChain != NULL)
        jf_network_stopChain(ls_pjncChain);
}

static u32 _wcTestOnResponse(
    jf_network_asocket_t * pAsocket, jf_webclient_event_t event,
    jf_httpparser_packet_header_t * header, void * user)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * buf = NULL;
    olsize_t size;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;

    jf_logger_logInfoMsg("wc test response, event %d", event);

    jf_httpparser_getRawPacket(header, &buf, &size);
    jf_logger_logDataMsgWithAscii(
        (u8 *)buf, size, "Wc test on response, body %d", header->jhph_sBody);
    jf_jiukun_freeMemory((void **)&buf);

    if (header->jhph_sBody > 0)
    {
        u32Ret = jf_file_openWithMode(
            "webclient-http-data.xls", O_WRONLY | O_CREAT | O_TRUNC,
            JF_FILE_DEFAULT_CREATE_MODE, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_file_writen(fd, header->jhph_pu8Body, header->jhph_sBody);

            jf_file_close(&fd);
        }
    }

    return u32Ret;
}

static u32 _getSinaQuotation(void * object)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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
    u32Ret = jf_webclient_sendHttpHeaderAndBody(
        ls_pwWebclient, &ls_jiServerAddr, 80, buffer, len, NULL, 0, _wcTestOnResponse, NULL);

//    jf_network_addUtimerItem(ls_pjnuUtimer, NULL, 30, _getSinaQuotation, NULL);

    return u32Ret;
}

static u32 _testWebclient(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_webclient_create_param_t jwcp;
    struct hostent * servp;

    u32Ret = jf_network_getHostByName(ls_pstrQuotationServer, &servp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&ls_jiServerAddr, *(long *)(servp->h_addr));

        u32Ret = jf_network_createChain(&ls_pjncChain);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_terminate);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(ls_pjncChain, &ls_pjnuUtimer, "webclient-test");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jwcp, sizeof(jwcp));
        jwcp.jwcp_u32PoolSize = 5;
        jwcp.jwcp_sBuffer = 1024 * 8;

        u32Ret = jf_webclient_create(ls_pjncChain, &ls_pwWebclient, &jwcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_addUtimerItem(ls_pjnuUtimer, ls_pjncChain, 5, _getSinaQuotation, NULL);

        jf_network_startChain(ls_pjncChain);

        sleep(3);

        jf_network_destroyUtimer(&ls_pjnuUtimer);
        jf_webclient_destroy(&ls_pwWebclient);
        jf_network_destroyChain(&ls_pjncChain);
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
    jlipParam.jlip_pstrCallerName = "WEBCLIENT-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_bLogToFile = TRUE;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseWebclientTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_initSocket();
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _testWebclient(argc, argv);

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
