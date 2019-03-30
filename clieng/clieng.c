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

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "clieng.h"
#include "engio.h"
#include "cmdhistory.h"
#include "cmdparser.h"

/* --- private data structures --------------------------------------------- */

#define DEFAULT_PROMPT         "cli> "
#define MAX_PROMPT_LEN         (24)

#define MAX_USER_NAME_LEN      (24)

typedef struct
{
    boolean_t ic_bInitialized;
    u8 ic_u8Reserved[7];
	clieng_cmd_history_t * ic_pcchCmdHistory;
	clieng_parser_t * ic_pcpParser;
    olsize_t ic_sMaxCmdLine;
    olchar_t ic_strUsername[MAX_USER_NAME_LEN];
    olchar_t ic_strInputBuffer[JF_CLIENG_MAX_COMMAND_LINE_SIZE];
    olchar_t ic_strPrompt[MAX_PROMPT_LEN];
    jf_clieng_fnPrintGreeting_t ic_fnPrintGreeting;
    jf_clieng_fnPreEnterLoop_t ic_fnPreEnterLoop;
    jf_clieng_fnPostExitLoop_t ic_fnPostExitLoop;
    void * ic_pMaster;
    boolean_t  ic_bTerminateClieng;
    boolean_t ic_bEnableScriptEngine;
    olchar_t ic_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_SIZE];
    u32 ic_u32Reserved[8];
} internal_clieng_t;

static olint_t ls_nTerminationSignal = -1;
static internal_clieng_t ls_icClieng;

/* --- private funciton routines ------------------------------------------- */

static u32 _createCmdHistory(
    internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_cmd_history_param_t param;
    
    logInfoMsg("create cmd history");

    param.cchp_sCmdHistroyBuf = pjcip->jcip_sCmdHistroyBuf;
    param.cchp_sMaxCmdLine = pjcip->jcip_sMaxCmdLine;
    
    u32Ret = createCommandHistory(&pic->ic_pcchCmdHistory, &param);
    
    return u32Ret;
}

static u32 _createInputOutput(
    internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_io_param_t param;
    
    logInfoMsg("create input output");
    
    param.cip_pstrNewLine = pjcip->jcip_pstrNewLine;
    param.cip_fhOutput = pjcip->jcip_fhOutput;

    u32Ret = initCliengIo(&param);
    
    return u32Ret;
}

static u32 _createParser(
    internal_clieng_t * pic, jf_clieng_init_param_t * pjcip)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_parser_param_t param;
    
    logInfoMsg("create parser");

    memset(&param, 0, sizeof(clieng_parser_param_t));

    param.cpp_u32MaxCmdSet = pjcip->jcip_u32MaxCmdSet;
    param.cpp_pMaster = pjcip->jcip_pMaster;

    u32Ret = createParser(&(pic->ic_pcpParser), &param);
    
    return u32Ret;
}

void _terminateCliengShell(olint_t signal)
{
    ls_nTerminationSignal = signal;
}
#ifndef WINDOWS
static u32 _registerSignalHandlers(internal_clieng_t * pic)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nRet;
    olint_t nSignals[] = { SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGTSTP };
    olint_t nIndex, nSignalCount = 6;
    sigset_t ssSignalSet;
    struct sigaction act, oact;

    nIndex = 0;
    while ((u32Ret == OLERR_NO_ERROR) && (nIndex<nSignalCount))
    {
        nRet = sigemptyset(&ssSignalSet);
    	if (nRet == -1)
        {
            u32Ret = OLERR_OPERATION_FAIL;
    	    logErrMsg(
                u32Ret, "register signal, sigemptyset() for signals[%d] failed",
                nIndex);
    	}
        else
    	{
            nRet = sigaddset(&ssSignalSet, nSignals[nIndex]);
            if (nRet == -1)
    	    {
                u32Ret = OLERR_OPERATION_FAIL;
	            logErrMsg(u32Ret, "register signal, sigaddset() failed");
    	    }
        }

    	if (u32Ret == OLERR_NO_ERROR)
        {
            /*act.sa_sigaction = terminate;*/
            act.sa_handler = _terminateCliengShell;
            act.sa_mask = ssSignalSet;
            act.sa_flags = SA_SIGINFO;

#ifdef SA_RESTART
            act.sa_flags |= SA_RESTART;
#endif

            nRet = sigaction(nSignals[nIndex], &act, &oact);
            if (nRet == -1)
            {
                logErrMsg(
                    u32Ret,
                    "register signal, sigaction() for signals[%d] failed",
                    nIndex);
                u32Ret = OLERR_OPERATION_FAIL;
            }
        }

    	nIndex = nIndex+1;
    }

    return u32Ret;
}
#endif

static u32 _printPrompt(internal_clieng_t * pic)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pic->ic_strPrompt[0] != '\0')
        u32Ret = engioOutput("%s", pic->ic_strPrompt);
    else
        u32Ret = engioOutput("%s", DEFAULT_PROMPT);
    
    return u32Ret;
}

static u32 _cliengLoop(internal_clieng_t * pic)
{
    u32 u32Ret = OLERR_NO_ERROR;
    clieng_input_type_t citInputType = cit_unknown;
    olsize_t length = JF_CLIENG_MAX_COMMAND_LINE_SIZE;
    olchar_t cmd_str[JF_CLIENG_MAX_COMMAND_LINE_SIZE];
    olchar_t * pstrDesc = NULL;
    
    /* Prompt */
    _printPrompt(pic);

    while (u32Ret == OLERR_NO_ERROR)
    {
    	u32Ret = engioInput(
            &citInputType, pic->ic_strInputBuffer, &length,
            ol_strlen(pic->ic_strPrompt));
        if (u32Ret == OLERR_NO_ERROR)
        {
            /* navigation command */
            if (citInputType == cit_navigation_cmd)
            {
                /* Upper Arrow */
                if (pic->ic_strInputBuffer[0] == 'A')
                {
                    getPreviousCommand(
                        pic->ic_pcchCmdHistory, pic->ic_strInputBuffer, length);
                }    
	    	    /* Down Arrow */
		        else if (pic->ic_strInputBuffer[0] == 'B')
		        {
                    getNextCommand(
                        pic->ic_pcchCmdHistory, pic->ic_strInputBuffer, length);
                }
            } 
            /* regular command */
            else if (citInputType == cit_line)
            {
    	        break;
            }
            else 
            {
                u32Ret = OLERR_NOT_IMPLEMENTED;
                logErrMsg(u32Ret, "_cliengLoop() got input with type %d",
                    citInputType);
    	        return u32Ret;
            }
        }
    }
    
    if (u32Ret == OLERR_NO_ERROR)
    {
    	ol_strcpy(cmd_str, pic->ic_strInputBuffer);
        u32Ret = parseCmd(pic->ic_pcpParser, pic->ic_strInputBuffer);
        if (u32Ret != OLERR_BLANK_CMD && u32Ret != OLERR_COMMENT_CMD)
	    {
	        appendCommand(pic->ic_pcchCmdHistory, cmd_str);
            engioOutput("");
    	}
    }
    else
    {
        pstrDesc = getErrorDescription(u32Ret);
        outputLine("Error (0x%x): %s", u32Ret, pstrDesc);
        outputLine("");
    }

    return u32Ret;
}

static u32 _processScriptCmd(internal_clieng_t * pic, olchar_t * pstrInput)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = parseCmd(pic->ic_pcpParser, pstrInput);   
    
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_clieng_clearCommandHistory(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    clearCommandHistory(pic->ic_pcchCmdHistory);

    return u32Ret;
}

u32 jf_clieng_init(jf_clieng_init_param_t * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    memset(pic, 0, sizeof(internal_clieng_t));

    pic->ic_fnPrintGreeting = pParam->jcip_fnPrintGreeting;
    pic->ic_fnPreEnterLoop = pParam->jcip_fnPreEnterLoop;
    pic->ic_fnPostExitLoop = pParam->jcip_fnPostExitLoop;
    pic->ic_pMaster = pParam->jcip_pMaster;
    pic->ic_bEnableScriptEngine = pParam->jcip_bEnableScriptEngine;
    if (pic->ic_bEnableScriptEngine)
        ol_strncpy(
            pic->ic_strInputCmd, pParam->jcip_strInputCmd,
            JF_CLIENG_MAX_COMMAND_LINE_SIZE - 1);

#ifndef WINDOWS
    _registerSignalHandlers(pic);
#endif       

    u32Ret = _createCmdHistory(pic, pParam);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _createInputOutput(pic, pParam);
    }
        
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _createParser(pic, pParam);
    }
        
    if (u32Ret == OLERR_NO_ERROR)
        pic->ic_bInitialized = TRUE;
    else
        jf_clieng_fini();

    return u32Ret;
}

u32 jf_clieng_run(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    logInfoMsg("run clieng");

    /* Print greeting */
    if (pic->ic_fnPrintGreeting != NULL)
        pic->ic_fnPrintGreeting(pic->ic_pMaster);

    if (pic->ic_fnPreEnterLoop != NULL)
        u32Ret = pic->ic_fnPreEnterLoop(pic->ic_pMaster);

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pic->ic_bEnableScriptEngine)
        {
            u32Ret = _processScriptCmd(pic, pic->ic_strInputCmd);
        }
        else
        {
            /* Enter clieng loop */
            while (! pic->ic_bTerminateClieng)
            {
                u32Ret = _cliengLoop(pic);
            }
        }
    }

    logInfoMsg(
        "Clieng receives a signal of %d and returns %d(0x%X)", 
        ls_nTerminationSignal, u32Ret, u32Ret);

    if (pic->ic_fnPostExitLoop != NULL)
        pic->ic_fnPostExitLoop(pic->ic_pMaster);
    
    return u32Ret;
}

u32 jf_clieng_stop(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    pic->ic_bTerminateClieng = TRUE;

    return u32Ret;
}

u32 jf_clieng_fini()
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;
    
    logInfoMsg("destroy clieng");

    if (pic->ic_pcpParser != NULL)
        destroyParser(&(pic->ic_pcpParser));

    finiCliengIo();

    if (pic->ic_pcchCmdHistory != NULL)
        destroyCommandHistory(&(pic->ic_pcchCmdHistory));

    return u32Ret;
}

u32 jf_clieng_newCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;

    assert((pstrName != NULL) && (pParam != NULL) &&
           (fnParseCmd != NULL) && (fnProcessCmd != NULL));

    u32Ret = newCmd(
        pic->ic_pcpParser, pstrName, fnSetDefaultParam,
        fnParseCmd, fnProcessCmd, pParam, ppCmd);

    return u32Ret;
}

u32 jf_clieng_newCmdSet(
    const olchar_t * pstrName, jf_clieng_cmd_set_t ** ppCmdSet)
{
    u32 u32Ret = OLERR_NOT_IMPLEMENTED;

    return u32Ret;
}

u32 jf_clieng_addToCmdSet(
    jf_clieng_cmd_t * pCmd, jf_clieng_cmd_set_t * pCmdSet)
{
    u32 u32Ret = OLERR_NOT_IMPLEMENTED;

    return u32Ret;
}

u32 jf_clieng_setPrompt(const olchar_t * pstrPrompt)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_clieng_t * pic = &ls_icClieng;;

    assert(pstrPrompt != NULL);

    if (strlen(pstrPrompt) > MAX_PROMPT_LEN - 1)
        u32Ret = OLERR_CLIENG_PROMPT_TOO_LONG;

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strcpy(pic->ic_strPrompt, pstrPrompt);
    }

    return u32Ret;
}


/*---------------------------------------------------------------------------*/

