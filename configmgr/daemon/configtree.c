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
#include <stdio.h>
#include <string.h>
#include <signal.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_jiukun.h"
#include "jf_sharedmemory.h"
#include "jf_filestream.h"
#include "jf_process.h"
#include "jf_time.h"
#include "jf_serv.h"
#include "jf_thread.h"
#include "jf_mutex.h"
#include "jf_ptree.h"

#include "configmgrcommon.h"
#include "configtree.h"
#include "configpersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    u32 ictr_u32TransactionId;
    jf_ptree_t * ictr_pjpConfig;
} internal_config_tree_trasaction_t;

typedef struct
{
    boolean_t ict_bInitialized;
    u8 ict_u8Reserved[7];

    internal_config_mgr_setting_t * ict_picmsSetting;

    jf_mutex_t ict_jmLock;
    jf_ptree_t * ict_pjpConfig;

    u16 ict_u16Reserved[3];
    u16 ict_u16NumOfTransaction;
    internal_config_tree_trasaction_t * ict_picttTransaction;

} internal_config_tree_t;

static internal_config_tree_t ls_ictConfigTree;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _initConfigTree(internal_config_tree_t * pict, config_tree_init_param_t * pctip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sMem = 0;

    ol_bzero(pict, sizeof(*pict));
    pict->ict_picmsSetting = pctip->ctip_picmsSetting;

    assert(pict->ict_picmsSetting->icms_u16MaxNumOfTransaction > 0);
    sMem = sizeof(internal_config_tree_trasaction_t) * pict->ict_picmsSetting->icms_u16MaxNumOfTransaction;

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

/* --- public routine section ------------------------------------------------------------------- */

u32 initConfigTree(config_tree_init_param_t * pctip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;

    assert(pctip != NULL);

    JF_LOGGER_INFO("persistency type: %u", pctip->ctip_picmsSetting->icms_u8ConfigPersistencyType);

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

    JF_LOGGER_INFO("fini config tree");

    if (pict->ict_picttTransaction != NULL)
        jf_jiukun_freeMemory((void **)&pict->ict_picttTransaction);

    /*Save config to persistency.*/
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
    jf_ptree_node_t * node = NULL;
    olchar_t * pstr = NULL;
    olsize_t sStr = 0, sValue = *psValue;

    JF_LOGGER_DATAA((u8 *)pstrName, sName, "%s", "name");

    *psValue = 0;

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = jf_ptree_findNode(pict->ict_pjpConfig, pstrName, sName, &node);

    if (u32Ret == JF_ERR_NO_ERROR)
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

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

u32 setConfigIntoConfigTree(
    u32 u32TransactionId, olchar_t * pstrName, olsize_t sName, const olchar_t * pstrValue,
    olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_config_tree_t * pict = &ls_ictConfigTree;
    jf_ptree_node_t * node = NULL;

    JF_LOGGER_DATAA((u8 *)pstrName, sName, "name");
    JF_LOGGER_DATAA((u8 *)pstrValue, sValue, "value");

    jf_mutex_acquire(&pict->ict_jmLock);

    u32Ret = jf_ptree_findNode(pict->ict_pjpConfig, pstrName, sName, &node);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_changeNodeValue(node, pstrValue, sValue);

    jf_mutex_release(&pict->ict_jmLock);

    return u32Ret;
}

u32 startConfigTreeTransaction(u32 * pu32TransactionId)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

u32 commitConfigTreeTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

u32 rollbackConfigTreeTransaction(u32 u32TransactionId)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
