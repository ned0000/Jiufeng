/**
 *  @file serviceconfig.c
 *
 *  @brief Service configuration file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
    #include <dirent.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_dir.h"
#include "jf_ptree.h"
#include "jf_xmlparser.h"
#include "jf_string.h"
#include "jf_jiukun.h"

#include "serviceconfig.h"
#include "dispatchercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum number of message in message queue for service.
 */
#define MAX_NUM_OF_SERVICE_MSG                           (500)

/** Maximum service message size.
 */
#define MAX_SERVICE_MSG_SIZE                             (128 * 1024)

#define DISPATCHER_CONFIG_FILE_EXT                       ".xml"

/* Dispatcher service config name definition */

#define DSCN_ROOT                                        "configuration"
#define DSCN_ROOT_LEN                                    ol_strlen(DSCN_ROOT)

#define DSCN_VERSION                                     "version"
#define DSCN_VERSION_LEN                                 ol_strlen(DSCN_VERSION)

#define DSCN_SERVICE_INFO                                DSCN_ROOT ".serviceInfo"

#define DSCN_PUBLISHED_MSG                               DSCN_ROOT ".publishedMessage"
#define DSCN_PUBLISHED_MSG_LEN                           ol_strlen(DSCN_PUBLISHED_MSG)

#define DSCN_SUBSCRIBED_MSG                              DSCN_ROOT ".subscribedMessage"
#define DSCN_SUBSCRIBED_MSG_LEN                          ol_strlen(DSCN_SUBSCRIBED_MSG)

#define DSCN_SERVICE_NAME                                DSCN_SERVICE_INFO ".serviceName"
#define DSCN_SERVICE_NAME_LEN                            ol_strlen(DSCN_SERVICE_NAME)
#define DSCN_SERVICE_USER_NAME                           DSCN_SERVICE_INFO ".userName"
#define DSCN_SERVICE_USER_NAME_LEN                       ol_strlen(DSCN_SERVICE_USER_NAME)
#define DSCN_SERVICE_MESSAGING_IN                        DSCN_SERVICE_INFO ".messagingIn"
#define DSCN_SERVICE_MESSAGING_IN_LEN                    ol_strlen(DSCN_SERVICE_MESSAGING_IN)
#define DSCN_SERVICE_MESSAGING_OUT                       DSCN_SERVICE_INFO ".messagingOut"
#define DSCN_SERVICE_MESSAGING_OUT_LEN                   ol_strlen(DSCN_SERVICE_MESSAGING_OUT)
#define DSCN_SERVICE_MAX_NUM_MSG                         DSCN_SERVICE_INFO ".maxNumMsg"
#define DSCN_SERVICE_MAX_NUM_MSG_LEN                     ol_strlen(DSCN_SERVICE_MAX_NUM_MSG)
#define DSCN_SERVICE_MAX_MSG_SIZE                        DSCN_SERVICE_INFO ".maxMsgSize"
#define DSCN_SERVICE_MAX_MSG_SIZE_LEN                    ol_strlen(DSCN_SERVICE_MAX_MSG_SIZE)

#define DSCN_MESSAGE                                     "message"

#define DSCN_MESSAGE_ID                                  "id"
#define DSCN_MESSAGE_ID_LEN                              ol_strlen(DSCN_MESSAGE_ID)

/** The name of message config cache.
 */
#define DISPATCHER_MSG_CACHE                             "dispatcher_msg_config"

typedef struct
{
    jf_jiukun_cache_t * psm_pjjcMsgConfig;
    jf_linklist_t * psm_pjlMsg;
    u16 psm_u16NumOfMsg;
    u16 psm_u16Reserved;
} parse_serv_msg_t;

/** The jiukun cache for message config data type.
 */
static jf_jiukun_cache_t * ls_pjjcMsgConfig = NULL;

/* --- private routine section ------------------------------------------------------------------ */

/** Free one message config.
 */
static u32 _fnFreeDispatcherMsgConfig(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_jiukun_freeObject(ls_pjjcMsgConfig, (void **)ppData);

    return u32Ret;
}

/** Free one service config.
 */
static u32 _fnFreeDispatcherServiceConfig(void ** pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_service_config_t * pdsc = (dispatcher_service_config_t *) *pData;

    /*Free the published message link list and all the entries in the list.*/
    jf_linklist_finiListAndData(&pdsc->dsc_jlPublishedMsg, _fnFreeDispatcherMsgConfig);

    /*Free the subscribed message link list and all the entries in the list.*/
    jf_linklist_finiListAndData(&pdsc->dsc_jlSubscribedMsg, _fnFreeDispatcherMsgConfig);

    /*Free the serivice config.*/
    jf_jiukun_freeMemory(pData);

    return u32Ret;
}

/** Check the message config, no duplicate id.
 */
static u32 _validateMsgConfig(parse_serv_msg_t * ppsm, dispatcher_msg_config_t * pMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    return u32Ret;
}

static u32 _fnParseServMsg(jf_ptree_t * pPtree, jf_ptree_node_t * pNode, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    parse_serv_msg_t * ppsm = (parse_serv_msg_t *)pArg;
    olchar_t * pstrValue = NULL, * pstrId = NULL;
    olsize_t sId = 0;
    jf_ptree_node_attribute_t * pAttr = NULL;
    dispatcher_msg_config_t * pMsg = NULL;

    /*Allocate message config object from cache.*/
    u32Ret = jf_jiukun_allocObject(ppsm->psm_pjjcMsgConfig, (void **)&pMsg);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, NULL);

    /*Copy the message description.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_strncpy(pMsg->dmc_strMsgDesc, pstrValue, MAX_DISPATCHER_MSG_DESC_LEN - 1);

    /*Parse the attribute with name "id".*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNodeAttribute(
            pNode, NULL, 0, DSCN_MESSAGE_ID, DSCN_MESSAGE_ID_LEN, &pAttr);

    /*Get attribute value, the id string is with format "id".*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributeValue(pAttr, &pstrId, &sId);

    /*Convert the string to integer.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU16FromString(pstrId, sId, &pMsg->dmc_u16MsgId);

    /*Validate message config.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _validateMsgConfig(ppsm, pMsg);

    /*Append the message config to the linked list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_linklist_appendTo(ppsm->psm_pjlMsg, pMsg);

    if (u32Ret == JF_ERR_NO_ERROR)
        ppsm->psm_u16NumOfMsg ++;
    else if (pMsg != NULL)
        jf_jiukun_freeObject(ppsm->psm_pjjcMsgConfig, (void **)&pMsg);

    return u32Ret;
}

static u32 _parseDispatcherMessage(
    jf_ptree_t * pPtree, dispatcher_service_config_t * pdsc,
    scan_dispatcher_config_dir_param_t * psdcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    parse_serv_msg_t psmArg;

    /*Parse the published message list.*/
    u32Ret = jf_ptree_findNode(pPtree, DSCN_PUBLISHED_MSG, DSCN_PUBLISHED_MSG_LEN, &pNode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&psmArg, sizeof(psmArg));
        psmArg.psm_pjjcMsgConfig = ls_pjjcMsgConfig;
        psmArg.psm_pjlMsg = &pdsc->dsc_jlPublishedMsg;

        /*Iterate node to add all message to linked list.*/
        u32Ret = jf_ptree_iterateNode(pPtree, pNode, _fnParseServMsg, &psmArg);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pdsc->dsc_u16NumOfPublishedMsg = psmArg.psm_u16NumOfMsg;

    /*Parse the subscribed message list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(pPtree, DSCN_SUBSCRIBED_MSG, DSCN_SUBSCRIBED_MSG_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&psmArg, sizeof(psmArg));
        psmArg.psm_pjjcMsgConfig = ls_pjjcMsgConfig;
        psmArg.psm_pjlMsg = &pdsc->dsc_jlSubscribedMsg;

        /*Iterate node to add all message to linked list.*/
        u32Ret = jf_ptree_iterateNode(pPtree, pNode, _fnParseServMsg, &psmArg);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        pdsc->dsc_u16NumOfSubscribedMsg = psmArg.psm_u16NumOfMsg;

    return u32Ret;
}

static u32 _parseDispatcherServiceInfo(jf_ptree_t * pPtree, dispatcher_service_config_t * pdsc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    olchar_t * pstrValue = NULL;
    olsize_t sValue = 0;

    /*Find the service name node.*/
    u32Ret = jf_ptree_findNode(pPtree, DSCN_SERVICE_NAME, DSCN_SERVICE_NAME_LEN, &pNode);

    /*Get name of the service.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, NULL);

    /*Copy the service name.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        ol_strncpy(pdsc->dsc_strName, pstrValue, MAX_DISPATCHER_SERVICE_NAME_LEN - 1);

    /*Find the service user name node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
            pPtree, DSCN_SERVICE_USER_NAME, DSCN_SERVICE_USER_NAME_LEN, &pNode);

    /*Get name of the service user.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, NULL);

    /*Get UID and GID of the user.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_user_getUidGid(pstrValue, &pdsc->dsc_uiUser, &pdsc->dsc_giGroup);

    /*Find the service messaging in node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
            pPtree, DSCN_SERVICE_MESSAGING_IN, DSCN_SERVICE_MESSAGING_IN_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_strncpy(pdsc->dsc_strMessagingIn, pstrValue, MAX_DISPATCHER_MESSAGING_NAME_LEN - 1);

    /*Find the service messaging out node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
            pPtree, DSCN_SERVICE_MESSAGING_OUT, DSCN_SERVICE_MESSAGING_OUT_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_strncpy(pdsc->dsc_strMessagingOut, pstrValue, MAX_DISPATCHER_MESSAGING_NAME_LEN - 1);

    /*Find the maximum number of message node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
            pPtree, DSCN_SERVICE_MAX_NUM_MSG, DSCN_SERVICE_MAX_NUM_MSG_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU32FromString(pstrValue, sValue, &pdsc->dsc_u32MaxNumMsg);

    /*Find the maximum message size node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNode(
            pPtree, DSCN_SERVICE_MAX_MSG_SIZE, DSCN_SERVICE_MAX_MSG_SIZE_LEN, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU32FromString(pstrValue, sValue, &pdsc->dsc_u32MaxMsgSize);

    return u32Ret;
}

static u32 _getDispatcherServiceConfigVersion(jf_ptree_t * pPtree, dispatcher_service_config_t * pdsc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    jf_ptree_node_attribute_t * pAttr = NULL;
    olchar_t * pstrVer = NULL;

    u32Ret = jf_ptree_findNode(pPtree, DSCN_ROOT, DSCN_ROOT_LEN, &pNode);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findNodeAttribute(pNode, NULL, 0, DSCN_VERSION, DSCN_VERSION_LEN, &pAttr);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeAttributeValue(pAttr, &pstrVer, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        ol_strncpy(pdsc->dsc_strVersion, pstrVer, MAX_DISPATCHER_SERVICE_VERSION_LEN - 1);        

    return u32Ret;
}

static u32 _validateDispatcherServiceConfig(dispatcher_service_config_t * pdsc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if ((pdsc->dsc_u32MaxNumMsg == 0) || (pdsc->dsc_u32MaxNumMsg > MAX_NUM_OF_SERVICE_MSG))
        return JF_ERR_INVALID_DISPATCHER_SERVICE_CONFIG;

    if ((pdsc->dsc_u32MaxMsgSize == 0) || (pdsc->dsc_u32MaxMsgSize > MAX_SERVICE_MSG_SIZE))
        return JF_ERR_INVALID_DISPATCHER_SERVICE_CONFIG;

    return u32Ret;
}

static u32 _parseDispatcherServiceConfigFile(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat,
    scan_dispatcher_config_dir_param_t * psdcdp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_t * pPtree = NULL;
    dispatcher_service_config_t * pdsc = NULL;

    JF_LOGGER_DEBUG("config file: %s", pstrFullpath);

    /*Parse the config XML file and build the property tree.*/
    u32Ret = jf_xmlparser_parseXmlFile(pstrFullpath, &pPtree);

    /*Allocate object from cache.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_jiukun_allocMemory((void **)&pdsc, sizeof(*pdsc));

    /*Parse the version.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdsc, sizeof(*pdsc));
        pdsc->dsc_u32ServiceId = DISPATCHER_INVALID_SERVICE_ID;

        u32Ret = _getDispatcherServiceConfigVersion(pPtree, pdsc);
    }

    /*Parse the service information.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseDispatcherServiceInfo(pPtree, pdsc);

    /*Parse the message list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseDispatcherMessage(pPtree, pdsc, psdcdp);
    
    if (pPtree != NULL)
        jf_ptree_destroy(&pPtree);

    /*Validate the config.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _validateDispatcherServiceConfig(pdsc);

    /*Add the config to list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_linklist_appendTo(psdcdp->sdcdp_pjlServiceConfig, pdsc);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Set the service ID.*/
        pdsc->dsc_u16ServiceConfigId = psdcdp->sdcdp_u16NumOfServiceConfig;
        psdcdp->sdcdp_u16NumOfServiceConfig ++;

        JF_LOGGER_INFO(
            "service config id: %u, version: %s, name: %s, msgin: %s, msgout: %s, maxnummsg: %u, maxmsgsize: %u",
            pdsc->dsc_u16ServiceConfigId, pdsc->dsc_strVersion, pdsc->dsc_strName, pdsc->dsc_strMessagingIn,
            pdsc->dsc_strMessagingOut, pdsc->dsc_u32MaxNumMsg, pdsc->dsc_u32MaxMsgSize);
    }
    else if (pdsc != NULL)
    {
        jf_logger_logErrMsg(u32Ret, "parse dispatcher config file");
        _fnFreeDispatcherServiceConfig((void **)&pdsc);
    }

    return u32Ret;
}

static u32 _handleDispatcherConfigDirEntry(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    scan_dispatcher_config_dir_param_t * psdcdp = (scan_dispatcher_config_dir_param_t *)pArg;

    if (jf_file_isRegFile(pStat->jfs_u32Mode) &&
        jf_file_isTypedFile(pstrFullpath, NULL, DISPATCHER_CONFIG_FILE_EXT))
    {
        /*Do not return error for one corrupted config file, continue to parse other config file.*/
        _parseDispatcherServiceConfigFile(pstrFullpath, pStat, psdcdp);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 scanDispatcherConfigDir(scan_dispatcher_config_dir_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_jiukun_cache_create_param_t jjccp;

    JF_LOGGER_INFO("scan dir: %s", pParam->sdcdp_pstrConfigDir);

    /*Create the message config cache.*/
    ol_bzero(&jjccp, sizeof(jjccp));
    jjccp.jjccp_pstrName = DISPATCHER_MSG_CACHE;
    jjccp.jjccp_sObj = sizeof(dispatcher_msg_config_t);
    JF_FLAG_SET(jjccp.jjccp_jfCache, JF_JIUKUN_CACHE_CREATE_FLAG_ZERO);

    u32Ret = jf_jiukun_createCache(&ls_pjjcMsgConfig, &jjccp);

    /*Parse the config directory.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dir_parse(
            pParam->sdcdp_pstrConfigDir, _handleDispatcherConfigDirEntry, (void *)pParam);

        if (u32Ret != JF_ERR_NO_ERROR)
            JF_LOGGER_ERR(u32Ret, "failed to parse config dir: %s", pParam->sdcdp_pstrConfigDir);
    }

    return u32Ret;
}

u32 destroyDispatcherServiceConfigList(jf_linklist_t * pjlServiceConfig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Free the service config linked list and all the entries in the list.*/
    jf_linklist_finiListAndData(pjlServiceConfig, _fnFreeDispatcherServiceConfig);

    /*Free the message config cache.*/
    if (ls_pjjcMsgConfig != NULL)
        jf_jiukun_destroyCache(&ls_pjjcMsgConfig);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

