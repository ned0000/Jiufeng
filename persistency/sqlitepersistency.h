/**
 *  @file sqlitepersistency.h
 *
 *  @brief Sqlite persistency header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef PERSISTENCY_SQLITE_PERSISTENCY_H
#define PERSISTENCY_SQLITE_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include "sqlite3.h"

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

#include "persistencycommon.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the sqlite persistency.
 *
 *  @param pManager [in] The persistency manager.
 *  @param pConfig [out] The config for sqlite persistency.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 initSqlitePersistency(
    persistency_manager_t * pManager, jf_persistency_config_sqlite_t * pConfig);

#endif /*PERSISTENCY_SQLITE_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/
