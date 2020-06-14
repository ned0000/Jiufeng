/**
 *  @file archive-test.c
 *
 *  @brief Test file for archive function defined in jf_archive library.
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
#include "jf_linklist.h"
#include "jf_err.h"
#include "jf_archive.h"
#include "jf_string.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrArchiveName = NULL;
static boolean_t ls_bCreateArchive;
static boolean_t ls_bExtractArchive;
static boolean_t ls_bListArchive;
static boolean_t ls_bVerbose;

#define ARCHIVE_TEST_OPT_STRING  "ctxvm:f:T:F:S:h"

/* --- private routine section ------------------------------------------------------------------ */

static void _printArchiveTestUsage(void)
{
    ol_printf("\
Usage: archive-test [-c | -x | -t] -f <archive file> [-m <member file>] [-v] [logger options] \n\
    -c create archive file.\n\
    -x extract archive file.\n\
    -t list archive file.\n\
    -f specify the archive file.\n\
    -m the member files added to archive file. Multiple files are supported.\n\
    -v be verbose.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
");

    ol_printf("\n");
}

static u32 _parseArchiveTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, ARCHIVE_TEST_OPT_STRING)) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printArchiveTestUsage();
            exit(0);
            break;
        case 'c':
            ls_bCreateArchive = TRUE;
            break;
        case 'm':
            /*parse this option in another function*/
            break;
        case 't':
            ls_bExtractArchive = TRUE;
            ls_bListArchive = TRUE;
            break;
        case 'x':
            ls_bExtractArchive = TRUE;
            break;
        case 'f':
            ls_pstrArchiveName = optarg;
            break;
        case 'v':
            ls_bVerbose = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
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

static u32 _parseArchiveTestCreateParam(
    olint_t argc, olchar_t ** argv, jf_linklist_t * pjl, jf_archive_create_param_t * pjacp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    optind = 0;
    while (((nOpt = getopt(argc, argv, ARCHIVE_TEST_OPT_STRING)) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'm':
            u32Ret = jf_linklist_appendTo(pjl, optarg);
            break;
        default:
            break;
        }
    }

    return u32Ret;
}

static u32 _processArchiveTestCommand(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_archive_create_param_t jacp;
    jf_archive_extract_param_t jaep;
    jf_linklist_t jlMemberFile;

    if (ls_bCreateArchive)
    {
        jf_linklist_init(&jlMemberFile);
        ol_bzero(&jacp, sizeof(jacp));

        u32Ret = _parseArchiveTestCreateParam(argc, argv, &jlMemberFile, &jacp);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (jf_linklist_isEmpty(&jlMemberFile))
            {
                ol_printf("Member files are not specified!!!\n");
                _printArchiveTestUsage();
                u32Ret = JF_ERR_MISSING_PARAM;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jacp.jacp_bVerbose = ls_bVerbose;
            u32Ret = jf_archive_create(&jlMemberFile, ls_pstrArchiveName, &jacp);
        }

        jf_linklist_fini(&jlMemberFile);
    }
    else if (ls_bExtractArchive)
    {
        ol_bzero(&jaep, sizeof(jaep));
        jaep.jaep_bListArchive = ls_bListArchive;
        jaep.jaep_bVerbose = ls_bVerbose;
        u32Ret = jf_archive_extract(ls_pstrArchiveName, &jaep);
    }
    else
    {
        jf_logger_logInfoMsg("Unrecognized action");
        u32Ret = JF_ERR_INVALID_PARAM;
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

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "ARCHIVE";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseArchiveTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        if (ls_pstrArchiveName == NULL)
        {
            ol_printf("Archive file is not specified!!!\n");
            _printArchiveTestUsage();
            u32Ret = JF_ERR_MISSING_PARAM;
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_jiukun_init(&jjip);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _processArchiveTestCommand(argc, argv);

                jf_jiukun_fini();
            }
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
