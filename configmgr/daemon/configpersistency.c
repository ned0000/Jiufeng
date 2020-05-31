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
#include "jf_conffile.h"
#include "jf_mutex.h"
#include "jf_ptree.h"

#include "configmgrcommon.h"
#include "configpersistency.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _fnAddConfigToTree(
    olchar_t * pstrName, olchar_t * pstrValue, void * pArg)
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

static u32 _loadConfigFromPersistencyFile(const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL;
    jf_conffile_open_param_t jcop;

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = (olchar_t *)pstrLocation;

    u32Ret = jf_conffile_open(&jcop, &pjc);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_conffile_traverse(pjc, _fnAddConfigToTree, pjpConfig);

        jf_conffile_close(&pjc);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        jf_ptree_dump(pjpConfig);

    return u32Ret;
}

static u32 _fnAddConfigToFile(jf_ptree_t * pPtree, jf_ptree_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = pArg;
    olchar_t * pstrValue = NULL;
    olchar_t str[JF_CONFFILE_MAX_LINE_LEN];
    olchar_t strName[JF_CONFFILE_MAX_LINE_LEN / 2];
    olsize_t sName = 0, sValue = 0, sStr = 0;

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

        if (pstrValue == NULL)
            sStr = snprintf(str, sizeof(str), "%s=\n", strName);
        else
            sStr = snprintf(str, sizeof(str), "%s=%s\n", strName, pstrValue);

        u32Ret = jf_conffile_write(pjc, str, sStr);
    }

    return u32Ret;
}

static u32 _saveConfigToPersistencyFile(const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_t * pjc = NULL;
    jf_conffile_open_param_t jcop;

    ol_bzero(&jcop, sizeof(jcop));
    jcop.jcop_pstrFile = (olchar_t *)pstrLocation;
    jcop.jcop_bWrite = TRUE;

    u32Ret = jf_conffile_open(&jcop, &pjc);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_traverse(pjpConfig, _fnAddConfigToFile, pjc);

    if (pjc != NULL)
        jf_conffile_close(&pjc);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 loadConfigFromPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("type: %u, location: %s", u8Type, pstrLocation);

    if (u8Type == CMCPT_CONF_FILE)
    {
        u32Ret = _loadConfigFromPersistencyFile(pstrLocation, pjpConfig);
    }

    return u32Ret;
}

u32 saveConfigToPersistency(u8 u8Type, const olchar_t * pstrLocation, jf_ptree_t * pjpConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("type: %u, location: %s", u8Type, pstrLocation);

    if (u8Type == CMCPT_CONF_FILE)
    {
        u32Ret = _saveConfigToPersistencyFile(pstrLocation, pjpConfig);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
