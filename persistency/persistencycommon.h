/**
 *  @file persistencycommon.h
 *
 *  @brief common data structure for persistency library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef PERSISTENCY_COMMON_H
#define PERSISTENCY_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "persistency.h"
#include "sqlite3.h"
#include "syncmutex.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
struct persistency_manager;

typedef u32 (* fnFiniPersistency_t)(struct persistency_manager * pMananger);
typedef u32 (* fnGetPersistencyValue_t)(
    struct persistency_manager * pMananger, olchar_t * pKey,
    olchar_t * pValue, olsize_t sValue);
typedef u32 (* fnSetPersistencyValue_t)(
    struct persistency_manager * pMananger, olchar_t * pKey, olchar_t * pValue);
typedef u32 (* fnStartPersistencyTransaction_t)(struct persistency_manager * pMananger);
typedef u32 (* fnCommitPersistencyTransaction_t)(struct persistency_manager * pMananger);
typedef u32 (* fnRollbackPersistencyTransaction_t)(struct persistency_manager * pMananger);


typedef struct sqlite_persistency
{
    sqlite3 * sp_psSqlite;
    persistency_config_sqlite_t sp_pcsConfigSqlite;
} sqlite_persistency_t;

typedef union persistency_data
{
    sqlite_persistency_t pd_spSqlite;
} persistency_data_t;

typedef struct persistency_manager 
{
    persistency_type_t pm_ptType; 

    fnFiniPersistency_t pm_fnFini;
    fnGetPersistencyValue_t pm_fnGetValue;
    fnSetPersistencyValue_t pm_fnSetValue;
    fnStartPersistencyTransaction_t pm_fnStartTransaction;
    fnCommitPersistencyTransaction_t pm_fnCommitTransaction;
    fnRollbackPersistencyTransaction_t pm_fnRollbackTransaction;

    boolean_t pm_bTransactionStarted;
    boolean_t pm_bInitialized;
    u8 pm_u8Reserved[6];

    persistency_data_t pm_pdData;

    sync_mutex_t pm_smLock;
} persistency_manager_t;

/* --- functional routines ------------------------------------------------- */


#endif /*PERSISTENCY_COMMON_H*/

/*---------------------------------------------------------------------------*/


