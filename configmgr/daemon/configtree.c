/**
 *  @file configtree.c
 *
 *  @brief The config tree implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_jiukun.h"
#include "jf_mutex.h"
#include "jf_ptree.h"
#include "jf_rand.h"
#include "jf_time.h"

#include "configmgrcommon.h"
#include "configtree.h"
#include "configpersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 ictt_u32TransactionId;
    jf_ptree_t * ictt_pjpConfig;
    u64 ictt_u64StartTime;
} internal_config_tree_transaction_t;

typedef struct
{
    boolean_t ict_bInitialized;
    u8 ict_u8Reserved[7];

    internal_config_mgr_setting_t * ict_picmsSetting;

    olint_t ict_nTransactionTimeOut;
    olint_t ict_nReserved;

    /**Lock for the critical data.*/
    jf_mutex_t ict_jmLock;
    /**Property tree for all config.*/
    jf_ptree_t * ict_pjpConfig;
    /**Config is changed if TRUE.*/
    boolean_t ict_bConfigChanged;
    u8 ict_u8Reserved2[7];

    u16 ict_u16Reserved[3];
    u16 ict_u16NumOfTransaction;
    internal_config_tree_transaction_t * ict_picttTransaction;

} internal_config_tree_t;

static internal_config_tree_t ls_ictConfigTree;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initConfigTree(internal_config_tree_t * pict, config_tree_init_param_t * pctip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sMem = 0;

    ol_bzero(pict, sizeof(*pict));
    pict->ict_picmsSetting = pctip->ctip_picmsSetting;
    pict->ict_nTransactionTimeOut = JF_CONFIG_DEFAULT_TRANSACTION_TIME_OUT;

    assert(pict->ict_picmsSetting->icms_u16MaxNumOfTransaction > 0);
    sMem = sizeof(internal_config_tree_transaction_t) * pict->ict_picmsSetting->icms_u16MaxNumOfTransaction;

    u32Ret = jf_jiukun_allocMemory((void **)&pict->ict_picttTransaction, sMem);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pict->ict_picttTransaction, sMem);

    }

    /*Initialize the mutex lock.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pict->ict_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_create(&pict->ict_pjpConfig);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = loadConfigFromPersistency(
            pict->ict_picmsSetting->icms_u8ConfigPersistencyType,
            pict->ict_picmsSetting->icms_pstrConfigPersistencyLocation, pict->ict_pjpConfig);

    return u32Ret;
}

static u32 _initConfigTreeTransaction(internal_config_tree_transaction_t * pictt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pictt->ictt_u32TransactionId = jf_rand_getU32();

    JF_LOGGER_DEBUG("id: %u", pictt->ictt_u32TransactionId);

    u32Ret = jf_ptree_create(&pictt->ictt_pjpConfig);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_time_getMonotonicRawTimeInSecond(&pictt->ictt_u64StartTime);

    /*Clear the transaction ID if there is error.*/
    if (u32Ret != JF_ERR_NO_ERROR)
        pictt->ictt_u32TransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;

    return u32Ret;
}

static u32 _finiConfigTreeTransaction(internal_config_tree_transaction_t * pictt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_DEBUG("id: %u", pictt->ictt_u32TransactionId);

    if (pictt->ictt_pjpConfig != NULL)
        u32Ret = jf_ptree_destroy(&pictt->ictt_pjpConfig);

    pictt->ictt_u32TransactionId = JF_CONFIG_INVALID_TRANSACTION_ID;
    pictt->ictt_u64StartTime = 0;

    return u32Ret;
}

static boolean_t _isExpiredConfigTreeTransaction(
    internal_config_tree_t * pict, internal_config_tree_transaction_t * pictt)
{
    boolean_t bRet = FALSE;
    u32 u32Ret = JF_ERR_NOT_FOUND;
    u64 u64CurTime = 0;

    u32Ret = jf_time_getMonotonicRawTimeInSecond(&u64CurTime);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pictt->ictt_u64StartTime + pict->ict_nTransactionTimeOut < u64CurTime)
            bRet = TRUE;
    }

    return bRet;
}

static u32 _findConfigTreeTransaction(
    internal_config_tree_t * pict, u32 u32TransactionId,
    internal_config_tree_transaction_t ** ppTransaction)
{
    u32 u32Ret = JF_ERR_TRANSACTION_NOT_FOUND;
    u16 index = 0;
    internal_config_tree_transaction_t * pictt = NULL;

    for (index = 0; index < pict->ict_picmsSetting->icms_u16MaxNumOfTransaction; index ++)
    {
        pictt = &pict->ict_picttTransaction[index];

        if ((pictt->ictt_u32TransactionId != JF_CONFIG_INVALID_TRANSACTION_ID) &&
            _isExpiredConfigTreeTransaction(pict, pictt))
        {
            JF_LOGGER_DEBUG("expired transaction, id: %u", pictt->ictt_u32TransactionId);
            _finiConfigTreeTransaction(pictt);
        }

        if (pictt->ictt_u32TransactionId == u32TransactionId)
        {
            *ppTransaction = pictt;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }
    }

    return u32Ret;
}

static u32 _getConfigTreeTransaction(
    internal_config_tree_t * pict, internal_config_tree_transaction_t ** ppTransaction)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_transaction_t * pictt = NULL;

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = _findConfigTreeTransaction(pict, JF_CONFIG_INVALID_TRANSACTION_ID, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        assert(pictt->ictt_pjpConfig == NULL);

        u32Ret = _initConfigTreeTransaction(pictt);
    }
    else
    {
        u32Ret = JF_ERR_REACH_MAX_TRANSACTION;
    }

    jf_mutex_release(&pict->ict_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppTransaction = pictt;

    return u32Ret;
}

static u32 _commitConfigTreeTransaction(
    internal_config_tree_t * pict, internal_config_tree_transaction_t * pictt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_ptree_dump(pictt->ictt_pjpConfig);
    jf_ptree_dump(pict->ict_pjpConfig);

    u32Ret = jf_ptree_merge(pict->ict_pjpConfig, pictt->ictt_pjpConfig);

    jf_ptree_dump(pict->ict_pjpConfig);

    return u32Ret;
}

static u32 _setConfigInTransaction(
    internal_config_tree_t * pict, u32 u32TransactionId, olchar_t * pstrName, olsize_t sName,
    const olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_transaction_t * pictt = NULL;
    jf_ptree_node_t * node = NULL;

    jf_mutex_acquire(&pict->ict_jmLock);

    /*Make sure the config is in the config tree.*/
    u32Ret = jf_ptree_findNode(pict->ict_pjpConfig, pstrName, sName, &node);

    /*Find the transaction.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _findConfigTreeTransaction(pict, u32TransactionId, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_replaceNode(
            pictt->ictt_pjpConfig, pstrName, sName, pstrValue, sValue, NULL);

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

static u32 _setConfigIntoConfigTree(
    internal_config_tree_t * pict, olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue,
    olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * node = NULL;

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = jf_ptree_findNode(pict->ict_pjpConfig, pstrName, sName, &node);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_changeNodeValue(node, pstrValue, sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        pict->ict_bConfigChanged = TRUE;

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

static u32 _copyConfigFromNode(jf_ptree_node_t * node, olchar_t * pstrValue, olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstr = NULL;
    olsize_t sStr = 0, sValue = *psValue;

    u32Ret = jf_ptree_getNodeValue(node, &pstr, &sStr);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("value: %s(%d)", pstr, sStr);
        if (sStr > sValue)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memcpy(pstrValue, pstr, sStr);
        *psValue = sStr;
    }

    return u32Ret;
}

static u32 _getConfigInTransaction(
    internal_config_tree_t * pict, u32 u32TransactionId, olchar_t * pstrName, olsize_t sName,
    olchar_t * pstrValue, olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * node = NULL;
    internal_config_tree_transaction_t * pictt = NULL;

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = _findConfigTreeTransaction(pict, u32TransactionId, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(pictt->ictt_pjpConfig, pstrName, sName, &node);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _copyConfigFromNode(node, pstrValue, psValue);

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

static u32 _getConfigFromConfigTree(
    internal_config_tree_t * pict, olchar_t * pstrName, olsize_t sName, olchar_t * pstrValue,
    olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * node = NULL;

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = jf_ptree_findNode(pict->ict_pjpConfig, pstrName, sName, &node);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _copyConfigFromNode(node, pstrValue, psValue);

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initConfigTree(config_tree_init_param_t * pctip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;

    assert(pctip != NULL);

    JF_LOGGER_INFO(
        "persistency type: %s",
        getStringConfigPersistencyType(pctip->ctip_picmsSetting->icms_u8ConfigPersistencyType));

    u32Ret = _initConfigTree(pict, pctip);

    if (u32Ret == JF_ERR_NO_ERROR)
        pict->ict_bInitialized = TRUE;
    else
        finiConfigTree();

    return u32Ret;
}

u32 finiConfigTree(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;

    JF_LOGGER_INFO("fini");

    if (pict->ict_picttTransaction != NULL)
        jf_jiukun_freeMemory((void **)&pict->ict_picttTransaction);

    /*Save config to persistency.*/
    if (pict->ict_bConfigChanged)
        saveConfigToPersistency(
            pict->ict_picmsSetting->icms_u8ConfigPersistencyType,
            pict->ict_picmsSetting->icms_pstrConfigPersistencyLocation, pict->ict_pjpConfig);

    if (pict->ict_pjpConfig != NULL)
        jf_ptree_destroy(&pict->ict_pjpConfig);

    /*Finalize the mutex lock.*/
    jf_mutex_fini(&pict->ict_jmLock);

    pict->ict_bInitialized = FALSE;
    
    return u32Ret;
}

u32 getConfigFromConfigTree(
    u32 u32TransactionId, olchar_t * pstrName, olsize_t sName, olchar_t * pstrValue,
    olsize_t * psValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;

    JF_LOGGER_DEBUG("id: %u", u32TransactionId);
    JF_LOGGER_DATAA((u8 *)pstrName, sName, "%s", "name");

    if (u32TransactionId == JF_CONFIG_INVALID_TRANSACTION_ID)
    {
        u32Ret = _getConfigFromConfigTree(pict, pstrName, sName, pstrValue, psValue);
    }
    else
    {
        /*Get config from the transaction.*/
        u32Ret = _getConfigInTransaction(pict, u32TransactionId, pstrName, sName, pstrValue, psValue);

        /*Get config from config tree if failed to get it from transaction.*/
        if (u32Ret != JF_ERR_NO_ERROR)
            u32Ret = _getConfigFromConfigTree(pict, pstrName, sName, pstrValue, psValue);
    }

    return u32Ret;
}

u32 setConfigIntoConfigTree(
    u32 u32TransactionId, olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue,
    olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;

    JF_LOGGER_DEBUG("id: %u", u32TransactionId);
    JF_LOGGER_DATAA((u8 *)pstrName, sName, "name");
    JF_LOGGER_DATAA((u8 *)pstrValue, sValue, "value");

    if (u32TransactionId == JF_CONFIG_INVALID_TRANSACTION_ID)
        u32Ret = _setConfigIntoConfigTree(pict, pstrName, sName, pstrValue, sValue);        
    else
        u32Ret = _setConfigInTransaction(pict, u32TransactionId, pstrName, sName, pstrValue, sValue);

    return u32Ret;
}

u32 startConfigTreeTransaction(u32 * pu32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;
    internal_config_tree_transaction_t * pictt = NULL;

    u32Ret = _getConfigTreeTransaction(pict, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *pu32TransactionId = pictt->ictt_u32TransactionId;

        JF_LOGGER_DEBUG("id: %u", pictt->ictt_u32TransactionId);
    }

    return u32Ret;
}

u32 commitConfigTreeTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;
    internal_config_tree_transaction_t * pictt = NULL;

    JF_LOGGER_DEBUG("id: %u", u32TransactionId);

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = _findConfigTreeTransaction(pict, u32TransactionId, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _commitConfigTreeTransaction(pict, pictt);

        if (u32Ret == JF_ERR_NO_ERROR)
            pict->ict_bConfigChanged = TRUE;

        _finiConfigTreeTransaction(pictt);
    }

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

u32 rollbackConfigTreeTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;
    internal_config_tree_transaction_t * pictt = NULL;

    JF_LOGGER_DEBUG("id: %u", u32TransactionId);

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = _findConfigTreeTransaction(pict, u32TransactionId, &pictt);

    if (u32Ret == JF_ERR_NO_ERROR)
        _finiConfigTreeTransaction(pictt);

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
