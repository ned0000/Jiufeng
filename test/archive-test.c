/**
 *  @file archive-test.c
 *
 *  @brief The test file for archive library
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
#include "bases.h"
#include "errcode.h"
#include "archive.h"

/* --- private data/data structure section --------------------------------- */
static olchar_t * ls_pstrArchiveName = NULL;
static jf_linklist_t ls_jlMemberFile;
static boolean_t ls_bCreateArchive;
static boolean_t ls_bExtractArchive;
static boolean_t ls_bListArchive;
static boolean_t ls_bVerbose;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: archive-test [-c | -x | -t] -f <archive file> [ -m <member file> ]\n\
    [logger options] \n\
    -c create archive file.\n\
    -x extract archive file.\n\
    -t list archive file.\n\
    -f specify the archive file.\n\
    -m the member files added to archive file. Multiple files are supported.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, 
    jf_archive_create_param_t * pjacp, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "ctxvm:f:T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 'c':
            ls_bCreateArchive = TRUE;
            break;
        case 'm':
            u32Ret = jf_linklist_appendTo(&ls_jlMemberFile, optarg);
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
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                pjlip->jlip_sLogFile = u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_PARAM;
            }
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_archive_create_param_t jacp;
    jf_archive_extract_param_t jaep;
    jf_logger_init_param_t jlipParam;

    memset(&jacp, 0, sizeof(jf_archive_create_param_t));
    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "ARCHIVE";

    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;
    jf_logger_init(&jlipParam);

    jf_linklist_init(&ls_jlMemberFile);

    u32Ret = _parseCmdLineParam(argc, argv, &jacp, &jlipParam);
    if ((u32Ret == JF_ERR_NO_ERROR) && (ls_bCreateArchive))
    {
        if (jf_linklist_isEmpty(&ls_jlMemberFile))
        {
            ol_printf("Member files are not specified!!!\n");
            _printUsage();
            u32Ret = JF_ERR_MISSING_PARAM;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_pstrArchiveName == NULL)
        {
            ol_printf("Archive file is not specified!!!\n");
            _printUsage();
            u32Ret = JF_ERR_MISSING_PARAM;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bCreateArchive)
        {
            jacp.jacp_bVerbose = ls_bVerbose;
            u32Ret = jf_archive_create(
                &ls_jlMemberFile, ls_pstrArchiveName, &jacp);
        }
        else if (ls_bExtractArchive)
        {
            jaep.jaep_bListArchive = ls_bListArchive;
            jaep.jaep_bVerbose = ls_bVerbose;
            u32Ret = jf_archive_extract(ls_pstrArchiveName, &jaep);
        }
        else
        {
            jf_logger_logInfoMsg("Unrecognized action");
        }
    }

    jf_logger_logErrMsg(u32Ret, "Quit");

    jf_logger_fini();

    jf_linklist_fini(&ls_jlMemberFile);
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


