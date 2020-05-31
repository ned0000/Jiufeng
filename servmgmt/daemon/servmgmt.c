/**
 *  @file servmgmt/daemon/servmgmt.c
 *
 *  @brief The service management routine.
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
#include "jf_sharedmemory.h"
#include "jf_filestream.h"
#include "jf_process.h"
#include "jf_time.h"
#include "jf_serv.h"
#include "jf_thread.h"
#include "jf_mutex.h"

#include "servmgmtsetting.h"
#include "servmgmtcommon.h"
#include "servmgmt.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    boolean_t ism_bInitialized;
    u8 ism_u8Reserved[7];

    jf_network_utimer_t * ism_pjnuUtimer;

    jf_mutex_t ism_jmLock;
    internal_serv_mgmt_setting_t ism_ismsSetting;

} internal_serv_mgmt_t;

static internal_serv_mgmt_t ls_ismServMgmt;

/*for utimer*/
typedef struct
{
    internal_serv_mgmt_t * smu_pismServMgmt;
    internal_service_info_t * smu_pisiServInfo;
    u32 smu_u32Reserved[4];
} serv_mgmt_utimer_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _readServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("serv setting: %s", pisms->isms_strSettingFile);

    u32Ret = readServMgmtSetting(pisms);

    return u32Ret;
}

static u32 _startServMgmtServ(internal_serv_mgmt_t * pism, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strCmdLine[2 * JF_LIMIT_MAX_PATH_LEN + 128];

    ol_bzero(strCmdLine, sizeof(strCmdLine));

    if (pisi->isi_pstrCmdParam == NULL)
    {
        ol_snprintf(strCmdLine, sizeof(strCmdLine) - 1, "%s", pisi->isi_pstrCmdPath);
    }
    else
    {
        ol_snprintf(
            strCmdLine, sizeof(strCmdLine) - 1, "%s %s", pisi->isi_pstrCmdPath,
            pisi->isi_pstrCmdParam);
    }

    JF_LOGGER_INFO("serv: %s", strCmdLine);
    
    u32Ret = jf_process_create(&pisi->isi_jphHandle, NULL, strCmdLine);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pisi->isi_u8Status = JF_SERV_STATUS_RUNNING;
    }
    else
    {
        JF_LOGGER_ERR(u32Ret, "start serv");
        pisi->isi_u8Status = JF_SERV_STATUS_ERROR;
    }

    return u32Ret;
}

static u32 _freeServMgmtUtimerItem(void ** ppServUtimer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_jiukun_freeMemory(ppServUtimer);

    return u32Ret;
}

static u32 _newServMgmtUtimerItem(
    serv_mgmt_utimer_t ** ppServUtimer, internal_serv_mgmt_t * pism,
    internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_utimer_t * psmu = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&psmu, sizeof(serv_mgmt_utimer_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(psmu, sizeof(serv_mgmt_utimer_t));
        psmu->smu_pismServMgmt = pism;
        psmu->smu_pisiServInfo = pisi;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppServUtimer = psmu;
    else if (psmu != NULL)
        _freeServMgmtUtimerItem((void **)&psmu);

    return u32Ret;
}

static u32 _utimerStartService(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_utimer_t * psmu = (serv_mgmt_utimer_t *)pData;
    internal_service_info_t * pisi = NULL;

    pisi = psmu->smu_pisiServInfo;

    JF_LOGGER_DEBUG(
        "service: %s, status: %s", pisi->isi_pstrName, getStringServStatus(pisi->isi_u8Status));

    /*Start the service in "running" status, the service is failed to be started before.*/
    if (pisi->isi_u8Status == JF_SERV_STATUS_RUNNING)
        u32Ret = _startServMgmtServ(psmu->smu_pismServMgmt, pisi);

    return u32Ret;
}

static u32 _tryStartServMgmtServ(internal_serv_mgmt_t * pism, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    u32 u32Delay;
    serv_mgmt_utimer_t * psmu = NULL;

    JF_LOGGER_DEBUG(
        "service: %s, restart count: %d", pisi->isi_pstrName, pisi->isi_u8RestartCount);

    /*Donot restart the service if we have tried many times than expected.*/
    if (pisi->isi_u8RestartCount < pisms->isms_u8FailureRetryCount)
    {
        u32Delay = pisi->isi_u8RestartCount;
        pisi->isi_u8RestartCount ++;
        /*Check the configured delay time in second.*/
        if (u32Delay == 0)
        {
            /*No delay, start the service now.*/
            u32Ret = _startServMgmtServ(pism, pisi);
        }
        else
        {
            /*Delay, start a timer to start the service.*/
            u32Ret = _newServMgmtUtimerItem(&psmu, pism, pisi);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_network_addUtimerItem(
                    pism->ism_pjnuUtimer, psmu, u32Delay, _utimerStartService,
                    _freeServMgmtUtimerItem);
            }
        }
    }
    else
    {
        /*Several retry fails, set the status to error.*/
        pisi->isi_u8Status = JF_SERV_STATUS_ERROR;
    }

    return u32Ret;
}

static u32 _startAllServices(internal_serv_mgmt_t * pism)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;
    internal_service_info_t * pisi = NULL;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;

    JF_LOGGER_INFO("start all services");

    /*Start all service.*/
    for (u32Index = 0;
         (u32Index < pisms->isms_u16NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index++)
    {
        pisi = &(pisms->isms_isiService[u32Index]);

        pisi->isi_u8Status = JF_SERV_STATUS_STOPPED;

        /*Only start the service with automatic startup type.*/
        if (pisi->isi_u8StartupType == JF_SERV_STARTUP_TYPE_AUTOMATIC)
            u32Ret = _startServMgmtServ(pism, pisi);
    }

    return u32Ret;
}

static u32 _stopServMgmtServ(
    internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("service: %s", pisi->isi_pstrName);

    /*Terminate the process.*/
    u32Ret = jf_process_terminate(&pisi->isi_jphHandle);
    if (u32Ret == JF_ERR_NO_ERROR)
        pisi->isi_u8Status = JF_SERV_STATUS_STOPPED;

    return u32Ret;
}

static u32 _stopAllServices(internal_serv_mgmt_t * pism)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0;
    internal_service_info_t * pisi = NULL;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;

    JF_LOGGER_INFO("stop all serv");

    /*Stop all service.*/
    for (u32Index = 0;
         (u32Index < pisms->isms_u16NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index++)
    {
        pisi = &(pisms->isms_isiService[u32Index]);

        /*Only when the status of service is "running", the service should be stopped.*/
        if (pisi->isi_u8Status == JF_SERV_STATUS_RUNNING)
            u32Ret = _stopServMgmtServ(pisms, pisi);

        /*Update the service status to "stopped".*/
        pisi->isi_u8Status = JF_SERV_STATUS_STOPPED;
    }

    return u32Ret;
}

static void _dumpServMgmtInfo(internal_service_info_t * pisi)
{
    JF_LOGGER_INFO(
        "serv: %s, startuptype: %s, status: %s",
        pisi->isi_pstrName, getStringServStartupType(pisi->isi_u8StartupType),
        getStringServStatus(pisi->isi_u8Status));
}

static u32 _waitForChildProcess(
    internal_serv_mgmt_t * pism, jf_process_handle_t pid[], u32 u32Count, boolean_t * pbRetry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex, u32Index, u32Reason;
    internal_service_info_t * pisi = NULL;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    s32 ret = 0;

    JF_LOGGER_DEBUG("wait for child");

    /*Wait for terminated child process.*/
    u32Ret = jf_process_waitForChildProcessTermination(pid, u32Count, 0, &u32Index, &u32Reason);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("wait for child, terminated by %u", u32Reason);

        jf_mutex_acquire(&pism->ism_jmLock);

        /*Iterate the service array.*/
        for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
        {
            pisi = &(pisms->isms_isiService[u32ServIndex]);

            ret = ol_memcmp(&(pid[u32Index]), &pisi->isi_jphHandle, sizeof(jf_process_handle_t));
            if ((pisi->isi_u8Status == JF_SERV_STATUS_RUNNING) && (ret == 0))
            {
                /*The terminated process is found.*/
                _dumpServMgmtInfo(pisi);
                _tryStartServMgmtServ(pism, pisi);
                *pbRetry = TRUE;
                break;
            }
        }

        jf_mutex_release(&pism->ism_jmLock);
    }

    return u32Ret;
}

static u32 _handleChildProcess(boolean_t * pbRetry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    jf_process_handle_t pid[JF_SERV_MAX_NUM_OF_SERV];
    u32 u32ServIndex, u32Count;
    internal_service_info_t * pisi = NULL;

    JF_LOGGER_INFO("handle child process");

    jf_mutex_acquire(&pism->ism_jmLock);

    /*Build the process handle array.*/
    for (u32Count = 0, u32ServIndex = 0;
         u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &(pisms->isms_isiService[u32ServIndex]);

        if ((pisi->isi_u8Status == JF_SERV_STATUS_RUNNING) &&
            jf_process_isValidHandle(&pisi->isi_jphHandle))
        {
            ol_memcpy(&pid[u32Count], &pisi->isi_jphHandle, sizeof(jf_process_handle_t));
            u32Count ++;
        }
    }

    jf_mutex_release(&pism->ism_jmLock);

    /*Waiting for terminated child process.*/
    if (u32Count > 0)
    {
        u32Ret = _waitForChildProcess(pism, pid, u32Count, pbRetry);
    }

    return u32Ret;
}

static u32 _handleSigchldForChildProcess(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    boolean_t bRetry = FALSE;

    do
    {
        /*If child process is terminated, we should retry checking the other child process, the
          testing shows only 1 SIGCHLD is received when there are more than 1 processes are
          terminated.*/
        bRetry = FALSE;

        _handleChildProcess(&bRetry);

    } while (bRetry);

    return u32Ret;
}

static u32 _initServMgmt(internal_serv_mgmt_t * pism, serv_mgmt_init_param_t * psmip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;

    ol_bzero(pism, sizeof(*pism));

    /*Save the setting file for writting operation.*/
    ol_strncpy(
        pisms->isms_strSettingFile, psmip->smip_pstrSettingFile, JF_LIMIT_MAX_PATH_LEN - 1);

    /*Read the setting file.*/
    u32Ret = _readServMgmtSetting(pisms);

    /*Initialize the mutex lock.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pism->ism_jmLock);

    /*Create the utimer.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(psmip->smip_pjncChain, &pism->ism_pjnuUtimer, "dongyuan");

    /*Start all services in setting file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _startAllServices(pism);

    return u32Ret;
}

static u32 _findServMgmtServ(
    const olchar_t * pstrName, internal_serv_mgmt_setting_t * pisms,
    internal_service_info_t ** ppServ)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex = 0;
    internal_service_info_t * pisi = NULL;

    /*Iterate the service array to find the service by name.*/
    for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &pisms->isms_isiService[u32ServIndex];

        if (ol_strcmp(pisi->isi_pstrName, pstrName) == 0)
        {
            *ppServ = pisi;
            break;
        }
    }

    /*Iterate to the end of the array, the service is not found.*/
    if (u32ServIndex == pisms->isms_u16NumOfService)
        u32Ret = JF_ERR_SERV_NOT_FOUND;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initServMgmt(serv_mgmt_init_param_t * psmip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    assert(psmip != NULL);

    JF_LOGGER_DEBUG("setting file: %s", psmip->smip_pstrSettingFile);

    u32Ret = _initServMgmt(pism, psmip);

    if (u32Ret == JF_ERR_NO_ERROR)
        pism->ism_bInitialized = TRUE;
    else
        finiServMgmt();

    return u32Ret;
}

u32 finiServMgmt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    JF_LOGGER_DEBUG("fini");

    if (pism->ism_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&pism->ism_pjnuUtimer);

    /*Ignore SIGCHLD before stopping daemon.*/
    jf_process_ignoreSignal(SIGCHLD);

    /*Stop all service.*/
    _stopAllServices(pism);

    /*Free the resource in setting.*/
    freeServMgmtSetting(&pism->ism_ismsSetting);

    /*Finalize the mutex lock.*/
    jf_mutex_fini(&pism->ism_jmLock);

    pism->ism_bInitialized = FALSE;
    
    return u32Ret;
}

u32 startServMgmt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    return u32Ret;
}

u32 stopServMgmt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    return u32Ret;
}

u32 getServMgmtServInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;

    ol_bzero(pjsi, sizeof(*pjsi));

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(pjsi->jsi_strName, pisi->isi_pstrName);
        pjsi->jsi_u8Status = pisi->isi_u8Status;
        pjsi->jsi_u8StartupType = pisi->isi_u8StartupType;

    }

    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 getServMgmtServInfoList(jf_serv_info_list_t * pjsil)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex = 0;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;
    jf_serv_info_t * pjsi = NULL;

    ol_bzero(pjsil, sizeof(*pjsil));
    
    jf_mutex_acquire(&pism->ism_jmLock);
    
    /*Copy the service information from internal data structure to the list.*/
    for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &pisms->isms_isiService[u32ServIndex];

        pjsi = &pjsil->jsil_jsiService[pjsil->jsil_u16NumOfService];

        ol_bzero(pjsi, sizeof(*pjsi));
        ol_strcpy(pjsi->jsi_strName, pisi->isi_pstrName);
        pjsi->jsi_u8Status = pisi->isi_u8Status;
        pjsi->jsi_u8StartupType = pisi->isi_u8StartupType;

        pjsil->jsil_u16NumOfService ++;
    }

    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 stopServMgmtServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);

    if ((u32Ret == JF_ERR_NO_ERROR) && (pisi->isi_u8Status == JF_SERV_STATUS_RUNNING))
    {
        u32Ret = _stopServMgmtServ(pisms, pisi);
    }

    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 startServMgmtServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);
    
    if ((u32Ret == JF_ERR_NO_ERROR) && (pisi->isi_u8Status != JF_SERV_STATUS_RUNNING))
    {
        pisi->isi_u8Status = JF_SERV_STATUS_STARTING;

        u32Ret = _startServMgmtServ(&ls_ismServMgmt, pisi);
    }

    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 setServMgmtServStartupType(const olchar_t * pstrName, const u8 u8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;

    assert(u8StartupType == JF_SERV_STARTUP_TYPE_AUTOMATIC ||
           u8StartupType == JF_SERV_STARTUP_TYPE_MANUAL);

    JF_LOGGER_INFO(
        "service: %s startup type to %s", pstrName, getStringServStartupType(u8StartupType));

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pisi->isi_u8StartupType != u8StartupType)
        {
            pisi->isi_u8StartupType = u8StartupType;

            u32Ret = modifyServiceStartupType(pisms, pisi);
        }
    }
    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 handleServMgmtSignal(olint_t sig)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;

    if (sig == SIGCHLD)
    {
        JF_LOGGER_INFO("get signal SIGCHLD");

        if (pism->ism_pjnuUtimer != NULL)
            jf_network_addUtimerItem(
                pism->ism_pjnuUtimer, pism, 0, _handleSigchldForChildProcess, NULL);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
