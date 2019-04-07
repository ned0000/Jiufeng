/**
 *  @file cmdparser.h
 *
 *  @brief command parser implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef CLIENG_CMDPARSER_H
#define CLIENG_CMDPARSER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_logger.h"
#include "engio.h"
#include "clieng.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef void  clieng_parser_t;

typedef struct
{
    u32 cpp_u32MaxCmdSet;
    u32 cpp_u32MaxCmd;
    void * cpp_pMaster;
} clieng_parser_param_t;

/* --- functional routines ------------------------------------------------- */

/** Create the CLI Parser.
 *
 *  @param ppcp [out] the parser to be created and returned
 *  @param pcpp [in] the parameters to the parser
 *
 *  @return the error code
 */
u32 createParser(clieng_parser_t ** ppcp, clieng_parser_param_t * pcpp);

/** Destroy the CLI Parser.
 *
 *  @param ppcp [in/out] the parser to be destroyed.
 *
 *  @return the error code
 */
u32 destroyParser(clieng_parser_t ** ppcp);

/** Parse and process a command.
 *
 *  @param pcp [in] the parser.
 *  @param pstrCmd [in] a command string ended with a '\0'.
*/
u32 parseCmd(clieng_parser_t * pcp, olchar_t * pstrCmd);

u32 newCmd(
    clieng_parser_t * pcp, const olchar_t * pstrName,
    jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd);

#endif /*CLIENG_CMDPARSER_H*/

/*---------------------------------------------------------------------------*/


