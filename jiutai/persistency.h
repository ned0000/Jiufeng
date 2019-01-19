/**
 *  @file persistency.h
 *
 *  @brief persistency library header file
 *
 *  @author Min Zhang
 *
 *  @note Link with sqlite3 library
 *  
 */
 
#ifndef JIUFENG_PERSISTENCY_H
#define JIUFENG_PERSISTENCY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef enum persistency_type
{
	SQLITE_PERSISTENCY = 0, /**< sqlite persistency */ 
} persistency_type_t;

typedef struct persistency_config_sqlite
{
#define MAX_PERSISTENCY_NAME_LEN    (64)
    /** The sqlite DB name */
    olchar_t pcs_strDbName[MAX_PERSISTENCY_NAME_LEN];
    /** The sqlite DB table name */
    olchar_t pcs_strTableName[MAX_PERSISTENCY_NAME_LEN];
    /** The name of the table column containing the keys */
    olchar_t pcs_strKeyColumnName[MAX_PERSISTENCY_NAME_LEN];
    /** The name of the table column containing the values */ 
    olchar_t pcs_strValueColumnName[MAX_PERSISTENCY_NAME_LEN];
} persistency_config_sqlite_t;

typedef union persistency_config
{
    persistency_config_sqlite_t pc_pcsConfigSqlite;
} persistency_config_t;

typedef void  persistency_t;

/* --- functional routines ------------------------------------------------- */

u32 createPersistency(
    persistency_type_t type, persistency_config_t * ppc,
    persistency_t ** ppPersist);

u32 destroyPersistency(persistency_t ** ppPersist);

u32 getPersistencyValue(
    persistency_t * pPersist, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue);

u32 setPersistencyValue(
    persistency_t * pPersist, olchar_t * pKey, olchar_t * pValue);

u32 startPersistencyTransaction(persistency_t * pPersist);

u32 commitPersistencyTransaction(persistency_t * pPersist);

u32 rollbackPersistencyTransaction(persistency_t * pPersist);

#endif  /*JIUFENG_PERSISTENCY_H*/

/*---------------------------------------------------------------------------*/

