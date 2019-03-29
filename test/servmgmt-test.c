/**
 *  @file servmgmt-test.c
 *
 *  @brief test file for servmgmt library
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
#include "syncsem.h"
#include "xmalloc.h"
#include "servmgmt.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bList = FALSE;
static boolean_t ls_bStop = FALSE;
static boolean_t ls_bStart = FALSE;
static boolean_t ls_bStartupType = FALSE;
static olchar_t * ls_pstrServName = NULL;
static u8 ls_u8StartupType = JF_SERVMGMT_SERV_STARTUPTYPE_UNKNOWN;

static const olchar_t * ls_pstrProgramName = "olservmgmt";
static const olchar_t * ls_pstrVersion = "1.0.0";

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: %s [-l] [-s] [-t] [-u automatic|manual] [-n service name] \n\
      [-V] [logger options]\n\
    -l list service.\n\
    -t start service.\n\
    -s stop service.\n\
    -u <automatic|manual> change the startup type of the service.\n\
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

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "ln:u:stVOT:F:S:h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'u':
            ls_bStartupType = TRUE;
            u32Ret = jf_servmgmt_getServStartupTypeFromString(
                optarg, &ls_u8StartupType);
            break;
        case 'l':
            ls_bList = TRUE;
            break;
        case 'n':
            ls_pstrServName = optarg;
            break;
        case 's':
            ls_bStop = TRUE;
            break;
        case 't':
            ls_bStart = TRUE;
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
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
            if (sscanf(optarg, "%d", &u32Value) == 1)
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

static u32 _listService(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;
    jf_servmgmt_info_t jsi;
    jf_servmgmt_serv_info_t * pjssi;
    u8 u8Index;

    u32Ret = jf_servmgmt_getInfo(&jsi);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("%-12s %-12s %-10s\n", "Name", "StartupType", "Status");
        ol_printf("----------------------------------------------\n");

        for (u8Index = 0; u8Index < jsi.jsi_u8NumOfService; u8Index ++)
        {
            pjssi = &jsi.jsi_jssiService[u8Index];
            if (name != NULL &&
                ol_strcmp(name, pjssi->jssi_strName) != 0)
                continue;

            ol_printf(
                "%-12s %-12s %-10s\n", pjssi->jssi_strName,
                jf_servmgmt_getStringServStartupType(pjssi->jssi_u8StartupType),
                jf_servmgmt_getStringServStatus(pjssi->jssi_u8Status));
        }
    }
    else
    {
        ol_printf("Failed to get service information\n");
    }

    return u32Ret;
}

static u32 _stopService(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = OLERR_INVALID_PARAM;
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = jf_servmgmt_stopServ(name);

    return u32Ret;
}

static u32 _startService(olchar_t * name)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = OLERR_INVALID_PARAM;
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = jf_servmgmt_startServ(name);

    return u32Ret;
}

static u32 _changeServiceStartupType(olchar_t * name, u8 u8Type)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = OLERR_INVALID_PARAM;
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = jf_servmgmt_setServStartupType(name, u8Type);

    return u32Ret;
}
/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    logger_param_t lpParam;
    jf_servmgmt_init_param_t jsip;

    memset(&lpParam, 0, sizeof(logger_param_t));

    lpParam.lp_pstrCallerName = "SERVMGMT-TEST";
//    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = 0;

    u32Ret = _parseCmdLineParam(argc, argv, &lpParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        initLogger(&lpParam);

        ol_memset(&jsip, 0, sizeof(jsip));

        u32Ret = jf_servmgmt_init(&jsip);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (ls_bStop)
                u32Ret = _stopService(ls_pstrServName);
            else if (ls_bStart)
                u32Ret = _startService(ls_pstrServName);
            else if (ls_bStartupType)
                u32Ret = _changeServiceStartupType(
                    ls_pstrServName, ls_u8StartupType);
            else
                u32Ret = _listService(ls_pstrServName);                

            jf_servmgmt_fini();
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

