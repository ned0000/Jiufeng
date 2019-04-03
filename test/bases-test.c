/**
 *  @file bases-test.c
 *
 *  @brief The test file for bases common object
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
//#include "files.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
static boolean_t ls_bListHead = FALSE;
static boolean_t ls_bHashTree = FALSE;
static boolean_t ls_bListArray = FALSE;

/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: bases-test [-l] [-t] \n\
         [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
     -l test list head \n\
     -t test hash tree\n\
     -a test list array\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "ltaT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
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
        case 't':
            ls_bHashTree = TRUE;
            break;
        case 'a':
            ls_bListArray = TRUE;
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
    list_head_t tl_lhList;
    u32 tl_u32Flag3;
    u32 tl_u32Flag4;
} test_listhead_t;

static u32 _initTestListhead(test_listhead_t * ptl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    static u32 u32Counter = 1;

    xcalloc((void **)ptl, sizeof(test_listhead_t));

    ptl->tl_u32Flag1 = u32Counter ++;

    listInit(&(ptl->tl_lhList));

    return u32Ret;
}

static void _listHeadEntry(list_head_t * head)
{
    list_head_t * plh;
    test_listhead_t * ptl;

    ol_printf("list entry\n");

    listForEach(head, plh)
    {
        ptl = listEntry(plh, test_listhead_t, tl_lhList);

        ol_printf("entry: %u\n", ptl->tl_u32Flag1);
    }

    ol_printf("\n");
}

static u32 _testListHead(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_listhead_t tl1, tl2, tl3, tl4, tl5;
    LIST_HEAD(head1);
    LIST_HEAD(head2);

    ol_printf("init entry\n");

    _initTestListhead(&tl1);
    _initTestListhead(&tl2);
    _initTestListhead(&tl3);
    _initTestListhead(&tl4);
    _initTestListhead(&tl5);

    listAddTail(&head1, &(tl1.tl_lhList));
    listAddTail(&head1, &(tl2.tl_lhList));
    listAddTail(&head1, &(tl3.tl_lhList));
    listAddTail(&head1, &(tl4.tl_lhList));

    _listHeadEntry(&head1);

    ol_printf("delete entry 2 from head 1\n");
    listDel(&(tl2.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("delete entry 4 from head 1\n");
    listDel(&(tl4.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("insert entry 2 to head 1\n");
    listAdd(&(tl1.tl_lhList), &(tl2.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("insert entry 4 to head 1\n");
    listAdd(&(tl3.tl_lhList), &(tl4.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("replace entry 2 with 5 of head 1\n");
    listReplace(&(tl2.tl_lhList), &(tl5.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("replace entry 5 with 2 of head 1\n");
    listReplace(&(tl5.tl_lhList), &(tl2.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("add entry 5 to head 2\n");
    listAdd(&head2, &(tl5.tl_lhList));
    _listHeadEntry(&head2);

    ol_printf("move entry 5 from head 2 to head 1\n");
    listMove(&head1, &(tl5.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("delete entry 5 from head 1\n");
    listDel(&(tl5.tl_lhList));
    _listHeadEntry(&head1);

    ol_printf("add entry 5 to head 2\n");
    listAdd(&head2, &(tl5.tl_lhList));
    _listHeadEntry(&head2);

    ol_printf("splice head 2 to head\n");
    listSplice(&head1, &head2);
    _listHeadEntry(&head1);

    return u32Ret;
}

static void _printHashTree(hashtree_t * pHashtree)
{
    hashtree_enumerator_t en;
    olchar_t * pstrKey;
    olsize_t sKey;
    olchar_t * data;

    /*Iterate through all the WebDataobjects*/
    initHashtreeEnumerator(pHashtree, &en);
    while (! isHashtreeEnumeratorEmptyNode(&en))
    {
        /*Free the WebDataobject*/
        getHashtreeValue(&en, &pstrKey, &sKey, (void **)&data);
        ol_printf("Entry %s, key %s\n", data, pstrKey);
        moveHashtreeNext(&en);
    }
    finiHashtreeEnumerator(&en);
}

static u32 _testHashTree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    hashtree_t hashtree;

    olchar_t * pstrEntry = "one";
    olchar_t strKey[64];
    olsize_t sKey;
    boolean_t bRet;
    olchar_t strKey2[64];
    olsize_t sKey2;
    olchar_t * pstrEntry2 = "two";
    olchar_t strKey3[64];
    olsize_t sKey3;
    olchar_t * pstrEntry3 = "three";
    olchar_t * data;
    olchar_t strKey4[64];
    olsize_t sKey4;

    initHashtree(&hashtree);

    bRet = isHashtreeEmpty(&hashtree);
    ol_printf("Hashtree is %s\n", bRet ? "empty" : "not empty");

    sKey = ol_sprintf(strKey, "%s:%u", "111.111.111.11", 111);
    sKey2 = ol_sprintf(strKey2, "%s:%u", "22.222.222.22", 222);
    sKey3 = ol_sprintf(strKey3, "%s:%u", "33.333.3.33", 333);
    sKey4 = ol_sprintf(strKey4, "%s:%u", "44.4.4.44", 444);

    bRet = hasHashtreeEntry(&hashtree, strKey, sKey);
    ol_printf("Entry with key %s is %s\n", strKey, bRet ? "found" : "not found");

    u32Ret = addHashtreeEntry(
        &hashtree, strKey, sKey, pstrEntry);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Add entry %s with key %s\n", pstrEntry, strKey);
        bRet = hasHashtreeEntry(&hashtree, strKey, sKey);
        ol_printf("Entry with key %s is %s\n", strKey, bRet ? "found" : "not found");

        u32Ret = addHashtreeEntry(
            &hashtree, strKey2, sKey2, pstrEntry2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Add entry %s with key %s\n", pstrEntry2, strKey2);
        if (getHashtreeEntry(&hashtree, strKey3, sKey3, (void **)&data) ==
            JF_ERR_NO_ERROR)
            ol_printf("Get entry %s with key %s\n", data, strKey3);
        else
            ol_printf("Cannot get entry with key %s\n", strKey3);

    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = addHashtreeEntry(
            &hashtree, strKey3, sKey3, pstrEntry3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Add entry %s with key %s\n", pstrEntry3, strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = deleteHashtreeEntry(&hashtree, strKey3, sKey3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = addHashtreeEntry(
            &hashtree, strKey3, sKey3, pstrEntry3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Add entry %s with key %s\n", pstrEntry3, strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = deleteHashtreeEntry(&hashtree, strKey2, sKey2);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey2);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = deleteHashtreeEntry(&hashtree, strKey, sKey);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (deleteHashtreeEntry(&hashtree, strKey4, sKey4) != JF_ERR_NO_ERROR)
            ol_printf("Failed to delete entry with key %s\n", strKey4);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = deleteHashtreeEntry(&hashtree, strKey3, sKey3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    bRet = isHashtreeEmpty(&hashtree);
    ol_printf("Hashtree is %s\n", bRet ? "empty" : "not empty");

    finiHashtree(&hashtree);

    return u32Ret;
}

static void _dumpListArray(list_array_t * pla)
{
    u32 u32Head = pla->la_u32Head;

    ol_printf("dump list array\n");
    ol_printf("number of node: %u\n", pla->la_u32NumOfNode);
    ol_printf("head of array: %u\n", pla->la_u32Head);

    while (u32Head != LIST_ARRAY_END)
    {
        ol_printf("array[%u]=%u\n", u32Head, LIST_ARRAY_NODE(pla)[u32Head]);
        u32Head = LIST_ARRAY_NODE(pla)[u32Head];
    }
}

#define DEBUG_LOOP_COUNT  50
#define DEBUG_NODE_COUNT  10
static u32 _testListArray(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    list_array_t * pla = NULL;
    u32 u32NumOfNode = DEBUG_NODE_COUNT;
    olsize_t size;
    olint_t nRand;
    u32 u32Loop, u32Index;
    u32 u32Save[DEBUG_NODE_COUNT], u32Node;

    ol_printf("testing list array\n");

    srand(time(NULL));

    nRand = rand();

    size = sizeOfListArray(u32NumOfNode);
    ol_printf("size of list array %u\n", size);

    u32Ret = xmalloc((void **)&pla, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        initListArray(pla, u32NumOfNode);
        _dumpListArray(pla);

        for (u32Loop = 0; u32Loop < DEBUG_NODE_COUNT; u32Loop ++)
        {
            u32Save[u32Loop] = LIST_ARRAY_END;
        }

        for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
        {
            ol_printf("\n");
            nRand = rand();

            if ((nRand % 2) != 0)
            {
                ol_printf("get list array node: ");
                u32Node = getListArrayNode(pla);
                if (u32Node == LIST_ARRAY_END)
                    ol_printf("end\n");
                else
                {
                    u32Save[u32Node] = u32Node;
                    ol_printf("%u\n", u32Node);
                }
            }
            else
            {
                ol_printf("put list array node: ");
                for (u32Index = 0; u32Index < DEBUG_NODE_COUNT; u32Index ++)
                    if (u32Save[u32Index] != LIST_ARRAY_END)
                        break;

                if (u32Index == DEBUG_NODE_COUNT)
                    ol_printf("none\n");
                else
                {
                    ol_printf("%u\n", u32Save[u32Index]);
                    putListArrayNode(pla, u32Save[u32Index]);
                    u32Save[u32Index] = LIST_ARRAY_END;
                }
            }

            _dumpListArray(pla);
        }
    }

    if (pla != NULL)
        xfree((void **)&pla);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "BASES-TEST";

    u32Ret = _parseCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jlipParam.jlip_bLogToStdout = TRUE;
        jlipParam.jlip_u8TraceLevel = 0;
        jf_logger_init(&jlipParam);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bListHead)
            u32Ret = _testListHead();
        else if (ls_bHashTree)
            u32Ret = _testHashTree();
        else if (ls_bListArray)
            u32Ret = _testListArray(); 
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUsage();
        }
    }

    jf_logger_logErrMsg(u32Ret, "Quit");

    jf_logger_fini();

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


