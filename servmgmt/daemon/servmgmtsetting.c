/**
 *  @file servmgmtsetting.c
 *
 *  @brief Parse service management setting file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# The setting file is in XML format. After parse, the property tree is generated. 
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_serv.h"
#include "jf_string.h"
#include "jf_xmlparser.h"
#include "jf_jiukun.h"

#include "servmgmtsetting.h"
#include "servmgmtcommon.h"


/* --- private data/data structure section ------------------------------------------------------ */

/** The root node name of the setting file.
 */
#define SERVMGMT_SETTING_ROOT                 "servMgmtSetting"

/** The node path of version.
 */
#define SERVMGMT_SETTING_VERSION              SERVMGMT_SETTING_ROOT ".version"

/** The node path of global setting.
 */
#define SERVMGMT_SETTING_GLOBALSETTING        SERVMGMT_SETTING_ROOT ".globalSetting"

/** The node path of max failure retry count.
 */
#define SERVMGMT_SETTING_MAXFAILURERETRYCOUNT SERVMGMT_SETTING_GLOBALSETTING ".maxFailureRetryCount"

/** The node path of all service setting.
 */
#define SERVMGMT_SETTING_SERVICESETTING       SERVMGMT_SETTING_ROOT ".serviceSetting"

/** The node path of one service setting.
 */
#define SERVMGMT_SETTING_SERVICE              SERVMGMT_SETTING_SERVICESETTING ".service"

/** The name of the setting name node.
 */
#define SERVICE_INFO_NAME                     "name"

/** The name of the setting description node.
 */
#define SERVICE_INFO_DESCRIPTION              "description"

/** The name of the setting startup type node.
 */
#define SERVICE_INFO_STARTUPTYPE              "startupType"

/** The name of the setting command path node.
 */
#define SERVICE_INFO_CMDPATH                  "cmdPath"

/** The name of the setting command parameter node.
 */
#define SERVICE_INFO_CMDPARAM                 "cmdParam"

/* --- private routine section ------------------------------------------------------------------ */

/** Parse the version node.
 */
static u32 _parseServMgmtVersion(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;

    u32Ret = jf_ptree_findNode(
        pisms->isms_pjpService, SERVMGMT_SETTING_VERSION, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pisms->isms_pstrVersion, NULL);

    return u32Ret;
}

/** Parse the global setting node.
 */
static u32 _parseGlobalSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    olchar_t * pstrValue = NULL;
    olsize_t sValue = 0;

    /*Find the child node with max failure retry count.*/
    u32Ret = jf_ptree_findNode(
        pisms->isms_pjpService, SERVMGMT_SETTING_MAXFAILURERETRYCOUNT, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU8FromString(pstrValue, sValue, &pisms->isms_u8FailureRetryCount);

    return u32Ret;
}

/** Parse one service node.
 */
static u32 _processServiceNode(internal_service_info_t * pisi, jf_ptree_node_t * pNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pChild = NULL;
    olchar_t * pstrValue = NULL;

    /*Find the child node with service name.*/
    u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_NAME, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrName, NULL);

    /*Find the child node with service description.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_DESCRIPTION, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrDescription, NULL);

    /*Find the child node with service command path.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_CMDPATH, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdPath, NULL);

    /*Find the child node with service command parameter.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_CMDPARAM, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdParam, NULL);

    /*Find the child node with service startup type.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_STARTUPTYPE, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pstrValue, NULL);

    /*Parse the startup type node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Save the startup type node for future use. This is to save the time finding the node When
          changing the startup type of the service.*/
        pisi->isi_pjpnStartupType = pChild;

        /*Parse the startup type string.*/
        u32Ret = getServStartupTypeFromString(pstrValue, &pisi->isi_u8StartupType);
    }

    return u32Ret;
}

/** Parse the setting of single service.
 */
static u32 _parseServiceSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode[JF_SERV_MAX_NUM_OF_SERV];
    u16 u16NumOfNode = JF_SERV_MAX_NUM_OF_SERV;    
    u16 u16Index = 0;

    /*Find all the service node.*/
    u32Ret = jf_ptree_findAllNode(
        pisms->isms_pjpService, SERVMGMT_SETTING_SERVICE, pNode, &u16NumOfNode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16NumOfNode; ++u16Index)
        {
            /*Parse the child node of service node.*/
            u32Ret = _processServiceNode(&pisms->isms_isiService[u16Index], pNode[u16Index]);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pisms->isms_u16NumOfService ++;

                jf_logger_logInfoMsg(
                    "service name: %s, startup type: %s, cmdPath: %s",
                    pisms->isms_isiService[u16Index].isi_pstrName,
                    getStringServStartupType(pisms->isms_isiService[u16Index].isi_u8StartupType),
                    pisms->isms_isiService[u16Index].isi_pstrCmdPath);
            }
        }
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pisms != NULL);

    /*Parse the XML file and build the property tree.*/
    u32Ret = jf_xmlparser_parseXmlFile(pisms->isms_strSettingFile, &pisms->isms_pjpService);

    /*Parse the version.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseServMgmtVersion(pisms);

    /*Parse the global setting.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseGlobalSetting(pisms);
    
    /*Parse service and add private data to each node.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _parseServiceSetting(pisms);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("version     : %s", pisms->isms_pstrVersion);
        jf_logger_logInfoMsg("number of service: %u", pisms->isms_u16NumOfService);
        jf_logger_logInfoMsg("failureRetry: %u", pisms->isms_u8FailureRetryCount);
    }
    
    return u32Ret;
}

u32 modifyServiceStartupType(internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const olchar_t * pstrStartupType = getStringServStartupType(pisi->isi_u8StartupType);
    
    /*Change the value of the node for startup type.*/
    u32Ret = jf_ptree_changeNodeValue(pisi->isi_pjpnStartupType, pstrStartupType);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Save the modified property tree to XML file.*/
        u32Ret = jf_xmlparser_saveXmlFile(pisms->isms_pjpService, pisms->isms_strSettingFile);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


