/**
 *  @file hlisthead-test.c
 *
 *  @brief Test file for jf_hlisthead object.
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
#include "jf_hlisthead.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_hashtable.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 th_u32MsgId;
    u32 th_u32Flag1;
    u32 th_u32Flag2;
    jf_hlisthead_node_t th_jhnList;
    u32 th_u32Flag3;
    u32 th_u32Flag4;
} test_hlisthead_t;

static boolean_t ls_bHListHead = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printHlistheadTestUsage(void)
{
    ol_printf("\
Usage: hlisthead-test [-l] \n\
    [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
  -l test hash list head \n");

    ol_printf("\n");
}

static u32 _parseHlistheadTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "lT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printHlistheadTestUsage();
            exit(0);
            break;
        case 'l':
            ls_bHListHead = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFile = optarg;
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

static u32 _createTestHListhead(test_hlisthead_t ** ppth, u32 u32MsgId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_hlisthead_t * pth = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pth, sizeof(test_hlisthead_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pth, sizeof(*pth));
        pth->th_u32MsgId = u32MsgId;
        JF_HLISTHEAD_INIT_NODE(&(pth->th_jhnList));

        *ppth = pth;
    }

    return u32Ret;
}

static void _showHlistheadEntry(jf_hlisthead_t * head, u32 index)
{
    jf_hlisthead_node_t * pjhn = NULL;
    test_hlisthead_t * pth = NULL;
    u32 u32Count = 0;

    ol_printf("list %u\n", index);

    ol_printf("    entry: ");
    jf_hlisthead_forEach(head, pjhn)
    {
        pth = jf_hlisthead_getEntry(pjhn, test_hlisthead_t, th_jhnList);

        ol_printf("%u ", pth->th_u32MsgId);
        u32Count ++;
    }

    ol_printf("\nTotal: %u\n", u32Count);
}

#define MAX_HLIST_ARRAY_BIT   (8)
#define MAX_HLIST_ARRRY_SIZE  (1 << MAX_HLIST_ARRAY_BIT)

static u32 _testHListHead(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_hlisthead_t * pth = NULL;
    jf_hlisthead_t hlistarray[MAX_HLIST_ARRRY_SIZE];
    u32 index;
    u32 u32MsgIdStart = 1, u32MsgIdEnd = 280;

    ol_printf("init entry\n");
    for (index = 0; index < MAX_HLIST_ARRRY_SIZE; index++)
    {
        JF_HLISTHEAD_INIT(&hlistarray[index]);
    }

    while (u32MsgIdStart < u32MsgIdEnd)
    {
        u32Ret = _createTestHListhead(&pth, u32MsgIdStart);
        if (u32Ret != JF_ERR_NO_ERROR)
            break;

        index = jf_hashtable_hashU32(u32MsgIdStart, MAX_HLIST_ARRAY_BIT);
        ol_printf("MsgId: %u, HashId: %u\n", u32MsgIdStart, index);

        jf_hlisthead_addHead(&hlistarray[index], &pth->th_jhnList);

        u32MsgIdStart ++;
    }

    for (index = 0; index < MAX_HLIST_ARRRY_SIZE; index++)
    {
        _showHlistheadEntry(&hlistarray[index], index);
    }    

    for (index = 0; index < MAX_HLIST_ARRRY_SIZE; index++)
    {
        jf_hlisthead_node_t * pjhn = NULL, * temp = NULL;
        test_hlisthead_t * pth = NULL;

        jf_hlisthead_forEachSafe(&hlistarray[index], pjhn, temp)
        {
            pth = jf_hlisthead_getEntry(pjhn, test_hlisthead_t, th_jhnList);

            jf_hlisthead_del(pjhn);

            jf_jiukun_freeMemory((void **)&pth);
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "HLISTHEAD-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 3;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseHlistheadTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bHListHead)
            {
                u32Ret = _testHListHead();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printHlistheadTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


