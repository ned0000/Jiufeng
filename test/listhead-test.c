/**
 *  @file listhead-test.c
 *
 *  @brief The test file for listhead common object
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
#include "jf_listhead.h"
#include "jf_err.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bListHead = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUsage(void)
{
    ol_printf("\
Usage: listhead-test [-l] \n\
         [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
     -l test list head \n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "lT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 'l':
            ls_bListHead = TRUE;
            break;
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
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
            if (sscanf(optarg, "%d", &u32Value) == 1)
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

typedef struct
{
    u32 tl_u32Flag1;
    u32 tl_u32Flag2;
    jf_listhead_t tl_jlList;
    u32 tl_u32Flag3;
    u32 tl_u32Flag4;
} test_listhead_t;

static u32 _initTestListhead(test_listhead_t * ptl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    static u32 u32Counter = 1;

    jf_mem_calloc((void **)ptl, sizeof(test_listhead_t));

    ptl->tl_u32Flag1 = u32Counter ++;

    jf_listhead_init(&(ptl->tl_jlList));

    return u32Ret;
}

static void _listHeadEntry(jf_listhead_t * head, s32 index)
{
    jf_listhead_t * pjl;
    test_listhead_t * ptl;

    ol_printf("list entry %d\n", index);

    jf_listhead_forEach(head, pjl)
    {
        ptl = jf_listhead_getEntry(pjl, test_listhead_t, tl_jlList);

        ol_printf("entry: %u\n", ptl->tl_u32Flag1);
    }

    ol_printf("\n");
}

static u32 _testListHead(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_listhead_t tl1, tl2, tl3, tl4, tl5, tl6;
    JF_LISTHEAD(list1);
    JF_LISTHEAD(list2);

    ol_printf("init entry\n");

    _initTestListhead(&tl1);
    _initTestListhead(&tl2);
    _initTestListhead(&tl3);
    _initTestListhead(&tl4);
    _initTestListhead(&tl5);
    _initTestListhead(&tl6);

    jf_listhead_addTail(&list1, &(tl1.tl_jlList));
    jf_listhead_addTail(&list1, &(tl2.tl_jlList));
    jf_listhead_addTail(&list1, &(tl3.tl_jlList));
    jf_listhead_addTail(&list1, &(tl4.tl_jlList));

    _listHeadEntry(&list1, 1);

    ol_printf("=== delete entry 2 from list 1\n");
    jf_listhead_del(&tl2.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== delete entry 4 from list 1\n");
    jf_listhead_del(&tl4.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== insert entry 4 to tail of list 1\n");
    jf_listhead_addTail(&list1, &tl4.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== delete entry 4 from list 1\n");
    jf_listhead_del(&tl4.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== insert entry 2 to list 1\n");
    jf_listhead_add(&list1, &tl2.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== insert entry 4 to list 1\n");
    jf_listhead_add(&list1, &tl4.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== replace entry 2 with 5 of list 1\n");
    jf_listhead_replace(&tl2.tl_jlList, &tl5.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== replace entry 5 with 2 of list 1\n");
    jf_listhead_replace(&tl5.tl_jlList, &tl2.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== add entry 5 to list 2\n");
    jf_listhead_add(&list2, &tl5.tl_jlList);
    _listHeadEntry(&list2, 2);

    ol_printf("=== move entry 5 from list 2 to list 1\n");
    jf_listhead_move(&list1, &tl5.tl_jlList);
    _listHeadEntry(&list1, 1);
    _listHeadEntry(&list2, 2);

    ol_printf("=== delete entry 5 from list 1\n");
    jf_listhead_del(&tl5.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== add entry 5 to list 2\n");
    jf_listhead_add(&list2, &tl5.tl_jlList);
    _listHeadEntry(&list2, 2);

    ol_printf("=== add entry 6 to the tail of list 2\n");
    jf_listhead_addTail(&list2, &tl6.tl_jlList);
    _listHeadEntry(&list2, 2);

    ol_printf("=== splice list 2 to list 1\n");
    jf_listhead_splice(&list1, &list2);
    _listHeadEntry(&list1, 1);
    _listHeadEntry(&list2, 2);

    ol_printf("=== delete entry 6 from list 1\n");
    jf_listhead_del(&tl6.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== add entry 6 to list 2\n");
    jf_listhead_add(&list2, &tl6.tl_jlList);
    _listHeadEntry(&list2, 2);

    ol_printf("=== delete entry 5 from list 1\n");
    jf_listhead_del(&tl5.tl_jlList);
    _listHeadEntry(&list1, 1);

    ol_printf("=== add entry 5 to list 2\n");
    jf_listhead_add(&list2, &tl5.tl_jlList);
    _listHeadEntry(&list2, 2);

    ol_printf("=== splice list 2 to the tail of list 1\n");
    jf_listhead_spliceTail(&list1, &list2);
    _listHeadEntry(&list1, 1);
    _listHeadEntry(&list2, 2);

    {
        jf_listhead_t * pos, * next;
        test_listhead_t * entry;
        ol_printf("=== remove all entries in list 1\n");
        jf_listhead_forEachSafe(&list1, pos, next)
        {
            entry = jf_listhead_getEntry(pos, test_listhead_t, tl_jlList);
            ol_printf("remove entries %d in list 1\n", entry->tl_u32Flag1);
            jf_listhead_del(pos);

            _listHeadEntry(&list1, 1);
        }

    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "LISTHEAD-TEST";

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jlipParam.jlip_bLogToStdout = TRUE;

        jf_logger_init(&jlipParam);

        if (ls_bListHead)
            u32Ret = _testListHead();
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUsage();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


