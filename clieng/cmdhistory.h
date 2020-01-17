/**
 *  @file cmdhistory.h
 *
 *  @brief Header file for command history module.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef CLIENG_CMDHISTORY_H
#define CLIENG_CMDHISTORY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef void clieng_cmd_history_t;

typedef struct
{
    /**Max number of characters for one command.*/
	olsize_t cchp_sMaxCmdLine;
    /**Number of commands in the command history buffer.*/
	olsize_t cchp_sCmdHistroyBuf;
} clieng_cmd_history_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the CLI command history buffer.
 *
 *  @param ppcch [out] The command history buffer to be created and returned.
 *  @param pcchp [in] The parameters to the command history.
 *
 *  @return The error code.
 */
u32 createCommandHistory(clieng_cmd_history_t ** ppcch, clieng_cmd_history_param_t * pcchp);

/** Destroy the CLI command history buffer.
 *
 *  @param ppcch [in/out] The command history buffer to be destroyed.
 *
 *  @return The error code.
 */
u32 destroyCommandHistory(clieng_cmd_history_t ** ppcch);

/** Clear the CLI Command History buffer.
 *
 *	@param ppcch [in/out] The pointer to the command history buffer.
 *
 *  @return The error code.
 */
u32 clearCommandHistory(clieng_cmd_history_t * ppcch);

/** Append a command to the command history buffer.
 *
 *	@param pcch [in] The pointer to the command history buffer.
 *  @param pstrCmd [in] The cmmand to append.
 *
 *  @return The error code.
 */
u32 appendCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmd);

/** Get previous command in the command history.
 *
 *	@param pcch [in] The pointer to the command history buffer.
 *  @param pstrCmdBuf [out] The pointer to a buffer where the previous command will be returned.
 *   The returned command will be ended with '\0'.
 *  @param sBuf [in] The size of the command buffer.
 *
 *  @return The error code.
 */
u32 getPreviousCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf);

/** Get next command in the command history.
 *
 *	@param pcch [in] The pointer to the command history buffer.
 *  @param pstrCmdBuf [out] The pointer to a buffer where the previous command will be returned.
 *   The returned command will be ended with '\0'.
 *  @param sBuf [in] The size of the command buffer.
 *
 *  @return The error code.
 */
u32 getNextCommand(clieng_cmd_history_t * pcch, olchar_t * pstrCmdBuf, olsize_t sBuf);

#endif /*CLIENG_CMDHISTORY_H*/

/*------------------------------------------------------------------------------------------------*/



