/**
 *  @file dlinklist-test.c
 *
 *  @brief Test file for double linked list defined in jf_linklist object.
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
#include "jf_dlinklist.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 td_u32MsgId;
    u32 td_u32Flag1;
    u32 td_u32Flag2;
    u32 td_u32Flag3;
    u32 td_u32Flag4;
} test_dlinklist_t;

static boolean_t ls_bDlinklist = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printDlinklistTestUsage(void)
{
    ol_printf("\
Usage: dlinklist-test [-l] \n\
    [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
  -l: test double linked list.\n");

    ol_printf("\n");
}

static u32 _parseDlinklistTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lT:F:S:h")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printDlinklistTestUsage();
            exit(0);
            break;
        case 'l':
            ls_bDlinklist = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(jf_option_getArg(), &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = jf_option_getArg();
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

static u32 _createTestDlinklist(test_dlinklist_t ** pptd, u32 u32MsgId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_dlinklist_t * ptd = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&ptd, sizeof(test_dlinklist_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(ptd, sizeof(*ptd));
        ptd->td_u32MsgId = u32MsgId;

        *pptd = ptd;
    }

    return u32Ret;
}

static u32 _showDlinklistEntry(jf_dlinklist_node_t * node, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_dlinklist_t * ptd = (test_dlinklist_t *) node->jdn_pData;

    ol_printf("Linked list entry, Msg ID: %u\n", ptd->td_u32MsgId);
    if (ptd->td_u32MsgId == 100)
    {
        ol_printf("Stop iterating the linked list\n");
        u32Ret = JF_ERR_NOT_MATCH;
    }

    return u32Ret;
}

static u32 _fnFreeTestDlinklistNodeData(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_freeMemory(ppData);    
    return u32Ret;
}

static u32 _testDlinklist(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_dlinklist_t * ptd = NULL;
    jf_dlinklist_t jlList;
    u32 u32MsgIdStart = 1, u32MsgIdEnd = 280;

    jf_dlinklist_init(&jlList);

    while ((u32MsgIdStart < u32MsgIdEnd) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _createTestDlinklist(&ptd, u32MsgIdStart);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_dlinklist_appendTo(&jlList, ptd);
            ol_printf("MsgId: %u\n", u32MsgIdStart);

            u32MsgIdStart ++;
        }
    }

    jf_dlinklist_iterate(&jlList, _showDlinklistEntry, NULL);

    jf_dlinklist_finiListAndData(&jlList, _fnFreeTestDlinklistNodeData);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "DLINKLIST-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseDlinklistTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bDlinklist)
            {
                u32Ret = _testDlinklist();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printDlinklistTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


