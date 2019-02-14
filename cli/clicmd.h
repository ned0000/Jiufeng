/**
 *  @file clicmd.h
 *
 *  @brief CLI command processor
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef CLI_CLICMD_H
#define CLI_CLICMD_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "clieng.h"
#include "main.h"

/* --- constant definitions ------------------------------------------------ */
#define CLI_ACTION_UNKNOWN    0
#define CLI_ACTION_SHOW_HELP  1

/* --- data structures ----------------------------------------------------- */
typedef struct
{
#define CLI_ACTION_ADD_USER  0x80
#define CLI_ACTION_LIST_USER  0x81
    u8 cup_u8Action;
    boolean_t cup_bVerbose;
    u8 cup_u8Reserved[6];
    olchar_t cup_strUsername[MAX_USER_NAME_LEN];
} cli_user_param_t;

typedef struct
{
    u8 chp_u8Action;
    u8 chp_u8Reserved[15];
} cli_help_param_t;

typedef struct
{
    u8 cep_u8Action;
    u8 cep_u8Reserved[15];
} cli_exit_param_t;

typedef union
{
    cli_exit_param_t cp_cepExit;
    cli_help_param_t cp_chpHelp;
    cli_user_param_t cp_cupUser;
} cli_param_t;

/* --- functional routines ------------------------------------------------- */

u32 addCmd(jiufeng_cli_master_t * pocm, cli_param_t * pcp);

#endif /*CLI_CLICMD_H*/

/*---------------------------------------------------------------------------*/


