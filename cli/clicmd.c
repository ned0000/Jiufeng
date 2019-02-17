/**
 *  @file clicmd.c
 *
 *  @brief cli command
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "clicmd.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

static u32 _setDefaultParamUser(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;

    memset(pcup, 0, sizeof(cli_user_param_t));

    return u32Ret;
}

static u32 _userHelp(jiufeng_cli_master_t * pocm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("Add, list user");
    cliengOutputLine("user [-l] [-u name]");
    cliengOutputLine("  -l: list user.");
    cliengOutputLine("  -u: add user with name.");

    return u32Ret;
}

static u32 _parseUser(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "lu:hv?")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case 'l':
            pcup->cup_u8Action = CLI_ACTION_LIST_USER;
            break;
        case 'u':
            pcup->cup_u8Action = CLI_ACTION_ADD_USER;
            ol_strncpy(pcup->cup_strUsername, optarg, MAX_USER_NAME_LEN - 1);
            break;		
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case 'v':
            pcup->cup_bVerbose = TRUE;
            break;
        case '?':
        case 'h':
            pcup->cup_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static clieng_caption_t ls_ccUserInfoVerbose[] =
{
    {"Name", CLIENG_CAP_FULL_LINE},
    {"Full Name", CLIENG_CAP_FULL_LINE},
    {"Email", CLIENG_CAP_FULL_LINE},
    {"Group Id", CLIENG_CAP_HALF_LINE}, {"Enabled", CLIENG_CAP_HALF_LINE},
};

static void _printUserVerbose(user_info_t * pui)
{
    clieng_caption_t * pcc = &ls_ccUserInfoVerbose[0];
    olchar_t strLeft[MAX_OUTPUT_LINE_LEN], strRight[MAX_OUTPUT_LINE_LEN];

    cliengPrintDivider();

    /* Name */
    cliengPrintOneFullLine(pcc, pui->ui_strUsername);
    pcc++;

    /* Full Name */
    cliengPrintOneFullLine(pcc, pui->ui_strFullname);
    pcc++;

    /* Email */
    cliengPrintOneFullLine(pcc, pui->ui_strEmail);
    pcc++;

    /* Group Id */
    ol_sprintf(strLeft, "%u", pui->ui_u8UserGroupId);
    ol_strcpy(strRight, getStringPositive(pui->ui_bEnable));
    cliengPrintTwoHalfLine(pcc, strLeft, strRight);
    pcc += 2;
}

static clieng_caption_t ls_ccUserInfoBrief[] =
{
    {"Name", 10},
    {"FullName", 20},
    {"Email", 30},
    {"GroupId", 10},
    {"Enabled", 9},
};

static void _printUserBrief(user_info_t * pui)
{
    clieng_caption_t * pcc = &ls_ccUserInfoBrief[0];
    olchar_t strInfo[MAX_OUTPUT_LINE_LEN], strField[MAX_OUTPUT_LINE_LEN];

    strInfo[0] = '\0';

    /* Name */
    cliengAppendBriefColumn(pcc, strInfo, pui->ui_strUsername);
    pcc++;

    /* Full Name */
    cliengAppendBriefColumn(pcc, strInfo, pui->ui_strFullname);
    pcc++;

    /* Email */
    cliengAppendBriefColumn(pcc, strInfo, pui->ui_strEmail);
    pcc++;

    /* Group Id */
    ol_sprintf(strField, "%u", pui->ui_u8UserGroupId);
    cliengAppendBriefColumn(pcc, strInfo, strField);
    pcc++;

    /* Enabled*/
    cliengAppendBriefColumn(
        pcc, strInfo, getStringPositive(pui->ui_bEnable));
    pcc++;

    cliengOutputLine(strInfo);
}

static u32 _processUser(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_user_param_t * pcup = (cli_user_param_t *)pParam;
    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    user_list_t userlist;
    u16 u16Index;

    if (pcup->cup_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _userHelp(pocm);
    else if (pcup->cup_u8Action == CLI_ACTION_LIST_USER)
    {
        memset(&userlist, 0, sizeof(user_list_t));
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
            cliengPrintHeader(
                ls_ccUserInfoBrief, ARRAY_SIZE(ls_ccUserInfoBrief));

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
    u32 u32Ret = OLERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    memset(pcep, 0, sizeof(cli_exit_param_t));

    return u32Ret;
}

static u32 _exitHelp(jiufeng_cli_master_t * pocm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    cliengOutputLine("exit: exit Jiufeng CLI");

    return u32Ret;
}

static u32 _parseExit(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv, "h?")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pcep->cep_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processExit(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_exit_param_t * pcep = (cli_exit_param_t *)pParam;

    if (pcep->cep_u8Action == CLI_ACTION_SHOW_HELP)
        u32Ret = _exitHelp(pocm);
    else
    {
        cliengOutputLine("Exit CLI");
        u32Ret = stopClieng();
    }

    return u32Ret;
}

static u32 _setDefaultParamHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    memset(pchp, 0, sizeof(cli_help_param_t));

    return u32Ret;
}

static u32 _parseHelp(void * pMaster, olint_t argc, olchar_t ** argv, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    olint_t nOpt;

    optind = 0;  /* initialize the opt index */

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        case '?':
        case 'h':
            pchp->chp_u8Action = CLI_ACTION_SHOW_HELP;
            break;
        default:
            u32Ret = cliengReportNotApplicableOpt(nOpt);
        }
    }

    return u32Ret;
}

static u32 _processHelp(void * pMaster, void * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    jiufeng_cli_master_t * pocm = (jiufeng_cli_master_t *)pMaster;
    cli_help_param_t * pchp = (cli_help_param_t *)pParam;

    if (pchp->chp_u8Action == CLI_ACTION_SHOW_HELP)
    {
        cliengOutputLine("show all available commands");
    }
    else
    {
        cliengOutputLine("exit: exit CLI.");
        cliengOutputLine("help: list command.");
        cliengOutputLine("user: list user.");
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 addCmd(jiufeng_cli_master_t * pocm, cli_param_t * pcp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_cmd_t * cmd;

    cliengNewCmd(
        "user", _setDefaultParamUser, pcp, _parseUser, _processUser, &cmd);

    cliengNewCmd(
        "exit", _setDefaultParamExit, pcp, _parseExit, _processExit, &cmd);

    cliengNewCmd(
        "help", _setDefaultParamHelp, pcp, _parseHelp, _processHelp, &cmd);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

