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
Usage: linklist-test [-l] [logger options] \n\
  -l: test linked list.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");
    ol_printf("\n");
}

static u32 _parseLinklistTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lT:F:OS:h")) != -1))
           
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

#define NUM_OF_TEST_LINKLIST_DATA           (10)

static u32 _testLinklist(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_linklist_t * ptl[NUM_OF_TEST_LINKLIST_DATA];
    jf_linklist_t jlList;
    u32 index = 0;

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Initialize the linked list.\n");
    jf_linklist_init(&jlList);
    jf_linklist_iterate(&jlList, _showLinklistEntry, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the first data.\n");
    jf_linklist_remove(&jlList, ptl[0]);
    jf_linklist_iterate(&jlList, _showLinklistEntry, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Create data and append to list.\n");
    while ((index < NUM_OF_TEST_LINKLIST_DATA) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _createTestLinklist(&ptl[index], index + 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_linklist_appendTo(&jlList, ptl[index]);
            ol_printf("MsgId: %u\n", index + 1);

            index ++;
        }
    }
    jf_linklist_iterate(&jlList, _showLinklistEntry, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the first data.\n");
    jf_linklist_remove(&jlList, ptl[0]);
    _fnFreeTestLinklistNodeData((void **)&ptl[0]);
    jf_linklist_iterate(&jlList, _showLinklistEntry, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the 5th data.\n");
    jf_linklist_remove(&jlList, ptl[4]);
    _fnFreeTestLinklistNodeData((void **)&ptl[4]);
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


