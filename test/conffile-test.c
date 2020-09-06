/**
 *  @file conffile-test.c
 *
 *  @brief Test file for configuration file function defined in jf_files library.
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
#include "jf_conffile.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define  CONFFILE_TEST_FILE_NAME      "conf.txt"

/* --- private routine section ------------------------------------------------------------------ */

static void _printConffileTestUsage(void)
{
    ol_printf("\
Usage: conffile-test [-h] [logger options] \n\
  -h: show this usage.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");
    ol_printf("\n");
}

static u32 _parseConffileTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "h?T:F:S:O")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printConffileTestUsage();
            exit(0);
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
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

typedef struct
{
    olchar_t * cft_pstrTag;
    olchar_t * cft_pstrValue;
    olchar_t * cft_pstrDefault;
    olint_t cft_nDefault;
} conf_file_tag_t;

static conf_file_tag_t ls_cftConfFileTag[] =
{
    {"a", "active", "default-a", 0},
    {"bga.aa", "", "default-b", 0},
    {"c", "call", "default-c", 0},
    {"d", "desk", "default-d", 0},
    {"e", "egg", "default-e", 0},
    {"sysctl.cc", "cc", "default-aa", 0},
    {"bb", "2342", NULL, 2000},
    {"bc", "2312312", NULL, 2000000},
    {"abcd", "char", "default-abcd", 0},

    {"no-such-tag", "", "", 0},
    {"count", "", "35", 0},
    {"discount", "", NULL, 790},
};

static u32 ls_u32NumOfConfFileTag = sizeof(ls_cftConfFileTag) / sizeof(conf_file_tag_t);

static u32 _testReadConfFile(jf_conffile_t * pjc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;
    olchar_t strValue[JF_CONFFILE_MAX_LINE_LEN];
    olint_t nValue;

    ol_printf("--------------------------------------------\n");
    ol_printf("read config from file\n");

    for (u32Index = 0; u32Index < ls_u32NumOfConfFileTag; u32Index ++)
    {
        if (ls_cftConfFileTag[u32Index].cft_pstrDefault == NULL)
        {
            u32Ret = jf_conffile_getInt(
                pjc, ls_cftConfFileTag[u32Index].cft_pstrTag,
                ls_cftConfFileTag[u32Index].cft_nDefault, &nValue);
            if (u32Ret == JF_ERR_NO_ERROR)
                ol_printf("%s=%d\n", ls_cftConfFileTag[u32Index].cft_pstrTag, nValue);
            else
                ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
        }
        else
        {
            u32Ret = jf_conffile_get(
                pjc, ls_cftConfFileTag[u32Index].cft_pstrTag,
                ls_cftConfFileTag[u32Index].cft_pstrDefault, strValue, sizeof(strValue));
            if (u32Ret == JF_ERR_NO_ERROR)
                ol_printf("%s=%s\n", ls_cftConfFileTag[u32Index].cft_pstrTag, strValue);
            else
                ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
        }
    }

    return u32Ret;
}

static u32 _testWriteConfFile(jf_conffile_t * pjc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;

    ol_printf("--------------------------------------------\n");
    ol_printf("write config to file\n");

    for (u32Index = 0; u32Index < ls_u32NumOfConfFileTag - 3; u32Index ++)
    {
        u32Ret = jf_conffile_set(
            pjc, ls_cftConfFileTag[u32Index].cft_pstrTag,
            ls_cftConfFileTag[u32Index].cft_pstrValue);

        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf(
                "%s=%s\n", ls_cftConfFileTag[u32Index].cft_pstrTag,
                ls_cftConfFileTag[u32Index].cft_pstrValue);
    }

    return u32Ret;
}

static u32 _fnTestConffileHandleConfig(
    olchar_t * pstrName, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjcWrite = (jf_conffile_t *)pArg;

    ol_printf(
        "name: %s(%d), value: %s(%d)\n", pstrName, (olint_t)ol_strlen(pstrName), pstrValue,
        (olint_t)ol_strlen(pstrValue));

    u32Ret = jf_conffile_write(pjcWrite, pstrName, pstrValue);

    return u32Ret;
}

static u32 _testTraversalConfFile(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL, * pjcWrite = NULL;
    jf_conffile_open_param_t jcop;
    olchar_t * pstrFile = "conf.temp";

    ol_printf("--------------------------------------------\n");
    ol_printf("tranverse config file: %s\n", pstrFilename);
    ol_printf("write config to file: %s\n", pstrFile);

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = (olchar_t *)pstrFilename;

    u32Ret = jf_conffile_open(&jcop, &pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jcop, sizeof(jcop));
        jcop.jcop_pstrFile = pstrFile;

        u32Ret = jf_conffile_open(&jcop, &pjcWrite);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_conffile_traverse(pjc, _fnTestConffileHandleConfig, pjcWrite);
    }

    if (pjc != NULL)
        jf_conffile_close(&pjc);

    if (pjcWrite != NULL)
        jf_conffile_close(&pjcWrite);

    return u32Ret;
}

static u32 _testConfFile(olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL;
    jf_conffile_open_param_t jcop;

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = pstrFilename;

    ol_printf("--------------------------------------------\n");
    ol_printf("open config file: %s\n", pstrFilename);

    u32Ret = jf_conffile_open(&jcop, &pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testWriteConfFile(pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testReadConfFile(pjc);

    if (pjc != NULL)
        jf_conffile_close(&pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testTraversalConfFile(pstrFilename);

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
    jlipParam.jlip_pstrCallerName = "CONFFILE-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseConffileTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testConfFile(CONFFILE_TEST_FILE_NAME);

            jf_jiukun_fini();
        }

        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    exit(0);
}

/*------------------------------------------------------------------------------------------------*/
