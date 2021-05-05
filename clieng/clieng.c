/**
 *  @file clieng.c
 *
 *  @brief Implement the exposed routine of the CLI Engine.
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
#include "jf_clieng.h"
#include "jf_process.h"
#include "jf_string.h"

#include "engio.h"
#include "cmdhistory.h"
#include "cmdparser.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define CLIENG_DEFAULT_PROMPT                     "cli> "

#define CLIENG_MAX_PROMPT_LEN                     (24)

#define CLIENG_CMD_HISTORY_SIZE                   (32)

typedef struct
{
    boolean_t ic_bInitialized;
    boolean_t ic_bTerminateClieng;
    u8 ic_u8Reserved[6];

    olchar_t ic_strInputBuffer[JF_CLIENG_MAX_COMMAND_LINE_LEN];
    olchar_t ic_strPrompt[CLIENG_MAX_PROMPT_LEN];

    jf_clieng_fnPrintGreeting_t ic_fnPrintGreeting;
    jf_clieng_fnPreEnterLoop_t ic_fnPreEnterLoop;
    jf_clieng_fnPostExitLoop_t ic_fnPostExitLoop;
    void * ic_pMaster;

    boolean_t ic_bEnableScriptEngine;
    u8 ic_u8Reserved3[7];
    olchar_t ic_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_LEN];
    u32 ic_u32Reserved[8];
} internal_clieng_t;

static olint_t ls_nTerminationSignal = -1;

static internal_clieng_t ls_icClieng;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initCmdHistory(
    internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    clieng_cmd_history_param_t param;

    param.cchp_sCmdHistroyBuf = CLIENG_CMD_HISTORY_SIZE;
    param.cchp_sMaxCmdLine = JF_CLIENG_MAX_COMMAND_LINE_LEN;
    
    u32Ret = initCommandHistory(&param);
    
    return u32Ret;
}

static u32 _initCliengIo(
    internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    clieng_io_param_t param;
    
    param.cip_pstrNewLine = pjcip->jcip_pstrNewLine;
    param.cip_pjfOutput = pjcip->jcip_pjfOutput;

    u32Ret = initCliengIo(&param);

    return u32Ret;
}

static u32 _initCliengParser(internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    clieng_parser_init_param_t param;
    
    ol_bzero(&param, sizeof(param));

    param.cpip_u32MaxCmdSet = pjcip->jcip_u32MaxCmdSet;
    param.cpip_pMaster = pjcip->jcip_pMaster;

    u32Ret = initCliengParser(&param);
    
    return u32Ret;
}

static void _terminateCliengShell(olint_t signal)
{
    ls_nTerminationSignal = signal;
}

static u32 _registerSignalHandlers(internal_clieng_t * pic)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_process_registerSignalHandlers(_terminateCliengShell);

    return u32Ret;
}

static u32 _printPrompt(internal_clieng_t * pic)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pic->ic_strPrompt[0] != JF_STRING_NULL_CHAR)
        u32Ret = engioOutput("%s", pic->ic_strPrompt);
    else
        u32Ret = engioOutput("%s", CLIENG_DEFAULT_PROMPT);
    
    return u32Ret;
}

static u32 _cliengLoop(internal_clieng_t * pic)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    clieng_input_type_t citInputType = cit_unknown;
    olsize_t length = JF_CLIENG_MAX_COMMAND_LINE_LEN;
    olchar_t cmd_str[JF_CLIENG_MAX_COMMAND_LINE_LEN];
    olchar_t * pstrDesc = NULL;
    
    /*Prompt.*/
    _printPrompt(pic);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
    	u32Ret = engioInput(
            &citInputType, pic->ic_strInputBuffer, &length, ol_strlen(pic->ic_strPrompt));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Navigation command.*/
            if (citInputType == cit_navigation_cmd)
            {
                /*Upper arrow.*/
                if (pic->ic_strInputBuffer[0] == 'A')
                {
                    getPreviousCommand(pic->ic_strInputBuffer, length);
                }    
	    	    /*Down arrow.*/
		        else if (pic->ic_strInputBuffer[0] == 'B')
		        {
                    getNextCommand(pic->ic_strInputBuffer, length);
                }
            } 
            /*Regular command.*/
            else if (citInputType == cit_line)
            {
    	        break;
            }
            else 
            {
                u32Ret = JF_ERR_NOT_IMPLEMENTED;
                JF_LOGGER_ERR(u32Ret, "input type: %d", citInputType);
    	        return u32Ret;
            }
        }
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
    	ol_strcpy(cmd_str, pic->ic_strInputBuffer);
        u32Ret = parseCliengCmd(pic->ic_strInputBuffer);
        if (u32Ret != JF_ERR_BLANK_CMD && u32Ret != JF_ERR_COMMENT_CMD)
	    {
	        appendCommand(cmd_str);
            engioOutput("");
    	}
    }
    else
    {
        pstrDesc = jf_err_getDescription(u32Ret);
        outputLine("Error (0x%x): %s", u32Ret, pstrDesc);
        outputLine("");
    }

    return u32Ret;
}

static u32 _processScriptCmd(internal_clieng_t * pic, olchar_t * pstrInput)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = parseCliengCmd(pstrInput);   
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_clieng_clearCommandHistory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_clieng_t * pic = &ls_icClieng;

    clearCommandHistory();

    return u32Ret;
}

u32 jf_clieng_init(jf_clieng_init_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    /*Initialize the internal cli engine.*/
    ol_bzero(pic, sizeof(internal_clieng_t));

    pic->ic_fnPrintGreeting = pParam->jcip_fnPrintGreeting;
    pic->ic_fnPreEnterLoop = pParam->jcip_fnPreEnterLoop;
    pic->ic_fnPostExitLoop = pParam->jcip_fnPostExitLoop;
    pic->ic_pMaster = pParam->jcip_pMaster;
    pic->ic_bEnableScriptEngine = pParam->jcip_bEnableScriptEngine;
    if (pic->ic_bEnableScriptEngine)
    {
        ol_strncpy(pic->ic_strInputCmd, pParam->jcip_strInputCmd, JF_CLIENG_MAX_COMMAND_LINE_LEN);
        pic->ic_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_LEN - 1] = JF_STRING_NULL_CHAR;
    }

#ifndef WINDOWS
    _registerSignalHandlers(pic);
#endif       

    /*Create command history module.*/
    u32Ret = _initCmdHistory(pic, pParam);

    /*Create input/output module.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initCliengIo(pic, pParam);

    /*Create command parser module.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initCliengParser(pic, pParam);
        
    if (u32Ret == JF_ERR_NO_ERROR)
        pic->ic_bInitialized = TRUE;
    else
        jf_clieng_fini();

    return u32Ret;
}

u32 jf_clieng_run(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    JF_LOGGER_INFO("run");

    /*Print greeting.*/
    if (pic->ic_fnPrintGreeting != NULL)
        pic->ic_fnPrintGreeting(pic->ic_pMaster);

    /*Callback function before entering loop.*/
    if (pic->ic_fnPreEnterLoop != NULL)
        u32Ret = pic->ic_fnPreEnterLoop(pic->ic_pMaster);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pic->ic_bEnableScriptEngine)
        {
            /*Execute script.*/
            u32Ret = _processScriptCmd(pic, pic->ic_strInputCmd);
        }
        else
        {
            /*Enter clieng loop.*/
            while (! pic->ic_bTerminateClieng)
            {
                u32Ret = _cliengLoop(pic);
            }
        }
    }

    /*Callback function after loop.*/
    if (pic->ic_fnPostExitLoop != NULL)
        pic->ic_fnPostExitLoop(pic->ic_pMaster);
    
    JF_LOGGER_INFO("exit");

    return u32Ret;
}

u32 jf_clieng_stop(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    pic->ic_bTerminateClieng = TRUE;

    return u32Ret;
}

u32 jf_clieng_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_clieng_t * pic = &ls_icClieng;
    
    JF_LOGGER_INFO("fini");

    finiCliengParser();

    finiCliengIo();

    finiCommandHistory();

    return u32Ret;
}

u32 jf_clieng_newCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd, void * pParam,
    jf_clieng_cmd_t ** ppCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_clieng_t * pic = &ls_icClieng;

    assert((pstrName != NULL) && (pParam != NULL) &&
           (fnParseCmd != NULL) && (fnProcessCmd != NULL));

    u32Ret = newCliengCmd(pstrName, fnSetDefaultParam, fnParseCmd, fnProcessCmd, pParam, ppCmd);

    return u32Ret;
}

u32 jf_clieng_newCmdSet(const olchar_t * pstrName, jf_clieng_cmd_set_t ** ppCmdSet)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;

    return u32Ret;
}

u32 jf_clieng_addToCmdSet(jf_clieng_cmd_t * pCmd, jf_clieng_cmd_set_t * pCmdSet)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;

    return u32Ret;
}

u32 jf_clieng_setPrompt(const olchar_t * pstrPrompt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;;

    assert(pstrPrompt != NULL);

    if (ol_strlen(pstrPrompt) >= CLIENG_MAX_PROMPT_LEN)
        u32Ret = JF_ERR_CLIENG_PROMPT_TOO_LONG;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(pic->ic_strPrompt, pstrPrompt);
    }

    return u32Ret;
}

void jf_clieng_clear(void)
{
#if defined(WINDOWS)

#elif defined(LINUX)
    ol_printf("\033[2J");
    ol_printf("\033[0;0H");
#endif

    return;
}

/*------------------------------------------------------------------------------------------------*/
