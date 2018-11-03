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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "hash.h"
#include "process.h"


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
         -h prolint_t the usage ");
    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "uth?")) != -1) && (u32Ret == OLERR_NO_ERROR))
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
            u32Ret = OLERR_MISSING_PARAM;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
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
    u32 u32Ret = OLERR_NO_ERROR;
    u32 rand;
    olint_t bits = 8;

    u32Ret = registerSignalHandlers(_terminate);
    if (u32Ret == OLERR_NO_ERROR)
    {
        srandom(time(NULL));

        terminate_flag = 0;

        while (! terminate_flag)
        {
            rand = random();
            ol_printf("hash random number %u to %d\n", rand, hashU32(rand, bits));

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
    u32 u32Ret = OLERR_NO_ERROR;
    hash_table_t * pht;
    hash_table_param_t htp;
    test_hash_entry_t entry1, entry2;

    memset(&htp, 0, sizeof(hash_table_param_t));

    htp.htp_u32MinSize = 48;
    htp.htp_fnHtCmpKeys = _testHtCmpKeys;
    htp.htp_fnHtHashKey = _testHtHashKey;
    htp.htp_fnHtGetKeyFromEntry = _testHtGetKeyFromEntry;

    u32Ret = createHashTable(&pht, &htp);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    memset(&entry1, 0, sizeof(test_hash_entry_t));
    memset(&entry2, 0, sizeof(test_hash_entry_t));

    entry1.the_nKey = 1;
    entry2.the_nKey = 1;

    entry1.the_nId = 1;
    entry2.the_nId = 1;

    insertHashTableEntry(pht, &entry1);
    insertHashTableEntry(pht, &entry2);



    destroyHashTable(&pht);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_bHashU32)
            u32Ret = _hashU32();
        else if (ls_bHashTable)
            u32Ret = _testHashTable();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

