/**
 *  @file cmdhistory.c
 *
 *  @brief The command history module.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"

#include "cmdhistory.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define INVALID_CMD_INDEX    (0xffffffff)

typedef struct
{
    /**The max number of characters for one command.*/
	olsize_t icch_sMaxCmdLine;
    /**The size of the command history buffer. In the number of commands.*/
	olsize_t icch_sCmdHistroyBuf;
	olindex_t icch_iCurrentCmd;
	olindex_t icch_iFirstCmd;
	olindex_t icch_iLastCmd;
	olchar_t  icch_strCommands[4];
} internal_clieng_cmd_history_t;

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

u32 createCommandHistory(clieng_cmd_history_t ** ppcch, clieng_cmd_history_param_t * pcchp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = 0;
    internal_clieng_cmd_history_t * picch = NULL;

    jf_logger_logInfoMsg("create cmd history" );
    
    assert((ppcch != NULL) && (pcchp != NULL));
        
    size = sizeof(internal_clieng_cmd_history_t) +
        (pcchp->cchp_sCmdHistroyBuf * (pcchp->cchp_sMaxCmdLine + 1)) - 4;

    u32Ret = jf_jiukun_allocMemory((void **)&picch, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(picch, size);
        picch->icch_sMaxCmdLine = pcchp->cchp_sMaxCmdLine;
        picch->icch_sCmdHistroyBuf = pcchp->cchp_sCmdHistroyBuf;
        picch->icch_iFirstCmd = INVALID_CMD_INDEX;
        picch->icch_iLastCmd = INVALID_CMD_INDEX;
        picch->icch_iCurrentCmd = 0;
            
        *ppcch = (clieng_cmd_history_t *)picch;
    }        
    
    return u32Ret;
}

u32 destroyCommandHistory(clieng_cmd_history_t ** ppcch)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_history_t * picch = NULL;
 
    assert((ppcch != NULL) && (*ppcch != NULL));

    picch = (internal_clieng_cmd_history_t *)*ppcch;
    *ppcch = NULL;
            
    jf_logger_logInfoMsg("destroy cmd history" );
            
    jf_jiukun_freeMemory((void **)&picch);
    
    return u32Ret;
}

u32 clearCommandHistory(clieng_cmd_history_t * pcch)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_history_t * picch = NULL;
    
    assert(pcch != NULL);

    picch = (internal_clieng_cmd_history_t *)pcch;
            
    jf_logger_logInfoMsg("clear cmd history");

    picch->icch_iFirstCmd = INVALID_CMD_INDEX;
    picch->icch_iLastCmd = INVALID_CMD_INDEX;
    picch->icch_iCurrentCmd = 0;
    
    return u32Ret;
}

u32 appendCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olchar_t * pstrLastCmd = NULL;
    internal_clieng_cmd_history_t * picch =
        (internal_clieng_cmd_history_t *)pcch;
    
    assert((pcch != NULL) && (pstrCmd != NULL));

    jf_logger_logInfoMsg("append cmd");
        
    length = ol_strlen(pstrCmd);
    if (length > picch->icch_sMaxCmdLine)
    {
        u32Ret = JF_ERR_CMD_TOO_LONG;
    }
    else
    {
        /* adjust the indexes */
        if (picch->icch_iLastCmd == INVALID_CMD_INDEX)
        {
            picch->icch_iLastCmd = 0;
            picch->icch_iFirstCmd = 0;
        }
        else
        {
            picch->icch_iLastCmd = (picch->icch_iLastCmd + 1) % picch->icch_sCmdHistroyBuf;
            if (picch->icch_iFirstCmd == picch->icch_iLastCmd)
            {
                /* the buffer is full, overwrite the oldest command */
                picch->icch_iFirstCmd = (picch->icch_iFirstCmd + 1) % picch->icch_sCmdHistroyBuf;
            }
        }
            
        /* current index would become the one after the last index */
        picch->icch_iCurrentCmd = picch->icch_iLastCmd + 1;

        /* copy the command over to the command buffer */
        pstrLastCmd = picch->icch_strCommands +
            (picch->icch_sMaxCmdLine + 1) * picch->icch_iLastCmd;
        ol_strcpy(pstrLastCmd, pstrCmd);
    }
    
    return u32Ret;
}

u32 getPreviousCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olindex_t previous = 0;
    olchar_t * pstrPreviousCmd = NULL;
    internal_clieng_cmd_history_t * picch = (internal_clieng_cmd_history_t *)pcch;

    assert(pcch != NULL && pstrCmdBuf != NULL && sBuf > 0);

    jf_logger_logInfoMsg("get previous command");    

    if (picch->icch_iLastCmd == INVALID_CMD_INDEX)
    {
        pstrCmdBuf[0] = 0;
    }
    else
    {
        if (picch->icch_iCurrentCmd > 0)
        {
            previous = picch->icch_iCurrentCmd - 1;
        }
        else
        {
            if (picch->icch_iFirstCmd == 0)
                previous = picch->icch_iLastCmd;
            else
                previous = picch->icch_sCmdHistroyBuf - 1;
        }
            
        pstrPreviousCmd = picch->icch_strCommands + (picch->icch_sMaxCmdLine + 1) * previous;
        length = ol_strlen(pstrPreviousCmd);
            
        if ((length + 1) > sBuf)
        {
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
        }
        else
        {
            ol_strcpy(pstrCmdBuf, pstrPreviousCmd);
            picch->icch_iCurrentCmd = previous;
        }
    }

    return u32Ret;
}

u32 getNextCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olindex_t next = 0;
    olchar_t * pstrNextCmd = NULL;
    internal_clieng_cmd_history_t * picch = (internal_clieng_cmd_history_t *)pcch;

    assert(pcch != NULL && pstrCmdBuf != NULL && sBuf > 0);

    jf_logger_logInfoMsg("get next command");
        
    if (picch->icch_iLastCmd == -1)
    {
        pstrCmdBuf[0] = 0; /* history buffer is empty */
    }
    else
    {
        if (((picch->icch_iCurrentCmd - 1) == picch->icch_iLastCmd) ||
            (picch->icch_iCurrentCmd == picch->icch_iLastCmd))
        {
            pstrCmdBuf[0] = 0; /* already reached the last command */
        }
        else
        {
            next = (picch->icch_iCurrentCmd + 1) % picch->icch_sCmdHistroyBuf;
            pstrNextCmd = picch->icch_strCommands + (picch->icch_sMaxCmdLine + 1) * next;
            length = ol_strlen(pstrNextCmd);
            if ((length + 1) > sBuf)
            {
                u32Ret = JF_ERR_BUFFER_TOO_SMALL;
            }
            else
            {
                ol_strcpy(pstrCmdBuf, pstrNextCmd);
                picch->icch_iCurrentCmd = next;
            }
        }
    }
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


