/**
 *  @file logserver/daemon/logsave.c
 *
 *  @brief Log save implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_mutex.h"
#include "jf_string.h"
#include "jf_thread.h"
#include "jf_listhead.h"
#include "jf_time.h"

/*Defined in logger library.*/
#include "common.h"
#include "log2tty.h"
#include "log2stdout.h"
#include "log2file.h"

#include "logsave.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Cache time in second.
 */
#define LOG_SAVE_CACHE_TIME                   (2)

/** Define the log save log data type.
 */
typedef struct
{
    jf_listhead_t lsl_jlLog;

    u64 lsl_u64Time;
    olchar_t * lsl_pstrBanner;
    olchar_t * lsl_pstrLog;
} log_save_log_t;

/** Define the log save log save data type.
 */
typedef struct
{
    boolean_t ils_bInitialized;
    u8 ils_u8Reserved[6];
    boolean_t ils_bToTerminateThread;

    jf_listhead_t ils_jlLogList;
    u16 ils_u16NumOfLog;
    u8 ils_u8Reserved2[6];
    jf_mutex_t ils_jmLock;

    /**Log to the stdout.*/
    boolean_t ils_bLogToStdout;
    /**Log to a file.*/
    boolean_t ils_bLogToFile;
    /**Log to the specified TTY.*/
    boolean_t ils_bLogToTty;
    u8 ils_u8Reserved3[3];

    jf_logger_log_location_t * ils_pjlllTty;
    jf_logger_log_location_t * ils_pjlllStdout;
    jf_logger_log_location_t * ils_pjlllFile;
} internal_log_save_t;

/** The internal log save instance.
 */
static internal_log_save_t ls_ilsLogSave;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _destroyLogSaveLog(log_save_log_t ** ppLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_save_log_t * plsl = *ppLog;

    if (plsl->lsl_pstrBanner != NULL)
        jf_string_free(&plsl->lsl_pstrBanner);

    if (plsl->lsl_pstrLog != NULL)
        jf_string_free(&plsl->lsl_pstrLog);

    jf_jiukun_freeMemory((void **)ppLog);

    return u32Ret;
}

static u32 _createLogSaveLog(
    u64 u64Time, const olchar_t * pstrBanner, const olchar_t * pstrLog, log_save_log_t ** ppLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_save_log_t * plsl = NULL;

    *ppLog = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&plsl, sizeof(*plsl));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(plsl, sizeof(*plsl));
        jf_listhead_init(&plsl->lsl_jlLog);
        plsl->lsl_u64Time = u64Time;

        u32Ret = jf_string_duplicate(&plsl->lsl_pstrBanner, pstrBanner);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_duplicate(&plsl->lsl_pstrLog, pstrLog);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLog = plsl;
    else if (plsl != NULL)
        _destroyLogSaveLog(&plsl);

    return u32Ret;
}

static u32 _destroyLogList(internal_log_save_t * pils)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL, * temp = NULL;
    log_save_log_t * plsl = NULL;

    jf_listhead_forEachSafe(&pils->ils_jlLogList, pjl, temp)
    {
        plsl = jf_listhead_getEntry(pjl, log_save_log_t, lsl_jlLog);

        jf_listhead_del(&plsl->lsl_jlLog);

        _destroyLogSaveLog(&plsl);
    }

    return u32Ret;
}

static log_save_log_t * _getFirstLogInQueue(internal_log_save_t * pils)
{
    log_save_log_t * pLog = NULL;

    if (jf_listhead_isEmpty(&pils->ils_jlLogList))
        return NULL;

    pLog = jf_listhead_getEntry(pils->ils_jlLogList.jl_pjlNext, log_save_log_t, lsl_jlLog);

    return pLog;
}

static u32 _flushSavedLog(internal_log_save_t * pils, u64 u64Expire)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL, * temp = NULL;
    log_save_log_t * plsl = NULL;
    olchar_t strStamp[64];

    getSaveLogTimeStamp(u64Expire, strStamp, sizeof(strStamp));
    JF_LOGGER_DEBUG("expire: %s", strStamp);

    jf_listhead_forEachSafe(&pils->ils_jlLogList, pjl, temp)
    {
        plsl = jf_listhead_getEntry(pjl, log_save_log_t, lsl_jlLog);

        if (plsl->lsl_u64Time > u64Expire)
            break;

        getSaveLogTimeStamp(plsl->lsl_u64Time, strStamp, sizeof(strStamp));
        JF_LOGGER_DEBUG("flush, time: %s", strStamp);

        /*Remove the log from list.*/
        jf_listhead_del(&plsl->lsl_jlLog);
        pils->ils_u16NumOfLog --;

        /*Save log to log location.*/
        if (pils->ils_bLogToStdout)
            saveLogToStdout(
                pils->ils_pjlllStdout, plsl->lsl_u64Time, plsl->lsl_pstrBanner, plsl->lsl_pstrLog);
#if defined(LINUX)
        if (pils->ils_bLogToTty)
            saveLogToTty(
                pils->ils_pjlllTty, plsl->lsl_u64Time, plsl->lsl_pstrBanner, plsl->lsl_pstrLog);
#endif
        if (pils->ils_bLogToFile)
            saveLogToFile(
                pils->ils_pjlllFile, plsl->lsl_u64Time, plsl->lsl_pstrBanner, plsl->lsl_pstrLog);

        /*Destory the log.*/
        _destroyLogSaveLog(&plsl);
    }

    return u32Ret;
}

static u32 _tryFlushSavedLog(internal_log_save_t * pils)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Curr = 0;
    log_save_log_t * pLog = _getFirstLogInQueue(pils);

    /*No log in the list.*/
    if (pLog == NULL)
        return u32Ret;

    /*Get current time in micro-second.*/
    u32Ret = jf_time_getUtcTimeInMicroSecond(&u64Curr);

    /*Quit if failed to get current time.*/
    if (u32Ret != JF_ERR_NO_ERROR)
        return u32Ret;

    /*Quit if time interval is less than cache time.*/
    u64Curr -= LOG_SAVE_CACHE_TIME * JF_TIME_SECOND_TO_MICROSECOND;
    if (u64Curr < pLog->lsl_u64Time)
        return u32Ret;

    u32Ret = _flushSavedLog(pils, u64Curr);

    return u32Ret;
}

static JF_THREAD_RETURN_VALUE _saveLogThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = pArg;

    JF_LOGGER_DEBUG("enter");

    /*Start the server chain.*/
    while (! pils->ils_bToTerminateThread)
    {
        jf_mutex_acquire(&pils->ils_jmLock);
        _tryFlushSavedLog(pils);
        jf_mutex_release(&pils->ils_jmLock);

        jf_time_sleep(LOG_SAVE_CACHE_TIME);
    }

    JF_LOGGER_DEBUG("quit");

    JF_THREAD_RETURN(u32Ret);
}

static u32 _createSaveLogThread(internal_log_save_t * pils)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Start a thread to save the log.*/
    u32Ret = jf_thread_create(NULL, NULL, _saveLogThread, pils);

    return u32Ret;
}

static u32 _insertLogToList(internal_log_save_t * pils, log_save_log_t * plsl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pjl = NULL;
    log_save_log_t * pLog = NULL;

    jf_listhead_forEach(&pils->ils_jlLogList, pjl)
    {
        pLog = jf_listhead_getEntry(pjl, log_save_log_t, lsl_jlLog);

        if (pLog->lsl_u64Time > plsl->lsl_u64Time)
            break;
    }

    /*Not found, append to the tail of the list*/
    if (pjl == &pils->ils_jlLogList)
        jf_listhead_addTail(&pils->ils_jlLogList, &plsl->lsl_jlLog);
    else /*Found, intert to the list.*/
        jf_listhead_addTail(&pLog->lsl_jlLog, &plsl->lsl_jlLog);

    return u32Ret;
}

static u32 _addLogToList(internal_log_save_t * pils, log_save_log_t * plsl)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (jf_listhead_isEmpty(&pils->ils_jlLogList))
    {
        jf_listhead_add(&pils->ils_jlLogList, &plsl->lsl_jlLog);
    }
    else
    {
        u32Ret = _insertLogToList(pils, plsl);
    }

    pils->ils_u16NumOfLog ++;

    return u32Ret;
}

static u32 _createSaveLogLocation(internal_log_save_t * pils, log_save_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    if (plsip->lsip_bLogToTty)
    {
        create_tty_log_location_param_t ctllp;

        pils->ils_bLogToTty = TRUE;

        ol_bzero(&ctllp, sizeof(ctllp));
        ctllp.ctllp_pstrTtyFile = plsip->lsip_pstrTtyFile;

        u32Ret = createTtyLogLocation(&ctllp, &pils->ils_pjlllTty);
    }
#endif

    if (plsip->lsip_bLogToStdout)
    {
        create_stdout_log_location_param_t csllp;

        pils->ils_bLogToStdout = TRUE;

        ol_bzero(&csllp, sizeof(csllp));

        u32Ret = createStdoutLogLocation(&csllp, &pils->ils_pjlllStdout);
    }

    if (plsip->lsip_bLogToFile)
    {
        create_file_log_location_param_t cfllp;

        pils->ils_bLogToFile = TRUE;

        ol_bzero(&cfllp, sizeof(cfllp));
        cfllp.cfllp_pstrLogFile = plsip->lsip_pstrLogFile;
        cfllp.cfllp_sLogFile = plsip->lsip_sLogFile;

        u32Ret = createFileLogLocation(&cfllp, &pils->ils_pjlllFile);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initLogSave(log_save_init_param_t * plsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = &ls_ilsLogSave;

    assert(plsip != NULL);
    assert(! pils->ils_bInitialized);
    
    JF_LOGGER_DEBUG("init");

    ol_bzero(pils, sizeof(internal_log_save_t));
    jf_listhead_init(&pils->ils_jlLogList);

    u32Ret = jf_mutex_init(&pils->ils_jmLock);

    /*Create log location.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createSaveLogLocation(pils, plsip);

    /*Create worker thread to save log.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createSaveLogThread(pils);

    if (u32Ret == JF_ERR_NO_ERROR)
        pils->ils_bInitialized = TRUE;
    else
        finiLogSave();

    return u32Ret;
}

u32 finiLogSave(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = &ls_ilsLogSave;

    JF_LOGGER_DEBUG("fini");

    /*Flush the log list.*/
    _flushSavedLog(pils, U64_MAX);

    /*Destroy the log list.*/
    _destroyLogList(pils);

    /*Destroy log locations.*/
    if (pils->ils_pjlllStdout != NULL)
        destroyStdoutLogLocation(&pils->ils_pjlllStdout);
    if (pils->ils_pjlllFile != NULL)
        destroyFileLogLocation(&pils->ils_pjlllFile);

#if defined(LINUX)
    if (pils->ils_pjlllTty != NULL)
        destroyTtyLogLocation(&pils->ils_pjlllTty);
#endif

    jf_mutex_fini(&pils->ils_jmLock);

    pils->ils_bInitialized = FALSE;

    return u32Ret;
}

u32 startLogSave(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = &ls_ilsLogSave;

    JF_LOGGER_INFO("start");
    
    if (! pils->ils_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    return u32Ret;
}

u32 stopLogSave(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = &ls_ilsLogSave;

    if (! pils->ils_bInitialized)
        u32Ret = JF_ERR_NOT_INITIALIZED;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Stop the save log thread.*/
        pils->ils_bToTerminateThread = TRUE;

    }

    return u32Ret;
}

/** Save log to queue.
 */
u32 saveLogToQueue(
    u64 u64Time, const olchar_t * pstrBanner, const olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_log_save_t * pils = &ls_ilsLogSave;
    log_save_log_t * plsl = NULL;

    if (! pils->ils_bInitialized)
        return JF_ERR_NOT_INITIALIZED;

    /*Create log.*/
    u32Ret = _createLogSaveLog(u64Time, pstrBanner, pstrLog, &plsl);

    /*Add log to list.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_mutex_acquire(&pils->ils_jmLock);
        u32Ret = _addLogToList(pils, plsl);
        jf_mutex_release(&pils->ils_jmLock);
    }

    if ((u32Ret != JF_ERR_NO_ERROR) && (plsl != NULL))
        _destroyLogSaveLog(&plsl);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
