/**
 *  @file jf_sqlite.h
 *
 *  @brief Header file which define sqlite object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Use transaction routines instead of sql statement to start, commit and rollback transaction.
 *
 */

#ifndef JIUTAI_SQLITE_H
#define JIUTAI_SQLITE_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include "sqlite3.h"

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The sqlite initialization parameter.
 */
typedef struct jf_sqlite_init_param
{
    /**The database file name.*/
    olchar_t * jsip_pstrDbName;
    u32 jsip_u32Reserved[4];
} jf_sqlite_init_param_t;

/** Define the sqlite data type.
 */
typedef struct jf_sqlite
{
    /**Transaction is started if it's TRUE.*/
    boolean_t js_bTransactionStarted;
    /**This object is initialized if it's TRUE.*/
    boolean_t js_bInitialized;
    u8 js_u8Reserved[6];
    /**The handle to sqlite database.*/
    sqlite3 * js_psSqlite;
} jf_sqlite_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the sqlite object.
 */
u32 jf_sqlite_init(jf_sqlite_t * pjs, jf_sqlite_init_param_t * param);

/** Finalize the sqlite object.
 */
u32 jf_sqlite_fini(jf_sqlite_t * pjs);

/** Rollback transaction.
 */
u32 jf_sqlite_rollbackTransaction(jf_sqlite_t * pjs);

/** Start transaction.
 */
u32 jf_sqlite_startTransaction(jf_sqlite_t * pjs);

/** Commit transaction.
 */
u32 jf_sqlite_commitTransaction(jf_sqlite_t * pjs);

/** Execute SQL statement.
 */
u32 jf_sqlite_execSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, olchar_t * pstrResult, olsize_t sResult);

#endif /*JIUTAI_SQLITE_H*/

/*------------------------------------------------------------------------------------------------*/


