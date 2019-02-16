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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
    #include <dirent.h>
#endif
#include <libxml/tree.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "files.h"
#include "servmgmt.h"
#include "servmgmtsetting.h"

/* --- private data/data structure section --------------------------------- */


/* --- private funciton routines ------------------------------------------- */

static void _parseServiceSettingValue(
    const olchar_t * pstrTag, const olchar_t * pstrValue,
    internal_serv_mgmt_setting_t * pisms, u32 u32Index)
{
    internal_service_info_t * pisi = &(pisms->isms_isiService[u32Index]);
    u32 u32Value;

    if (pstrValue == NULL)
        return;

    if (ol_strcmp(pstrTag, "name") == 0)
    { 
        ol_strncpy(pisi->isi_strName, pstrValue, MAX_SERVICE_NAME_LEN - 1);
    }
    else if (ol_strcmp(pstrTag, "startupType") == 0)
    { 
        if (ol_strcmp(pstrValue, "automatic") == 0)
            pisi->isi_u8StartupType = SERVICE_STARTUPTYPE_AUTOMATIC;
        else if (ol_strcmp(pstrValue, "manual") == 0)
            pisi->isi_u8StartupType = SERVICE_STARTUPTYPE_MANUAL;
    }
    else if (ol_strcmp(pstrTag, "role") == 0)
    {
        if (ol_strcmp(pstrValue, "wakeup") == 0)
            pisi->isi_u8Role = SERVICE_ROLE_WAKEUP;
        else if (ol_strcmp(pstrValue, "external") == 0)
            pisi->isi_u8Role = SERVICE_ROLE_EXTERNAL;
        else
            pisi->isi_u8Role = SERVICE_ROLE_INTERNAL;
    }
    else if (ol_strcmp(pstrTag, "cmdPath") == 0)
    {
        ol_strncpy(pisi->isi_strCmdPath, pstrValue, MAX_PATH_LEN - 1);
    }
    else if (ol_strcmp(pstrTag, "cmdParam") == 0)
    {
        ol_strncpy(
            pisi->isi_strCmdParam, pstrValue, MAX_SERVICE_CMD_PARAM_LEN - 1);
    }
    else if (ol_strcmp(pstrTag, "restartDelay") == 0)
    {
        ol_sscanf(pstrValue, "%d", &u32Value);
        pisi->isi_u8RestartDelay = (u8)u32Value;
    }
}

static olchar_t * _getStringServRole(u8 u8Role)
{
    if (u8Role == SERVICE_ROLE_WAKEUP)
        return "wakeup";
    else if (u8Role == SERVICE_ROLE_EXTERNAL)
        return "external";
    else
        return "internal";
}

static void _parseSettingValue(
    const olchar_t * pstrTag, const olchar_t * pstrValue, 
    internal_serv_mgmt_setting_t * pisms)
{
    if (pstrValue == NULL)
        return;

    if (strcmp(pstrTag, "version") == 0)
    {
        ol_strncpy(pisms->isms_strVersion, pstrValue, 7);
    }
}

static u32 _servmgmtXmlNewChild(
    xmlNodePtr root, olchar_t * name, olchar_t * content, xmlNodePtr * ppNode)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xmlNodePtr node = NULL;

    node = xmlNewChild(root, NULL, BAD_CAST name, BAD_CAST content);
    if (node == NULL)
    {
        u32Ret = OLERR_OUT_OF_MEMORY;
    }

    if ((u32Ret == OLERR_NO_ERROR) && (ppNode != NULL))
        *ppNode = node;

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 readServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = OLERR_NO_ERROR;
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL, cur = NULL, overall = NULL, repository = NULL;
    xmlNodePtr service = NULL;
    xmlChar *key;

    assert(pisms != NULL);

    xmlKeepBlanksDefault(0);
    doc = xmlParseFile(pisms->isms_strSettingFile);
    if (doc == NULL)
    {
        u32Ret = OLERR_LIBXML_PARSE_ERROR;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /*Get the root element node */
        root = xmlDocGetRootElement(doc);

        if ((root != NULL) &&
            (strcmp((olchar_t *)root->name, "servMgmtSetting") == 0))
            overall = root->xmlChildrenNode;
        
        while ((overall != NULL) && (u32Ret == OLERR_NO_ERROR))
        {   
            key = xmlNodeListGetString(doc, overall->xmlChildrenNode, 1);

            _parseSettingValue((olchar_t *)overall->name, (olchar_t *)key, pisms);

            if (ol_strcmp((olchar_t *)overall->name, "serviceSetting") == 0)
                repository = overall;

            xmlFree(key);

            overall = overall->next;
        }

        if (repository != NULL)
            service = repository->xmlChildrenNode;

        while ((service != NULL) && (u32Ret == OLERR_NO_ERROR))
        {
            if ((service != NULL) &&
                (strcmp((olchar_t *)service->name, "service") == 0))
            {
                cur = service->xmlChildrenNode;
                while (cur != NULL && u32Ret == OLERR_NO_ERROR)
                {
                    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

                    _parseServiceSettingValue(
                        (olchar_t *)cur->name, (olchar_t *)key,
                        pisms, pisms->isms_u32NumOfService);

                    xmlFree(key);

                    cur = cur->next;
                }
                pisms->isms_u32NumOfService ++;
            }
            service = service->next;           
        }
        xmlFreeDoc(doc);
    }
    
    return u32Ret;
}

u32 modifyServiceStartupType(
    const olchar_t * pstrSettingFile, olchar_t * pstrServiceName,
    u8 u8StartupType)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strSettingFilename[MAX_PATH_LEN];
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL, servicesetting = NULL;
    xmlNodePtr overall = NULL, repository = NULL;
    xmlNodePtr cur2 = NULL;
    xmlChar *key;
    boolean_t bFound = FALSE;
    
    assert((pstrSettingFile != NULL) &&
           ((u8StartupType == SERVICE_STARTUPTYPE_MANUAL) ||
            (u8StartupType == SERVICE_STARTUPTYPE_AUTOMATIC)));
    
    xmlKeepBlanksDefault(0);
    doc = xmlParseFile(strSettingFilename);
    if (doc == NULL)
    {
        u32Ret = OLERR_LIBXML_PARSE_ERROR;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
         /*Get the root element node */
        root = xmlDocGetRootElement(doc);
        overall = root->xmlChildrenNode;
        repository = overall->next;
        
        repository = repository->xmlChildrenNode;
        while (repository != NULL && u32Ret == OLERR_NO_ERROR)
        {
            if (strcmp((olchar_t *)repository->name, "serviceSetting") == 0)
            {
                servicesetting = repository->xmlChildrenNode;
                break;
            }
        }

        while (servicesetting != NULL && u32Ret == OLERR_NO_ERROR)
        {
            if (strcmp((olchar_t *)servicesetting->name, "name") == 0)
            {
                key = xmlNodeListGetString(doc, servicesetting->xmlChildrenNode, 1);
                if (strcmp((olchar_t *)key, pstrServiceName) == 0)
                {
                    cur2 = repository->xmlChildrenNode;
                    while (cur2 != NULL && u32Ret == OLERR_NO_ERROR)
                    {
                        if (strcmp((olchar_t *)cur2->name, "startupType") == 0)
                        {
                            if (u8StartupType == SERVICE_STARTUPTYPE_AUTOMATIC)
                                xmlNodeSetContent(cur2, (xmlChar *)"automatic");
                            else if (u8StartupType == SERVICE_STARTUPTYPE_MANUAL)
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

u32 writeServMgmtSetting(
    internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_service_info_t * pisi;
    xmlDocPtr doc = NULL;       /* document pointer */
    xmlNodePtr root = NULL, setting_node = NULL, serv_node = NULL;
    olchar_t str[100];
    u32 u32Index;

    xmlKeepBlanksDefault(0);
    /** Creates a new document, a node and set it as a root node
     */
    doc = xmlNewDoc(BAD_CAST "1.0");
    if (doc == NULL)
        u32Ret = OLERR_OUT_OF_MEMORY;

    if (u32Ret == OLERR_NO_ERROR)
    {
        root = xmlNewNode(NULL, BAD_CAST "servMgmtSetting");
        if (root == NULL)
            u32Ret = OLERR_OUT_OF_MEMORY;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        xmlDocSetRootElement(doc, root);

        u32Ret = _servmgmtXmlNewChild(root, "version", "1.0", NULL);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _servmgmtXmlNewChild(
            root, "serviceSetting", NULL, &setting_node);

    for (u32Index = 0;
         (u32Index < pisms->isms_u32NumOfService) && (u32Ret == OLERR_NO_ERROR);
         u32Index ++)
    {
        pisi = &pisms->isms_isiService[u32Index];

        u32Ret = _servmgmtXmlNewChild(
            setting_node, "service", NULL, &serv_node);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "name", pisi->isi_strName, NULL);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "startupType",
                (olchar_t *)getStringServMgmtServStartupType(
                    pisi->isi_u8StartupType), NULL);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "role",
                _getStringServRole(pisi->isi_u8Role), NULL);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "cmdPath", pisi->isi_strCmdPath, NULL);

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_sprintf(str, "%u", pisi->isi_u8RestartDelay);
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "restartDelay", str, NULL);
        }

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _servmgmtXmlNewChild(
                serv_node, "cmdParam", pisi->isi_strCmdParam, NULL);
    }

    if (u32Ret == OLERR_NO_ERROR)
        xmlSaveFormatFileEnc(pisms->isms_strSettingFile, doc, "UTF-8", 1);

    if (doc != NULL)
    {
        /*free the document */
        xmlFreeDoc(doc);
        doc = NULL;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


