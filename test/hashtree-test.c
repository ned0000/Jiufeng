/**
 *  @file hashtree-test.c
 *
 *  @brief The test file for hashtree common object
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
#include "jf_hashtree.h"
#include "jf_err.h"
#include "jf_mem.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bHashTree = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUsage(void)
{
    ol_printf("\
Usage: bases-test [-t] \n\
         [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
     -t test hash tree\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv,
        "tT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(0);
            break;
        case 't':
            ls_bHashTree = TRUE;
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

static void _printHashTree(jf_hashtree_t * pHashtree)
{
    jf_hashtree_enumerator_t en;
    olchar_t * pstrKey;
    olsize_t sKey;
    olchar_t * data;

    /*Iterate through all the WebDataobjects*/
    jf_hashtree_initEnumerator(pHashtree, &en);
    while (! jf_hashtree_isEnumeratorEmptyNode(&en))
    {
        /*Free the WebDataobject*/
        jf_hashtree_getEnumeratorValue(&en, &pstrKey, &sKey, (void **)&data);
        ol_printf("Entry %s, key %s\n", data, pstrKey);
        jf_hashtree_moveEnumeratorNext(&en);
    }
    jf_hashtree_finiEnumerator(&en);
}

static u32 _testHashTree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtree_t hashtree;

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

    jf_hashtree_init(&hashtree);

    bRet = jf_hashtree_isEmpty(&hashtree);
    ol_printf("Hashtree is %s\n", bRet ? "empty" : "not empty");

    sKey = ol_sprintf(strKey, "%s:%u", "111.111.111.11", 111);
    sKey2 = ol_sprintf(strKey2, "%s:%u", "22.222.222.22", 222);
    sKey3 = ol_sprintf(strKey3, "%s:%u", "33.333.3.33", 333);
    sKey4 = ol_sprintf(strKey4, "%s:%u", "44.4.4.44", 444);

    bRet = jf_hashtree_hasEntry(&hashtree, strKey, sKey);
    ol_printf("Entry with key %s is %s\n", strKey, bRet ? "found" : "not found");

    u32Ret = jf_hashtree_addEntry(
        &hashtree, strKey, sKey, pstrEntry);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Add entry %s with key %s\n", pstrEntry, strKey);
        bRet = jf_hashtree_hasEntry(&hashtree, strKey, sKey);
        ol_printf("Entry with key %s is %s\n", strKey, bRet ? "found" : "not found");

        u32Ret = jf_hashtree_addEntry(
            &hashtree, strKey2, sKey2, pstrEntry2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Add entry %s with key %s\n", pstrEntry2, strKey2);
        if (jf_hashtree_getEntry(&hashtree, strKey3, sKey3, (void **)&data) ==
            JF_ERR_NO_ERROR)
            ol_printf("Get entry %s with key %s\n", data, strKey3);
        else
            ol_printf("Cannot get entry with key %s\n", strKey3);

    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_addEntry(
            &hashtree, strKey3, sKey3, pstrEntry3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Add entry %s with key %s\n", pstrEntry3, strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_deleteEntry(&hashtree, strKey3, sKey3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_addEntry(
            &hashtree, strKey3, sKey3, pstrEntry3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Add entry %s with key %s\n", pstrEntry3, strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_deleteEntry(&hashtree, strKey2, sKey2);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey2);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_deleteEntry(&hashtree, strKey, sKey);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_hashtree_deleteEntry(&hashtree, strKey4, sKey4) != JF_ERR_NO_ERROR)
            ol_printf("Failed to delete entry with key %s\n", strKey4);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hashtree_deleteEntry(&hashtree, strKey3, sKey3);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Delete entry with key %s\n", strKey3);
    }
    _printHashTree(&hashtree);
    ol_printf("\n");

    bRet = jf_hashtree_isEmpty(&hashtree);
    ol_printf("Hashtree is %s\n", bRet ? "empty" : "not empty");

    jf_hashtree_fini(&hashtree);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

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
        if (ls_bHashTree)
            u32Ret = _testHashTree();
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

/*------------------------------------------------------------------------------------------------*/


