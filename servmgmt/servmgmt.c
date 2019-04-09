/**
 *  @file servmgmt.c
 *
 *  @brief The service management library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_mem.h"
#include "sharedmemory.h"
#include "jf_filestream.h"
#include "process.h"
#include "jf_attask.h"
#include "xtime.h"
#include "servmgmt.h"

#include "servmgmtsetting.h"

/* --- private data/data structure section --------------------------------- */

#define MAX_RESTART_COUNT       (10)

#define SERV_MGMT_SETTING_FILE  "servmgmt.setting"
#define SERV_MGMT_STATUS_FILE   "service.status"

/*the wait time to start a service when the termination of the service
  is detected, in second*/

typedef struct
{
    /*the shared memory contains the status of all services*/
    boolean_t ism_bInitialized;
    boolean_t ism_bToTerminate;
    u8 ism_u8Reserved[6];

    u32 ism_u32Reserved[4];
} internal_serv_mgmt_t;

/*for attask*/
typedef struct
{
    internal_serv_mgmt_t * sma_pismServMgmt;
    internal_serv_mgmt_setting_t * sma_pismsServSetting;
    internal_service_info_t * sma_pisiServInfo;
    u32 sma_u32Reserved[3];
} serv_mgmt_attask_t;

static internal_serv_mgmt_t ls_ismServMgmt;

/* --- private routine section---------------------------------------------- */

static u32 _readServMgmtSetting(
    internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("read serv setting, %s", pisms->isms_strSettingFile);

    u32Ret = readServMgmtSetting(pisms);

    return u32Ret;
}

static u32 _writeServMgmtSetting(
    internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("write serv setting, %s", pisms->isms_strSettingFile);

    u32Ret = writeServMgmtSetting(pisms);

    return u32Ret;
}

static u32 _startService(
    internal_serv_mgmt_t * pism, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strCmdLine[2 * JF_LIMIT_MAX_PATH_LEN + 128];

    jf_logger_logInfoMsg("start serv, %s", pisi->isi_strCmdPath);

    memset(strCmdLine, 0, sizeof(strCmdLine));

    ol_snprintf(
        strCmdLine, sizeof(strCmdLine) - 1, "%s %s",
        pisi->isi_strCmdPath, pisi->isi_strCmdParam);

    u32Ret = jf_process_create(&(pisi->isi_jpiProcessId), NULL, strCmdLine);
    if (u32Ret == JF_ERR_NO_ERROR)
        pisi->isi_u8Status = JF_SERVMGMT_SERV_STATUS_RUNNING;
    else
    {
        jf_logger_logErrMsg(u32Ret, "start serv");
        pisi->isi_u8Status = JF_SERVMGMT_SERV_STATUS_ERROR;
    }

    return u32Ret;
}

static u32 _freeServMgmtAttask(void ** ppServAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mem_free(ppServAttask);

    return u32Ret;
}

static u32 _newServMgmtAttask(
    serv_mgmt_attask_t ** ppServAttask, internal_serv_mgmt_t * pism,
    internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_attask_t * psma = NULL;

    u32Ret = jf_mem_calloc((void **)&psma, sizeof(serv_mgmt_attask_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        psma->sma_pismServMgmt = pism;
        psma->sma_pismsServSetting = pisms;
        psma->sma_pisiServInfo = pisi;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppServAttask = psma;
    else if (psma != NULL)
        _freeServMgmtAttask((void **)&psma);

    return u32Ret;
}

static u32 _lockStatusFile(const olchar_t * pstrFile, jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_file_open(pstrFile, O_RDONLY, pFile);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_file_lock(*pFile);
        if (u32Ret != JF_ERR_NO_ERROR)
            jf_file_close(pFile);
    }

    return u32Ret;
}

static u32 _unlockStatusFile(jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (*pFile != JF_FILE_INVALID_FILE_VALUE)
    {
        jf_file_unlock(*pFile);

        jf_file_close(pFile);
    }

    return u32Ret;
}

static u32 _attaskStartService(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_attask_t * psma = (serv_mgmt_attask_t *)pData;
    internal_service_info_t * pisi;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;

    jf_logger_logInfoMsg("attask start serv");

    pisi = psma->sma_pisiServInfo;

    _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);

    if (pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_ERROR)
        _startService(psma->sma_pismServMgmt, pisi);

    _unlockStatusFile(&fd);

    return u32Ret;
}

static u32 _tryStartService(
    jf_attask_t * pAttask, internal_serv_mgmt_t * pism,
    internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_attask_t * psma;
    u32 u32Delay;

    jf_logger_logInfoMsg("try to restart service %s", pisi->isi_strName);
    pisi->isi_u8Status = JF_SERVMGMT_SERV_STATUS_ERROR;

    u32Delay = pisi->isi_u8RestartDelay + pisi->isi_u8RestartCount;
    if (pisi->isi_u8RestartCount <= MAX_RESTART_COUNT)
    {
        pisi->isi_u8RestartCount ++;

        if (u32Delay == 0)
        {
            u32Ret = _startService(pism, pisi);
        }
        else
        {
            u32Ret = _newServMgmtAttask(&psma, pism, pisms, pisi);
            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = jf_attask_addItem(
                    pAttask, psma, u32Delay, _attaskStartService,
                    _freeServMgmtAttask);
        }
    }

    return u32Ret;
}

static u32 _startAllServices(
    internal_serv_mgmt_t * pism, internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index;
    internal_service_info_t * pisi;

    jf_logger_logInfoMsg("start all serv");

    for (u32Index = 0;
         (u32Index < pisms->isms_u32NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index++)
    {
        pisi = &(pisms->isms_isiService[u32Index]);

        pisi->isi_u8Status = JF_SERVMGMT_SERV_STATUS_STOPPED;

        if (pisi->isi_u8StartupType == JF_SERVMGMT_SERV_STARTUPTYPE_AUTOMATIC)
            u32Ret = _startService(pism, pisi);
    }

    return u32Ret;
}

static u32 _stopService(
    internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("stop serv %s", pisi->isi_strName);

    u32Ret = jf_process_terminate(&(pisi->isi_jpiProcessId));
    if (u32Ret == JF_ERR_NO_ERROR)
        pisi->isi_u8Status = JF_SERVMGMT_SERV_STATUS_STOPPED;

    return u32Ret;
}

static void _dumpServMgmtInfo(internal_service_info_t * pisi)
{
    jf_logger_logInfoMsg(
        "serv %s, role %d, startuptype %s, status %s",
        pisi->isi_strName, pisi->isi_u8Role,
        jf_servmgmt_getStringServStartupType(pisi->isi_u8StartupType),
        jf_servmgmt_getStringServStatus(pisi->isi_u8Status));
}

static u32 _waitForChildProcess(
    internal_serv_mgmt_t * pism, internal_serv_mgmt_setting_t * pisms,
    jf_attask_t * pAttask, u32 u32BlockTime, jf_process_id_t pid[], u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex, u32Index, u32Reason;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    internal_service_info_t * pisi;

    jf_logger_logInfoMsg("wait for child");

    u32Ret = jf_process_waitForChildProcessTermination(
        pid, u32Count, u32BlockTime, &u32Index, &u32Reason);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("wait for child, terminated by %u",
            u32Reason);

        _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);

        for (u32ServIndex = 0; u32ServIndex < pisms->isms_u32NumOfService;
             u32ServIndex ++)
        {
            pisi = &(pisms->isms_isiService[u32ServIndex]);

            _dumpServMgmtInfo(pisi);
            if (pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_STARTING)
            {
                _startService(pism, pisi);
                continue;
            }
            else if (pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_TERMINATED)
            {
                _tryStartService(pAttask, pism, pisms, pisi);
                continue;
            }
            else if (jf_process_isValidId(&pisi->isi_jpiProcessId) &&
                     ol_memcmp(&(pid[u32Index]), &pisi->isi_jpiProcessId,
                               sizeof(jf_process_id_t)) == 0 &&
                     (pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_RUNNING))
            {
                _tryStartService(pAttask, pism, pisms, pisi);
                continue;
            }
        }

        _unlockStatusFile(&fd);
    }

    return u32Ret;
}

static u32 _monitorServices(
    internal_serv_mgmt_t * pism, internal_serv_mgmt_setting_t * pisms,
    jf_attask_t * pAttask, u32 u32BlockTime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_process_id_t pid[JF_SERVMGMT_MAX_NUM_OF_SERVICE];
    u32 u32ServIndex, u32Count;
    internal_service_info_t * pisi;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;

    jf_logger_logInfoMsg("monitor serv");

    _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);

    for (u32Count = 0, u32ServIndex = 0;
         u32ServIndex < pisms->isms_u32NumOfService; u32ServIndex ++)
    {
        pisi = &(pisms->isms_isiService[u32ServIndex]);

        if ((pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_RUNNING) &&
            jf_process_isValidId(&pisi->isi_jpiProcessId))
        {
            ol_memcpy(
                &pid[u32Count], &pisi->isi_jpiProcessId, sizeof(jf_process_id_t));
            u32Count ++;
        }
    }

    _unlockStatusFile(&fd);

    if (u32Count > 0)
    {
        u32Ret = _waitForChildProcess(
            pism, pisms, pAttask, u32BlockTime, pid, u32Count);
    }
    else
    {
        jf_time_msleep(u32BlockTime);
    }

    return u32Ret;
}

static u32 _saveServiceStatus(
    internal_serv_mgmt_t * pism, jf_sharedmemory_id_t * pjsiServiceStatus)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;

    jf_logger_logInfoMsg("save service status");

    u32Ret = jf_filestream_open(SERV_MGMT_STATUS_FILE, "w", &fp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_writen(
            fp, pjsiServiceStatus, ol_strlen(pjsiServiceStatus));
    }

    if (fp != NULL)
        jf_filestream_close(&fp);

    return u32Ret;
}

static u32 _enterMonitorServices(
    internal_serv_mgmt_t * pism,
    internal_serv_mgmt_setting_t * pisms, jf_attask_t * pAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32BlockTime = INFINITE; /*in minisecond*/

    while (! pism->ism_bToTerminate)
    {
        jf_logger_logInfoMsg("enter monitor, %u", u32BlockTime);

        u32Ret = _monitorServices(pism, pisms, pAttask, u32BlockTime);

        jf_attask_check(pAttask, &u32BlockTime);
    }

    return u32Ret;
}

static u32 _wakeupServMgmt(internal_service_info_t * pisiWakeup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("wakeup servmgmt");

    if ((pisiWakeup->isi_u8Status == JF_SERVMGMT_SERV_STATUS_RUNNING) &&
        jf_process_isValidId(&pisiWakeup->isi_jpiProcessId))
    {
        jf_logger_logInfoMsg("wakeup servmgmt, %s", pisiWakeup->isi_strName);
        u32Ret = jf_process_terminate(&pisiWakeup->isi_jpiProcessId);
        if (u32Ret == JF_ERR_NO_ERROR)
            pisiWakeup->isi_u8Status = JF_SERVMGMT_SERV_STATUS_TERMINATED;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_servmgmt_init(jf_servmgmt_init_param_t * pjsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    assert(pjsip != NULL);

    jf_logger_logInfoMsg("init serv mgmt");

    if (u32Ret == JF_ERR_NO_ERROR)
        pism->ism_bInitialized = TRUE;
    else if (pism != NULL)
        jf_servmgmt_fini();

    return u32Ret;
}

u32 jf_servmgmt_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("fini serv mgmt");

    return u32Ret;
}

u32 jf_servmgmt_start(jf_servmgmt_start_param_t * pjssp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = NULL;
    /*the shared memory contains the status of all services*/
    jf_sharedmemory_id_t * pjsiServSetting = NULL;
    jf_attask_t * pAttask = NULL;

    jf_logger_logInfoMsg("start serv mgmt");

    assert(pjssp != NULL);

    u32Ret = jf_attask_create((jf_attask_t **)&pAttask);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_sharedmemory_create(
            &pjsiServSetting, sizeof(internal_serv_mgmt_setting_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_sharedmemory_attach(pjsiServSetting, (void **)&pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pisms, 0, sizeof(internal_serv_mgmt_setting_t));

        if (pjssp->jssp_pstrSettingFile != NULL)
            ol_strncpy(
                pisms->isms_strSettingFile,
                pjssp->jssp_pstrSettingFile, JF_LIMIT_MAX_PATH_LEN - 1);
        else
            ol_strncpy(
                pisms->isms_strSettingFile,
                SERV_MGMT_SETTING_FILE, JF_LIMIT_MAX_PATH_LEN - 1);

        u32Ret = _readServMgmtSetting(pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _startAllServices(pism, pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _saveServiceStatus(pism, pjsiServSetting);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _enterMonitorServices(pism, pisms, pAttask);
    }

    if (pisms != NULL)
    {
        jf_file_remove(SERV_MGMT_STATUS_FILE);

//        if (u32Ret == JF_ERR_NO_ERROR)
//            _writeServMgmtSetting(pism, pisms);

        jf_sharedmemory_detach((void **)&pisms);
    }

    if (pjsiServSetting != NULL)
        jf_sharedmemory_destroy(&pjsiServSetting);

    if (pAttask != NULL)
        jf_attask_destroy(&pAttask);

    pism->ism_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_servmgmt_stop(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    jf_logger_logInfoMsg("stop serv mgmt");

    pism->ism_bToTerminate = TRUE;

    return u32Ret;
}

u32 jf_servmgmt_getInfo(jf_servmgmt_info_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    internal_serv_mgmt_setting_t * pisms = NULL;
    olchar_t strServ[60];
    olsize_t size = 60;
    jf_sharedmemory_id_t * pjsiServSetting = NULL;
    internal_service_info_t * pisi;
    jf_servmgmt_serv_info_t * pjssi;

    ol_memset(pjsi, 0, sizeof(*pjsi));

    u32Ret = _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_file_readn(fd, strServ, &size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjsiServSetting = strServ;

        u32Ret = jf_sharedmemory_attach(pjsiServSetting, (void **)&pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32ServIndex = 0;
             u32ServIndex < pisms->isms_u32NumOfService; u32ServIndex ++)
        {
            pisi = &pisms->isms_isiService[u32ServIndex];

            if (pisi->isi_u8Role == SERVICE_ROLE_EXTERNAL)
            {
                pjssi = &pjsi->jsi_jssiService[pjsi->jsi_u8NumOfService];

                ol_strcpy(pjssi->jssi_strName, pisi->isi_strName);
                pjssi->jssi_u8Status = pisi->isi_u8Status;
                pjssi->jssi_u8StartupType = pisi->isi_u8StartupType;

                pjsi->jsi_u8NumOfService ++;
            }
        }
    }

    _unlockStatusFile(&fd);

    if (pjsiServSetting != NULL)
        jf_sharedmemory_detach((void **)&pisms);

    return u32Ret;
}

u32 jf_servmgmt_stopServ(olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    internal_serv_mgmt_setting_t * pisms = NULL;
    olchar_t strServ[60];
    olsize_t size = 60;
    jf_sharedmemory_id_t * pjsiServSetting = NULL;
    internal_service_info_t * pisi;

    u32Ret = _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_file_readn(fd, strServ, &size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjsiServSetting = strServ;

        u32Ret = jf_sharedmemory_attach(pjsiServSetting, (void **)&pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32ServIndex = 0;
             u32ServIndex < pisms->isms_u32NumOfService; u32ServIndex ++)
        {
            pisi = &pisms->isms_isiService[u32ServIndex];

            if ((pisi->isi_u8Role == SERVICE_ROLE_EXTERNAL) &&
                ol_strcmp(pisi->isi_strName, pstrName) == 0)
            {
                if (pisi->isi_u8Status == JF_SERVMGMT_SERV_STATUS_RUNNING)
                    u32Ret = _stopService(pisms, pisi);

                break;
            }
        }

        if (u32ServIndex == pisms->isms_u32NumOfService)
            u32Ret = JF_ERR_NOT_FOUND;
    }

    _unlockStatusFile(&fd);

    if (pjsiServSetting != NULL)
        jf_sharedmemory_detach((void **)&pisms);

    return u32Ret;
}

u32 jf_servmgmt_startServ(olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    internal_serv_mgmt_setting_t * pisms = NULL;
    olchar_t strServ[60];
    olsize_t size = 60;
    jf_sharedmemory_id_t * pjsiServSetting = NULL;
    internal_service_info_t * pisi, * pisiServ = NULL, * pisiWakeup = NULL;

    u32Ret = _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_file_readn(fd, strServ, &size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjsiServSetting = strServ;

        u32Ret = jf_sharedmemory_attach(pjsiServSetting, (void **)&pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32ServIndex = 0;
             u32ServIndex < pisms->isms_u32NumOfService; u32ServIndex ++)
        {
            pisi = &pisms->isms_isiService[u32ServIndex];

            if ((pisiWakeup == NULL) &&
                (pisi->isi_u8Role == SERVICE_ROLE_WAKEUP))
            {
                pisiWakeup = pisi;
                continue;
            }

            if ((pisiServ == NULL) &&
                ol_strcmp(pisi->isi_strName, pstrName) == 0)
            {
                pisiServ = pisi;
                continue;
            }
        }

        if ((pisiServ == NULL) ||
            (pisiServ->isi_u8Role != SERVICE_ROLE_EXTERNAL))
            u32Ret = JF_ERR_NOT_FOUND;
    }

    if ((u32Ret == JF_ERR_NO_ERROR) &&
        (pisiServ->isi_u8Status != JF_SERVMGMT_SERV_STATUS_RUNNING))
    {
        if (pisiWakeup == NULL)
            u32Ret = JF_ERR_WAKEUP_SERV_NOT_FOUND;
        else
        {
            pisiServ->isi_u8Status = JF_SERVMGMT_SERV_STATUS_STARTING;

            _wakeupServMgmt(pisiWakeup);
        }
    }

    _unlockStatusFile(&fd);

    if (pjsiServSetting != NULL)
        jf_sharedmemory_detach((void **)&pisms);

    return u32Ret;
}

const olchar_t * jf_servmgmt_getStringServStatus(u8 u8Status)
{
    olchar_t * pstr = NULL;

    if (u8Status == JF_SERVMGMT_SERV_STATUS_STOPPED)
        pstr = "stopped";
    else if (u8Status == JF_SERVMGMT_SERV_STATUS_STARTING)
        pstr = "starting";
    else if (u8Status == JF_SERVMGMT_SERV_STATUS_RUNNING)
        pstr = "running";
    else if (u8Status == JF_SERVMGMT_SERV_STATUS_STOPPING)
        pstr = "stopping";
    else if (u8Status == JF_SERVMGMT_SERV_STATUS_ERROR)
        pstr = "error";
    else if (u8Status == JF_SERVMGMT_SERV_STATUS_TERMINATED)
        pstr = "terminated";
    else
        pstr = "unknown";

    return pstr;
}

const olchar_t * jf_servmgmt_getStringServStartupType(u8 u8StartupType)
{
    olchar_t * pstr = NULL;

    if (u8StartupType == JF_SERVMGMT_SERV_STARTUPTYPE_AUTOMATIC)
        pstr = "automatic";
    else if (u8StartupType == JF_SERVMGMT_SERV_STARTUPTYPE_MANUAL)
        pstr = "manual";
    else
        pstr = "unknown";

    return pstr;
}

u32 jf_servmgmt_getServStartupTypeFromString(
    const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (strcmp(pstrType, "automatic") == 0)
        *pu8StartupType = JF_SERVMGMT_SERV_STARTUPTYPE_AUTOMATIC;
    else if (strcmp(pstrType, "manual") == 0)
        *pu8StartupType = JF_SERVMGMT_SERV_STARTUPTYPE_MANUAL;
    else
        u32Ret = JF_ERR_INVALID_PARAM;

    return u32Ret;
}

u32 jf_servmgmt_setServStartupType(olchar_t * pstrName, u8 u8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    internal_serv_mgmt_setting_t * pisms = NULL;
    olchar_t strServ[60];
    olsize_t size = 60;
    jf_sharedmemory_id_t * pjsiServSetting = NULL;
    internal_service_info_t * pisi, * pisiServ = NULL;

    assert(u8StartupType == JF_SERVMGMT_SERV_STARTUPTYPE_AUTOMATIC ||
           u8StartupType == JF_SERVMGMT_SERV_STARTUPTYPE_MANUAL);

    jf_logger_logInfoMsg(
        "set serv %s startup type to %s", pstrName,
        jf_servmgmt_getStringServStartupType(u8StartupType));

    u32Ret = _lockStatusFile(SERV_MGMT_STATUS_FILE, &fd);
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    u32Ret = jf_file_readn(fd, strServ, &size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjsiServSetting = strServ;

        u32Ret = jf_sharedmemory_attach(pjsiServSetting, (void **)&pisms);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u32ServIndex = 0;
             u32ServIndex < pisms->isms_u32NumOfService; u32ServIndex ++)
        {
            pisi = &pisms->isms_isiService[u32ServIndex];

            if (ol_strcmp(pisi->isi_strName, pstrName) == 0)
            {
                pisiServ = pisi;
                break;
            }
        }

        if ((pisiServ == NULL) || (pisiServ->isi_u8Role != SERVICE_ROLE_EXTERNAL))
            u32Ret = JF_ERR_NOT_FOUND;
        else if (pisiServ->isi_u8StartupType != u8StartupType)
        {
            pisiServ->isi_u8StartupType = u8StartupType;
            u32Ret = _writeServMgmtSetting(pisms);
        }
    }

    _unlockStatusFile(&fd);

    if (pjsiServSetting != NULL)
        jf_sharedmemory_detach((void **)&pisms);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


