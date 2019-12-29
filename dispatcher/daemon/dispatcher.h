/**
 *  @file dispatcher.h
 *
 *  @brief software management header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_DISPATCHER_H
#define JIUFENG_DISPATCHER_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t * dp_pstrCmdLine;
    olchar_t * dp_pstrConfigDir;
    u8 dp_u8Reserved[16];
} dispatcher_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 setDefaultDispatcherParam(dispatcher_param_t * pdp);

u32 initDispatcher(dispatcher_param_t * pdp);

u32 finiDispatcher(void);

u32 startDispatcher(void);

u32 stopDispatcher(void);

#endif /*JIUFENG_DISPATCHER_H*/

/*------------------------------------------------------------------------------------------------*/


