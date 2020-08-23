/**
 *  @file httpparser-test.c
 *
 *  @brief Test file for HTTP parser function defined in jf_httpparser library.
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
#include "jf_httpparser.h"
#include "jf_jiukun.h"
#include "jf_hex.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bParseHttp = FALSE;
static boolean_t ls_bParseUri = FALSE;
static boolean_t ls_bGenerateHttpMsg = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printHttpparserTestUsage(void)
{
    ol_printf("\
Usage: httpparser-test [-p] [-u] [-g] [-h] [logger options] \n\
  -p: parse http header.\n\
  -u: parse URI.\n\
  -g: generating http message.\n\
  -h: print the usage.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parseHttpparserTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "gupT:F:S:h")) != -1))
           
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printHttpparserTestUsage();
            exit(0);
            break;
        case 'p':
            ls_bParseHttp = TRUE;
            break;
        case 'u':
            ls_bParseUri = TRUE;
            break;
        case 'g':
            ls_bGenerateHttpMsg = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
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

static u32 _getRawHeader(jf_httpparser_packet_header_t ** ppHeader)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph;
    u8 u8Body[10];
    olchar_t str[100];
    olint_t len;

    u32Ret = jf_httpparser_createEmptyPacketHeader(&pjhph);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_httpparser_setDirective(pjhph, "POST", 4, "/login", 6);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Accept", 6, "*/*", 3, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Accept-Language", 15, "zh-CN", 5, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(
                pjhph, "x-flash-version", 15, "10,1,53,64", 10, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(
                pjhph, "Content-Type", 12, "application/octet-stream", 24, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            len = ol_snprintf(str, sizeof(str) - 1, "%u", (u32)sizeof(u8Body));
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Content-Length", 14, str, len, TRUE);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(
                pjhph, "Accept-Encoding", 15, "gzip, deflate", 13, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "User-Agent", 10, "greaty", 6, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Host", 4, "192.168.88.3:8080", 17, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Connection", 10, "Keep-Alive", 10, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_addHeaderLine(pjhph, "Cache-Control", 13, "no-cache", 8, TRUE);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_httpparser_setBody(pjhph, u8Body, sizeof(u8Body), TRUE);

        *ppHeader = pjhph;
    }

    return u32Ret;
}

static u32 _testHttpMsg(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_httpparser_packet_header_t * pjhph = NULL;
    olchar_t * pstr;
    olsize_t size;

    u32Ret = _getRawHeader(&pjhph);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_httpparser_getRawPacket(pjhph, &pstr, &size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("HTTP message\n");
            jf_hex_dumpByteDataBuffer((u8 *)pstr, size);

            jf_jiukun_freeMemory((void **)&pstr);
        }
    }

    if (pjhph != NULL)
        jf_httpparser_destroyPacketHeader(&pjhph);

    return u32Ret;
}

static void _copyHeaderDataString(
    olchar_t * value, olchar_t * data, olsize_t sData)
{
    ol_memcpy(value, data, sData);
    value[sData] = '\0';
}

static void _printHttpPacketHeader(jf_httpparser_packet_header_t * pjhph)
{
    olchar_t value[256];
    jf_httpparser_packet_header_field_t * field;

    _copyHeaderDataString(value, pjhph->jhph_pstrDirective, pjhph->jhph_sDirective);
    ol_printf("Directive: %s(%d)\n", value, pjhph->jhph_sDirective);

    _copyHeaderDataString(
        value, pjhph->jhph_pstrDirectiveObj, pjhph->jhph_sDirectiveObj);
    ol_printf("DirectiveObj: %s(%d)\n", value, pjhph->jhph_sDirectiveObj);

    ol_printf("Status: %d\n", pjhph->jhph_nStatusCode);

    _copyHeaderDataString(value, pjhph->jhph_pstrStatusData, pjhph->jhph_sStatusData);
    ol_printf("StatusData: %s(%d)\n", value,  pjhph->jhph_sStatusData);

    _copyHeaderDataString(value, pjhph->jhph_pstrVersion, pjhph->jhph_sVersion);
    ol_printf("Version: %s(%d)\n", value, pjhph->jhph_sVersion);

    _copyHeaderDataString(value, (char *)pjhph->jhph_pu8Body, pjhph->jhph_sBody);
    ol_printf("Body: %s(%d)\n", value, pjhph->jhph_sBody);

    field = pjhph->jhph_pjhphfFirst;
    while (field != NULL)
    {
        _copyHeaderDataString(value, field->jhphf_pstrName, field->jhphf_sName);
        ol_printf("Header Name: %s(%d)\n", value, field->jhphf_sName);

        _copyHeaderDataString(value, field->jhphf_pstrData, field->jhphf_sData);
        ol_printf("Header Value: %s(%d)\n", value, field->jhphf_sData);

        field = field->jhphf_pjhphfNext;
    }
}

#define HTTP_MSG_1 "\
HTTP/1.1 200 OK\r\n\
Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n\
Server: Apache\r\n\
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n\
ETag: \"34aa387-d-1568eb00\"\r\n\
Accept-Ranges: bytes\r\n\
Content-Length: 10\r\n\
Vary: Accept-Encoding\r\n\
Content-Type: text/plain\r\n\
\r\n\
1234567890\
"

#define HTTP_MSG_2 "\
GET /hello.txt HTTP/1.1\r\n\
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n\
Host: www.example.com\r\n\
Accept-Language: en, mi\r\n\
\r\n\
"

#define HTTP_MSG_2 "\
GET /hello.txt HTTP/1.1\r\n\
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n\
Host: www.example.com\r\n\
Accept-Language: en, mi\r\n\
\r\n\
"

#define HTTP_MSG_3 "\
GET HTTP/1.1\r\n\
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n\
Host: www.example.com\r\n\
Accept-Language: en, mi\r\n\
\r\n\
"

#define HTTP_MSG_4 "\
GET /hello.txt HTTP/1.1\r\n\
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n\
Host: www.example.com\r\n\
Accept-Language: en, mi\r\n\
invalid-content\r\n\
\r\n\
"

#define HTTP_MSG_5 "\
GET /hello.txt HTTP/1.1\r\n\
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n\
Host: www.example.com\r\n\
Accept-Language: en, mi\r\n\
: invalid-content\r\n\
\r\n\
"

typedef struct
{
    olchar_t * pstrHttp;
    u32 u32ErrCode;
} test_http_parser_t;

static u32 _testParseHttp(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    test_http_parser_t thp[] = {
        {HTTP_MSG_1, JF_ERR_NO_ERROR},
        {HTTP_MSG_2, JF_ERR_NO_ERROR},
        {HTTP_MSG_3, JF_ERR_INVALID_HTTP_HEADER_START_LINE},
        {HTTP_MSG_4, JF_ERR_INVALID_HTTP_HEADER_LINE},
        {HTTP_MSG_5, JF_ERR_INVALID_HTTP_HEADER_LINE},
    };
    u32 u32NumOfCase = sizeof(thp) / sizeof(test_http_parser_t);
    u32 u32Index;
    jf_httpparser_packet_header_t * pjhph = NULL;

    for (u32Index = 0; u32Index < u32NumOfCase; u32Index ++)
    {
        ol_printf("---------------------------------------------------\n");
        ol_printf("Parse http message:\n%s\n", thp[u32Index].pstrHttp);

        u32Ret = jf_httpparser_parsePacketHeader(
            &pjhph, thp[u32Index].pstrHttp, 0, strlen(thp[u32Index].pstrHttp));

        if (u32Ret != thp[u32Index].u32ErrCode)
        {
            u32Ret = JF_ERR_PROGRAM_ERROR;
            break;
        }
        else if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("Parse result:\n");            
            _printHttpPacketHeader(pjhph);
            jf_httpparser_destroyPacketHeader(&pjhph);
        }
        else
        {
            ol_printf("Parse result:\n");
            jf_err_readDescription(u32Ret, strErrMsg, 300);
            ol_printf("%s\n", strErrMsg);
        }

        ol_printf("\n");
        ol_printf("Http message after parse:\n%s\n", thp[u32Index].pstrHttp);
    }

    return u32Ret;
}

static u32 _testParseUri(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    olchar_t * pIp, * pPath;
    u16 u16Port;
    test_http_parser_t thp[] = {
        {"http:/192.168.1.1:8080/home/index.html", JF_ERR_INVALID_HTTP_URI},
        {"http://192.168.1.1:8080/home/index.html", JF_ERR_NO_ERROR},
        {"http://www.google.com/application/index.html", JF_ERR_NO_ERROR},
    };
    u32 u32NumOfCase = sizeof(thp) / sizeof(test_http_parser_t);
    u32 u32Index;

    for (u32Index = 0; u32Index < u32NumOfCase; u32Index ++)
    {
        ol_printf("---------------------------------------------------\n");

        u32Ret = jf_httpparser_parseUri(
            thp[u32Index].pstrHttp, &pIp, &u16Port, &pPath);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("URI: %s\n", thp[u32Index].pstrHttp);
            ol_printf("IP: %s\n", pIp);
            ol_printf("Port: %d\n", u16Port);
            ol_printf("Path: %s\n", pPath);

            jf_jiukun_freeMemory((void **)&pIp);
            jf_jiukun_freeMemory((void **)&pPath);
        }
        else if (u32Ret != thp[u32Index].u32ErrCode)
        {
            u32Ret = JF_ERR_PROGRAM_ERROR;
            break;
        }
        else
        {
            jf_err_readDescription(u32Ret, strErrMsg, 300);
            ol_printf("%s\n", strErrMsg);
        }

        ol_printf("\n");
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
    jlipParam.jlip_pstrCallerName = "HTTPPARSER-TEST";
    jlipParam.jlip_bLogToStdout = FALSE;
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseHttpparserTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bParseHttp)
            {
                u32Ret = _testParseHttp();
            }
            else if (ls_bParseUri)
            {
                u32Ret = _testParseUri();
            }
            else if (ls_bGenerateHttpMsg)
            {
                u32Ret = _testHttpMsg();
            }
            else 
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printHttpparserTestUsage();
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
