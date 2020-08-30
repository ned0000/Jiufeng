/**
 *  @file jf_sqlite.h
 *
 *  @brief Header file which define sqlite object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_sqlite object.
 *  -# This object is only available on Linux platform.
 *  -# Sqlite is a SQL database engine.
 *  -# Sqlite3 development files including header files and libraries must be installed, otherwise
 *   there are compile errors.
 *  -# Use transaction routines instead of sql statement to start, commit and rollback transaction.
 *  -# Link with jf_rand object to get random number.
 *  -# Link with sqlite3 library on Linux platform.
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

/** Define the statement handle data type.
 */
typedef void  jf_sqlite_stmt_t;

/** Define the function data type which is used to handle row data returned by executing the SQL
 *  statement.
 *
 *  @note
 *  -# The iteration of row data will stop if the return code is not JF_ERR_NO_ERROR.
 *  -# Use column access function in this function to get value of column.
 *
 *  @param pStmt [in] The statement handle.
 *  @param pArg [in] The argument for the callback function. It's passed by jf_sqlite_execSql().
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_sqlite_fnHandleRowData_t)(jf_sqlite_stmt_t * pStmt, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the sqlite object.
 *
 *  @param pjs [in] The sqlite object to be initialized.
 *  @param param [in] The parameter for Initializing the object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_sqlite_init(jf_sqlite_t * pjs, jf_sqlite_init_param_t * param);

/** Finalize the sqlite object.
 *
 *  @param pjs [in] The sqlite object to be finalized.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_sqlite_fini(jf_sqlite_t * pjs);

/** Rollback transaction.
 *
 *  @param pjs [in] The sqlite object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_INITIALIZED Sqlite object is not initialized.
 */
u32 jf_sqlite_rollbackTransaction(jf_sqlite_t * pjs);

/** Start transaction.
 *
 *  @param pjs [in] The sqlite object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_INITIALIZED Sqlite object is not initialized.
 */
u32 jf_sqlite_startTransaction(jf_sqlite_t * pjs);

/** Commit transaction.
 *
 *  @param pjs [in] The sqlite object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_INITIALIZED Sqlite object is not initialized.
 */
u32 jf_sqlite_commitTransaction(jf_sqlite_t * pjs);

/** Execute SQL statement.
 *
 *  @note
 *  -# The iteration of row data will stop if the return code of callback function is not
 *   JF_ERR_NO_ERROR.
 *  -# The callback function is used to retrieve data, so it's availble for reading data. It can be
 *   set to NULL if the SQL statement is not an reading operation.
 *  -# The callback function is called for each row data.
 *  -# To get column value of each row, use column access function in the callback function.
 *
 *  @param pjs [in] The sqlite object.
 *  @param pstrSql [in] The SQL statement.
 *  @param fnHandleRowData [out] The callback function to handle row data, it can be set to NULL.
 *  @param pArg [in] The argument for the callback function, it can be set to NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_INITIALIZED Sqlite object is not initialized.
 *  @retval JF_ERR_SQL_COMPILE_ERROR Error in compiling SQL statement.
 */
u32 jf_sqlite_execSql(
    jf_sqlite_t * pjs, olchar_t * pstrSql, jf_sqlite_fnHandleRowData_t fnHandleRowData, void * pArg);

/** Get the text string value of column in a row.
 *
 *  @note
 *  -# The column must be created with TEXT data type.
 *
 *  @param pStmt [in] The statement handle.
 *  @param nCol [in] The column id, start from 0.
 *
 *  @return The text string value.
 */
olchar_t * jf_sqlite_getColumnText(jf_sqlite_stmt_t * pStmt, olint_t nCol);

/** Get the integer value of column in a row.
 *
 *  @note
 *  -# The column must be created with INTEGER data type.
 *
 *  @param pStmt [in] The statement handle.
 *  @param nCol [in] The column id, start from 0.
 *
 *  @return The integer string value.
 */
olint_t jf_sqlite_getColumnInt(jf_sqlite_stmt_t * pStmt, olint_t nCol);

/** Get the double value of column in a row.
 *
 *  @note
 *  -# The column must be created with DOUBLE data type.
 *
 *  @param pStmt [in] The statement handle.
 *  @param nCol [in] The column id, start from 0.
 *
 *  @return The double string value.
 */
oldouble_t jf_sqlite_getColumnDouble(jf_sqlite_stmt_t * pStmt, olint_t nCol);

#endif /*JIUTAI_SQLITE_H*/

/*------------------------------------------------------------------------------------------------*/
