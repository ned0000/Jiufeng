/**
 *  @file logger/log2file.c
 *
 *  @brief Implementation file for logging to file.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)
    #include <sys/errno.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h" 
#include "jf_err.h"
#include "jf_limit.h"
#include "jf_mem.h"

#include "log2file.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the file log location data type.
 */
typedef struct
{
    /**Log file name.*/
    olchar_t lfll_strLogFileName[JF_LIMIT_MAX_PATH_LEN];

    /**Maximum size of the log file. Zero (0) means no limit.*/
    olsize_t lfll_sLogFile;

    /**FILE pointer to the log file.*/
    FILE * lfll_fpLogFile;

    /**The name of the calling module.*/
    olchar_t lfll_strCallerName[JF_LOGGER_MAX_CALLER_NAME_LEN];

} logger_file_log_location_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _logToFile(
    logger_file_log_location_t * plfll, boolean_t bBanner, olchar_t * pstrBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = 0;
    olint_t retry = 0, count = JF_LOGGER_RETRY_COUNT;

    while (retry < count)
    {
        if (bBanner)
            nRet = fprintf(plfll->lfll_fpLogFile, "%s%s\n", pstrBanner, pstrLog);
        else
            nRet = fprintf(plfll->lfll_fpLogFile, "%s\n", pstrLog);

        if (nRet > 0)
            break;

        /*Reopen the file and retry writing if there is error.*/
        fclose(plfll->lfll_fpLogFile);

        plfll->lfll_fpLogFile = fopen(plfll->lfll_strLogFileName, "a");
        if (plfll->lfll_fpLogFile == NULL)
            break;

        retry ++;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 logToFile(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_file_log_location_t * plfll = pLocation;
    olchar_t strBanner[256];

    if (bBanner)
        u32Ret = getCommonLogBanner(plfll->lfll_strCallerName, strBanner, sizeof(strBanner));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _logToFile(plfll, bBanner, strBanner, pstrLog);

    return u32Ret;
}

u32 saveLogToFile(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_file_log_location_t * plfll = pLocation;
    olchar_t strBanner[256];

    getSaveLogBanner(u64Time, pstrSource, strBanner, sizeof(strBanner));

    u32Ret = _logToFile(plfll, TRUE, strBanner, pstrLog);

    return u32Ret;
}

u32 destroyFileLogLocation(jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_file_log_location_t * plfll = *ppLocation;

    if (plfll->lfll_fpLogFile != NULL)
    {
        fclose(plfll->lfll_fpLogFile);
        plfll->lfll_fpLogFile = NULL;
    }

    jf_mem_free(ppLocation);

    return u32Ret;
}

u32 createFileLogLocation(
    create_file_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_file_log_location_t * plfll = NULL;

    u32Ret = jf_mem_alloc((void **)&plfll, sizeof(*plfll));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(plfll, sizeof(*plfll));

        plfll->lfll_sLogFile = pParam->cfllp_sLogFile;
        if (pParam->cfllp_pstrCallerName != NULL)
            ol_strcpy(plfll->lfll_strCallerName, pParam->cfllp_pstrCallerName);

        /*Use log file name then caller name then default caller name.*/
        if (pParam->cfllp_pstrLogFile != NULL)
        {
            ol_strncpy(
                plfll->lfll_strLogFileName, pParam->cfllp_pstrLogFile,
                sizeof(plfll->lfll_strLogFileName) - 1);
        }
        else if (pParam->cfllp_pstrCallerName != NULL)
        {
            ol_snprintf(
                plfll->lfll_strLogFileName, sizeof(plfll->lfll_strLogFileName) - 1, "%s.log",
                pParam->cfllp_pstrCallerName);
        }
        else
        {
            ol_snprintf(
                plfll->lfll_strLogFileName, sizeof(plfll->lfll_strLogFileName) - 1, "%s.log",
                JF_LOGGER_DEF_CALLER_NAME);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        plfll->lfll_fpLogFile = fopen(plfll->lfll_strLogFileName, "w");
        if (plfll->lfll_fpLogFile == NULL)
        {
#if defined(WINDOWS)
            ol_fprintf(stderr, "Init logger failed, cannot open file - (%d)\n", GetLastError());
#elif defined(LINUX)
            ol_fprintf(
                stderr, "Init logger failed, cannot open file - (%d) - %s\n", errno, strerror(errno));
#endif
            ol_fflush(stderr);
            u32Ret = JF_ERR_FAIL_OPEN_FILE;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLocation = plfll;
    else if (plfll != NULL)
        destroyFileLogLocation((void **)&plfll);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
