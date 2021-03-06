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

static olchar_t * ls_pstrWctSummaryServer = "quotes.money.163.com";

static olchar_t * ls_pstrWctQuotationServer = "hq.sinajs.cn";
static jf_ipaddr_t ls_jiServerAddr;
static olchar_t * ls_pstrStocks = "sh000001,sh600000";

static boolean_t ls_bTestWebclientTransfter = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printWebclientTestUsage(void)
{
    ol_printf("\
Usage: %s [-t] [-V] [logger options] [-h]\n\
  -h: show this usage.\n\
  -V: show version information.\n\
  -t: test transfer function.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
  By default, test chain function.\n",
              ls_pstrProgramName);

    ol_printf("\n");
}

static u32 _parseWebclientTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = 0;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "tVT:F:S:Oh")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printWebclientTestUsage();
            exit(0);
            break;
        case 't':
            ls_bTestWebclientTransfter = TRUE;
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
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
        "\r\n", ls_pstrStocks, ls_pstrWctQuotationServer);
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
        "\r\n", ls_pstrWctQuotationServer);

#endif
    u32Ret = jf_webclient_sendHttpHeaderAndBody(
        ls_pwWebclient, &ls_jiServerAddr, 80, buffer, len, NULL, 0, _wcTestOnResponse, NULL);

//    jf_network_addUtimerItem(ls_pjnuUtimer, NULL, 30, _getSinaQuotation, NULL);

    return u32Ret;
}

static u32 _testWebclient(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_webclient_create_param_t jwcp;
    struct hostent * servp = NULL;

    u32Ret = jf_network_getHostByName(ls_pstrWctQuotationServer, &servp);

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

        ol_sleep(3);

        jf_network_destroyUtimer(&ls_pjnuUtimer);
        jf_webclient_destroy(&ls_pwWebclient);
        jf_network_destroyChain(&ls_pjncChain);
    }

    return u32Ret;
}

static u32 _testWebclientTransfer(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_webclient_transfer_data_param_t jwtdp;
    struct hostent * servp = NULL;
    jf_ipaddr_t jiServerAddr;
    olchar_t buffer[1024];
    olsize_t len = 0;

    u32Ret = jf_network_getHostByName(ls_pstrWctSummaryServer, &servp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ipaddr_setIpV4Addr(&jiServerAddr, *(long *)(servp->h_addr));

        /*Generate the HTTP request.*/
        len = ol_snprintf(
            buffer, sizeof(buffer),
            "GET /service/chddata.html?code=0600000&start=20100101&end=20200101&"
            "fields=TCLOSE;HIGH;LOW;TOPEN;LCLOSE;CHG;PCHG;TURNOVER;VOTURNOVER;VATURNOVER;TCAP;MCAP HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:12.0) Gecko/20100101 Firefox/12.0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Accept-Language: en-us,en;q=0.5\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Connection: close\r\n"
            "\r\n", ls_pstrWctSummaryServer);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jwtdp, sizeof(jwtdp));
        jwtdp.jwtdp_pjiServer = &jiServerAddr;
        jwtdp.jwtdp_u16Port = 80;
        jwtdp.jwtdp_u32Timeout = 5;
        jwtdp.jwtdp_pSendBuf = buffer;
        jwtdp.jwtdp_sSendBuf = len;
        jwtdp.jwtdp_sRecvData = 1024 * 1024;

        u32Ret = jf_webclient_transferData(&jwtdp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Body length: %d\n", jwtdp.jwtdp_pjhphHeader->jhph_sBody);
        ol_printf("Body content:\n%s\n", jwtdp.jwtdp_pjhphHeader->jhph_pu8Body);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_httpparser_destroyPacketHeader(&jwtdp.jwtdp_pjhphHeader);

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
                if (ls_bTestWebclientTransfter)
                    u32Ret = _testWebclientTransfer();
                else
                    u32Ret = _testWebclient();

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
