/**
 *  @file cli/clicmd.c
 *
 *  @brief Implementation file for cli command.
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
#include "jf_option.h"
#include "jf_string.h"

#include "clicmd.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _setDefaultParamClear(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_clear_param_t * pccp = (cli_clear_param_t *)pParam;

    ol_bzero(pccp, sizeof(*pccp));

    pccp->ccp_u8Action = CLI_ACTION_CLEAR;

    return u32Ret;
}

static u32 _clearHelp(jiufeng_cli_master_t * pocm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("Clear terminal screen.");
    jf_clieng_outputLine("clear [-h]");
    jf_clieng_outputLine("  -h: print this usage.");

    return u32Ret;
}

static u32 _parseClear(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_clear_param_t * pccp = (cli_clear_param_t *)pParam;
    olint_t nOpt = 0;

    jf_option_reset();

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "h?")) != -1))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        case '?':
        case 'h':
            pccp->ccp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processClear(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_clear_param_t * pccp = (cli_clear_param_t *)pParam;
    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;

    if (pccp->ccp_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _clearHelp(pocm);
    else if (pccp->ccp_u8Action == CLI_ACTION_CLEAR)
        jf_clieng_clear();

    return u32Ret;
}

static u32 _setDefaultParamUser(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;

    ol_bzero(pcup, sizeof(cli_user_param_t));

    return u32Ret;
}

static u32 _userHelp(jiufeng_cli_master_t * pocm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("Add, list user");
    jf_clieng_outputLine("user [-l] [-u name]");
    jf_clieng_outputLine("  -l: list user.");
    jf_clieng_outputLine("  -u: add user with name.");

    return u32Ret;
}

static u32 _parseUser(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt = 0;

    jf_option_reset();

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lu:hv?")) != -1))
    {
        switch (nOpt)
        {
        case 'l':
            pcup->cup_u8Action = CLI_ACTION_LIST_USER;
            break;
        case 'u':
            pcup->cup_u8Action = CLI_ACTION_ADD_USER;
            ol_strncpy(pcup->cup_strUsername, jf_option_getArg(), MAX_USER_NAME_LEN - 1);
            break;		
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        case 'v':
            pcup->cup_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcup->cup_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static jf_clieng_caption_t ls_jccUserInfoVerbose[] =
{
    {"Name", JF_CLIENG_CAP_FULL_LINE},
    {"Full Name", JF_CLIENG_CAP_FULL_LINE},
    {"Email", JF_CLIENG_CAP_FULL_LINE},
    {"Group Id", JF_CLIENG_CAP_HALF_LINE}, {"Enabled", JF_CLIENG_CAP_HALF_LINE},
};

static void _printUserVerbose(user_info_t * pui)
{
    jf_clieng_caption_t * pjcc = &ls_jccUserInfoVerbose[0];
    olchar_t strLeft[JF_CLIENG_MAX_OUTPUT_LINE_LEN], strRight[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    jf_clieng_printDivider();

    /* Name */
    jf_clieng_printOneFullLine(pjcc, pui->ui_strUsername);
    pjcc++;

    /* Full Name */
    jf_clieng_printOneFullLine(pjcc, pui->ui_strFullname);
    pjcc++;

    /* Email */
    jf_clieng_printOneFullLine(pjcc, pui->ui_strEmail);
    pjcc++;

    /* Group Id */
    ol_sprintf(strLeft, "%u", pui->ui_u8UserGroupId);
    ol_strcpy(strRight, jf_string_getStringPositive(pui->ui_bEnable));
    jf_clieng_printTwoHalfLine(pjcc, strLeft, strRight);
    pjcc += 2;
}

static jf_clieng_caption_t ls_jccUserInfoBrief[] =
{
    {"Name", 15},
    {"FullName", 25},
    {"Email", 40},
    {"GroupId", 10},
    {"Enabled", 9},
};

static void _printUserBrief(user_info_t * pui)
{
    jf_clieng_caption_t * pjcc = &ls_jccUserInfoBrief[0];
    olchar_t strInfo[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    olchar_t strField[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Name */
    jf_clieng_appendBriefColumn(pjcc, strInfo, pui->ui_strUsername);
    pjcc++;

    /* Full Name */
    jf_clieng_appendBriefColumn(pjcc, strInfo, pui->ui_strFullname);
    pjcc++;

    /* Email */
    jf_clieng_appendBriefColumn(pjcc, strInfo, pui->ui_strEmail);
    pjcc++;

    /* Group Id */
    ol_sprintf(strField, "%u", pui->ui_u8UserGroupId);
    jf_clieng_appendBriefColumn(pjcc, strInfo, strField);
    pjcc++;

    /* Enabled*/
    jf_clieng_appendBriefColumn(
        pjcc, strInfo, jf_string_getStringPositive(pui->ui_bEnable));
    pjcc++;

    jf_clieng_outputLine(strInfo);
}

static u32 _processUser(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;
    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    user_list_t userlist;
    u16 u16Index;

    if (pcup->cup_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _userHelp(pocm);
    else if (pcup->cup_u8Action == CLI_ACTION_LIST_USER)
    {
        ol_memset(&userlist, 0, sizeof(user_list_t));
        userlist.ul_u16NumUser = 3;

        ol_strcpy(userlist.ul_tUser[0].ui_strUsername, "min");
        ol_strcpy(userlist.ul_tUser[0].ui_strFullname, "Min Zhang");
        ol_strcpy(userlist.ul_tUser[0].ui_strEmail, "min@min.com");
        userlist.ul_tUser[0].ui_u8UserGroupId = 0;
        userlist.ul_tUser[0].ui_bEnable = FALSE;

        ol_strcpy(userlist.ul_tUser[1].ui_strUsername, "frank");
        ol_strcpy(userlist.ul_tUser[1].ui_strFullname, "Frank Wu");
        ol_strcpy(userlist.ul_tUser[1].ui_strEmail, "fr@fr.com");
        userlist.ul_tUser[1].ui_u8UserGroupId = 1;
        userlist.ul_tUser[1].ui_bEnable = TRUE;

        ol_strcpy(userlist.ul_tUser[2].ui_strUsername, "alex");
        ol_strcpy(userlist.ul_tUser[2].ui_strFullname, "Alex Wang");
        ol_strcpy(userlist.ul_tUser[2].ui_strEmail, "alex@alex.com");
        userlist.ul_tUser[2].ui_u8UserGroupId = 2;
        userlist.ul_tUser[2].ui_bEnable = TRUE;

        if (pcup->cup_bVerbose)
        {
            for (u16Index=0; u16Index < userlist.ul_u16NumUser; u16Index++)
                _printUserVerbose(&userlist.ul_tUser[u16Index]);
        }
        else
        {
            jf_clieng_printHeader(
                ls_jccUserInfoBrief, ARRAY_SIZE(ls_jccUserInfoBrief));

            for (u16Index=0; u16Index < userlist.ul_u16NumUser; u16Index++)
                _printUserBrief(&userlist.ul_tUser[u16Index]);
        }
    }
    else if (pcup->cup_u8Action == CLI_ACTION_ADD_USER)
    {
        ol_strcpy(pocm->ocm_strUsername, pcup->cup_strUsername);
    }

    return u32Ret;
}

static u32 _setDefaultParamExit(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    ol_bzero(pcep, sizeof(*pcep));

    return u32Ret;
}

static u32 _exitHelp(jiufeng_cli_master_t * pocm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_clieng_outputLine("exit: exit Jiufeng CLI");

    return u32Ret;
}

static u32 _parseExit(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    jf_option_reset();

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "h?")) != -1))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processExit(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
    {
        u32Ret = _exitHelp(pocm);
    }
    else
    {
        jf_clieng_outputLine("Exit CLI");
        u32Ret = jf_clieng_stop();
    }

    return u32Ret;
}

static u32 _setDefaultParamHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    ol_bzero(pchp, sizeof(*pchp));

    return u32Ret;
}

static u32 _parseHelp(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt = 0;

    jf_option_reset();

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "h?")) != -1))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        case '?':
        case 'h':
            pchp->chp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = jf_clieng_reportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    if (pchp->chp_u8Action == CLI_ACTION_SHOW_HELP)
    {
        jf_clieng_outputLine("show all available commands");
    }
    else
    {
        jf_clieng_outputLine("clear: clear screen.");
        jf_clieng_outputLine("exit: exit CLI.");
        jf_clieng_outputLine("help: list command.");
        jf_clieng_outputLine("user: list user.");
        jf_clieng_outputLine("");
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 addCmd(jiufeng_cli_master_t * pocm, cli_param_t * pcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_clieng_cmd_t * cmd;

    jf_clieng_newCmd(
        "clear", _setDefaultParamClear, _parseClear, _processClear, pcp, &cmd);

    jf_clieng_newCmd(
        "exit", _setDefaultParamExit, _parseExit, _processExit, pcp, &cmd);

    jf_clieng_newCmd(
        "help", _setDefaultParamHelp, _parseHelp, _processHelp, pcp, &cmd);

    jf_clieng_newCmd(
        "user", _setDefaultParamUser, _parseUser, _processUser, pcp, &cmd);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
