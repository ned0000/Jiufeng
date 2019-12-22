/**
 *  @file linklist-test.c
 *
 *  @brief Test file for linked list defined in jf_linklist object.
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
#include "jf_linklist.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 tl_u32MsgId;
    u32 tl_u32Flag1;
    u32 tl_u32Flag2;
    u32 tl_u32Flag3;
    u32 tl_u32Flag4;
} test_linklist_t;

static boolean_t ls_bLinklist = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printLinklistTestUsage(void)
{
    ol_printf("\
Usage: linklist-test [-l] \n\
    [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
  -l test linked list.\n");

    ol_printf("\n");
}

static u32 _parseLinklistTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "lT:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printLinklistTestUsage();
            exit(0);
            break;
        case 'l':
            ls_bLinklist = TRUE;
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

static u32 _createTestLinklist(test_linklist_t ** pptl, u32 u32MsgId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_linklist_t * ptl = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&ptl, sizeof(test_linklist_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptl, sizeof(*ptl));
        ptl->tl_u32MsgId = u32MsgId;

        *pptl = ptl;
    }

    return u32Ret;
}

static u32 _showLinklistEntry(jf_linklist_node_t * node, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_linklist_t * ptl = (test_linklist_t *) node->jln_pData;

    ol_printf("Linked list entry, Msg ID: %u\n", ptl->tl_u32MsgId);
    if (ptl->tl_u32MsgId == 100)
    {
        ol_printf("Stop iterating the linked list\n");
        u32Ret = JF_ERR_NOT_MATCH;
    }

    return u32Ret;
}

static u32 _fnFreeTestLinklistNodeData(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_freeMemory(ppData);    
    return u32Ret;
}

static u32 _testLinklist(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_linklist_t * ptl = NULL;
    jf_linklist_t jlList;
    u32 u32MsgIdStart = 1, u32MsgIdEnd = 280;

    jf_linklist_init(&jlList);

    while ((u32MsgIdStart < u32MsgIdEnd) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _createTestLinklist(&ptl, u32MsgIdStart);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_linklist_appendTo(&jlList, ptl);
            ol_printf("MsgId: %u\n", u32MsgIdStart);

            u32MsgIdStart ++;
        }
    }

    jf_linklist_iterate(&jlList, _showLinklistEntry, NULL);

    jf_linklist_finiListAndData(&jlList, _fnFreeTestLinklistNodeData);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "LINKLIST-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseLinklistTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bLinklist)
            {
                u32Ret = _testLinklist();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printLinklistTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


