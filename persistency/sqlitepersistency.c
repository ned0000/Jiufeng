/**
 *  @file sqlitepersistency.c
 *
 *  @brief Sqlite persistency implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_persistency.h"
#include "jf_jiukun.h"
#include "jf_sqlite.h"

#include "persistencycommon.h"
#include "sqlitepersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Default table name in sqlite DB for key-value pair.
 */
#define SQLITE_PERSISTENCY_DEF_TABLE_NAME             "data"

/** Default key colume name in sqlite DB for key.
 */
#define SQLITE_PERSISTENCY_DEF_KEY_COLUMN_NAME        "key"

/** Default value colume name in sqlite DB for value.
 */
#define SQLITE_PERSISTENCY_DEF_VALUE_COLUMN_NAME      "value"

/** Maximum name length in sqlite persistency.
 */
#define SQLITE_PERSISTENCY_MAX_NAME_LEN               (64)

/** Define the sqlite persistency data type.
 */
typedef struct sqlite_persistency
{
    /**Sqlite instance.*/
    jf_sqlite_t spd_jsSqlite;
    /**The sqlite DB file.*/
    olchar_t spd_strSqliteDb[JF_LIMIT_MAX_PATH_LEN];
    /**The sqlite DB table name.*/
    olchar_t spd_strTableName[SQLITE_PERSISTENCY_MAX_NAME_LEN];
    /**The name of the table column containing the keys.*/
    olchar_t spd_strKeyColumnName[SQLITE_PERSISTENCY_MAX_NAME_LEN];
    /**The name of the table column containing the values.*/
    olchar_t spd_strValueColumnName[SQLITE_PERSISTENCY_MAX_NAME_LEN];

} sqlite_persistency_data_t;

/** Sqlite persistency get value data type.
 */
typedef struct
{
    olchar_t * spgv_pstrValue;
    olsize_t spgv_sValue;
} sqlite_persistency_get_value_t;

/** Sqlite persistency traverse data type.
 */
typedef struct
{
    jf_persistency_fnHandleKeyValue_t spdt_fnHandleKeyValue;
    void * spdt_pArg;
} sqlite_persistency_data_traverse_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _rollbackTransactionOfSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;

    u32Ret = jf_sqlite_rollbackTransaction(&pspd->spd_jsSqlite);

    return u32Ret;
}

static u32 _initSqlite(sqlite_persistency_data_t * pspd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_sqlite_init_param_t jsip;
    olchar_t strSql[256];

    ol_bzero(&jsip, sizeof(jf_sqlite_init_param_t));
    jsip.jsip_pstrDbName = pspd->spd_strSqliteDb;
    
    u32Ret = jf_sqlite_init(&pspd->spd_jsSqlite, &jsip);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_sprintf(
            strSql, "CREATE TABLE IF NOT EXISTS %s(%s TEXT PRIMARY KEY, %s TEXT);",
            pspd->spd_strTableName, pspd->spd_strKeyColumnName, pspd->spd_strValueColumnName);

        u32Ret = jf_sqlite_execSql(&pspd->spd_jsSqlite, strSql, NULL, NULL);
    }

    return u32Ret;
}

static u32 _fnHandleSqliteValue(jf_sqlite_stmt_t * pStmt, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_get_value_t * pspgv = pArg;

    ol_strncpy(pspgv->spgv_pstrValue, jf_sqlite_getColumnText(pStmt, 0), pspgv->spgv_sValue - 1);
    pspgv->spgv_pstrValue[pspgv->spgv_sValue - 1] = '\0';

    return u32Ret;
}

static u32 _getValueFromSqlite(
    persistency_manager_t * ppm, const olchar_t * pKey, olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;
    olchar_t strSql[512];
    sqlite_persistency_get_value_t spgv;

    ol_snprintf(
        strSql, sizeof(strSql), "SELECT %s FROM %s WHERE %s='%s';",
        pspd->spd_strValueColumnName, pspd->spd_strTableName, pspd->spd_strKeyColumnName, pKey);

    ol_bzero(&spgv, sizeof(spgv));
    spgv.spgv_pstrValue = pValue;
    spgv.spgv_sValue = sValue;

    u32Ret = jf_sqlite_execSql(&pspd->spd_jsSqlite, strSql, _fnHandleSqliteValue, &spgv);

    return u32Ret;
}

static u32 _setValueToSqlite(
    persistency_manager_t * ppm, const olchar_t * pKey, const olchar_t * pValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;
    olchar_t * pstrSql = NULL;
    olsize_t nsize = ol_strlen(pValue) + ol_strlen(pKey) + 256;

    u32Ret = jf_jiukun_allocMemory((void **)&pstrSql, nsize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*update or insert the value into the DB*/
        ol_snprintf(
            pstrSql, nsize, "REPLACE INTO %s(%s, %s) VALUES ('%s', '%s');",
            pspd->spd_strTableName, pspd->spd_strKeyColumnName, pspd->spd_strValueColumnName, pKey,
            pValue);

        u32Ret = jf_sqlite_execSql(&pspd->spd_jsSqlite, pstrSql, NULL, NULL);
    }
    
    if (pstrSql != NULL)
        jf_jiukun_freeMemory((void **)&pstrSql);

    return u32Ret;
}

static u32 _startTransactionOfSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;

    u32Ret = jf_sqlite_startTransaction(&pspd->spd_jsSqlite);

    return u32Ret;
}

static u32 _commitTransactionOfSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;

    u32Ret = jf_sqlite_commitTransaction(&pspd->spd_jsSqlite);

    return u32Ret;
}

static u32 _destroySqlitePersistency(sqlite_persistency_data_t ** ppSqlite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = *ppSqlite;

    u32Ret = jf_sqlite_fini(&pspd->spd_jsSqlite);

    jf_jiukun_freeMemory((void **)ppSqlite);

    return u32Ret;
}

static u32 _createSqlitePersistency(
    sqlite_persistency_data_t ** ppSqlite, jf_persistency_config_sqlite_t * pConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pspd, sizeof(*pspd));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pspd, sizeof(*pspd));
        ol_memcpy(pspd->spd_strSqliteDb, pConfig->jpcs_strSqliteDb, sizeof(pspd->spd_strSqliteDb));
        ol_strcpy(pspd->spd_strTableName, SQLITE_PERSISTENCY_DEF_TABLE_NAME);
        ol_strcpy(pspd->spd_strKeyColumnName, SQLITE_PERSISTENCY_DEF_KEY_COLUMN_NAME);
        ol_strcpy(pspd->spd_strValueColumnName, SQLITE_PERSISTENCY_DEF_VALUE_COLUMN_NAME);

        u32Ret = _initSqlite(pspd);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppSqlite = pspd;
    else if (pspd != NULL)
        _destroySqlitePersistency(&pspd);

    return u32Ret;
}

static u32 _finiSqlite(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(ppm->pm_ppdData != NULL);

    u32Ret = _destroySqlitePersistency((sqlite_persistency_data_t **)&ppm->pm_ppdData);

    return u32Ret;
}

static u32 _fnHandleSqliteTraversal(jf_sqlite_stmt_t * pStmt, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_traverse_t * pspdt = pArg;
    olchar_t * pstrKey = NULL, * pstrValue = NULL;

    pstrKey = jf_sqlite_getColumnText(pStmt, 0);
    pstrValue = jf_sqlite_getColumnText(pStmt, 1);

    u32Ret = pspdt->spdt_fnHandleKeyValue(pstrKey, pstrValue, pspdt->spdt_pArg);

    return u32Ret;
}

static u32 _traverseSqlite(
    persistency_manager_t * ppm, jf_persistency_fnHandleKeyValue_t fnHandleKeyValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = ppm->pm_ppdData;
    olchar_t strSql[512];
    sqlite_persistency_data_traverse_t spdt;

    ol_snprintf(strSql, sizeof(strSql), "SELECT * FROM %s;", pspd->spd_strTableName);

    ol_bzero(&spdt, sizeof(spdt));
    spdt.spdt_fnHandleKeyValue = fnHandleKeyValue;
    spdt.spdt_pArg = pArg;

    u32Ret = jf_sqlite_execSql(&pspd->spd_jsSqlite, strSql, _fnHandleSqliteTraversal, &spdt);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initSqlitePersistency(
    persistency_manager_t * pManager, jf_persistency_config_sqlite_t * pConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    sqlite_persistency_data_t * pspd = NULL;

    JF_LOGGER_INFO("sqlite db: %s", pConfig->jpcs_strSqliteDb);

    u32Ret = _createSqlitePersistency(&pspd, pConfig);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pManager->pm_fnFini = _finiSqlite;
        pManager->pm_fnGetValue = _getValueFromSqlite;
        pManager->pm_fnSetValue = _setValueToSqlite;
        pManager->pm_fnStartTransaction = _startTransactionOfSqlite;
        pManager->pm_fnCommitTransaction = _commitTransactionOfSqlite;
        pManager->pm_fnRollbackTransaction = _rollbackTransactionOfSqlite;
        pManager->pm_fnTraverse = _traverseSqlite;

        pManager->pm_ppdData = pspd;
    }
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
