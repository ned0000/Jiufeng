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

/* --- private data/data structure section ------------------------------------------------------ */

/* Example of configuration file.
# test
system.aa=7
system.bb=mon
bga.aa = 5
bga.aa=10
bga.dd=  
 bga.ee=
   bga.ee  =               
 sysctl.aa = yu
  sysctl.cc    = ui

 */

/* --- private routine section ------------------------------------------------------------------ */

static void _printConffileTestUsage(void)
{
    ol_printf("\
Usage: conffile-test [-h] conffile\n\
         ");

    ol_printf("\n");
}

static u32 _parseConffileTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printConffileTestUsage();
            exit(0);
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

typedef struct
{
    olchar_t * cft_pstrTag;
    olchar_t * cft_pstrDefault;
    olint_t cft_nDefault;
} conf_file_tag_t;

static conf_file_tag_t ls_cftConfFileTag[] =
{
    {"a", "default-a", 0},
    {"bga.aa", "default-b", 0},
    {"c", "default-c", 0},
    {"d", "default-d", 0},
    {"e", "default-e", 0},
    {"sysctl.cc", "default-aa", 0},
    {"bb", NULL, 2342},
    {"bc", NULL, 2312312},
    {"abcd", "default-abcd", 0},
};

static u32 ls_u32NumOfConfFileTag = sizeof(ls_cftConfFileTag) / sizeof(conf_file_tag_t);

static u32 _testConfFile(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL;
    u32 u32Index;
    olchar_t strValue[JF_CONFFILE_MAX_LINE_LEN];
    olint_t nValue;
    jf_conffile_open_param_t jcop;

    ol_printf("conffile: %s\n", pstrFilename);

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = (olchar_t *)pstrFilename;

    u32Ret = jf_conffile_open(&jcop, &pjc);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0; u32Index < ls_u32NumOfConfFileTag; u32Index ++)
        {
            if (ls_cftConfFileTag[u32Index].cft_pstrDefault == NULL)
            {
                u32Ret = jf_conffile_getInt(
                    pjc, ls_cftConfFileTag[u32Index].cft_pstrTag,
                    ls_cftConfFileTag[u32Index].cft_nDefault, &nValue);
                if (u32Ret == JF_ERR_NO_ERROR)
                    ol_printf(
                        "%s=%d(%d)\n", ls_cftConfFileTag[u32Index].cft_pstrTag,
                        nValue, ls_cftConfFileTag[u32Index].cft_nDefault);
                else
                    ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
            }
            else
            {
                u32Ret = jf_conffile_getString(
                    pjc, ls_cftConfFileTag[u32Index].cft_pstrTag,
                    ls_cftConfFileTag[u32Index].cft_pstrDefault, strValue, sizeof(strValue));
                if (u32Ret == JF_ERR_NO_ERROR)
                    ol_printf(
                        "%s=%s(%s)\n", ls_cftConfFileTag[u32Index].cft_pstrTag,
                        strValue, ls_cftConfFileTag[u32Index].cft_pstrDefault);
                else
                    ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
            }
        }

        jf_conffile_close(&pjc);
    }

    return u32Ret;
}

static u32 _fnTestConffileHandleConfig(
    olchar_t * pstrName, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjcWrite = (jf_conffile_t *)pArg;
    olchar_t str[JF_CONFFILE_MAX_LINE_LEN];
    olsize_t sStr;

    ol_printf("name: %s(%ld), value: %s(%ld)\n", pstrName, ol_strlen(pstrName), pstrValue, ol_strlen(pstrValue));

    sStr = snprintf(str, sizeof(str), "%s=%s\n", pstrName, pstrValue);

    u32Ret = jf_conffile_write(pjcWrite, str, sStr);

    return u32Ret;
}

static u32 _testTraversalConfFile(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL, * pjcWrite = NULL;
    jf_conffile_open_param_t jcop;

    ol_printf("tranversal conffile: %s\n", pstrFilename);

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = (olchar_t *)pstrFilename;

    u32Ret = jf_conffile_open(&jcop, &pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&jcop, sizeof(jcop));
        jcop.jcop_pstrFile = "temp.conf";
        jcop.jcop_bWrite = TRUE;

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

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "CONFFILE-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    if (argc < 2)
    {
        ol_printf("Missing parameters!!!\n\n");
        _printConffileTestUsage();
        exit(0);
    }

    u32Ret = _parseConffileTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testTraversalConfFile(argv[argc - 1]);

            if (u32Ret == JF_ERR_NO_ERROR)
                _testConfFile(argv[argc - 1]);

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
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
