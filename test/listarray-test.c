/**
 *  @file listarray-test.c
 *
 *  @brief The test file for list array function defined in jf_listarray common object.
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
#include "jf_listarray.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bListArray = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printListarrayTestUsage(void)
{
    ol_printf("\
Usage: listarray-test [-a] [-T <trace level>] [-F <trace log file>] [-S <trace file size>]\n\
    -a test list array\n");

    ol_printf("\n");
}

static u32 _parseListarrayTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "aT:F:S:h")) != -1) && (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printListarrayTestUsage();
            exit(0);
            break;
        case 'a':
            ls_bListArray = TRUE;
            break;
        case 'T':
            u32Ret = jf_option_getU8FromString(optarg, &pjlip->jlip_u8TraceLevel);
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
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

static void _dumpListArray(jf_listarray_t * pjl)
{
    u32 u32Head = pjl->jl_u32Head;

    ol_printf("dump list array\n");
    ol_printf("number of node: %u\n", pjl->jl_u32NumOfNode);
    ol_printf("head of array: %u\n", pjl->jl_u32Head);

    while (u32Head != JF_LISTARRAY_END)
    {
        ol_printf("array[%u]=%u\n", u32Head, JF_LISTARRAY_NODE(pjl)[u32Head]);
        u32Head = JF_LISTARRAY_NODE(pjl)[u32Head];
    }
}

#define DEBUG_LOOP_COUNT  50
#define DEBUG_NODE_COUNT  10
static u32 _testListArray(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listarray_t * pjl = NULL;
    u32 u32NumOfNode = DEBUG_NODE_COUNT;
    olsize_t size;
    olint_t nRand;
    u32 u32Loop, u32Index;
    u32 u32Save[DEBUG_NODE_COUNT], u32Node;

    ol_printf("testing list array\n");

    srand(time(NULL));

    nRand = rand();

    size = jf_listarray_getSize(u32NumOfNode);
    ol_printf("size of list array %u\n", size);

    u32Ret = jf_jiukun_allocMemory((void **)&pjl, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listarray_init(pjl, u32NumOfNode);
        _dumpListArray(pjl);

        for (u32Loop = 0; u32Loop < DEBUG_NODE_COUNT; u32Loop ++)
        {
            u32Save[u32Loop] = JF_LISTARRAY_END;
        }

        for (u32Loop = 0; u32Loop < DEBUG_LOOP_COUNT; u32Loop ++)
        {
            ol_printf("\n");
            nRand = rand();

            if ((nRand % 2) != 0)
            {
                ol_printf("get list array node: ");
                u32Node = jf_listarray_getNode(pjl);
                if (u32Node == JF_LISTARRAY_END)
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
                    if (u32Save[u32Index] != JF_LISTARRAY_END)
                        break;

                if (u32Index == DEBUG_NODE_COUNT)
                    ol_printf("none\n");
                else
                {
                    ol_printf("%u\n", u32Save[u32Index]);
                    jf_listarray_putNode(pjl, u32Save[u32Index]);
                    u32Save[u32Index] = JF_LISTARRAY_END;
                }
            }

            _dumpListArray(pjl);
        }
    }

    if (pjl != NULL)
        jf_jiukun_freeMemory((void **)&pjl);

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
    jlipParam.jlip_pstrCallerName = "LISTARRAY-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 0;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parseListarrayTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (ls_bListArray)
                u32Ret = _testListArray(); 
            else
            {
                ol_printf("No operation is specified !!!!\n\n");
                _printListarrayTestUsage();
            }

            jf_jiukun_fini();
        }

        jf_logger_logErrMsg(u32Ret, "Quit");
        jf_logger_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
