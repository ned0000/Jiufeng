/**
 *  @file configpersistency.h
 *
 *  @brief Config persistency header file.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef CONFIG_MGR_CONFIG_PERSISTENCY_H
#define CONFIG_MGR_CONFIG_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_config.h"

#include "configmgrsetting.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

u32 loadConfigFromPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig);

u32 saveConfigToPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig);

#endif /*CONFIG_MGR_CONFIG_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/


