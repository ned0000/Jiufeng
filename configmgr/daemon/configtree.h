/**
 *  @file configtree.h
 *
 *  @brief Config tree header file, provide some functional routine for config tree.
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef CONFIG_MGR_CONFIGTREE_H
#define CONFIG_MGR_CONFIGTREE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_config.h"
#include "jf_network.h"

#include "configmgrsetting.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for initializing the config tree module.
 */
typedef struct
{
    internal_config_mgr_setting_t * ctip_picmsSetting;

} config_tree_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the config tree module.
 */
u32 initConfigTree(config_tree_init_param_t * pctip);

/** Finalize the config tree module.
 */
u32 finiConfigTree(void);

/** Get config.
 */
u32 getConfigFromConfigTree(
    u32 u32TransactionId, olchar_t * pstrName, olsize_t sName, olchar_t * pstrValue,
    olsize_t * psValue);

/** Set config.
 */
u32 setConfigIntoConfigTree(
    u32 u32TransactionId, olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue,
    olsize_t sValue);

/** Start a transaction.
 */
u32 startConfigTreeTransaction(u32 * pu32TransactionId);

/** commit a transaction.
 */
u32 commitConfigTreeTransaction(u32 u32TransactionId);

/** Rollback a transaction.
 */
u32 rollbackConfigTreeTransaction(u32 u32TransactionId);

#endif /*CONFIG_MGR_CONFIGTREE_H*/

/*------------------------------------------------------------------------------------------------*/


