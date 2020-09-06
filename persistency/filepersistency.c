/**
 *  @file filepersistency.c
 *
 *  @brief File persistency implementation file.
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
#include "jf_conffile.h"
#include "jf_hashtree.h"
#include "jf_string.h"

#include "persistencycommon.h"
#include "filepersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Default table name in file DB for key-value pair.
 */
#define FILE_PERSISTENCY_DEF_TABLE_NAME             "data"

/** Default key colume name in file DB for key.
 */
#define FILE_PERSISTENCY_DEF_KEY_COLUMN_NAME        "key"

/** Default value colume name in file DB for value.
 */
#define FILE_PERSISTENCY_DEF_VALUE_COLUMN_NAME      "value"

/** Define the file persistency data type.
 */
typedef struct file_persistency
{
    /**File instance.*/
    jf_conffile_t * fp_pjcFile;
    /**The file name.*/
    olchar_t fp_strFile[JF_LIMIT_MAX_PATH_LEN];

    jf_hashtree_t fp_jhKeyValue;

} file_persistency_t;

typedef struct
{
    olchar_t * fpkv_pstrKey;
    olchar_t * fpkv_pstrValue;

    /**The key is found and value is saved if TRUE.*/
    boolean_t fpkv_bSet;
    u8 fpkv_u8Reserved[7];
} file_persistency_key_value_t;

typedef struct
{
    jf_conffile_t * fpsv_pjcFile;
    jf_hashtree_t * fpsv_pjhKeyValue;
} file_persistency_set_value_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _freeFilePersistencyKeyValue(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_key_value_t * pfpkv = *ppData;

    if (pfpkv->fpkv_pstrKey != NULL)
        jf_string_free(&pfpkv->fpkv_pstrKey);

    if (pfpkv->fpkv_pstrValue != NULL)
        jf_string_free(&pfpkv->fpkv_pstrValue);

    jf_jiukun_freeMemory(ppData);

    return u32Ret;
}

static u32 _newFilePersistencyKeyValue(
    const olchar_t * pKey, const olchar_t * pValue, file_persistency_key_value_t ** ppKeyValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_key_value_t * pfpkv = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pfpkv, sizeof(*pfpkv));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pfpkv, sizeof(*pfpkv));

        u32Ret = jf_string_duplicate(&pfpkv->fpkv_pstrKey, pKey);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_duplicate(&pfpkv->fpkv_pstrValue, pValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppKeyValue = pfpkv;
    else if (pfpkv != NULL)
        _freeFilePersistencyKeyValue((void **)&pfpkv);

    return u32Ret;
}

static u32 _rollbackTransactionOfPersistencyFile(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;

    jf_hashtree_finiHashtreeAndData(&pfp->fp_jhKeyValue, _freeFilePersistencyKeyValue);

    return u32Ret;
}

static u32 _initFile(file_persistency_t * pfp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_open_param_t jcop;

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = pfp->fp_strFile;
    
    u32Ret = jf_conffile_open(&jcop, &pfp->fp_pjcFile);

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _addKeyValueToHashtree(
    file_persistency_t * pfp, const olchar_t * pKey, const olchar_t * pValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_key_value_t * pfpkv = NULL;

    JF_LOGGER_DEBUG("key: %s, value: %s", pKey, pValue);

    u32Ret = _newFilePersistencyKeyValue(pKey, pValue, &pfpkv);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_hashtree_addEntry(
            &pfp->fp_jhKeyValue, (olchar_t *)pKey, ol_strlen(pKey), pfpkv);

    if ((u32Ret != JF_ERR_NO_ERROR) && (pfpkv != NULL))
        _freeFilePersistencyKeyValue((void **)&pfpkv);

    return u32Ret;
}

static u32 _getValueFromPersistencyFile(
    persistency_manager_t * ppm, const olchar_t * pKey, olchar_t * pValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;
    file_persistency_key_value_t * pfpkv = NULL;

    if (ppm->pm_bTransactionStarted)
        u32Ret = jf_hashtree_getEntry(
            &pfp->fp_jhKeyValue, (olchar_t *)pKey, ol_strlen(pKey), (void **)&pfpkv);

    if (pfpkv == NULL)
    {
        u32Ret = jf_conffile_get(pfp->fp_pjcFile, pKey, NULL, pValue, sValue);

        if (u32Ret == JF_ERR_NOT_FOUND)
            u32Ret = JF_ERR_NO_ERROR;
    }
    else
    {
        ol_snprintf(pValue, sValue, "%s", pfpkv->fpkv_pstrValue);
        pValue[sValue - 1] = '\0';
    }

    return u32Ret;
}

static u32 _setValueToPersistencyFile(
    persistency_manager_t * ppm, const olchar_t * pKey, const olchar_t * pValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;

    if (ppm->pm_bTransactionStarted)
        u32Ret = _addKeyValueToHashtree(pfp, pKey, pValue);
    else
        u32Ret = jf_conffile_set(pfp->fp_pjcFile, pKey, pValue);

    return u32Ret;
}

static u32 _startTransactionOfPersistencyFile(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;

    assert(! ppm->pm_bTransactionStarted);
    assert(jf_hashtree_isEmpty(&pfp->fp_jhKeyValue));

    jf_hashtree_init(&pfp->fp_jhKeyValue);

    return u32Ret;
}

static u32 _writeOneKeyValueToConfFile(
    jf_conffile_t * pjc, const olchar_t * pstrKey, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_conffile_write(pjc, pstrKey, pstrValue);

    return u32Ret;
}

static u32 _fnSaveKeyValueToTempFile(olchar_t * pstrKey, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_set_value_t * pfpsv = pArg;
    const olchar_t * value = pstrValue;
    file_persistency_key_value_t * pfpkv = NULL;

    u32Ret = jf_hashtree_getEntry(
        pfpsv->fpsv_pjhKeyValue, pstrKey, ol_strlen(pstrKey), (void **)&pfpkv);

    /*Use the new value If the key is found.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        value = pfpkv->fpkv_pstrValue;
        pfpkv->fpkv_bSet = TRUE;
    }

    u32Ret = _writeOneKeyValueToConfFile(pfpsv->fpsv_pjcFile, pstrKey, value);

    return u32Ret;
}

static u32 _appendKeyValueToTempFile(jf_conffile_t * pjc, jf_hashtree_t * pjhKeyValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hashtree_enumerator_t jhe;
    file_persistency_key_value_t * pfpkv = NULL;

    jf_hashtree_initEnumerator(pjhKeyValue, &jhe);

    while ((! jf_hashtree_isEndOfEnumerator(&jhe)) && (u32Ret == JF_ERR_NO_ERROR))
    {
        jf_hashtree_getEnumeratorNodeData(&jhe, NULL, NULL, (void **)&pfpkv);

        if (! pfpkv->fpkv_bSet)
            u32Ret = _writeOneKeyValueToConfFile(pjc, pfpkv->fpkv_pstrKey, pfpkv->fpkv_pstrValue);

        jf_hashtree_moveEnumerator(&jhe);
    }

    jf_hashtree_finiEnumerator(&jhe);

    return u32Ret;
}

static u32 _writeKeyValueToFilePersistency(file_persistency_t * pfp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strTempFile[JF_LIMIT_MAX_PATH_LEN];
    file_persistency_set_value_t fpsv;
    jf_conffile_open_param_t jcop;

    ol_bzero(&fpsv, sizeof(fpsv));
    fpsv.fpsv_pjhKeyValue = &pfp->fp_jhKeyValue;

    ol_snprintf(strTempFile, sizeof(strTempFile), "%s.temp", pfp->fp_strFile);

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = strTempFile;

    /*Open a temperory file to save all key-value pairs.*/
    u32Ret = jf_conffile_open(&jcop, &fpsv.fpsv_pjcFile);

    /*Traverse the source config file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_conffile_traverse(pfp->fp_pjcFile, _fnSaveKeyValueToTempFile, &fpsv);

    /*Iterate the hashtree to save those keys not found, append the key-value to temp file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _appendKeyValueToTempFile(fpsv.fpsv_pjcFile, &pfp->fp_jhKeyValue);

    if (fpsv.fpsv_pjcFile != NULL)
        jf_conffile_close(&fpsv.fpsv_pjcFile);

    /*Remove the original config file and rename the new one.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Close the original config file.*/
        jf_conffile_close(&pfp->fp_pjcFile);

        /*Remove the original config file.*/
        jf_file_remove(pfp->fp_strFile);

        /*Rename the temporary file.*/
        jf_file_rename(strTempFile, pfp->fp_strFile);

        /*Reopen the config file.*/
        ol_bzero(&jcop, sizeof(jcop));
        jcop.jcop_pstrFile = pfp->fp_strFile;

        u32Ret = jf_conffile_open(&jcop, &pfp->fp_pjcFile);
    }

    return u32Ret;
}

static u32 _commitTransactionOfPersistencyFile(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;

    u32Ret = _writeKeyValueToFilePersistency(pfp);

    /*Free all key-value pairs.*/
    jf_hashtree_finiHashtreeAndData(&pfp->fp_jhKeyValue, _freeFilePersistencyKeyValue);

    return u32Ret;
}

static u32 _destroyFilePersistency(file_persistency_t ** ppFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = *ppFile;

    if (pfp->fp_pjcFile != NULL)
        jf_conffile_close(&pfp->fp_pjcFile);

    jf_jiukun_freeMemory((void **)ppFile);

    return u32Ret;
}

static u32 _createFilePersistency(
    file_persistency_t ** ppFile, jf_persistency_config_file_t * pConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pfp, sizeof(*pfp));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pfp, sizeof(*pfp));
        ol_memcpy(pfp->fp_strFile, pConfig->jpcf_strFile, sizeof(pfp->fp_strFile));
        jf_hashtree_init(&pfp->fp_jhKeyValue);

        u32Ret = _initFile(pfp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppFile = pfp;
    else if (pfp != NULL)
        _destroyFilePersistency(&pfp);

    return u32Ret;
}

static u32 _finiPersistencyFile(persistency_manager_t * ppm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(ppm->pm_ppdData != NULL);

    u32Ret = _destroyFilePersistency((file_persistency_t **)&ppm->pm_ppdData);

    return u32Ret;
}

static u32 _traversePersistencyFile(
    persistency_manager_t * ppm, jf_persistency_fnHandleKeyValue_t fnHandleKeyValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = ppm->pm_ppdData;

    u32Ret = jf_conffile_traverse(pfp->fp_pjcFile, fnHandleKeyValue, pArg);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initFilePersistency(
    persistency_manager_t * pManager, jf_persistency_config_file_t * pConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    file_persistency_t * pfp = NULL;

    JF_LOGGER_INFO("file: %s", pConfig->jpcf_strFile);

    u32Ret = _createFilePersistency(&pfp, pConfig);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pManager->pm_fnFini = _finiPersistencyFile;
        pManager->pm_fnGetValue = _getValueFromPersistencyFile;
        pManager->pm_fnSetValue = _setValueToPersistencyFile;
        pManager->pm_fnStartTransaction = _startTransactionOfPersistencyFile;
        pManager->pm_fnCommitTransaction = _commitTransactionOfPersistencyFile;
        pManager->pm_fnRollbackTransaction = _rollbackTransactionOfPersistencyFile;
        pManager->pm_fnTraverse = _traversePersistencyFile;

        pManager->pm_ppdData = pfp;
    }
    
    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
