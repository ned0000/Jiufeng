/**
 *  @file ptree-test.c
 *
 *  @brief Test file for property tree defined in jf_ptree common object.
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
#include "jf_ptree.h"
#include "jf_option.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static void _printPtreeTestUsage(void)
{
    ol_printf("\
Usage: ptree-test [-h] [logger options] \n\
    -h print the usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug, 4: data.\n\
    -F <log file> the log file.\n\
    -S <log file size> the size of log file. No limit if not specified.\n\
    ");

    ol_printf("\n");
}

static u32 _parsePtreeTestCmdLineParam(
    olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv, "T:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printPtreeTestUsage();
            exit(0);
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

static u32 _testPtreeNode(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_t * pPtree = NULL;
    jf_ptree_node_t * pNode = NULL, * pService = NULL, * pTemp = NULL, * pRoot2 = NULL;
    olchar_t * pstrName = "servmgmtsetting";
    olchar_t * pstrVer = "version";
    olchar_t * pstrVerValue = "1.0";
    olchar_t * pstrService = "service";

    u32Ret = jf_ptree_create(&pPtree);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(pPtree, NULL, NULL, 0, pstrName, ol_strlen(pstrName), NULL, 0, &pNode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(pPtree, NULL, NULL, 0, "root2", 5, NULL, 0, &pRoot2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pPtree);

        u32Ret = jf_ptree_addNodeAttribute(pNode, "xmlns", 5, "serv", 4, "url-1", 5);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pPtree);

        u32Ret = jf_ptree_addChildNode(
            pPtree, pNode, NULL, 0, pstrVer, ol_strlen(pstrVer), pstrVerValue,
            ol_strlen(pstrVerValue), &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pPtree);

        u32Ret = jf_ptree_addChildNode(
            pPtree, pNode, NULL, 0, pstrService, ol_strlen(pstrService), NULL, 0, &pService);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "name", 4, "a-service", 9, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "path", 4, "/bin/aserv", 10, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "startup", 7, "auto", 4, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "messaging", 9, "a-in", 4, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_dump(pPtree);

        u32Ret = jf_ptree_addChildNode(
            pPtree, pNode, NULL, 0, pstrService, ol_strlen(pstrService), NULL, 0, &pService);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "name", 4, "b-serv", 6, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "path", 4, "/bin/bserv", 10, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "startup", 7, "manual", 6, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_addChildNode(
            pPtree, pService, NULL, 0, "messaging", 9, "b-in", 4, &pTemp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_ptree_dump(pPtree);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t str[512];
        olsize_t sStr = sizeof(str);

        u32Ret = jf_ptree_getNodeFullName(pPtree, pTemp, str, &sStr);
        if (u32Ret == JF_ERR_NO_ERROR)
            ol_printf("Node full name: %s(%d)\n", str, sStr);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_ptree_node_t * node = NULL;
        olchar_t * pstrKey = "root3.sch.name";

        u32Ret = jf_ptree_replaceNode(pPtree, pstrKey, ol_strlen(pstrKey), "xy", 2, &node);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_ptree_dump(pPtree);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t * pstr[] = {
            "NOT_FOUND",
            "servmgmtsetting",
            "servmgmtsetting.version",            
            "servmgmtsetting.service",
            "root2",
        };
        u32 index = 0;

        ol_printf("\n");
        for (index = 0; index < ARRAY_SIZE(pstr); index ++)
        {
            jf_ptree_node_t * fnode[100];
            u16 u16NumOfNode = ARRAY_SIZE(fnode);

            ol_printf("Get node: \"%s\"\n", pstr[index]);

            u32Ret = jf_ptree_findAllNode(
                pPtree, pstr[index], ol_strlen(pstr[index]), fnode, &u16NumOfNode);

            if (u32Ret == JF_ERR_NO_ERROR)
                ol_printf("Number of node: %u\n", u16NumOfNode);
            else
                ol_printf("Not found\n");

            ol_printf("\n");
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        olchar_t * pstr = "servmgmtsetting.service";
        jf_ptree_node_t * fnode[100];
        u16 u16NumOfNode = ARRAY_SIZE(fnode);

        ol_printf("Delete the node: %s\n", pstr);

        u32Ret = jf_ptree_findAllNode(pPtree, pstr, ol_strlen(pstr), fnode, &u16NumOfNode);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_ptree_deleteNode(&fnode[0]);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_ptree_dump(pPtree);
        }
    }

    if (pPtree != NULL)
        jf_ptree_destroy(&pPtree);

    return u32Ret;
}

static u32 _testPtree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _testPtreeNode();

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_logger_init_param_t jlipParam;
	char strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "PTREE-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = _parsePtreeTestCmdLineParam(argc, argv, &jlipParam);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_init(&jlipParam);

        u32Ret = jf_jiukun_init(&jjip);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testPtree();

            jf_jiukun_fini();
        }

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
