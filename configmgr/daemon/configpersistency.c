/**
 *  @file configpersistency.c
 *
 *  @brief The config persistency implementation file.
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
#include "jf_mutex.h"
#include "jf_ptree.h"
#include "jf_persistency.h"

#include "configmgrcommon.h"
#include "configpersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _convertConfigPersistencyType(
    u8 u8Type, const olchar_t * pstrLocation, jf_persistency_type_t * pType,
    jf_persistency_config_t * pjpc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *pType = JF_PERSISTENCY_TYPE_UNKNOWN;

    if (u8Type == CMCPT_CONF_FILE)
    {
        *pType = JF_PERSISTENCY_TYPE_FILE;
        ol_strcpy(pjpc->jpc_jpcfConfigFile.jpcf_strFile, pstrLocation);
    }
    else if (u8Type == CMCPT_SQLITE_DB)
    {
        *pType = JF_PERSISTENCY_TYPE_SQLITE;
        ol_strcpy(pjpc->jpc_jpcsConfigSqlite.jpcs_strSqliteDb, pstrLocation);
    }
    else
    {
        u32Ret = JF_ERR_UNSUPPORTED_PERSISTENCY_TYPE;
    }

    return u32Ret;
}

static u32 _fnAddConfigToTree(olchar_t * pstrName, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_t * pjpConfig = pArg;
    jf_ptree_node_t * node = NULL;

    JF_LOGGER_INFO(
        "name: %s(%ld), value: %s(%ld)",
        pstrName, ol_strlen(pstrName), pstrValue, ol_strlen(pstrValue));

    u32Ret = jf_ptree_replaceNode(
        pjpConfig, pstrName, ol_strlen(pstrName), pstrValue, ol_strlen(pstrValue), &node);

    return u32Ret;
}

static u32 _loadConfigFromPersistency(jf_persistency_t * pPersistency, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_persistency_traverse(pPersistency, _fnAddConfigToTree, pjpConfig);

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_ptree_dump(pjpConfig);

    return u32Ret;
}

static u32 _fnAddConfigToPersistency(jf_ptree_t * pPtree, jf_ptree_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersistency = pArg;
    olchar_t * pstrValue = NULL;
    olchar_t strName[JF_CONFIG_MAX_CONFIG_NAME_LEN];
    olsize_t sName = 0, sValue = 0;

    if (! jf_ptree_isLeafNode(pNode))
        return u32Ret;

    sName = sizeof(strName);
    u32Ret = jf_ptree_getNodeFullName(pPtree, pNode, strName, &sName);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG(
            "name: %s(%ld), value: %s(%ld)", strName, sName, pstrValue, sValue);

        u32Ret = jf_persistency_setValue(pPersistency, strName, pstrValue);
    }

    return u32Ret;
}

static u32 _saveConfigToPersistency(jf_persistency_t * pPersistency, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_persistency_startTransaction(pPersistency);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_ptree_traverse(pjpConfig, _fnAddConfigToPersistency, pPersistency);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_persistency_commitTransaction(pPersistency);
        else
            u32Ret = jf_persistency_rollbackTransaction(pPersistency);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 loadConfigFromPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_persistency_t * pPersistency = NULL;
    jf_persistency_config_t config;
    jf_persistency_type_t ptype = 0;

    JF_LOGGER_INFO("type: %s, location: %s", getStringConfigPersistencyType(u8Type), pstrLocation);

    u32Ret = _convertConfigPersistencyType(u8Type, pstrLocation, &ptype, &config);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_persistency_create(ptype, &config, &pPersistency);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _loadConfigFromPersistency(pPersistency, pjpConfig);;

    if (pPersistency != NULL)
        jf_persistency_destroy(&pPersistency);

    return u32Ret;
}

u32 saveConfigToPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("type: %s, location: %s", getStringConfigPersistencyType(u8Type), pstrLocation);

    jf_persistency_t * pPersistency = NULL;
    jf_persistency_config_t config;
    jf_persistency_type_t ptype = 0;

    JF_LOGGER_INFO("type: %s, location: %s", getStringConfigPersistencyType(u8Type), pstrLocation);

    u32Ret = _convertConfigPersistencyType(u8Type, pstrLocation, &ptype, &config);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_persistency_create(ptype, &config, &pPersistency);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _saveConfigToPersistency(pPersistency, pjpConfig);;

    if (pPersistency != NULL)
        jf_persistency_destroy(&pPersistency);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
