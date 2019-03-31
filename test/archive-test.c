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
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "errcode.h"
#include "archive.h"

/* --- private data/data structure section --------------------------------- */
static olchar_t * ls_pstrArchiveName = NULL;
static link_list_t ls_llMemberFile;
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
    jf_archive_create_param_t * pjacp, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "ctxvm:f:T:F:S:h")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
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
            u32Ret = appendToLinkList(&ls_llMemberFile, optarg);
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
                plp->lp_u8TraceLevel = (u8)u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (ol_sscanf(optarg, "%d", &u32Value) == 1)
            {
                plp->lp_sLogFile = u32Value;
            }
            else
            {
                u32Ret = OLERR_INVALID_PARAM;
            }
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    jf_archive_create_param_t jacp;
    jf_archive_extract_param_t jaep;
    logger_param_t lpParam;

    memset(&jacp, 0, sizeof(jf_archive_create_param_t));
    memset(&lpParam, 0, sizeof(logger_param_t));
    lpParam.lp_pstrCallerName = "ARCHIVE";

    lpParam.lp_bLogToStdout = TRUE;
    lpParam.lp_u8TraceLevel = LOGGER_TRACE_DEBUG;
    initLogger(&lpParam);

    initLinkList(&ls_llMemberFile);

    u32Ret = _parseCmdLineParam(argc, argv, &jacp, &lpParam);
    if ((u32Ret == OLERR_NO_ERROR) && (ls_bCreateArchive))
    {
        if (isLinkListEmpty(&ls_llMemberFile))
        {
            ol_printf("Member files are not specified!!!\n");
            _printUsage();
            u32Ret = OLERR_MISSING_PARAM;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_pstrArchiveName == NULL)
        {
            ol_printf("Archive file is not specified!!!\n");
            _printUsage();
            u32Ret = OLERR_MISSING_PARAM;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_bCreateArchive)
        {
            jacp.jacp_bVerbose = ls_bVerbose;
            u32Ret = jf_archive_create(
                &ls_llMemberFile, ls_pstrArchiveName, &jacp);
        }
        else if (ls_bExtractArchive)
        {
            jaep.jaep_bListArchive = ls_bListArchive;
            jaep.jaep_bVerbose = ls_bVerbose;
            u32Ret = jf_archive_extract(ls_pstrArchiveName, &jaep);
        }
        else
        {
            logInfoMsg("Unrecognized action");
        }
    }

    logErrMsg(u32Ret, "Quit");

    finiLogger();

    finiLinkList(&ls_llMemberFile);
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


