/**
 *  @file ifmgmt-test.c
 *
 *  @brief test file for ifmgmt library
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
#include "jf_basic.h"
#include "jf_limit.h"
#include "ifmgmt.h"
#include "jf_err.h"
#include "stringparse.h"
#include "process.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bShowIpInfo = FALSE;
static boolean_t ls_bShowIfInfo = FALSE;
static boolean_t ls_bShowAllIfInfo = FALSE;
static boolean_t ls_bUpIf = FALSE;
static boolean_t ls_bDownIf = FALSE;

static olchar_t * ls_pstrIfName = NULL;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: ifmgmt-test [-i] [-f if-name] [-u if-name] [-d if-name] [-a] \n\
    -i: show IP information and MAC address of first interface.\n\
    -a: show all interface information.\n\
    -f: show interface information.\n\
    -u: up interface\n\
    -d: down interface\n\
         \n");
    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "au:d:f:iph?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
        case 'i':
            ls_bShowIpInfo = TRUE;
            break;
        case 'f':
            ls_bShowIfInfo = TRUE;
            ls_pstrIfName = (olchar_t *)optarg;
            break;
        case 'a':
            ls_bShowAllIfInfo = TRUE;
            break;
        case 'u':
            ls_bUpIf = TRUE;
            ls_pstrIfName = (olchar_t *)optarg;
            break;
        case 'd':
            ls_bDownIf = TRUE;
            ls_pstrIfName = (olchar_t *)optarg;
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _showOneIfInfo(jf_ifmgmt_if_t * pif)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t str[JF_IFMGMT_IF_FLAGS_STR_SIZE];
 
    ol_printf("Interface: %s\n", pif->jii_strName);

    jf_ipaddr_getStringIpAddr(str, &pif->jii_jiAddr);
    ol_printf("  IP: %s\n", str);
    jf_ipaddr_getStringIpAddr(str, &pif->jii_jiNetmask);
    ol_printf("  Netmask: %s\n", str);
    jf_ipaddr_getStringIpAddr(str, &pif->jii_jiBroadcast);
    ol_printf("  Broadcast: %s\n", str);
    jf_string_getStringMACAddress(str, pif->jii_u8Mac);
    ol_printf("  Mac: %s\n", str);

    jf_ifmgmt_getStringIfFlags(str, pif);
    ol_printf("  Flags: %s\n", str);
 
    return u32Ret;
}

static u32 _showIfInfo(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ifmgmt_if_t jiif;

    u32Ret = jf_ifmgmt_getIf(name, &jiif);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _showOneIfInfo(&jiif);
    }

    return u32Ret;
}

static u32 _showAllIfInfo(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#define MAX_IF   (6)
    jf_ifmgmt_if_t jiif[MAX_IF];
    u32 u32NumOfIf = MAX_IF, index;

    u32Ret = jf_ifmgmt_getAllIf(jiif, &u32NumOfIf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Total %d interface(s).\n", u32NumOfIf);
        for (index = 0; index < u32NumOfIf; index ++)
        {
            u32Ret = _showOneIfInfo(&jiif[index]);
        }

    }

    return u32Ret;
}

static u32 _showIpInfo()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t ipaddr[16];
    u16 u16Count = 16, u16Index;
    olchar_t strIp[20];
    u8 u8Mac[JF_LIMIT_MAC_LEN];

    u32Ret = jf_ipaddr_getLocalIpAddrList(JF_IPADDR_TYPE_V4, &ipaddr[0], &u16Count);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Local IP Address List:\n");
        for (u16Index = 0; u16Index < u16Count; u16Index ++)
        {
            jf_ipaddr_getStringIpAddr(strIp, (jf_ipaddr_t *)&(ipaddr[u16Index]));
            ol_printf("    %s\n", strIp);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ifmgmt_getMacOfFirstIf(u8Mac);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_string_getStringMACAddress(strIp, u8Mac);
            ol_printf("MAC of First Interface: %s\n", strIp);
        }
        else
        {
            ol_printf("Failed to get MAC of First Interface.\n");
        }
    }

    return u32Ret;
}

static u32 _upIf(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_ifmgmt_upIf(name);
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("Interface is up\n");

    return u32Ret;
}

static u32 _downIf(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_ifmgmt_downIf(name);
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("Interface is down\n");

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_initSocket();
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bShowIpInfo)
                u32Ret = _showIpInfo();
            else if (ls_bShowIfInfo)
                u32Ret = _showIfInfo(ls_pstrIfName);
            else if (ls_bShowAllIfInfo)
                u32Ret = _showAllIfInfo();
            else if (ls_bUpIf)
                u32Ret = _upIf(ls_pstrIfName);
            else if (ls_bDownIf)
                u32Ret = _downIf(ls_pstrIfName);
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printUsage();
            }
        }

        jf_process_finiSocket();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

