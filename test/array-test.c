/**
 *  @file array-test.c
 *
 *  @brief Test file for array defined in jf_array object.
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
#include "jf_array.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 ta_u32MsgId;
    u32 ta_u32Flag1;
    u32 ta_u32Flag2;
    u32 ta_u32Flag3;
    u32 ta_u32Flag4;
} test_array_t;

static boolean_t ls_bTestArray = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printArrayTestUsage(void)
{
    ol_printf("\
Usage: array-test [-l] [logger options] \n\
  -a: test array common object.\n\
logger options: [-T <0|1|2|3|4|5>] [-O] [-F log file] [-S log file size] \n\
  -T: the log level. 0: no log, 1: error, 2: warn, 3: info, 4: debug, 5: data.\n\
  -O: output the log to stdout.\n\
  -F: output the log to file.\n\
  -S: the size of log file. No limit if not specified.\n\
    ");
    ol_printf("\n");
}

static u32 _parseArrayTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "aT:F:OS:h")) != -1))
           
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printArrayTestUsage();
            exit(0);
            break;
        case 'a':
            ls_bTestArray = TRUE;
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

static u32 _createTestArray(test_array_t ** ppta, u32 u32MsgId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_array_t * pta = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pta, sizeof(test_array_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pta, sizeof(*pta));
        pta->ta_u32MsgId = u32MsgId;

        *ppta = pta;
    }

    return u32Ret;
}

static u32 _showEntryInTestArray(jf_array_element_t * element, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_array_t * pta = element;

    ol_printf("array entry, Msg ID: %u\n", pta->ta_u32MsgId);
    if (pta->ta_u32MsgId == 100)
    {
        ol_printf("Stop iterating the array\n");
        u32Ret = JF_ERR_TERMINATED;
    }

    return u32Ret;
}

static u32 _fnFreeTestArrayElement(jf_array_element_t ** ppjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_freeMemory(ppjae);
    return u32Ret;
}

#define NUM_OF_TEST_ARRAY_DATA           (10)

static u32 _testArray(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    test_array_t * pta[NUM_OF_TEST_ARRAY_DATA];
    jf_array_t * pjaArray = NULL;
    u32 index = 0;

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Initialize the array.\n");
    jf_array_create(&pjaArray);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the first element.\n");
    jf_array_removeElementAt(pjaArray, 0);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Create element and append to array.\n");
    while ((index < NUM_OF_TEST_ARRAY_DATA) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ret = _createTestArray(&pta[index], index + 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_array_appendElementTo(pjaArray, pta[index]);
            ol_printf("MsgId: %u\n", index + 1);

            index ++;
        }
    }
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the first element.\n");
    jf_array_removeElementAt(pjaArray, 0);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the 5th element.\n");
    jf_array_removeElementAt(pjaArray, 4);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Insert at first element.\n");
    jf_array_insertElementAt(pjaArray, 0, pta[0]);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Insert at 5th element.\n");
    jf_array_insertElementAt(pjaArray, 5, pta[5]);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the 9th element.\n");
    jf_array_removeElementAt(pjaArray, 8);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Insert at 5th element.\n");
    jf_array_appendElementTo(pjaArray, pta[8]);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);

    ol_printf("---------------------------------------------------------------------------\n");
    ol_printf("Remove the 2th element.\n");
    jf_array_removeElement(pjaArray, pta[2]);
    jf_array_traverse(pjaArray, _showEntryInTestArray, NULL);


    jf_array_destroyArrayAndElements(&pjaArray, _fnFreeTestArrayElement);
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "ARRAY-TEST";
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseArrayTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bTestArray)
            {
                u32Ret = _testArray();
            }
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printArrayTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
