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

#define SERVMGMT_SETTING_ROOT                 "servMgmtSetting"

#define SERVMGMT_SETTING_VERSION              SERVMGMT_SETTING_ROOT ".version"

#define SERVMGMT_SETTING_GLOBALSETTING        SERVMGMT_SETTING_ROOT ".globalSetting"

#define SERVMGMT_SETTING_MAXFAILURERETRYCOUNT SERVMGMT_SETTING_GLOBALSETTING ".maxFailureRetryCount"

#define SERVMGMT_SETTING_SERVICESETTING       SERVMGMT_SETTING_ROOT ".serviceSetting"

#define SERVMGMT_SETTING_SERVICE              SERVMGMT_SETTING_SERVICESETTING ".service"

#define SERVICE_INFO_NAME                     "name"
#define SERVICE_INFO_DESCRIPTION              "description"
#define SERVICE_INFO_STARTUPTYPE              "startupType"
#define SERVICE_INFO_CMDPATH                  "cmdPath"
#define SERVICE_INFO_CMDPARAM                 "cmdParam"

/* --- private routine section ------------------------------------------------------------------ */

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

static u32 _parseGlobalSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode = NULL;
    olchar_t * pstrValue = NULL;
    olsize_t sValue = 0;

    u32Ret = jf_ptree_findNode(
        pisms->isms_pjpService, SERVMGMT_SETTING_MAXFAILURERETRYCOUNT, &pNode);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pNode, &pstrValue, &sValue);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_getU8FromString(pstrValue, sValue, &pisms->isms_u8FailureRetryCount);

    return u32Ret;
}

static u32 _processServiceNode(internal_service_info_t * pisi, jf_ptree_node_t * pNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pChild = NULL;
    olchar_t * pstrValue = NULL;

    /*Create a service info data type and attach it to the property tree.*/
    u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_NAME, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrName, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_DESCRIPTION, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrDescription, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_CMDPATH, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdPath, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_CMDPARAM, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pisi->isi_pstrCmdParam, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_findChildNode(pNode, NULL, SERVICE_INFO_STARTUPTYPE, &pChild);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_ptree_getNodeValue(pChild, &pstrValue, NULL);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pisi->isi_pjpnStartupType = pChild;

        u32Ret = getServStartupTypeFromString(pstrValue, &pisi->isi_u8StartupType);
    }

    return u32Ret;
}

static u32 _parseServiceSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ptree_node_t * pNode[JF_SERV_MAX_NUM_OF_SERV];
    u16 u16NumOfNode = JF_SERV_MAX_NUM_OF_SERV;    
    u16 u16Index = 0;

    u32Ret = jf_ptree_findAllNode(pisms->isms_pjpService, SERVMGMT_SETTING_SERVICE, pNode, &u16NumOfNode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16NumOfNode; ++u16Index)
        {
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
    
    /*Change the value of the node for startup typ.*/
    u32Ret = jf_ptree_changeNodeValue(pisi->isi_pjpnStartupType, pstrStartupType);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Save the modified property tree to XML file.*/
        u32Ret = jf_xmlparser_saveXmlFile(pisms->isms_pjpService, pisms->isms_strSettingFile);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


