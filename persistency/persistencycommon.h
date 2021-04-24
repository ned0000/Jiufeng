/**
 *  @file persistencycommon.h
 *
 *  @brief Common data structure for persistency library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef PERSISTENCY_COMMON_H
#define PERSISTENCY_COMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_persistency.h"
#include "jf_mutex.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Declaration of the persistency manager data type.
 */
struct persistency_manager;

/** Define the function data type to finalize persistency.
 */
typedef u32 (* fnFiniPersistency_t)(struct persistency_manager * pMananger);

/** Define the function data type to get value from persistency.
 */
typedef u32 (* fnGetPersistencyValue_t)(
    struct persistency_manager * pMananger, const olchar_t * pKey, olchar_t * pValue, olsize_t sValue);

/** Define the function data type to set value into persistency.
 */
typedef u32 (* fnSetPersistencyValue_t)(
    struct persistency_manager * pMananger, const olchar_t * pKey, const olchar_t * pValue);

/** Define the function data type to start transaction.
 */
typedef u32 (* fnStartPersistencyTransaction_t)(struct persistency_manager * pMananger);

/** Define the function data type to commit transaction.
 */
typedef u32 (* fnCommitPersistencyTransaction_t)(struct persistency_manager * pMananger);

/** Define the function data type to rollback transaction.
 */
typedef u32 (* fnRollbackPersistencyTransaction_t)(struct persistency_manager * pMananger);

/** Define the function data type to traverse persistency.
 */
typedef u32 (* fnTraversePersistency)(
    struct persistency_manager * pMananger, jf_persistency_fnHandleKeyValue_t fnHandleKeyValue,
    void * pArg);

/** Define the persistency data data type.
 */
typedef void  persistency_data_t;

/** Define the persistency manager data type.
 */
typedef struct persistency_manager 
{
    /**Persistency type.*/
    jf_persistency_type_t pm_jptType; 

    /**Function to Finalize persistency.*/
    fnFiniPersistency_t pm_fnFini;
    /**Function to get value from persistency.*/
    fnGetPersistencyValue_t pm_fnGetValue;
    /**Function to set value into persistency.*/
    fnSetPersistencyValue_t pm_fnSetValue;
    /**Function to start transaction.*/
    fnStartPersistencyTransaction_t pm_fnStartTransaction;
    /**Function to commit transaction.*/
    fnCommitPersistencyTransaction_t pm_fnCommitTransaction;
    /**Function to rollback transaction.*/
    fnRollbackPersistencyTransaction_t pm_fnRollbackTransaction;
    /**Function to traverse persistency.*/
    fnTraversePersistency pm_fnTraverse;

    /**Transaction is started if it's TRUE.*/
    boolean_t pm_bTransactionStarted;
    u8 pm_u8Reserved[7];

    /**Persistency data.*/
    persistency_data_t * pm_ppdData;

    /**Lock for get/set value.*/
    jf_mutex_t pm_jmLock;
} persistency_manager_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Get string of persistency type.
 *
 *  @param u8Type [in] The persistency type.
 *
 *  @return The string of persistency type.
 */
const olchar_t * getStringPersistencyType(u8 u8Type);

#endif /*PERSISTENCY_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/
