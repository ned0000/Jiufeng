/**
 *  @file main.h
 *
 *  @brief cli main header file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef JIUFENG_CLI_MAIN_H
#define JIUFENG_CLI_MAIN_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "user.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u32 ocm_u32Reserved[32];

    olchar_t ocm_strUsername[MAX_USER_NAME_LEN];
} jiufeng_cli_master_t;



/* --- functional routines ------------------------------------------------- */


#endif /*JIUFENG_CLI_MAIN_H*/

/*---------------------------------------------------------------------------*/


