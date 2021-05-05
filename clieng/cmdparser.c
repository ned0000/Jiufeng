/**
 *  @file cmdparser.c
 *
 *  @brief Command parser implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_hashtable.h"
#include "jf_string.h"
#include "jf_process.h"
#include "jf_jiukun.h"

#include "cmdparser.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum argument count.
 */
#define MAX_ARGC                        (JF_CLIENG_MAX_COMMAND_LINE_LEN / 4)

/** Maximum number of command supported by default.
 */
#define MAX_CMD                         (40)

/** Maximum command name length.
 */
#define MAX_CMD_NAME_LEN                (24)

/** Define the internal command data type.
 */
typedef struct
{
    /**Command name.*/
    olchar_t icc_strName[MAX_CMD_NAME_LEN];
    /**Callback function to set the default parameters of command.*/
    jf_clieng_fnSetDefaultParam_t icc_fnSetDefaultParam;
    /**Callback function to parse parameters of command.*/
    jf_clieng_fnParseCmd_t icc_fnParseCmd;
    /**Callback function to process command.*/
    jf_clieng_fnProcessCmd_t icc_fnProcessCmd;
    /**Parameters of command.*/
    void * icc_pParam;

    u32 icc_u32Reserved[8];
} internal_clieng_cmd_t;

/** Define the internal command set data type.
 */
typedef struct
{
    /**Command set name.*/
    olchar_t * iccs_pstrName;
    u32 iccs_u32Reserved[8];
} internal_clieng_cmd_set_t;

/** Define the internal command parser data type.
 */
typedef struct
{
    /**Command parser is initialized if it's TRUE.*/
    boolean_t icp_bInitialized;
    u8 icp_u8Reserved[7];

    /**User's data for callback function of command.*/
    void * icp_pMaster;

    /**Argument count of the command.*/
    olsize_t icp_sArgc;
    /**Argument array of the command.*/
    olchar_t * icp_pstrArgv[MAX_ARGC];
    /**The command line to be parsed.*/
    olchar_t icp_strCommandLine[JF_CLIENG_MAX_COMMAND_LINE_LEN * 2];

    /**Maximum number of command set supported.*/
    u32 icp_u32MaxCmdSet;
    /**Number of command set.*/
    u32 icp_u32NumOfCmdSet;
    /**Command set array.*/
    internal_clieng_cmd_set_t * icp_piccsCmdSet;

    /**Maximum number of command supported.*/
    u32 icp_u32MaxCmd;
    /**Hash table for commands.*/
    jf_hashtable_t * icp_jhCmd;
} internal_clieng_parser_t;

/** Declare the internal command parser object.
 */
static internal_clieng_parser_t ls_icpCliengParser;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _printError(internal_clieng_parser_t * picp, const u32 u32ErrorCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrDesc = NULL;

    pstrDesc = jf_err_getDescription(u32ErrorCode);

    u32Ret = outputLine("Error (0x%x): %s", u32ErrorCode, pstrDesc);

    return u32Ret;
}

/** Trim the left blank spaces, and the eol at the end of the string.
 *
 *  @param pstrCmd [in] The command string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_BLANK_CMD If the command line is blank.
 *  @retval JF_ERR_COMMENT_CMD If the command line is comment.
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

    /*Check if it's a blank command.*/
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

    /*Remove the leading spaces.*/
    j = 0;
    length = length - i;
    while (j < length)
    {
        pstrCmd[j] = pstrCmd[j + i];
        j = j+1;
    }

    pstrCmd[j] = 0;

    /*Remove the eol.*/
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

/** Form the command line arguments, and set argc and argv.
 *
 *  @param picp [in/out] The pointer to the CLI Parser. Its member attributes icp_sArgc and
 *   icp_pstrArgv will be set accordingly.
 *  @param pstrCmd [in/out] The command line. It will be changed during the process, and it will
 *   be referred by picp->icp_pstrArgv.
 *
 *  @return The error code.
 */
static u32 _formCmdLineArguments(internal_clieng_parser_t * picp, olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;

    u32Ret = _trimCmdLine(pstrCmd);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("pstrCmd: %s", pstrCmd);

        picp->icp_sArgc = MAX_ARGC;
        u32Ret = jf_process_formCmdLineArguments(pstrCmd, &picp->icp_sArgc, picp->icp_pstrArgv);

        JF_LOGGER_DEBUG("argc: %d", picp->icp_sArgc);
        for (u32Index = 0; u32Index < picp->icp_sArgc; u32Index ++)
            JF_LOGGER_DEBUG("arg %d: %s", u32Index, picp->icp_pstrArgv[u32Index]);

        if ((picp->icp_sArgc < 1) && (u32Ret == JF_ERR_NO_ERROR))
            u32Ret = JF_ERR_BLANK_CMD;
    }

    return u32Ret;
}

/** Process the command line.
 *
 *  @note
 *  -# For arguments quoted by '"', remove it.
 */
static u32 _preProcessCmdLine(internal_clieng_parser_t * picp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * argv = NULL;
    olsize_t length = 0, j = 0;
    u32 u32Index = 0;

    /*For those argument quoted by '"', remove it.*/
    for (u32Index = 1; u32Index < picp->icp_sArgc; u32Index++)
    {
        if (picp->icp_pstrArgv[u32Index][0] == '"')
        {
            argv = picp->icp_pstrArgv[u32Index];
            JF_LOGGER_DEBUG("original option %d: %s", u32Index, argv);

            j = 0;
            length = ol_strlen(argv) - 1;
            while (j < length)
            {
                argv[j] = argv[j + 1];
                j ++;
            }

            argv[j - 1] = 0;

            JF_LOGGER_DEBUG("new option %d: %s", u32Index, argv);
        }
    }
    
    return u32Ret;
}

static boolean_t _findMore(olsize_t * psArgc, olchar_t ** ppstrArgv)
{
    if (*psArgc < 2)
        return FALSE;

    /*Check "| more".*/
    if ((ol_strcmp(ppstrArgv[*psArgc - 1], "more") == 0) &&
        (ol_strcmp(ppstrArgv[*psArgc - 2], "|") == 0))
    {
        (*psArgc)--;
        (*psArgc)--;
        return TRUE;
    }

    /*Check "|more".*/
    if (ol_strcmp(ppstrArgv[*psArgc - 1], "|more") == 0)
    {
        (*psArgc)--;
        return TRUE;
    }

    return FALSE;
}

/** Parse and process the command according to the argc and argv.
 *
 *  @param picp [in] The pointer to the CLI parser. Its member attributes icp_sArgc and
 *   icp_pstrArgv must be set before this func is called.
 *
 *  @return The error code.
 */
static u32 _parseAndProcess(internal_clieng_parser_t * picp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_t * picc = NULL;

    u32Ret = jf_hashtable_getEntry(
        picp->icp_jhCmd, (void *)picp->icp_pstrArgv[0], (void **)&picc);

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        /*No such command.*/
        u32Ret = JF_ERR_INVALID_COMMAND;
    }
    else
    {
        /*Set default parameters of the command.*/
        u32Ret = picc->icc_fnSetDefaultParam(picp->icp_pMaster, picc->icc_pParam);

        /*Parse the parameters of the command.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = picc->icc_fnParseCmd(
                picp->icp_pMaster, picp->icp_sArgc, picp->icp_pstrArgv, picc->icc_pParam);

        /*Process the command.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = picc->icc_fnProcessCmd(picp->icp_pMaster, picc->icc_pParam);
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
//    internal_clieng_cmd_t * picc = *ppCmd;

    jf_jiukun_freeMemory(ppCmd);

    return u32Ret;
}

static olint_t _hashKey(void * pKey)
{
    return jf_hashtable_hashPJW(pKey);
}

static void * _getKeyFromCmd(void * pCmd)
{
    internal_clieng_cmd_t * picc = pCmd;

    /*Use command name as the key.*/
    return picc->icc_strName;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initCliengParser(clieng_parser_init_param_t * pcpip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = &ls_icpCliengParser;
    jf_hashtable_create_param_t jhcp;
    olsize_t size = 0;

    assert(pcpip != NULL);

    JF_LOGGER_INFO("MaxCmdSet: %u", pcpip->cpip_u32MaxCmdSet);

    /*Initialize the command parser.*/
    ol_bzero(picp, sizeof(*picp));

    picp->icp_sArgc = 0;
    picp->icp_pMaster = pcpip->cpip_pMaster;
    picp->icp_u32MaxCmdSet = pcpip->cpip_u32MaxCmdSet;

    /*Initialize the command set.*/
    if (picp->icp_u32MaxCmdSet != 0)
    {
        size = sizeof(internal_clieng_cmd_set_t) * picp->icp_u32MaxCmdSet;

        u32Ret = jf_jiukun_allocMemory((void **)&picp->icp_piccsCmdSet, size);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(picp->icp_piccsCmdSet, size);
        }
    }

    /*Initialize the hash table for commands.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        picp->icp_u32MaxCmd = (pcpip->cpip_u32MaxCmd > 0) ? pcpip->cpip_u32MaxCmd : MAX_CMD;

        ol_bzero(&jhcp, sizeof(jhcp));

        jhcp.jhcp_u32MinSize = picp->icp_u32MaxCmd;
        jhcp.jhcp_fnCmpKeys = _cmpKeys;
        jhcp.jhcp_fnHashKey = _hashKey;
        jhcp.jhcp_fnGetKeyFromEntry = _getKeyFromCmd;
        jhcp.jhcp_fnFreeEntry = _freeCmd;

        u32Ret = jf_hashtable_create(&picp->icp_jhCmd, &jhcp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        picp->icp_bInitialized = TRUE;
    else if (picp != NULL)
        finiCliengParser();

    return u32Ret;
}

u32 finiCliengParser(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = &ls_icpCliengParser;

    JF_LOGGER_INFO("fini");

    if (picp->icp_piccsCmdSet != NULL)
        jf_jiukun_freeMemory((void **)&picp->icp_piccsCmdSet);

    if (picp->icp_jhCmd != NULL)
        jf_hashtable_destroy(&picp->icp_jhCmd);

    picp->icp_bInitialized = FALSE;

    return u32Ret;
}

u32 parseCliengCmd(olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = &ls_icpCliengParser;

    assert(pstrCmd != NULL);

    JF_LOGGER_DEBUG("cmd: %s", pstrCmd);

    u32Ret = _formCmdLineArguments(picp, pstrCmd);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Check for "| more" command.*/
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
    else if ((u32Ret != JF_ERR_BLANK_CMD) && (u32Ret != JF_ERR_COMMENT_CMD))
    {
        _printError(picp, u32Ret);
    }

    return u32Ret;
}

u32 newCliengCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd, void * pParam,
    jf_clieng_cmd_t ** ppCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_parser_t * picp = &ls_icpCliengParser;
    internal_clieng_cmd_t * picc = NULL;

    assert(pstrName != NULL);

    if (ol_strlen(pstrName) > MAX_CMD_NAME_LEN - 1)
        u32Ret = JF_ERR_CMD_NAME_TOO_LONG;

    /*Check if the command is already added.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_hashtable_isKeyInTable(picp->icp_jhCmd, (void *)pstrName))
            u32Ret = JF_ERR_CMD_ALREADY_EXIST;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_jiukun_allocMemory((void **)&picc, sizeof(internal_clieng_cmd_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(picc, sizeof(internal_clieng_cmd_t));
        ol_strncpy(picc->icc_strName, pstrName, MAX_CMD_NAME_LEN - 1);
        picc->icc_fnSetDefaultParam = fnSetDefaultParam;
        picc->icc_fnParseCmd = fnParseCmd;
        picc->icc_fnProcessCmd = fnProcessCmd;
        picc->icc_pParam = pParam;

        /*Insert the command to hash table.*/
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

/*------------------------------------------------------------------------------------------------*/
