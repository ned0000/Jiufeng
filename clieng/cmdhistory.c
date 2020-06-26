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


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"

#include "cmdhistory.h"

/* --- private data/data structure section ------------------------------------------------------ */

#define INVALID_CMD_INDEX                (0xffffffff)

typedef struct
{
    boolean_t icch_bInitialized;
    u8 icch_u8Reserved[7];

    /**The max number of characters for one command.*/
	olsize_t icch_sMaxCmdLine;
    /**The size of the command history buffer. It's the number of commands.*/
	olsize_t icch_sCmdHistroyBuf;

	olindex_t icch_iCurrentCmd;
	olindex_t icch_iFirstCmd;
	olindex_t icch_iLastCmd;

    /**Command history.*/
	olchar_t * icch_strCommands;
} internal_clieng_cmd_history_t;

static internal_clieng_cmd_history_t ls_icchCmdHistory;

/* --- private routine section ------------------------------------------------------------------ */



/* --- public routine section ------------------------------------------------------------------- */

u32 initCommandHistory(clieng_cmd_history_param_t * pcchp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;
    olsize_t size = 0;

    assert(pcchp != NULL);

    JF_LOGGER_INFO(
        "sMaxCmdLine: %d, sCmdHistroyBuf: %d", pcchp->cchp_sMaxCmdLine, pcchp->cchp_sCmdHistroyBuf);
        
    ol_bzero(picch, sizeof(*picch));

    picch->icch_sMaxCmdLine = pcchp->cchp_sMaxCmdLine;
    picch->icch_sCmdHistroyBuf = pcchp->cchp_sCmdHistroyBuf;
    picch->icch_iFirstCmd = INVALID_CMD_INDEX;
    picch->icch_iLastCmd = INVALID_CMD_INDEX;
    picch->icch_iCurrentCmd = 0;

    size = pcchp->cchp_sCmdHistroyBuf * (pcchp->cchp_sMaxCmdLine + 1);

    u32Ret = jf_jiukun_allocMemory((void **)&picch->icch_strCommands, size);

    if (u32Ret == JF_ERR_NO_ERROR)
        picch->icch_bInitialized = TRUE;
    else
        finiCommandHistory();

    return u32Ret;
}

u32 finiCommandHistory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;
 
    JF_LOGGER_INFO("destroy");
            
    if (picch->icch_strCommands != NULL)
        jf_jiukun_freeMemory((void **)&picch->icch_strCommands);

    picch->icch_bInitialized = FALSE;

    return u32Ret;
}

u32 clearCommandHistory(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;
    
    JF_LOGGER_DEBUG("clear");

    picch->icch_iFirstCmd = INVALID_CMD_INDEX;
    picch->icch_iLastCmd = INVALID_CMD_INDEX;
    picch->icch_iCurrentCmd = 0;
    
    return u32Ret;
}

u32 appendCommand(olchar_t * pstrCmd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olchar_t * pstrLastCmd = NULL;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;
    
    assert(pstrCmd != NULL);

    JF_LOGGER_DEBUG("append cmd");
        
    length = ol_strlen(pstrCmd);
    if (length > picch->icch_sMaxCmdLine)
    {
        u32Ret = JF_ERR_CMD_TOO_LONG;
    }
    else
    {
        /*Adjust the indexes.*/
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
                /*The buffer is full, overwrite the oldest command.*/
                picch->icch_iFirstCmd = (picch->icch_iFirstCmd + 1) % picch->icch_sCmdHistroyBuf;
            }
        }
            
        /*Current index would become the one after the last index.*/
        picch->icch_iCurrentCmd = picch->icch_iLastCmd + 1;

        /*Copy the command over to the command buffer.*/
        pstrLastCmd = picch->icch_strCommands +
            (picch->icch_sMaxCmdLine + 1) * picch->icch_iLastCmd;
        ol_strcpy(pstrLastCmd, pstrCmd);
    }
    
    return u32Ret;
}

u32 getPreviousCommand(olchar_t * pstrCmdBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olindex_t previous = 0;
    olchar_t * pstrPreviousCmd = NULL;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;

    assert(pstrCmdBuf != NULL && sBuf > 0);

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

u32 getNextCommand(olchar_t * pstrCmdBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length = 0;
    olindex_t next = 0;
    olchar_t * pstrNextCmd = NULL;
    internal_clieng_cmd_history_t * picch = &ls_icchCmdHistory;

    assert(pstrCmdBuf != NULL && sBuf > 0);

    if (picch->icch_iLastCmd == -1)
    {
        /*History buffer is empty.*/
        pstrCmdBuf[0] = 0;
    }
    else
    {
        if (((picch->icch_iCurrentCmd - 1) == picch->icch_iLastCmd) ||
            (picch->icch_iCurrentCmd == picch->icch_iLastCmd))
        {
            /*Already reached the last command.*/
            pstrCmdBuf[0] = 0;
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
