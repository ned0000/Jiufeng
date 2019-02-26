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

typedef struct jt_sqlite_param
{
    olchar_t * jsp_pstrDbName;
    u32 jsp_u32Reserved[4];
} jt_sqlite_param_t;

typedef struct jt_sqlite
{
    boolean_t js_bTransactionStarted;
    boolean_t js_bInitialized;
    u8 js_u8Reserved[6];

    sqlite3 * js_psSqlite;
} jt_sqlite_t;

/* --- functional routines ------------------------------------------------- */

u32 initJtSqlite(jt_sqlite_t * pjs, jt_sqlite_param_t * param);

u32 finiJtSqlite(jt_sqlite_t * pjs);

u32 rollbackJtSqliteTransaction(jt_sqlite_t * pjs);

u32 startJtSqliteTransaction(jt_sqlite_t * pjs);

u32 commitJtSqliteTransaction(jt_sqlite_t * pjs);

u32 execJtSqliteSql(
    jt_sqlite_t * pjs, olchar_t * pstrSql, olchar_t * pstrResult,
    olsize_t sResult);

#endif /*JIUTAI_JTSQLITE_H*/

/*---------------------------------------------------------------------------*/


