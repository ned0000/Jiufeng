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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_sem.h"
#include "jf_mem.h"
#include "jf_serv.h"

/* --- private data/data structure section ------------------------------------------------------ */
static boolean_t ls_bList = FALSE;
static boolean_t ls_bStop = FALSE;
static boolean_t ls_bStart = FALSE;
static boolean_t ls_bStartupType = FALSE;
static olchar_t * ls_pstrServName = NULL;
static u8 ls_u8StartupType = JF_SERV_STARTUPTYPE_UNKNOWN;

static const olchar_t * ls_pstrProgramName = "jf_serv";
static const olchar_t * ls_pstrVersion = "1.0.0";

/* --- private routine section ------------------------------------------------------------------ */
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

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "ln:u:stVOT:F:S:h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'u':
            ls_bStartupType = TRUE;
            u32Ret = jf_serv_getServStartupTypeFromString(
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
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        case 'V':
            ol_printf("%s %s\n", ls_pstrProgramName, ls_pstrVersion);
            exit(0);
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_sLogFile = u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _listService(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_serv_t js;
    jf_serv_info_t * pjsi;
    u8 u8Index;

    u32Ret = jf_serv_getInfo(&js);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("%-12s %-12s %-10s\n", "Name", "StartupType", "Status");
        ol_printf("----------------------------------------------\n");

        for (u8Index = 0; u8Index < js.js_u16NumOfService; u8Index ++)
        {
            pjsi = &js.js_jsiService[u8Index];
            if (name != NULL && ol_strcmp(name, pjsi->jsi_strName) != 0)
                continue;

            ol_printf(
                "%-12s %-12s %-10s\n", pjsi->jsi_strName,
                jf_serv_getStringServStartupType(pjsi->jsi_u8StartupType),
                jf_serv_getStringServStatus(pjsi->jsi_u8Status));
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
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_serv_stopServ(name);

    return u32Ret;
}

static u32 _startService(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_serv_startServ(name);

    return u32Ret;
}

static u32 _changeServiceStartupType(olchar_t * name, u8 u8Type)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("service name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_serv_setServStartupType(name, u8Type);

    return u32Ret;
}
/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_serv_init_param_t jsip;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));

    jlipParam.jlip_pstrCallerName = "JF_SERV";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        ol_memset(&jsip, 0, sizeof(jsip));

        u32Ret = jf_serv_init(&jsip);
        if (u32Ret == JF_ERR_NO_ERROR)
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

            jf_serv_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

