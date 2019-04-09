/**
 *  @file hash-test.c
 *
 *  @brief test file for hash common object file
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
#include "jf_err.h"
#include "jf_hashtable.h"
#include "jf_process.h"


/* --- private data/data structure section --------------------------------- */
static olint_t terminate_flag;
static boolean_t ls_bHashU32 = FALSE;
static boolean_t ls_bHashTable = FALSE;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: hash-test [-u] [-t] [-h]\n\
    -u hash u32\n\
    -t hash table\n\
    -h print the usage\n");
    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "uth?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'u':
            ls_bHashU32 = TRUE;
            break;
        case 't':
            ls_bHashTable = TRUE;
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

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    terminate_flag = 1;
}

static u32 _hashU32(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 rand;
    olint_t bits = 8;

    u32Ret = jf_process_registerSignalHandlers(_terminate);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        srandom(time(NULL));

        terminate_flag = 0;

        while (! terminate_flag)
        {
            rand = random();
            ol_printf(
                "hash random number %u to %d\n",
                rand, jf_hashtable_hashU32(rand, bits));

            sleep(1);
        }
    }

    return u32Ret;
}

typedef struct test_hash_entry
{
    olint_t the_nKey;
    olint_t the_nId;
    olchar_t * the_pstrKey;
    olchar_t * the_pstrName;
    olint_t the_nField;
} test_hash_entry_t;

static olint_t _testHtCmpKeys(void * pKey1, void * pKey2)
{
    return 0;
}

static olint_t _testHtHashKey(void * pKey)
{

    return (olint_t)(ulong)pKey;
}

static void * _testHtGetKeyFromEntry(void * pEntry)
{
    test_hash_entry_t * entry = (test_hash_entry_t *)pEntry;

    return (void *)(ulong)entry->the_nKey;
}

static u32 _testHashTable(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_t * pjh;
    jf_hashtable_create_param_t jhcp;
    test_hash_entry_t entry1, entry2;

    memset(&jhcp, 0, sizeof(jf_hashtable_create_param_t));

    jhcp.jhcp_u32MinSize = 48;
    jhcp.jhcp_fnCmpKeys = _testHtCmpKeys;
    jhcp.jhcp_fnHashKey = _testHtHashKey;
    jhcp.jhcp_fnGetKeyFromEntry = _testHtGetKeyFromEntry;

    u32Ret = jf_hashtable_create(&pjh, &jhcp);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    memset(&entry1, 0, sizeof(test_hash_entry_t));
    memset(&entry2, 0, sizeof(test_hash_entry_t));

    entry1.the_nKey = 1;
    entry2.the_nKey = 1;

    entry1.the_nId = 1;
    entry2.the_nId = 1;

    jf_hashtable_insertEntry(pjh, &entry1);
    jf_hashtable_insertEntry(pjh, &entry2);



    jf_hashtable_destroy(&pjh);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bHashU32)
            u32Ret = _hashU32();
        else if (ls_bHashTable)
            u32Ret = _testHashTable();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

