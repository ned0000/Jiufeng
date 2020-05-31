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

/* Service management setting name definition.*/

/** The root node name of the setting file.
 */
#define SMSN_ROOT                             "servMgmtSetting"

/** The node path of version.
 */
#define SMSN_VERSION                          SMSN_ROOT ".version"
#define SMSN_VERSION_LEN                      ol_strlen(SMSN_VERSION)

/** The node path of global setting.
 */
#define SMSN_GLOBAL_SETTING                   SMSN_ROOT ".globalSetting"

/** The node path of max failure retry count.
 */
#define SMSN_MAX_FAILURE_RETRY_COUNT          SMSN_GLOBAL_SETTING ".maxFailureRetryCount"
#define SMSN_MAX_FAILURE_RETRY_COUNT_LEN      ol_strlen(SMSN_MAX_FAILURE_RETRY_COUNT)

/** The node path of all service setting.
 */
#define SMSN_SERVICE_SETTING                  SMSN_ROOT ".serviceSetting"

/** The node path of one service setting.
 */
#define SMSN_SERVICE                          SMSN_SERVICE_SETTING ".service"
#define SMSN_SERVICE_LEN                      ol_strlen(SMSN_SERVICE)

/** The name of the setting name node.
 */
#define SMSN_SERVICE_NAME                     "name"
#define SMSN_SERVICE_NAME_LEN                 ol_strlen(SMSN_SERVICE_NAME)

/** The name of the setting description node.
 */
#define SMSN_SERVICE_DESCRIPTION              "description"
#define SMSN_SERVICE_DESCRIPTION_LEN          ol_strlen(SMSN_SERVICE_DESCRIPTION)

/** The name of the setting startup type node.
 */
#define SMSN_SERVICE_STARTUPTYPE              "startupType"
#define SMSN_SERVICE_STARTUPTYPE_LEN          ol_strlen(SMSN_SERVICE_STARTUPTYPE)

/** The name of the setting command path node.
 */
#define SMSN_SERVICE_CMDPATH                  "cmdPath"
#define SMSN_SERVICE_CMDPATH_LEN              ol_strlen(SMSN_SERVICE_CMDPATH)

/** The name of the setting command parameter node.
 */
#define SMSN_SERVICE_CMDPARAM                 "cmdParam"
#define SMSN_SERVICE_CMDPARAM_LEN             ol_strlen(SMSN_SERVICE_CMDPARAM)

/* --- private routine section ------------------------------------------------------------------ */

/** Parse the version node.
 */
static u32 _parseServMgmtVersion(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;

    u32Ret = jf_ptree_findNode(
        pisms->isms_pjpService, SMSN_VERSION, SMSN_VERSION_LEN, &pNode);

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
        pisms->isms_pjpService, SMSN_MAX_FAILURE_RETRY_COUNT, SMSN_MAX_FAILURE_RETRY_COUNT_LEN,
        &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU8FromString(pstrValue, sValue, &pisms->isms_u8FailureRetryCount);

    return u32Ret;
}

/** Parse one service node.
 */
static u32 _processServiceNode(
    jf_ptree_t * pjpService, internal_service_info_t * pisi, jf_ptree_node_t * pNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pChild = NULL;
    olchar_t * pstrValue = NULL;

    /*Find the child node with service name.*/
    u32Ret = jf_ptree_findChildNode(
        pjpService, pNode, NULL, 0, SMSN_SERVICE_NAME, SMSN_SERVICE_NAME_LEN, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrName, NULL);

    /*Find the child node with service description.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(
            pjpService, pNode, NULL, 0, SMSN_SERVICE_DESCRIPTION, SMSN_SERVICE_DESCRIPTION_LEN,
            &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrDescription, NULL);

    /*Find the child node with service command path.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(
            pjpService, pNode, NULL, 0, SMSN_SERVICE_CMDPATH, SMSN_SERVICE_CMDPATH_LEN, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdPath, NULL);

    /*Find the child node with service command parameter.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(
            pjpService, pNode, NULL, 0, SMSN_SERVICE_CMDPARAM, SMSN_SERVICE_CMDPARAM_LEN, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdParam, NULL);

    /*Find the child node with service startup type.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(
            pjpService, pNode, NULL, 0, SMSN_SERVICE_STARTUPTYPE, SMSN_SERVICE_STARTUPTYPE_LEN,
            &pChild);

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
        pisms->isms_pjpService, SMSN_SERVICE, SMSN_SERVICE_LEN, pNode, &u16NumOfNode);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16NumOfNode; ++u16Index)
        {
            /*Parse the child node of service node.*/
            u32Ret = _processServiceNode(
                pisms->isms_pjpService, &pisms->isms_isiService[u16Index], pNode[u16Index]);

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
        JF_LOGGER_INFO("version     : %s", pisms->isms_pstrVersion);
        JF_LOGGER_INFO("number of service: %u", pisms->isms_u16NumOfService);
        JF_LOGGER_INFO("failureRetry: %u", pisms->isms_u8FailureRetryCount);
    }
    
    return u32Ret;
}

u32 freeServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Free the property tree of setting.*/
    if (pisms->isms_pjpService != NULL)
        jf_ptree_destroy(&pisms->isms_pjpService);

    return u32Ret;
}

u32 modifyServiceStartupType(internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const olchar_t * pstrStartupType = getStringServStartupType(pisi->isi_u8StartupType);
    
    /*Change the value of the node for startup type.*/
    u32Ret = jf_ptree_changeNodeValue(
        pisi->isi_pjpnStartupType, pstrStartupType, ol_strlen(pstrStartupType));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Save the modified property tree to XML file.*/
        u32Ret = jf_xmlparser_saveXmlFile(pisms->isms_pjpService, pisms->isms_strSettingFile);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


