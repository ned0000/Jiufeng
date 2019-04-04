/**
 *  @file jtsqlite.h
 *
 *  @brief sqlite common object header file
 *
 *  @author Min Zhang
 *
 *  @note Use transaction routines instead of sql statement to start, commit
 *   and rollback transaction
 *
 */

#ifndef JIUTAI_JTSQLITE_H
#define JIUTAI_JTSQLITE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "sqlite3.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */

typedef struct jf_sqlite_init_param
{
    olchar_t * jsip_pstrDbName;
    u32 jsip_u32Reserved[4];
} jf_sqlite_init_param_t;

typedef struct jf_sqlite
{
    boolean_t js_bTransactionStarted;
    boolean_t js_bInitialized;
    u8 js_u8Reserved[6];

    sqlite3 * js_psSqlite;
} jf_sqlite_t;

/* --- functional routines ------------------------------------------------- */

u32 jf_sqlite_init(jf_sqlite_t * pjs, jf_sqlite_init_param_t * param);

u32 jf_sqlite_fini(jf_sqlite_t * pjs);

u32 jf_sqlite_rollbackTransaction(jf_sqlite_t * pjs);

u32 jf_sqlite_startTransaction(jf_sqlite_t * pjs);

u32 jf_sqlite_commitTransaction(jf_sqlite_t * pjs);

u32 jf_sqlite_execSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, olchar_t * pstrResult,
    olsize_t sResult);

#endif /*JIUTAI_JTSQLITE_H*/

/*---------------------------------------------------------------------------*/


