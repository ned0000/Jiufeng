/**
 *  @file cmdparser.h
 *
 *  @brief Header file of CLI engine command parser module.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef CLIENG_CMDPARSER_H
#define CLIENG_CMDPARSER_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_logger.h"
#include "jf_clieng.h"

#include "engio.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameters for initializing the command parser module.
 */
typedef struct
{
    /**Maximum number of command set supported.*/
    u32 cpip_u32MaxCmdSet;
    /**Maximum number of command supported.*/
    u32 cpip_u32MaxCmd;
    void * cpip_pMaster;
} clieng_parser_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the CLI engine Parser.
 *
 *  @param pcpip [in] The parameters for initializing the parser.
 *
 *  @return The error code.
 */
u32 initCliengParser(clieng_parser_init_param_t * pcpip);

/** Finalize the CLI engine Parser.
 *
 *  @return The error code.
 */
u32 finiCliengParser(void);

/** Parse and process a command.
 *
 *  @param pstrCmd [in] A command string ended with a '\0'.
 *
 *  @return The error code.
 */
u32 parseCliengCmd(olchar_t * pstrCmd);

/** Create a command.
 *
 *  @param pstrName [in] Name of the command.
 *  @param fnSetDefaultParam [in] Callback function to set the default parameters of command.
 *  @param fnParseCmd [in] Callback function to parse parameters of command.
 *  @param fnProcessCmd [in] Callback function to process command.
 *  @param pParam [in] Parameters of command.
 *  @param ppCmd [out] The command created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 newCliengCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd, void * pParam,
    jf_clieng_cmd_t ** ppCmd);

#endif /*CLIENG_CMDPARSER_H*/

/*------------------------------------------------------------------------------------------------*/
