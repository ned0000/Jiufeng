/**
 *  @file ipaddr-test.c
 *
 *  @brief Test file for IP address function defined in jf_ifmgmt library.
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
#include "jf_ipaddr.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_option.h"
#include "jf_jiukun.h"
#include "jf_process.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTestIpaddrString = FALSE;

static boolean_t ls_bTestIpaddrList = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printIpaddrTestUsage(void)
{
    ol_printf("\
Usage: ipaddr-test [-l] [-s] [-h] \n\
  -l: get local IP address list.\n\
  -s: test address string.\n\
  -h: show this usage.\n");

    ol_printf("\n");
}

static u32 _parseIpaddrTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lsh?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printIpaddrTestUsage();
            exit(0);
        case 's':
            ls_bTestIpaddrString = TRUE;
            break;
        case 'l':
            ls_bTestIpaddrList = TRUE;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testIpaddrString(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrIpv4[] = {
        "192.168.10.1:80",
        "192.168.10.1:65536",
        "192.168.1000.1:24",
        "23dfsf",
        "10.1.1.1:23456",
        "192.168.0.1"};
    olint_t i = 0;
    jf_ipaddr_t ipaddr;
    u16 u16Port = 0;

    for (i = 0; i < ARRAY_SIZE(pstrIpv4); i ++)
    {
        u32Ret = jf_ipaddr_getIpAddrPortFromString(pstrIpv4[i], JF_IPADDR_TYPE_V4, &ipaddr, &u16Port);

        ol_printf("IPv4 string: %s\n", pstrIpv4[i]);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            olchar_t str[100];
            jf_ipaddr_getStringIpAddr(str, sizeof(str), &ipaddr);
            ol_printf("Address: %s, Port: %u\n", str, u16Port);
        }
        else
        {
            ol_printf("%s\n", jf_err_getDescription(u32Ret));
        }
        ol_printf("\n");
    }

    return u32Ret;
}

static u32 _testIpaddrList(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t ipaddr[16];
    u16 u16Count = 16, index = 0;
    olchar_t strIp[100];

    u32Ret = jf_ipaddr_getLocalIpAddrList(JF_IPADDR_TYPE_V4, ipaddr, &u16Count);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (index = 0; index < u16Count; index ++)
        {
            jf_ipaddr_getStringIpAddr(strIp, sizeof(strIp), &ipaddr[index]);
            ol_printf("Interface %u, %s\n", index, strIp);;
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseIpaddrTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_initSocket();

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bTestIpaddrString)
            {
                u32Ret = _testIpaddrString();
            }
            else if (ls_bTestIpaddrList)
            {
                u32Ret = _testIpaddrList();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printIpaddrTestUsage();
            }

            jf_process_finiSocket();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
