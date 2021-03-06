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

typedef u32 (* fnFiniPersistency_t)(struct persistency_manager * pMananger);

typedef u32 (* fnGetPersistencyValue_t)(
    struct persistency_manager * pMananger, const olchar_t * pKey, olchar_t * pValue, olsize_t sValue);

typedef u32 (* fnSetPersistencyValue_t)(
    struct persistency_manager * pMananger, const olchar_t * pKey, const olchar_t * pValue);

typedef u32 (* fnStartPersistencyTransaction_t)(struct persistency_manager * pMananger);

typedef u32 (* fnCommitPersistencyTransaction_t)(struct persistency_manager * pMananger);

typedef u32 (* fnRollbackPersistencyTransaction_t)(struct persistency_manager * pMananger);

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
    jf_persistency_type_t pm_jptType; 

    fnFiniPersistency_t pm_fnFini;
    fnGetPersistencyValue_t pm_fnGetValue;
    fnSetPersistencyValue_t pm_fnSetValue;
    fnStartPersistencyTransaction_t pm_fnStartTransaction;
    fnCommitPersistencyTransaction_t pm_fnCommitTransaction;
    fnRollbackPersistencyTransaction_t pm_fnRollbackTransaction;
    fnTraversePersistency pm_fnTraverse;

    boolean_t pm_bTransactionStarted;
    u8 pm_u8Reserved[7];

    persistency_data_t * pm_ppdData;

    jf_mutex_t pm_jmLock;
} persistency_manager_t;

/* --- functional routines ---------------------------------------------------------------------- */

const olchar_t * getStringPersistencyType(u8 u8Type);

#endif /*PERSISTENCY_COMMON_H*/

/*------------------------------------------------------------------------------------------------*/
