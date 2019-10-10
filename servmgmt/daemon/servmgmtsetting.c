/**
 *  @file servmgmtsetting.c
 *
 *  @brief parse serv mgmt setting file
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
#include <libxml/tree.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_serv.h"
#include "jf_string.h"

#include "servmgmtsetting.h"
#include "servmgmtcommon.h"


/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _parseServiceSettingValue(
    const olchar_t * pstrTag, const olchar_t * pstrValue, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (ol_strcmp(pstrTag, "name") == 0)
    { 
        ol_strncpy(pisi->isi_strName, pstrValue, JF_SERV_MAX_SERV_NAME_LEN - 1);
    }
    else if (ol_strcmp(pstrTag, "startupType") == 0)
    { 
        if (ol_strcmp(pstrValue, "automatic") == 0)
            pisi->isi_u8StartupType = JF_SERV_STARTUP_TYPE_AUTOMATIC;
        else if (ol_strcmp(pstrValue, "manual") == 0)
            pisi->isi_u8StartupType = JF_SERV_STARTUP_TYPE_MANUAL;
    }
    else if (ol_strcmp(pstrTag, "cmdPath") == 0)
    {
        ol_strncpy(pisi->isi_strCmdPath, pstrValue, JF_LIMIT_MAX_PATH_LEN - 1);
    }
    else if (ol_strcmp(pstrTag, "cmdParam") == 0)
    {
        ol_strncpy(pisi->isi_strCmdParam, pstrValue, MAX_SERVICE_CMD_PARAM_LEN - 1);
    }

    return u32Ret;
}

static u32 _servmgmtXmlNewChild(
    xmlNodePtr root, olchar_t * name, olchar_t * content, xmlNodePtr * ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    xmlNodePtr node = NULL;

    node = xmlNewChild(root, NULL, BAD_CAST name, BAD_CAST content);
    if (node == NULL)
    {
        u32Ret = JF_ERR_OUT_OF_MEMORY;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (ppNode != NULL))
        *ppNode = node;

    return u32Ret;
}

static u32 _parseGlobalSetting(
    xmlDocPtr doc, xmlNodePtr overall, internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    xmlNodePtr cur = NULL;
    xmlChar * key = NULL; 

    cur = overall->xmlChildrenNode;
    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, BAD_CAST "maxFailureRetryCount") == 0)
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key != NULL)
            {
                jf_string_getU8FromString(
                    (olchar_t *)key, strlen((olchar_t *)key), &pisms->isms_u8FailureRetryCount);
                xmlFree(key);
            }
        }

        cur = cur->next;
    }

    return u32Ret;
}

static u32 _parseServiceSetting(
    xmlDocPtr doc, xmlNodePtr overall, internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    xmlNodePtr service = NULL, cur = NULL;
    xmlChar * key = NULL; 

    service = overall->xmlChildrenNode;
    while ((service != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        if (xmlStrcmp(service->name, BAD_CAST "service") == 0)
        {
            cur = service->xmlChildrenNode;
            while ((cur != NULL) && (u32Ret == JF_ERR_NO_ERROR))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                if (key != NULL)
                {
                    u32Ret = _parseServiceSettingValue(
                        (olchar_t *)cur->name, (olchar_t *)key,
                        &pisms->isms_isiService[pisms->isms_u16NumOfService]);

                    xmlFree(key);
                }
                cur = cur->next;
            }

            pisms->isms_u16NumOfService ++;
        }
        service = service->next;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL, overall = NULL;
    xmlChar * key = NULL;
    u16 u16Index = 0;

    assert(pisms != NULL);

    xmlKeepBlanksDefault(0);
    doc = xmlParseFile(pisms->isms_strSettingFile);
    if (doc == NULL)
    {
        u32Ret = JF_ERR_LIBXML_PARSE_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Get the root element node */
        root = xmlDocGetRootElement(doc);

        if ((root != NULL) &&
            (xmlStrcmp(root->name, BAD_CAST "servMgmtSetting") == 0))
            overall = root->xmlChildrenNode;
        
        while ((overall != NULL) && (u32Ret == JF_ERR_NO_ERROR))
        {   
            if (xmlStrcmp(overall->name, BAD_CAST "version") == 0)
            {
                key = xmlNodeListGetString(doc, overall->xmlChildrenNode, 1);
                if (key != NULL)
                {
                    ol_strncpy(pisms->isms_strVersion, (olchar_t *)key, 7);
                    xmlFree(key);
                }
            }
            if (xmlStrcmp(overall->name, BAD_CAST "globalSetting") == 0)
            {
                u32Ret = _parseGlobalSetting(doc, overall, pisms);
            }
            else if (xmlStrcmp(overall->name, BAD_CAST "serviceSetting") == 0)
            {
                u32Ret = _parseServiceSetting(doc, overall, pisms);
            }

            overall = overall->next;
        }

        xmlFreeDoc(doc);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("version     : %s", pisms->isms_strVersion);
        jf_logger_logInfoMsg("failureRetry: %u", pisms->isms_u8FailureRetryCount);

        for (u16Index = 0; u16Index < pisms->isms_u16NumOfService; u16Index ++)
        {
            jf_logger_logInfoMsg(
                "service name: %s, startup type: %s, cmdPath: %s",
                pisms->isms_isiService[u16Index].isi_strName,
                getStringServStartupType(pisms->isms_isiService[u16Index].isi_u8StartupType),
                pisms->isms_isiService[u16Index].isi_strCmdPath);
        }
    }
    
    return u32Ret;
}

u32 modifyServiceStartupType(
    const olchar_t * pstrSettingFile, olchar_t * pstrServName, jf_serv_startup_type_t startupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strSettingFilename[JF_LIMIT_MAX_PATH_LEN];
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL, servicesetting = NULL;
    xmlNodePtr overall = NULL, repository = NULL;
    xmlNodePtr cur2 = NULL;
    xmlChar *key;
    boolean_t bFound = FALSE;
    
    assert((pstrSettingFile != NULL) &&
           ((startupType == JF_SERV_STARTUP_TYPE_MANUAL) ||
            (startupType == JF_SERV_STARTUP_TYPE_AUTOMATIC)));
    
    xmlKeepBlanksDefault(0);
    doc = xmlParseFile(strSettingFilename);
    if (doc == NULL)
    {
        u32Ret = JF_ERR_LIBXML_PARSE_ERROR;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
         /*Get the root element node */
        root = xmlDocGetRootElement(doc);
        overall = root->xmlChildrenNode;
        repository = overall->next;
        
        repository = repository->xmlChildrenNode;
        while (repository != NULL && u32Ret == JF_ERR_NO_ERROR)
        {
            if (xmlStrcmp(repository->name, BAD_CAST "serviceSetting") == 0)
            {
                servicesetting = repository->xmlChildrenNode;
                break;
            }
        }

        while (servicesetting != NULL && u32Ret == JF_ERR_NO_ERROR)
        {
            if (xmlStrcmp(servicesetting->name, BAD_CAST "name") == 0)
            {
                key = xmlNodeListGetString(doc, servicesetting->xmlChildrenNode, 1);
                if (xmlStrcmp(key, BAD_CAST pstrServName) == 0)
                {
                    cur2 = repository->xmlChildrenNode;
                    while (cur2 != NULL && u32Ret == JF_ERR_NO_ERROR)
                    {
                        if (xmlStrcmp(cur2->name, BAD_CAST "startupType") == 0)
                        {
                            if (startupType == JF_SERV_STARTUP_TYPE_AUTOMATIC)
                                xmlNodeSetContent(cur2, (xmlChar *)"automatic");
                            else if (startupType == JF_SERV_STARTUP_TYPE_MANUAL)
                                xmlNodeSetContent(cur2, (xmlChar *)"manual");

                            bFound = TRUE;

                            break;
                        }
                        cur2 = cur2->next;
                    }
                }

                xmlFree(key);

                if (bFound)
                    break;
            }
            servicesetting = servicesetting->next;
        }

        if (bFound)
            xmlSaveFormatFileEnc(strSettingFilename, doc, "UTF-8", 1);

        /*free the document */
        xmlFreeDoc(doc);
    }

    return u32Ret;
}

u32 writeServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_service_info_t * pisi = NULL;
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root = NULL, setting_node = NULL, serv_node = NULL, globalsetting = NULL;
    u32 u32Index;
    olchar_t str[128];

    xmlKeepBlanksDefault(0);
    /** Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    if (doc == NULL)
        u32Ret = JF_ERR_OUT_OF_MEMORY;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        root = xmlNewNode(NULL, BAD_CAST "servMgmtSetting");
        if (root == NULL)
            u32Ret = JF_ERR_OUT_OF_MEMORY;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        xmlDocSetRootElement(doc, root);

        u32Ret = _servmgmtXmlNewChild(root, "version", pisms->isms_strVersion, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _servmgmtXmlNewChild(root, "globalSetting", NULL, &globalsetting);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_sprintf(str, "%u", pisms->isms_u8FailureRetryCount);

        u32Ret = _servmgmtXmlNewChild(globalsetting, "maxFailureRetryCount", str, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _servmgmtXmlNewChild(root, "serviceSetting", NULL, &setting_node);

    for (u32Index = 0;
         (u32Index < pisms->isms_u16NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index ++)
    {
        pisi = &pisms->isms_isiService[u32Index];

        u32Ret = _servmgmtXmlNewChild(setting_node, "service", NULL, &serv_node);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(serv_node, "name", pisi->isi_strName, NULL);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "startupType",
                (olchar_t *)getStringServStartupType(pisi->isi_u8StartupType), NULL);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(serv_node, "cmdPath", pisi->isi_strCmdPath, NULL);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(serv_node, "cmdParam", pisi->isi_strCmdParam, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        xmlSaveFormatFileEnc(pisms->isms_strSettingFile, doc, "UTF-8", 1);

    if (doc != NULL)
    {
        /*free the document */
        xmlFreeDoc(doc);
        doc = NULL;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


