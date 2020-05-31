/**
 *  @file configmgr/daemon/configmgr.h
 *
 *  @brief Config manager header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_CONFIG_MGR_H
#define JIUFENG_CONFIG_MGR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    olchar_t * cmp_pstrCmdLine;
    olchar_t * cmp_pstrSettingFile;
    u8 cmp_u8Reserved[16];
} config_mgr_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 setDefaultConfigMgrParam(config_mgr_param_t * pgp);

u32 initConfigMgr(config_mgr_param_t * pgp);

u32 finiConfigMgr(void);

u32 startConfigMgr(void);

u32 stopConfigMgr(void);

#endif /*JIUFENG_CONFIG_MGR_H*/

/*------------------------------------------------------------------------------------------------*/


