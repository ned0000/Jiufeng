/**
 *  @file servmgmt.c
 *
 *  @brief The service management routine
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
#include "jf_mem.h"
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

/**
 *  Retry service start delay in second. After failing to start service, wait for seconds to retry
 *  starting service 
 */
#define RETRY_START_SERV_DELAY   (3)

/**
 *  Default setting file name
 */
#define SERV_MGMT_SETTING_FILE   "../config/servmgmt.setting"

typedef struct
{
    /*the shared memory contains the status of all services*/
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

    jf_logger_logInfoMsg("read serv setting: %s", pisms->isms_strSettingFile);

    u32Ret = readServMgmtSetting(pisms);

    return u32Ret;
}

static u32 _writeServMgmtSetting(internal_serv_mgmt_setting_t * pisms)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("write serv setting, %s", pisms->isms_strSettingFile);

    u32Ret = writeServMgmtSetting(pisms);

    return u32Ret;
}

static u32 _startServMgmtServ(internal_serv_mgmt_t * pism, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strCmdLine[2 * JF_LIMIT_MAX_PATH_LEN + 128];

    ol_memset(strCmdLine, 0, sizeof(strCmdLine));

    ol_snprintf(
        strCmdLine, sizeof(strCmdLine) - 1, "%s %s", pisi->isi_strCmdPath, pisi->isi_strCmdParam);

    jf_logger_logInfoMsg("start serv, %s", strCmdLine);
    
    u32Ret = jf_process_create(&(pisi->isi_jpiProcessId), NULL, strCmdLine);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pisi->isi_u8Status = JF_SERV_STATUS_RUNNING;
    }
    else
    {
        jf_logger_logErrMsg(u32Ret, "start serv");
        pisi->isi_u8Status = JF_SERV_STATUS_ERROR;
    }

    return u32Ret;
}

static u32 _freeServMgmtUtimerItem(void ** ppServUtimer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mem_free(ppServUtimer);

    return u32Ret;
}

static u32 _newServMgmtUtimerItem(
    serv_mgmt_utimer_t ** ppServUtimer, internal_serv_mgmt_t * pism,
    internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    serv_mgmt_utimer_t * psmu = NULL;

    u32Ret = jf_mem_calloc((void **)&psmu, sizeof(serv_mgmt_utimer_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
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

    jf_logger_logInfoMsg(
        "utimer start serv %s, status: %s", pisi->isi_strName,
        getStringServStatus(pisi->isi_u8Status));

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

    jf_logger_logInfoMsg(
        "try to start service %s, restart count: %d", pisi->isi_strName,
        pisi->isi_u8RestartCount);

    if (pisi->isi_u8RestartCount < pisms->isms_u8FailureRetryCount)
    {
        u32Delay = pisi->isi_u8RestartCount;
        pisi->isi_u8RestartCount ++;

        if (u32Delay == 0)
        {
            u32Ret = _startServMgmtServ(pism, pisi);
        }
        else
        {
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
        /*several retry fails, set the status to error */
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

    jf_logger_logInfoMsg("start all serv");

    for (u32Index = 0;
         (u32Index < pisms->isms_u16NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index++)
    {
        pisi = &(pisms->isms_isiService[u32Index]);

        pisi->isi_u8Status = JF_SERV_STATUS_STOPPED;

        if (pisi->isi_u8StartupType == JF_SERV_STARTUP_TYPE_AUTOMATIC)
            u32Ret = _startServMgmtServ(pism, pisi);
    }

    return u32Ret;
}

static u32 _stopServMgmtServ(
    internal_serv_mgmt_setting_t * pisms, internal_service_info_t * pisi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("stop serv %s", pisi->isi_strName);

    u32Ret = jf_process_terminate(&(pisi->isi_jpiProcessId));
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

    jf_logger_logInfoMsg("stop all serv");

    for (u32Index = 0;
         (u32Index < pisms->isms_u16NumOfService) && (u32Ret == JF_ERR_NO_ERROR);
         u32Index++)
    {
        pisi = &(pisms->isms_isiService[u32Index]);

        if (pisi->isi_u8Status == JF_SERV_STATUS_RUNNING)
            u32Ret = _stopServMgmtServ(pisms, pisi);

        pisi->isi_u8Status = JF_SERV_STATUS_STOPPED;
    }

    return u32Ret;
}

static void _dumpServMgmtInfo(internal_service_info_t * pisi)
{
    jf_logger_logInfoMsg(
        "serv: %s, startuptype: %s, status: %s",
        pisi->isi_strName, getStringServStartupType(pisi->isi_u8StartupType),
        getStringServStatus(pisi->isi_u8Status));
}

static u32 _waitForChildProcess(
    internal_serv_mgmt_t * pism, jf_process_id_t pid[], u32 u32Count, boolean_t * pbRetry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex, u32Index, u32Reason;
    internal_service_info_t * pisi = NULL;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    s32 ret = 0;

    jf_logger_logInfoMsg("wait for child");

    u32Ret = jf_process_waitForChildProcessTermination(
        pid, u32Count, 0, &u32Index, &u32Reason);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_logger_logInfoMsg("wait for child, terminated by %u", u32Reason);

        jf_mutex_acquire(&pism->ism_jmLock);

        for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
        {
            pisi = &(pisms->isms_isiService[u32ServIndex]);

            ret = ol_memcmp(&(pid[u32Index]), &pisi->isi_jpiProcessId, sizeof(jf_process_id_t));
            if ((pisi->isi_u8Status == JF_SERV_STATUS_RUNNING) && (ret == 0))
            {
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
    jf_process_id_t pid[JF_SERV_MAX_NUM_OF_SERV];
    u32 u32ServIndex, u32Count;
    internal_service_info_t * pisi = NULL;

    jf_logger_logInfoMsg("handle child process");

    jf_mutex_acquire(&pism->ism_jmLock);

    for (u32Count = 0, u32ServIndex = 0;
         u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &(pisms->isms_isiService[u32ServIndex]);

        if ((pisi->isi_u8Status == JF_SERV_STATUS_RUNNING) &&
            jf_process_isValidId(&pisi->isi_jpiProcessId))
        {
            ol_memcpy(&pid[u32Count], &pisi->isi_jpiProcessId, sizeof(jf_process_id_t));
            u32Count ++;
        }
    }

    jf_mutex_release(&pism->ism_jmLock);

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
        bRetry = FALSE;

        _handleChildProcess(&bRetry);

    } while (bRetry);

    return u32Ret;
}

static u32 _initServMgmt(internal_serv_mgmt_t * pism, serv_mgmt_init_param_t * psmip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;

    ol_memset(pism, 0, sizeof(*pism));

    if (psmip->smip_pstrSettingFile != NULL)
        ol_strncpy(
            pisms->isms_strSettingFile, psmip->smip_pstrSettingFile, JF_LIMIT_MAX_PATH_LEN - 1);
    else
        ol_strncpy(
            pisms->isms_strSettingFile, SERV_MGMT_SETTING_FILE, JF_LIMIT_MAX_PATH_LEN - 1);

    u32Ret = _readServMgmtSetting(pisms);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_mutex_init(&pism->ism_jmLock);
    
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_network_createUtimer(psmip->smip_pjncChain, &pism->ism_pjnuUtimer);

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

    for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &pisms->isms_isiService[u32ServIndex];

        if (ol_strcmp(pisi->isi_strName, pstrName) == 0)
        {
            *ppServ = pisi;
            break;
        }
    }

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

    jf_logger_logInfoMsg("init serv mgmt");

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

    jf_logger_logInfoMsg("fini serv mgmt");

    if (pism->ism_pjnuUtimer != NULL)
        jf_network_destroyUtimer(&pism->ism_pjnuUtimer);

    /*ignore SIGCHLD before stopping daemon*/
    jf_process_ignoreSignal(SIGCHLD);
    
    u32Ret = _stopAllServices(pism);

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

    ol_memset(pjsi, 0, sizeof(*pjsi));

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(pjsi->jsi_strName, pisi->isi_strName);
        pjsi->jsi_u8Status = pisi->isi_u8Status;
        pjsi->jsi_u8StartupType = pisi->isi_u8StartupType;

    }

    jf_mutex_release(&pism->ism_jmLock);

    return u32Ret;
}

u32 getServMgmtServInfoList(jf_serv_info_list_t * pjsil)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32ServIndex;
    internal_serv_mgmt_t * pism = &ls_ismServMgmt;
    internal_serv_mgmt_setting_t * pisms = &pism->ism_ismsSetting;
    internal_service_info_t * pisi = NULL;
    jf_serv_info_t * pjsi = NULL;

    jf_mutex_acquire(&pism->ism_jmLock);
    
    for (u32ServIndex = 0; u32ServIndex < pisms->isms_u16NumOfService; u32ServIndex ++)
    {
        pisi = &pisms->isms_isiService[u32ServIndex];

        pjsi = &pjsil->jsil_jsiService[pjsil->jsil_u16NumOfService];

        ol_memset(pjsi, 0, sizeof(*pjsi));
        ol_strcpy(pjsi->jsi_strName, pisi->isi_strName);
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

    jf_logger_logInfoMsg(
        "set serv %s startup type to %s", pstrName, getStringServStartupType(u8StartupType));

    jf_mutex_acquire(&pism->ism_jmLock);

    u32Ret = _findServMgmtServ(pstrName, pisms, &pisi);

    if (pisi->isi_u8StartupType != u8StartupType)
    {
        pisi->isi_u8StartupType = u8StartupType;

        u32Ret = _writeServMgmtSetting(pisms);
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
        jf_logger_logInfoMsg("get signal SIGCHLD");

        if (pism->ism_pjnuUtimer != NULL)
            jf_network_addUtimerItem(
                pism->ism_pjnuUtimer, pism, 0, _handleSigchldForChildProcess, NULL);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

