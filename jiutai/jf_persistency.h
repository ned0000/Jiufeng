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

typedef enum jf_persistency_type
{
	JF_PERSISTENCY_TYPE_SQLITE = 0, /**<Sqlite persistency.*/ 
} jf_persistency_type_t;

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

typedef union jf_persistency_config
{
    jf_persistency_config_sqlite_t jpc_pcsConfigSqlite;
} jf_persistency_config_t;

typedef void  jf_persistency_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_persistency_create(
    jf_persistency_type_t type, jf_persistency_config_t * ppc,
    jf_persistency_t ** ppPersist);

u32 jf_persistency_destroy(jf_persistency_t ** ppPersist);

u32 jf_persistency_getValue(
    jf_persistency_t * pPersist, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue);

u32 jf_persistency_setValue(
    jf_persistency_t * pPersist, olchar_t * pKey, olchar_t * pValue);

u32 jf_persistency_startTransaction(jf_persistency_t * pPersist);

u32 jf_persistency_commitTransaction(jf_persistency_t * pPersist);

u32 jf_persistency_rollbackTransaction(jf_persistency_t * pPersist);

#endif  /*JIUFENG_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/

