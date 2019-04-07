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

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef void  dongyuan_t;

typedef struct
{
    olchar_t * gp_pstrCmdLine;
    olchar_t * gp_pstrSettingFile;
    u8 gp_u8Reserved[16];
} dongyuan_param_t;

/* --- functional routines ------------------------------------------------- */
u32 setDefaultDongyuanParam(dongyuan_param_t * pgp);

u32 createDongyuan(dongyuan_t ** ppDongyuan, dongyuan_param_t * pgp);

u32 destroyDongyuan(dongyuan_t ** ppDongyuan);

u32 startDongyuan(dongyuan_t * pDongyuan);

u32 stopDongyuan(dongyuan_t * pDongyuan);

#endif /*JIUFENG_DONGYUAN_H*/

/*---------------------------------------------------------------------------*/


