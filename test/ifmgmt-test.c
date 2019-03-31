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
#include "olbasic.h"
#include "ollimit.h"
#include "comminit.h"
#include "ifmgmt.h"
#include "errcode.h"
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
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "au:d:f:iph?")) != -1) && (u32Ret == OLERR_NO_ERROR))
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
            u32Ret = OLERR_MISSING_PARAM;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _showOneIfInfo(ifmgmt_if_t * pif)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t str[IF_FLAGS_STR_SIZE];
 
    ol_printf("Interface: %s\n", pif->ii_strName);

    getStringIpAddr(str, &pif->ii_iaAddr);
    ol_printf("  IP: %s\n", str);
    getStringIpAddr(str, &pif->ii_iaNetmask);
    ol_printf("  Netmask: %s\n", str);
    getStringIpAddr(str, &pif->ii_iaBroadcast);
    ol_printf("  Broadcast: %s\n", str);
    getStringMACAddress(str, pif->ii_u8Mac);
    ol_printf("  Mac: %s\n", str);

    getStringIfmgmtIfFlags(str, pif);
    ol_printf("  Flags: %s\n", str);
 
    return u32Ret;
}

static u32 _showIfInfo(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;
    ifmgmt_if_t iif;

    u32Ret = getIfmgmtIf(name, &iif);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _showOneIfInfo(&iif);
    }

    return u32Ret;
}

static u32 _showAllIfInfo(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
#define MAX_IF   (6)
    ifmgmt_if_t iif[MAX_IF];
    u32 u32NumOfIf = MAX_IF, index;

    u32Ret = getAllIfmgmtIf(iif, &u32NumOfIf);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Total %d interface(s).\n", u32NumOfIf);
        for (index = 0; index < u32NumOfIf; index ++)
        {
            u32Ret = _showOneIfInfo(&iif[index]);
        }

    }

    return u32Ret;
}

static u32 _showIpInfo()
{
    u32 u32Ret = OLERR_NO_ERROR;
    ip_addr_t ipaddr[16];
    u16 u16Count = 16, u16Index;
    olchar_t strIp[20];
    u8 u8Mac[MAC_LEN];

    u32Ret = getLocalIpAddrList(IP_ADDR_TYPE_V4, &ipaddr[0], &u16Count);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Local IP Address List:\n");
        for (u16Index = 0; u16Index < u16Count; u16Index ++)
        {
            getStringIpAddr(strIp, (ip_addr_t *)&(ipaddr[u16Index]));
            ol_printf("    %s\n", strIp);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getMacOfFirstIf(u8Mac);
        if (u32Ret == OLERR_NO_ERROR)
        {
            getStringMACAddress(strIp, u8Mac);
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
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = upIfmgmtIf(name);
    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("Interface is up\n");

    return u32Ret;
}

static u32 _downIf(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = downIfmgmtIf(name);
    if (u32Ret == OLERR_NO_ERROR)
        ol_printf("Interface is down\n");

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = jf_network_initLib();
        if (u32Ret == OLERR_NO_ERROR)
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

        jf_network_finiLib();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

