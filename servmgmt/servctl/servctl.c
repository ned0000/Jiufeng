/**
 *  @file servctl.c
 *
 *  @brief Utility for service control.
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
#include "jf_err.h"
#include "jf_serv.h"
#include "jf_jiukun.h"
#include "jf_option.h"
#include "jf_file.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** List all service or specified service.
 */
static boolean_t ls_bList = FALSE;

/** Stop the specified service.
 */
static boolean_t ls_bStop = FALSE;

/** Start the specified service.
 */
static boolean_t ls_bStart = FALSE;

/** Set the startup type of a service.
 */
static boolean_t ls_bStartupType = FALSE;

/** Specify the service name.
 */
static olchar_t * ls_pstrServName = NULL;

/** Specify the service startup type.
 */
static u8 ls_u8StartupType = JF_SERV_STARTUP_TYPE_UNKNOWN;

/** The name of the executable file for this utility.
 */
static olchar_t ls_strServCtlProgramName[64];

/** The version of the program.
 */
static const olchar_t * ls_pstrServCtlVersion = "1.0.0";

/* --- private routine section ------------------------------------------------------------------ */

static void _printServCtlUsage(void)
{
    ol_printf("\
Usage: %s [-l] [-s] [-t] [-u automatic|manual] [-n service name] [-V] [logger options]\n\
    -l list service. List all services is \"-n\" is not specified.\n\
    -t start service.\n\
    -s stop service.\n\
    -u <automatic|manual> change the startup type of the service.\n\
    -n specify the service name.\n\
    -V show version information.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n",
              ls_strServCtlProgramName);

    ol_printf("\n");

}

static u32 _parseServCtlCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "ln:u:stVOT:F:S:h?")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printServCtlUsage();
            exit(0);
            break;
        case 'u':
            ls_bStartupType = TRUE;
            u32Ret = jf_serv_getServStartupTypeFromString(optarg, &ls_u8StartupType);
            break;
        case 'l':
            ls_bList = TRUE;
            break;
        case 'n':
            ls_bList = TRUE;
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
            ol_printf("%s %s\n", ls_strServCtlProgramName, ls_pstrServCtlVersion);
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

#define SERVICE_CAPTION  "%-12s %-12s %-10s\n"

static void _printServiceCaption(void)
{
    ol_printf(SERVICE_CAPTION, "Name", "StartupType", "Status");
    ol_printf("-----------------------------------------------------------------\n");
}

static u32 _listService(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_serv_info_t jsi;

    u32Ret = jf_serv_getInfo(name, &jsi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printServiceCaption();
        ol_printf(
            SERVICE_CAPTION, jsi.jsi_strName,
            jf_serv_getStringServStartupType(jsi.jsi_u8StartupType),
            jf_serv_getStringServStatus(jsi.jsi_u8Status));
    }
    else
    {
        ol_printf("Failed to get service information. %s\n", jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

static u32 _listAllServices(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Buffer[2048];
    jf_serv_info_list_t * pjsil = (jf_serv_info_list_t *)u8Buffer;
    jf_serv_info_t * pjsi = NULL;
    u16 u16Index;

    u32Ret = jf_serv_getInfoList(pjsil);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printServiceCaption();

        for (u16Index = 0; u16Index < pjsil->jsil_u16NumOfService; u16Index ++)
        {
            pjsi = &pjsil->jsil_jsiService[u16Index];

            ol_printf(
                SERVICE_CAPTION, pjsi->jsi_strName,
                jf_serv_getStringServStartupType(pjsi->jsi_u8StartupType),
                jf_serv_getStringServStatus(pjsi->jsi_u8Status));
        }
    }
    else
    {
        ol_printf("Failed to get service information, %s\n", jf_err_getDescription(u32Ret));
    }

    return u32Ret;
}

static u32 _stopService(olchar_t * name)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (name == NULL)
    {
        ol_printf("Service name is not specified\n");
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
        ol_printf("Service name is not specified\n");
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
        ol_printf("Service name is not specified\n");
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_serv_setServStartupType(name, u8Type);

    return u32Ret;
}

static u32 _processServCtlCommand(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_serv_init_param_t jsip;

    ol_bzero(&jsip, sizeof(jsip));

    /*Initialize the service library.*/
    u32Ret = jf_serv_init(&jsip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bStop)
        {
            u32Ret = _stopService(ls_pstrServName);
        }
        else if (ls_bStart)
        {
            u32Ret = _startService(ls_pstrServName);
        }
        else if (ls_bStartupType)
        {
            u32Ret = _changeServiceStartupType(ls_pstrServName, ls_u8StartupType);
        }
        else if (ls_bList)
        {
            if (ls_pstrServName != NULL)
                u32Ret = _listService(ls_pstrServName);                
            else
                u32Ret = _listAllServices();
        }
        else
        {
            ol_printf("Operation is not specified!\n");
            u32Ret = JF_ERR_INVALID_PARAM;
        }

        jf_serv_fini();
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

    /*Get programe name of the service.*/
    jf_file_getFileName(ls_strServCtlProgramName, sizeof(ls_strServCtlProgramName), argv[0]);

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "JF_SERV";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseServCtlCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Initialize the logger library.*/
        jf_logger_init(&jlipParam);

        /*Initialize the jiukun library.*/
        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Process the service control command.*/
            u32Ret = _processServCtlCommand();

            jf_jiukun_fini();
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

