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

/** Define the parameters for initializing the command history module.
 */
typedef struct
{
    /**Max number of characters for one command.*/
	olsize_t cchp_sMaxCmdLine;
    /**Number of commands in the command history buffer.*/
	olsize_t cchp_sCmdHistroyBuf;
} clieng_cmd_history_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the CLI command history buffer.
 *
 *  @param pcchp [in] The parameters to the command history.
 *
 *  @return The error code.
 */
u32 initCommandHistory(clieng_cmd_history_param_t * pcchp);

/** Finalize the CLI command history buffer.
 *
 *  @return The error code.
 */
u32 finiCommandHistory(void);

/** Clear the CLI Command History buffer.
 *
 *  @return The error code.
 */
u32 clearCommandHistory(void);

/** Append a command to the command history buffer.
 *
 *  @param pstrCmd [in] The cmmand to append.
 *
 *  @return The error code.
 */
u32 appendCommand(olchar_t * pstrCmd);

/** Get previous command in the command history.
 *
 *  @param pstrCmdBuf [out] The pointer to a buffer where the previous command will be returned.
 *   The returned command will be ended with '\0'.
 *  @param sBuf [in] The size of the command buffer.
 *
 *  @return The error code.
 */
u32 getPreviousCommand(olchar_t * pstrCmdBuf, olsize_t sBuf);

/** Get next command in the command history.
 *
 *  @param pstrCmdBuf [out] The pointer to a buffer where the previous command will be returned.
 *   The returned command will be ended with '\0'.
 *  @param sBuf [in] The size of the command buffer.
 *
 *  @return The error code.
 */
u32 getNextCommand(olchar_t * pstrCmdBuf, olsize_t sBuf);

#endif /*CLIENG_CMDHISTORY_H*/

/*------------------------------------------------------------------------------------------------*/
