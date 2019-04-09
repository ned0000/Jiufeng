/**
 *  @file cmdparser.c
 *
 *  @brief Command parser implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(LINUX)
    #include <errno.h>
    #include <pthread.h>
    #include <unistd.h>
    #include <signal.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "hash.h"
#include "stringparse.h"
#include "jf_process.h"
#include "jf_mem.h"

#include "cmdparser.h"

#if defined(WINDOWS)
    #include "getopt.h"
#endif

/* --- private data structures --------------------------------------------- */

#define MAX_ARGC           (JF_CLIENG_MAX_COMMAND_LINE_SIZE / 4)

#define MAX_CMD            (40)

#define MAX_CMD_NAME_LEN   (24)

typedef struct
{
    olchar_t icc_strName[MAX_CMD_NAME_LEN];
    jf_clieng_fnSetDefaultParam_t icc_fnSetDefaultParam;
    jf_clieng_fnParseCmd_t icc_fnParseCmd;
    jf_clieng_fnProcessCmd_t icc_fnProcessCmd;
    void * icc_pParam;

    u32 icc_u32Reserved[8];
} internal_clieng_cmd_t;

typedef struct
{
    olchar_t * iccs_pstrName;
    u32 iccs_u32Reserved[8];
} internal_clieng_cmd_set_t;

typedef struct
{
    void * icp_pMaster;
    olsize_t icp_sArgc;
    olchar_t * icp_pstrArgv[MAX_ARGC];
    olchar_t icp_strCommandLine[JF_CLIENG_MAX_COMMAND_LINE_SIZE * 2];
    u32 icp_u32MaxCmdSet;
    u32 icp_u32NumOfCmdSet;
    internal_clieng_cmd_set_t * icp_piccsCmdSet;
    u32 icp_u32MaxCmd;
    jf_hashtable_t * icp_jhCmd;
} internal_clieng_parser_t;

/* --- private funciton routines ------------------------------------------- */

static u32 _printError(internal_clieng_parser_t * picp, const u32 u32ErrorCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrDesc = NULL;

    pstrDesc = jf_err_getDescription(u32ErrorCode);

    u32Ret = outputLine(
        "Error (0x%x): %s", u32ErrorCode, pstrDesc);

    return u32Ret;
}

/** Trim the left blank spaces, and the eol at the end of the string
 *
 *  @param pstrCmd [in] the command string
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR on success
 *  @retval JF_ERR_BLANK_CMD if the command line is blank;
 *  @retval JF_ERR_COMMENT_CMD if the command line is comment;
 */
static u32 _trimCmdLine(olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i = 0, j = 0;
    olint_t length = 0;

    if (pstrCmd == NULL || ol_strlen(pstrCmd) == 0)
        return JF_ERR_BLANK_CMD;

    if (pstrCmd[0] == '#')
        return JF_ERR_COMMENT_CMD;

    length = ol_strlen(pstrCmd);
    i = 0;
    while (i < length)
    {
        if (pstrCmd[i] != ' ')
            break;
        i++;
    }

    if (i == length)
        return JF_ERR_BLANK_CMD;

    j = 0;
    length = length - i;
    while (j < length)
    {
        pstrCmd[j] = pstrCmd[j + i];
        j = j+1;
    }

    pstrCmd[j] = 0;

    /* remove the eol */
    j = length - 1;
    while (j >= 0)
    {
        if ((pstrCmd[j] == '\n') || (pstrCmd[j] == '\a') ||
            (pstrCmd[j] == '\r'))
            j--;
        else
            break;
    }
    pstrCmd[j + 1] = 0;

    return u32Ret;
}

/** Form the command line arguments, and set the picp->icp_sArgc and
 *  picp->icp_pstrArgv.
 *
 *  @param picp [in/out] the pointer to the CLI Parser. Its member attributes
 *   icp_sArgc and icp_pstrArgv will be set accordingly.
 *  @param pstrCmd [in/out] the command line. It will be changed during the
 *   process, and it will be referred by picp->icp_pstrArgv.
 *
 *  @return the error code
 */
static u32 _formCmdLineArguments(
    internal_clieng_parser_t * picp, olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;

    u32Ret = _trimCmdLine(pstrCmd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("clieng form arg, pstrCmd %s", pstrCmd);

        picp->icp_sArgc = MAX_ARGC;
        u32Ret = jf_process_formCmdLineArguments(
            pstrCmd, &picp->icp_sArgc, picp->icp_pstrArgv);

        jf_logger_logInfoMsg("clieng form arg, argc %d", picp->icp_sArgc);
        for (u32Index = 0; u32Index < picp->icp_sArgc; u32Index ++)
            jf_logger_logInfoMsg("arg %d: %s", u32Index, picp->icp_pstrArgv[u32Index]);

        if (picp->icp_sArgc < 1 && u32Ret == JF_ERR_NO_ERROR)
            u32Ret = JF_ERR_BLANK_CMD;
    }

    return u32Ret;
}

/** For argv without leading - option, merge them to the previous argv to better
 *  handle invalid input
 */
static u32 _preProcessCmdLine(internal_clieng_parser_t * picp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * argv;
    olsize_t length = 0, j = 0;
    u32 u32Index;

    /*for those argument quoted by '"', remove "*/
    for (u32Index = 1; u32Index < picp->icp_sArgc; u32Index++)
    {
        if (picp->icp_pstrArgv[u32Index][0] == '"')
        {
            argv = picp->icp_pstrArgv[u32Index];
            jf_logger_logInfoMsg("original option %d: %s", u32Index, argv);

            j = 0;
            length = ol_strlen(argv) - 1;
            while (j < length)
            {
                argv[j] = argv[j + 1];
                j ++;
            }

            argv[j - 1] = 0;

            jf_logger_logInfoMsg("new option %d: %s", u32Index, argv);
        }
    }
    
    return u32Ret;
}

static boolean_t _findMore(olsize_t * psArgc, olchar_t ** ppstrArgv)
{
    if (*psArgc < 2)
        return FALSE;

    if ((strcmp(ppstrArgv[*psArgc -1], "more") == 0) &&
        (strcmp(ppstrArgv[*psArgc - 2], "|") == 0))
    {
        (*psArgc)--;
        (*psArgc)--;
        return TRUE;
    }
    if (strcmp(ppstrArgv[*psArgc -1], "|more") == 0)
    {
        (*psArgc)--;
        return TRUE;
    }

    return FALSE;
}

/** Parse and process the command according to the picp->icp_sArgc and
 *  picp->icp_pstrArgv.
 *
 *  @param picp [in] the pointer to the CLI parser. Its member attributes
 *   icp_sArgc and icp_pstrArgv must be set before this func is called.
 *
 *  @return the error code
 */
static u32 _parseAndProcess(internal_clieng_parser_t * picp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_t * picc;

    u32Ret = jf_hashtable_getEntry(
        picp->icp_jhCmd, (void *)picp->icp_pstrArgv[0], (void **)&picc);
    if (u32Ret != JF_ERR_NO_ERROR)
        u32Ret = JF_ERR_INVALID_COMMAND;
    else
    {
        u32Ret = picc->icc_fnSetDefaultParam(
            picp->icp_pMaster, picc->icc_pParam);
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = picc->icc_fnParseCmd(
                picp->icp_pMaster, picp->icp_sArgc, picp->icp_pstrArgv,
                picc->icc_pParam);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = picc->icc_fnProcessCmd(
                picp->icp_pMaster, picc->icc_pParam);
    }

    setMoreDisable();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        _printError(picp, u32Ret);
    }

    return u32Ret;
}

static olint_t _cmpKeys(void * pKey1, void * pKey2)
{
    return strcmp((const olchar_t *)pKey1, (const olchar_t *)pKey2);
}

static u32 _freeCmd(void ** ppCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_t * picc = *ppCmd;

    free(picc);

    *ppCmd = NULL;

    return u32Ret;
}

static olint_t _hashKey(void * pKey)
{
    return jf_hashtable_hashPJW(pKey);
}

static void * _getKeyFromCmd(void * pCmd)
{
    internal_clieng_cmd_t * picc = pCmd;

    return picc->icc_strName;
}

/* --- public routine section ---------------------------------------------- */

u32 createParser(clieng_parser_t ** pcp, clieng_parser_param_t * pcpp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = NULL;
    jf_hashtable_create_param_t jhcp;

    assert((pcp != NULL) && (pcpp != NULL));

    jf_logger_logInfoMsg("create parser");

    u32Ret = jf_mem_calloc((void **)&picp, sizeof(internal_clieng_parser_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        picp->icp_sArgc = 0;
        picp->icp_pMaster = pcpp->cpp_pMaster;
        picp->icp_u32MaxCmdSet = pcpp->cpp_u32MaxCmdSet;
        if (picp->icp_u32MaxCmdSet != 0)
        {
            u32Ret = jf_mem_calloc(
                (void **)&picp->icp_piccsCmdSet,
                sizeof(internal_clieng_cmd_set_t) * picp->icp_u32MaxCmdSet);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        picp->icp_u32MaxCmd = (pcpp->cpp_u32MaxCmd > 0) ?
            pcpp->cpp_u32MaxCmd : MAX_CMD;

        memset(&jhcp, 0, sizeof(jhcp));

        jhcp.jhcp_u32MinSize = picp->icp_u32MaxCmd;
        jhcp.jhcp_fnCmpKeys = _cmpKeys;
        jhcp.jhcp_fnHashKey = _hashKey;
        jhcp.jhcp_fnGetKeyFromEntry = _getKeyFromCmd;
        jhcp.jhcp_fnFreeEntry = _freeCmd;

        u32Ret = jf_hashtable_create(&(picp->icp_jhCmd), &jhcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *pcp = picp;
    else if (picp != NULL)
        destroyParser((clieng_parser_t **)&picp);

    return u32Ret;
}

u32 destroyParser(clieng_parser_t ** pcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp;

    assert((pcp != NULL) && (*pcp != NULL));

    picp = (internal_clieng_parser_t *)*pcp;

    jf_logger_logInfoMsg("destroy parser");

    if (picp->icp_piccsCmdSet != NULL)
        free(picp->icp_piccsCmdSet);

    if (picp->icp_jhCmd != NULL)
        jf_hashtable_destroy(&(picp->icp_jhCmd));

    jf_mem_free(pcp);

    return u32Ret;
}

u32 parseCmd(clieng_parser_t * pcp, olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp;

    assert((pcp != NULL) && (pstrCmd != NULL));

    picp = (internal_clieng_parser_t *)pcp;

    jf_logger_logInfoMsg("parse cmd %s", pstrCmd);

    u32Ret = _formCmdLineArguments(picp, pstrCmd);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* check for "| more" command*/
        setMoreDisable();
        if (_findMore(&(picp->icp_sArgc), picp->icp_pstrArgv))
        {
            setMoreEnable();
        }

        u32Ret = _preProcessCmdLine(picp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _parseAndProcess(picp);
    }
    else if (u32Ret != JF_ERR_BLANK_CMD && u32Ret != JF_ERR_COMMENT_CMD)
    {
        _printError(picp, u32Ret);
    }

    return u32Ret;
}

u32 newCmd(
    clieng_parser_t * pcp, const olchar_t * pstrName,
    jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = (internal_clieng_parser_t *)pcp;
    internal_clieng_cmd_t * picc = NULL;

    assert(pstrName != NULL);

    if (strlen(pstrName) > MAX_CMD_NAME_LEN - 1)
        u32Ret = JF_ERR_CMD_NAME_TOO_LONG;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_hashtable_isKeyInTable(picp->icp_jhCmd, (void *)pstrName))
            u32Ret = JF_ERR_CMD_ALREADY_EXIST;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_mem_calloc((void **)&picc, sizeof(internal_clieng_cmd_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strncpy(picc->icc_strName, pstrName, MAX_CMD_NAME_LEN - 1);
        picc->icc_fnSetDefaultParam = fnSetDefaultParam;
        picc->icc_fnParseCmd = fnParseCmd;
        picc->icc_fnProcessCmd = fnProcessCmd;
        picc->icc_pParam = pParam;

        u32Ret = jf_hashtable_insertEntry(picp->icp_jhCmd, (void *)picc);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ppCmd != NULL)
            *ppCmd = picc;
    }
    else if (picc != NULL)
    {
        _freeCmd((void **)&picc);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


