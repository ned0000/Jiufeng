/**
 *  @file cmdhistory.h
 *
 *  @brief command history header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef CLIENG_CMDHISTORY_H
#define CLIENG_CMDHISTORY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef void clieng_cmd_history_t;

typedef struct
{
    /** Max number of characters for one command */
	olsize_t cchp_sMaxCmdLine;
    /** Number of commands in the command history buffer */
	olsize_t cchp_sCmdHistroyBuf;
} clieng_cmd_history_param_t;

/* --- functional routines ------------------------------------------------- */

/** Create the CLI command history buffer
 *
 *  @param ppcch [out] the command history buffer to be created and returned
 *  @param pcchp [in] the parameters to the command history
 *
 *  @return the error code
 */
u32 createCommandHistory(
    clieng_cmd_history_t ** ppcch, clieng_cmd_history_param_t * pcchp);

/** Destroy the CLI command history buffer
 *
 *  @param ppcch [in/out] the command history buffer to be destroyed
 *
 *  @return the error code
 */
u32 destroyCommandHistory(clieng_cmd_history_t ** ppcch);

/** Clear the CLI Command History buffer
 *
 *	@param ppcch [in/out] the pointer to the command history buffer.
 *
 *  @return the error code
 */
u32 clearCommandHistory(clieng_cmd_history_t * ppcch);

/** Append a command to the command history buffer.
 *
 *	@param pcch [in] the pointer to the command history buffer.
 *  @param pstrCmd [in] the cmmand to append
 *
 *  @return the error code
 */
u32 appendCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmd);

/** Get previous command in the command history.
 *
 *	@param pcch [in] the pointer to the command history buffer.
 *  @param pstrCmdBuf [out] the pointer to a buffer where the previous command
 *   will be returned. The returned command will be ended with '\0'.
 *  @param sBuf [in] the size of the command buffer
 *
 *  @return the error code
 */
u32 getPreviousCommand(
    clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf);

/** Get next command in the command history.
 *	@param pcch [in] the pointer to the command history buffer.
 *  @param pstrCmdBuf [out] the pointer to a buffer where the previous command
 *   will be returned. The returned command will be ended with '\0'.
 *  @param sBuf [in] the size of the command buffer
 *
 *  @return the error code
 */
u32 getNextCommand(
    clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf);

#endif /*CLIENG_CMDHISTORY_H*/

/*---------------------------------------------------------------------------*/



