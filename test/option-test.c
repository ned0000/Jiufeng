/**
 *  @file option-test.c
 *
 *  @brief Test file for option function defined in jf_option common object.
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
#include "jf_option.h"
#include "jf_jiukun.h"
#include "jf_string.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Version of the option test.
 */
static olchar_t * ls_pstrOptionTestVersion = "1.0.0";

static boolean_t ls_bOptionTestActive = FALSE;

static u16 ls_u16OptionTestPort = FALSE;

static olchar_t * ls_pstrOptionTestSettingFile = NULL;

static olchar_t * ls_pstrOptionTestDir = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static void _printOptionTestUsage(void)
{
    ol_printf("\
Usage: option-test [-a] [-p port] [-d dir] [-s setting-file] [-V] [logger options]\n\
  -a: active the test.\n\
  -p: specify the port.\n\
  -d: specify the directory.\n\
  -s: specify the setting file.\n\
  -V: show version information.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n");

    ol_printf("\n");
}

static u32 _parseOptionTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) &&
           ((nOpt = jf_option_get(argc, argv, "ap:d:s:VT:F:S:Oh")) != -1))
           
    {
        switch (nOpt)
        {
        case 'a':
            ls_bOptionTestActive = TRUE;
            break;
        case 'p':
            u32Ret = jf_option_getU16FromString(jf_option_getArg(), &ls_u16OptionTestPort);
            break;
        case 'd':
            ls_pstrOptionTestDir = jf_option_getArg();
            break;
        case 's':
            ls_pstrOptionTestSettingFile = jf_option_getArg();
            break;
        case ':':
        case '?':
        case 'h':
            _printOptionTestUsage();
            exit(0);
        case 'V':
            ol_printf("Version: %s\n", ls_pstrOptionTestVersion);
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

static u32 _checkOptionTestOptions(jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Active           : %s\n", jf_string_getStringPositive(ls_bOptionTestActive));
    ol_printf("Port             : %u\n", ls_u16OptionTestPort);

    if (ls_pstrOptionTestSettingFile == NULL)
        ol_printf("Setting File     : NULL\n");
    else
        ol_printf("Setting File     : %s\n", ls_pstrOptionTestSettingFile);

    if (ls_pstrOptionTestDir == NULL)
        ol_printf("Dir              : NULL\n");
    else
        ol_printf("Dir              : %s\n", ls_pstrOptionTestDir);

    ol_printf("Log Level        : %u\n", pjlip->jlip_u8TraceLevel);
    ol_printf("Log To Stdout    : %s\n", jf_string_getStringPositive(pjlip->jlip_bLogToStdout));
    ol_printf("Log To File      : %s\n", jf_string_getStringPositive(pjlip->jlip_bLogToFile));
    ol_printf("Size Of Log File : %d\n", pjlip->jlip_sLogFile);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "OPTION-TEST";

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseOptionTestCmdLineParam(argc, argv, &jlipParam);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Init the logger.*/
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _checkOptionTestOptions(&jlipParam);

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        ol_printf("ERR: %s\n", jf_err_getDescription(u32Ret));
        
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
