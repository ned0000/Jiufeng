/**
 *  @file configmgrsetting.c
 *
 *  @brief Parse config manager setting file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The setting file is in XML format.
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_xmlparser.h"
#include "jf_jiukun.h"

#include "configmgrsetting.h"
#include "configmgrcommon.h"


/* --- private data/data structure section ------------------------------------------------------ */

/* Config manager setting name definition.*/

/** The root node name of the setting file.
 */
#define CMSN_ROOT                               "configMgrSetting"

/** The node path of version.
 */
#define CMSN_VERSION                            CMSN_ROOT ".version"
#define CMSN_VERSION_LEN                        ol_strlen(CMSN_VERSION)

/** The node path of global setting.
 */
#define CMSN_GLOBAL_SETTING                     CMSN_ROOT ".globalSetting"

/** The node path of max number of transaction.
 */
#define CMSN_MAX_NUM_TRAN                       CMSN_GLOBAL_SETTING ".maxNumOfTransaction"
#define CMSN_MAX_NUM_TRAN_LEN                   ol_strlen(CMSN_MAX_NUM_TRAN)

/** The node path of max number of connection.
 */
#define CMSN_MAX_NUM_CONN                       CMSN_GLOBAL_SETTING ".maxNumOfConnection"
#define CMSN_MAX_NUM_CONN_LEN                   ol_strlen(CMSN_MAX_NUM_CONN)

/** The node path of config setting.
 */
#define CMSN_CONFIG_SETTING                     CMSN_ROOT ".configSetting"

/** The node path of config persistency setting.
 */
#define CMSN_PERSISTENCY                        CMSN_CONFIG_SETTING ".configPersistency"
#define CMSN_PERSISTENCY_LEN                    ol_strlen(CMSN_PERSISTENCY)

/** The property name of persistency setting node.
 */
#define CMSN_PERSISTENCY_LOCATION               "location"
#define CMSN_PERSISTENCY_LOCATION_LEN           ol_strlen(CMSN_PERSISTENCY_LOCATION)

/* --- private routine section ------------------------------------------------------------------ */

/** Parse the version node.
 */
static u32 _parseConfigMgrVersion(internal_config_mgr_setting_t * picms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;

    u32Ret = jf_ptree_findNode(
        picms->icms_pjpSetting, CMSN_VERSION, CMSN_VERSION_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &picms->icms_pstrVersion, NULL);

    return u32Ret;
}

/** Parse the global setting node.
 */
static u32 _parseConfigMgrGlobalSetting(internal_config_mgr_setting_t * picms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    olchar_t * pstrValue = NULL;
    olsize_t sValue = 0;

    /*Find the child node with max num of transaction.*/
    u32Ret = jf_ptree_findNode(
        picms->icms_pjpSetting, CMSN_MAX_NUM_TRAN, CMSN_MAX_NUM_TRAN_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU16FromString(pstrValue, sValue, &picms->icms_u16MaxNumOfTransaction);

    /*Find the child node with max num of connection.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
        picms->icms_pjpSetting, CMSN_MAX_NUM_CONN, CMSN_MAX_NUM_CONN_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU16FromString(pstrValue, sValue, &picms->icms_u16MaxNumOfConnection);

    return u32Ret;
}

/** Parse the config setting.
 */
static u32 _parseConfigMgrConfigSetting(internal_config_mgr_setting_t * picms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    olchar_t * pstrValue = NULL;
    olsize_t sValue = 0;
    jf_ptree_node_attribute_t * pAttr = NULL;

    /*Find the child node of config persistency.*/
    u32Ret = jf_ptree_findNode(
        picms->icms_pjpSetting, CMSN_PERSISTENCY, CMSN_PERSISTENCY_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    /*Parse the persistency type string.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = getConfigPersistencyTypeFromString(pstrValue, &picms->icms_u8ConfigPersistencyType);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNodeAttribute(
            pNode, NULL, 0, CMSN_PERSISTENCY_LOCATION, CMSN_PERSISTENCY_LOCATION_LEN, &pAttr);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributeValue(pAttr, &picms->icms_pstrConfigPersistencyLocation, NULL);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 readConfigMgrSetting(internal_config_mgr_setting_t * picms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(picms != NULL);

    /*Parse the XML file and build the property tree.*/
    u32Ret = jf_xmlparser_parseXmlFile(picms->icms_strSettingFile, &picms->icms_pjpSetting);

    /*Parse the version.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseConfigMgrVersion(picms);

    /*Parse the global setting.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseConfigMgrGlobalSetting(picms);
    
    /*Parse service and add private data to each node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseConfigMgrConfigSetting(picms);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_INFO("version     : %s", picms->icms_pstrVersion);
        JF_LOGGER_INFO("max number of transaction: %u", picms->icms_u16MaxNumOfTransaction);
        JF_LOGGER_INFO("max number of connection: %u", picms->icms_u16MaxNumOfConnection);
        JF_LOGGER_INFO(
            "config persistency type: %s(%u)",
            getStringConfigPersistencyType(picms->icms_u8ConfigPersistencyType),
            picms->icms_u8ConfigPersistencyType);
        JF_LOGGER_INFO(
            "config persistency location: %s", picms->icms_pstrConfigPersistencyLocation);
    }
    
    return u32Ret;
}

u32 freeConfigMgrSetting(internal_config_mgr_setting_t * picms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Free the property tree of setting.*/
    if (picms->icms_pjpSetting != NULL)
        jf_ptree_destroy(&picms->icms_pjpSetting);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
