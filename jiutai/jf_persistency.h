/**
 *  @file jf_persistency.h
 *
 *  @brief Header file defines interfaces of persistency library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_persistency library.
 *  -# If fails to set persistency value, user should rollback transaction if transaction started.
 *  -# Link with sqlite3 library as sqlite3 is used as backend database.
 */
 
#ifndef JIUFENG_PERSISTENCY_H
#define JIUFENG_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the persistency type data type.
 */
typedef enum jf_persistency_type
{
    /**Sqlite persistency.*/ 
	JF_PERSISTENCY_TYPE_SQLITE = 0,
} jf_persistency_type_t;

/** Define the persistency config data type for sqlite backend.
 */
typedef struct jf_persistency_config_sqlite
{
#define JF_PERSISTENCY_MAX_NAME_LEN    (64)
    /**The sqlite DB name.*/
    olchar_t jpcs_strDbName[JF_PERSISTENCY_MAX_NAME_LEN];
    /**The sqlite DB table name.*/
    olchar_t jpcs_strTableName[JF_PERSISTENCY_MAX_NAME_LEN];
    /**The name of the table column containing the keys.*/
    olchar_t jpcs_strKeyColumnName[JF_PERSISTENCY_MAX_NAME_LEN];
    /**The name of the table column containing the values.*/ 
    olchar_t jpcs_strValueColumnName[JF_PERSISTENCY_MAX_NAME_LEN];
} jf_persistency_config_sqlite_t;

/** Define the persistency config data type
 */
typedef union jf_persistency_config
{
    /**The sqlite backend.*/
    jf_persistency_config_sqlite_t jpc_pcsConfigSqlite;
} jf_persistency_config_t;

/** Define the persistency data type
 */
typedef void  jf_persistency_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create persistency.
 *
 *  @param type [in] The persistency type.
 *  @param pjpc [in] The configuration for creating the persistency.
 *  @param ppPersist [out] The persistency handle.
 *
 *  @return The error code.
 */
u32 jf_persistency_create(
    jf_persistency_type_t type, jf_persistency_config_t * pjpc, jf_persistency_t ** ppPersist);

/** Destroy persistency.
 *
 *  @param ppPersist [in/out] The persistency handle.
 *
 *  @return The error code.
 */
u32 jf_persistency_destroy(jf_persistency_t ** ppPersist);

/** Get value of the key from persistency.
 *
 *  @param pPersist [in] The persistency handle.
 *  @param pKey [in] The key in string with NULL terminated.
 *  @param pValue [out] The buffer for the value.
 *  @param sValue [in] The size of the buffer.
 *
 *  @return The error code.
 */
u32 jf_persistency_getValue(
    jf_persistency_t * pPersist, const olchar_t * pKey, olchar_t * pValue, olsize_t sValue);

/** Set value of the key from persistency.
 *
 *  @note
 *  -# If the key is not existing, the key with value is created.
 *  -# If the key is existing, old key is updated with new value.
 *
 *  @param pPersist [in] The persistency handle.
 *  @param pKey [in] The key in string with NULL terminated.
 *  @param pValue [in] The value in string with NULL terminated.
 *
 *  @return The error code.
 */
u32 jf_persistency_setValue(
    jf_persistency_t * pPersist, const olchar_t * pKey, const olchar_t * pValue);

/** Start transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 */
u32 jf_persistency_startTransaction(jf_persistency_t * pPersist);

/** Commit transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 */
u32 jf_persistency_commitTransaction(jf_persistency_t * pPersist);

/** Rollback transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 */
u32 jf_persistency_rollbackTransaction(jf_persistency_t * pPersist);

#endif  /*JIUFENG_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/

