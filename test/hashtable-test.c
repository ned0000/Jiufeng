/**
 *  @file hashtable-test.c
 *
 *  @brief Test file for hash table function defined in jf_hashtable common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_hashtable.h"
#include "jf_process.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bTerminateFlag = FALSE;
static boolean_t ls_bHashU32 = FALSE;
static boolean_t ls_bHashTable = FALSE;

#define TEST_HASHTABLE_HASHU32_BITS       (8)
#define TEST_HASHTABLE_HASHU32_HIT_COUNT  (1 << TEST_HASHTABLE_HASHU32_BITS)

/* --- private routine section ------------------------------------------------------------------ */

static void _printHashTableTestUsage(void)
{
    ol_printf("\
Usage: hashtable-test [-u] [-t] [-h]\n\
  -u: test hash u32.\n\
  -t: test hash table.\n\
  -h: print the usage\n");

    ol_printf("\n");
}

static u32 _parseHashTableTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "uth?")) != -1))
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

    ls_bTerminateFlag = TRUE;
}

static u32 _hashU32(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Rand;
    olint_t bits = TEST_HASHTABLE_HASHU32_BITS;
    u32 count[TEST_HASHTABLE_HASHU32_HIT_COUNT];
    u32 index = 0, result = 0, total = 0;

    ol_bzero(count, sizeof(u32) * TEST_HASHTABLE_HASHU32_HIT_COUNT);

    u32Ret = jf_process_registerSignalHandlers(_terminate);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_srand(time(NULL));

        ls_bTerminateFlag = FALSE;

        while (! ls_bTerminateFlag)
        {
            u32Rand = ol_random();

            result = jf_hashtable_hashU32(u32Rand, bits);
            ol_printf("hash random number %u to %d\n", u32Rand, result);
            count[result] ++;

            total ++;
//            ol_sleep(1);
        }

        for (index = 0; index < TEST_HASHTABLE_HASHU32_HIT_COUNT; index ++)
        {
            ol_printf("count[%03u] = %u\n", index, count[index]);
        }
        ol_printf("Total: %u\n", total);
        ol_printf("Average: %u\n", total / TEST_HASHTABLE_HASHU32_HIT_COUNT);
    }

    return u32Ret;
}

typedef struct test_hash_entry
{
    olint_t the_nKey;
    olint_t the_nId;
    olchar_t the_strName[32];
    olint_t the_nField;
} test_hash_entry_t;

static olint_t _testHtCmpKeys(void * pKey1, void * pKey2)
{
    return ol_strcmp((olchar_t *)pKey1, (olchar_t *)pKey2);
}

static olint_t _testHtHashKey(void * pKey)
{
    olint_t ret = jf_hashtable_hashPJW(pKey);
    ol_printf("key: %s, hash: %d\n", (olchar_t *)pKey, ret);
    return ret;
}

static void * _testHtGetKeyFromEntry(void * pEntry)
{
    test_hash_entry_t * entry = (test_hash_entry_t *)pEntry;

    return entry->the_strName;
}

static olint_t _testHtCmpKeys2(void * pKey1, void * pKey2)
{
    test_hash_entry_t * entry1 = (test_hash_entry_t *)pKey1;
    test_hash_entry_t * entry2 = (test_hash_entry_t *)pKey2;
    return (entry1->the_nKey != entry2->the_nKey);
}

static olint_t _testHtHashKey2(void * pKey)
{
    test_hash_entry_t * entry = (test_hash_entry_t *)pKey;
    olint_t ret = entry->the_nKey;
    ol_printf("entry: %s, hash: %d\n", entry->the_strName, ret);
    return ret;
}

static void * _testHtGetKeyFromEntry2(void * pEntry)
{
    return pEntry;
}

static u32 _testHashTableIterator1(jf_hashtable_t * pjh)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_iterator_t iter;
    test_hash_entry_t * pEntry = NULL;

    ol_printf("\n---- iterate hash table start ----\n");

    jf_hashtable_setupIterator(pjh, &iter);

    while (! jf_hashtable_isEndOfIterator(&iter))
    {
        pEntry = jf_hashtable_getEntryFromIterator(&iter);
        ol_printf(
            "position: %d, entry: %s, key: %s\n", iter.jhi_nPos, pEntry->the_strName,
            pEntry->the_strName);

        jf_hashtable_incrementIterator(&iter);
    }

    ol_printf("---- iterate hash table end ----\n\n");

    return u32Ret;
}

static u32 _testHashTableIterator2(jf_hashtable_t * pjh)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_iterator_t iter;
    test_hash_entry_t * pEntry = NULL;

    ol_printf("\n---- iterate hash table start ----\n");

    jf_hashtable_setupIterator(pjh, &iter);

    while (! jf_hashtable_isEndOfIterator(&iter))
    {
        pEntry = jf_hashtable_getEntryFromIterator(&iter);
        ol_printf(
            "position: %d, entry: %s, key: %d\n", iter.jhi_nPos, pEntry->the_strName,
            pEntry->the_nKey);

        jf_hashtable_incrementIterator(&iter);
    }

    ol_printf("---- iterate hash table end ----\n\n");

    return u32Ret;
}

static u32 _showHashTableStats(jf_hashtable_t * pjh)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_stat_t jhstat;

    jf_hashtable_getStat(pjh, &jhstat);

    ol_printf("---- hash table stat ----\n");
    ol_printf("NumOfEntry : %u\n", jhstat.jhs_u32NumOfEntry);
    ol_printf("Size       : %u\n", jhstat.jhs_u32Size);
    ol_printf("Collisions : %u\n", jhstat.jhs_u32Collisions);
    ol_printf("MaxEntries : %u\n", jhstat.jhs_u32MaxEntries);
    ol_printf("Resize     : %u\n", jhstat.jhs_u32CountOfResizeOp);

    return u32Ret;
}

#define TEST_HASH_TABLE_MAX_ENTRY     (16)

static u32 _testHashTable1(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_t * pjh = NULL;
    jf_hashtable_create_param_t jhcp;
    olint_t index = 0;
    test_hash_entry_t * entry[TEST_HASH_TABLE_MAX_ENTRY];

    ol_bzero(&jhcp, sizeof(jf_hashtable_create_param_t));

    jhcp.jhcp_u32MinSize = 7;
    jhcp.jhcp_fnCmpKeys = _testHtCmpKeys;
    jhcp.jhcp_fnHashKey = _testHtHashKey;
    jhcp.jhcp_fnGetKeyFromEntry = _testHtGetKeyFromEntry;
    
    u32Ret = jf_hashtable_create(&pjh, &jhcp);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_printf("hash table is created\n");
    _testHashTableIterator1(pjh);

    for (index = 0; index < TEST_HASH_TABLE_MAX_ENTRY; index ++)
    {
        u32Ret = jf_jiukun_allocMemory((void **)&entry[index], sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(entry[index], sizeof(test_hash_entry_t));
            ol_sprintf(entry[index]->the_strName, "entry%d", index);

            u32Ret = jf_hashtable_insertEntry(pjh, entry[index]);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _testHashTableIterator1(pjh);

        u32Ret = jf_hashtable_insertEntry(pjh, entry[1]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is inserted: %s\n", entry[1]->the_strName);
        _testHashTableIterator1(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_insertEntry(pjh, entry[2]);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is inserted: %s\n", entry[2]->the_strName);
        _testHashTableIterator1(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_removeEntry(pjh, entry[1]);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is removed: %s\n", entry[1]->the_strName);
        _testHashTableIterator1(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olint_t nId = 8;
        test_hash_entry_t * saveentry = entry[nId];

        u32Ret = jf_jiukun_allocMemory((void **)&entry[nId], sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_memcpy(entry[nId], saveentry, sizeof(*saveentry));

            u32Ret = jf_hashtable_overwriteEntry(pjh, entry[nId]);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("entry is overwritten: %s, old: %p, new: %p\n", saveentry->the_strName, saveentry, entry[nId]);
            jf_jiukun_freeMemory((void **)&saveentry);
            _testHashTableIterator1(pjh);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olint_t nId = 100;
        test_hash_entry_t * newentry = NULL;

        u32Ret = jf_jiukun_allocMemory((void **)&newentry, sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(newentry, sizeof(*newentry));
            ol_sprintf(newentry->the_strName, "entry%d", nId);

            u32Ret = jf_hashtable_overwriteEntry(pjh, newentry);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("entry is overwritten: %s\n", newentry->the_strName);
            _testHashTableIterator1(pjh);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        _showHashTableStats(pjh);

    ol_printf("hash table entry is destroyed\n");
    if (pjh != NULL)
        jf_hashtable_destroy(&pjh);

    return u32Ret;
}

static u32 _testHashTable2(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtable_t * pjh = NULL;
    jf_hashtable_create_param_t jhcp;
    olint_t index = 0;
    test_hash_entry_t * entry[TEST_HASH_TABLE_MAX_ENTRY];

    ol_bzero(&jhcp, sizeof(jf_hashtable_create_param_t));

    jhcp.jhcp_u32MinSize = 7;
    jhcp.jhcp_fnCmpKeys = _testHtCmpKeys2;
    jhcp.jhcp_fnHashKey = _testHtHashKey2;
    jhcp.jhcp_fnGetKeyFromEntry = _testHtGetKeyFromEntry2;

    u32Ret = jf_hashtable_create(&pjh, &jhcp);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    ol_printf("hash table is created\n");
    _testHashTableIterator2(pjh);

    for (index = 0; index < TEST_HASH_TABLE_MAX_ENTRY; index ++)
    {
        u32Ret = jf_jiukun_allocMemory((void **)&entry[index], sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(entry[index], sizeof(test_hash_entry_t));
            ol_sprintf(entry[index]->the_strName, "entry%d", index);
            entry[index]->the_nKey = 5 * index + 1;

            u32Ret = jf_hashtable_insertEntry(pjh, entry[index]);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _testHashTableIterator2(pjh);

        u32Ret = jf_hashtable_insertEntry(pjh, entry[1]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is inserted: %s\n", entry[1]->the_strName);
        _testHashTableIterator2(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_insertEntry(pjh, entry[2]);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is inserted: %s\n", entry[2]->the_strName);
        _testHashTableIterator2(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtable_removeEntry(pjh, entry[1]);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("entry is removed: %s\n", entry[1]->the_strName);
        _testHashTableIterator2(pjh);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olint_t nId = 8;
        test_hash_entry_t * saveentry = entry[nId];

        u32Ret = jf_jiukun_allocMemory((void **)&entry[nId], sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_memcpy(entry[nId], saveentry, sizeof(*saveentry));

            u32Ret = jf_hashtable_overwriteEntry(pjh, entry[nId]);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("entry is overwritten: %s, old: %p, new: %p\n", saveentry->the_strName, saveentry, entry[nId]);
            jf_jiukun_freeMemory((void **)&saveentry);
            _testHashTableIterator2(pjh);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olint_t nId = 100;
        test_hash_entry_t * newentry = NULL;

        u32Ret = jf_jiukun_allocMemory((void **)&newentry, sizeof(test_hash_entry_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(newentry, sizeof(*newentry));
            ol_sprintf(newentry->the_strName, "entry%d", nId);
            newentry->the_nId = 62;

            u32Ret = jf_hashtable_overwriteEntry(pjh, newentry);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("entry is overwritten: %s\n", newentry->the_strName);
            _testHashTableIterator2(pjh);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        _showHashTableStats(pjh);

    ol_printf("hash table entry is destroyed\n");
    if (pjh != NULL)
        jf_hashtable_destroy(&pjh);

    return u32Ret;
}

static u32 _testHashTable(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("\n---------------------------- hash table test 1 -----------------------------\n");

    u32Ret = _testHashTable1();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf(
            "\n---------------------------- hash table test 2 -----------------------------\n");

        u32Ret = _testHashTable2();
    }

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
    jlipParam.jlip_pstrCallerName = "HASHTABLE-TEST";
    jlipParam.jlip_bLogToFile = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

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
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
