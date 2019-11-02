/**
 *  @file hashtable-test.c
 *
 *  @brief test file for hashtable common object file
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
#include "jf_err.h"
#include "jf_hashtable.h"
#include "jf_process.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */
static olint_t terminate_flag;
static boolean_t ls_bHashU32 = FALSE;
static boolean_t ls_bHashTable = FALSE;

/* --- private routine section ------------------------------------------------------------------ */
static void _printHashTableTestUsage(void)
{
    ol_printf("\
Usage: hash-test [-u] [-t] [-h]\n\
    -u hash u32\n\
    -t hash table\n\
    -h print the usage\n");
    ol_printf("\n");

}

static u32 _parseHashTableTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "uth?")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printHashTableTestUsage();
            exit(0);
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

    ol_bzero(&jhcp, sizeof(jf_hashtable_create_param_t));

    jhcp.jhcp_u32MinSize = 48;
    jhcp.jhcp_fnCmpKeys = _testHtCmpKeys;
    jhcp.jhcp_fnHashKey = _testHtHashKey;
    jhcp.jhcp_fnGetKeyFromEntry = _testHtGetKeyFromEntry;
    
    u32Ret = jf_hashtable_create(&pjh, &jhcp);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_printf("hash table is created\n");

    ol_bzero(&entry1, sizeof(test_hash_entry_t));
    ol_bzero(&entry2, sizeof(test_hash_entry_t));

    entry1.the_nKey = 1;
    entry2.the_nKey = 1;

    entry1.the_nId = 1;
    entry2.the_nId = 1;

    u32Ret = jf_hashtable_insertEntry(pjh, &entry1);
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("hash table entry is inserted\n");

    u32Ret = jf_hashtable_insertEntry(pjh, &entry2);
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_printf("hash table entry is inserted\n");

    ol_printf("hash table entry is destroyed\n");
    jf_hashtable_destroy(&pjh);

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
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseHashTableTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bHashU32)
            {
                u32Ret = _hashU32();
            }
            else if (ls_bHashTable)
            {
                u32Ret = _testHashTable();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printHashTableTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

