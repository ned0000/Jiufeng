/**
 *  @file conffile-test.c
 *
 *  @brief the test file for conffile common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "conffile.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: conffile-test [-h] conffile\n\
         ");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
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
    {"b", "default-b", 0},
    {"c", "default-c", 0},
    {"d", "default-d", 0},
    {"e", "default-e", 0},
    {"aa", "default-aa", 0},
    {"bb", NULL, 2342},
    {"bc", NULL, 2312312},
    {"abcd", "default-abcd", 0},
};

static u32 ls_u32NumOfConfFileTag = sizeof(ls_cftConfFileTag) / \
    sizeof(conf_file_tag_t);

static u32 _testConfFile(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    conf_file_t cfConfFile;
    conf_file_t * pcf = &cfConfFile;
    u32 u32Index;
    olchar_t strValue[MAX_CONFFILE_LINE_LEN];
    olint_t nValue;

    ol_printf("conffile: %s\n", pstrFilename);

    u32Ret = openConfFile(pcf, pstrFilename);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32Index = 0; u32Index < ls_u32NumOfConfFileTag; u32Index ++)
        {
            if (ls_cftConfFileTag[u32Index].cft_pstrDefault == NULL)
            {
                u32Ret = getConfFileInt(pcf,
                    ls_cftConfFileTag[u32Index].cft_pstrTag,
                    ls_cftConfFileTag[u32Index].cft_nDefault,
                    &nValue);
                if (u32Ret == JF_ERR_NO_ERROR)
                    ol_printf("%s=%d(%d)\n", ls_cftConfFileTag[u32Index].cft_pstrTag,
                        nValue, ls_cftConfFileTag[u32Index].cft_nDefault);
                else
                    ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
            }
            else
            {
                u32Ret = getConfFileString(pcf,
                    ls_cftConfFileTag[u32Index].cft_pstrTag,
                    ls_cftConfFileTag[u32Index].cft_pstrDefault,
                    strValue, sizeof(strValue));
                if (u32Ret == JF_ERR_NO_ERROR)
                    ol_printf("%s=%s(%s)\n", ls_cftConfFileTag[u32Index].cft_pstrTag,
                        strValue, ls_cftConfFileTag[u32Index].cft_pstrDefault);
                else
                    ol_printf("%s error\n", ls_cftConfFileTag[u32Index].cft_pstrTag);
            }
        }

        closeConfFile(pcf);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    if (argc < 2)
    {
        ol_printf("Missing parameters!!!\n\n");
        _printUsage();
        exit(0);
    }

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testConfFile(argv[argc - 1]);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    exit(0);
}

/*--------------------------------------------------------------------------*/

