/**
 *  @file dongyuan.h
 *
 *  @brief software management header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_DONGYUAN_H
#define JIUFENG_DONGYUAN_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t * dp_pstrCmdLine;
    olchar_t * dp_pstrSettingFile;
    u8 dp_u8Reserved[16];
} dongyuan_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 setDefaultDongyuanParam(dongyuan_param_t * pgp);

u32 initDongyuan(dongyuan_param_t * pgp);

u32 finiDongyuan(void);

u32 startDongyuan(void);

u32 stopDongyuan(void);

#endif /*JIUFENG_DONGYUAN_H*/

/*------------------------------------------------------------------------------------------------*/


