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
 *  -# Key-value pairs are supported for persistency.
 */
 
#ifndef JIUFENG_PERSISTENCY_H
#define JIUFENG_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"

#undef PERSISTENCYAPI
#undef PERSISTENCYCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_PERSISTENCY_DLL)
        #define PERSISTENCYAPI  __declspec(dllexport)
        #define PERSISTENCYCALL
    #else
        #define PERSISTENCYAPI
        #define PERSISTENCYCALL __cdecl
    #endif
#else
    #define PERSISTENCYAPI
    #define PERSISTENCYCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the persistency type data type.
 */
typedef enum jf_persistency_type
{
    /**Unknown persistency type.*/
    JF_PERSISTENCY_TYPE_UNKNOWN = 0,
    /**Sqlite persistency.*/ 
	JF_PERSISTENCY_TYPE_SQLITE,
    /**File persistency.*/
	JF_PERSISTENCY_TYPE_FILE,
    /**Maximum persistency type.*/
    JF_PERSISTENCY_TYPE_MAX,
} jf_persistency_type_t;

/** Define the persistency config data type for sqlite backend.
 */
typedef struct jf_persistency_config_sqlite
{
    /**The sqlite DB file.*/
    olchar_t jpcs_strSqliteDb[JF_LIMIT_MAX_PATH_LEN];
} jf_persistency_config_sqlite_t;

/** Define the persistency config data type for file backend.
 */
typedef struct jf_persistency_config_file
{
    /**The persistency file.*/
    olchar_t jpcf_strFile[JF_LIMIT_MAX_PATH_LEN];
} jf_persistency_config_file_t;

/** Define the persistency config data type
 */
typedef union jf_persistency_config
{
    /**The sqlite backend.*/
    jf_persistency_config_sqlite_t jpc_jpcsConfigSqlite;
    /**The file backend.*/
    jf_persistency_config_file_t jpc_jpcfConfigFile;
} jf_persistency_config_t;

/** Define the persistency data type
 */
typedef void  jf_persistency_t;

/** Define the function data type which is used to handle key-value pair. The function is used in
 *  traversal.
 *
 *  @note
 *  -# The traversal will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @param pstrKey [in] The key.
 *  @param pstrValue [in] The value.
 *  @param pArg [in] The argument for the callback function. It's passed by
 *   jf_persistency_traverse().
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_persistency_fnHandleKeyValue_t)(
    olchar_t * pstrKey, olchar_t * pstrValue, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create persistency.
 *
 *  @param type [in] The persistency type.
 *  @param pjpc [in] The configuration for creating the persistency.
 *  @param ppPersist [out] The persistency handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_create(
    jf_persistency_type_t type, jf_persistency_config_t * pjpc, jf_persistency_t ** ppPersist);

/** Destroy persistency.
 *
 *  @param ppPersist [in/out] The persistency handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_destroy(jf_persistency_t ** ppPersist);

/** Get value of the key from persistency.
 *
 *  @note
 *  -# If the key is not existing, the value is set to empty and no error is returned.
 *
 *  @param pPersist [in] The persistency handle.
 *  @param pKey [in] The key in string with NULL terminated.
 *  @param pValue [out] The buffer for the value.
 *  @param sValue [in] The size of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_getValue(
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_setValue(
    jf_persistency_t * pPersist, const olchar_t * pKey, const olchar_t * pValue);

/** Start transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_startTransaction(jf_persistency_t * pPersist);

/** Commit transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_commitTransaction(jf_persistency_t * pPersist);

/** Rollback transaction.
 *
 *  @param pPersist [in] The persistency handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_rollbackTransaction(jf_persistency_t * pPersist);

/** Traverse persistency to get all key-value pairs.
 *
 *  @note
 *  -# The traversal is based on key-value pair. 
 *  -# The traversal will stop if return code of callback function is not JF_ERR_NO_ERROR.
 *
 *  @param pPersist [in] The persistency handle.
 *  @param fnHandleKeyValue [in] The callback function for each key-value pair.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
PERSISTENCYAPI u32 PERSISTENCYCALL jf_persistency_traverse(
    jf_persistency_t * pPersist, jf_persistency_fnHandleKeyValue_t fnHandleKeyValue, void * pArg);

#endif  /*JIUFENG_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/
