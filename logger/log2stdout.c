/**
 *  @file logger/log2stdout.c
 *
 *  @brief Implementation file for logging to stdout.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h" 
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_mem.h"

#include "log2stdout.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the stdout log location data type.
 */
typedef struct
{
    /**The name of the calling module.*/
    olchar_t lsll_strCallerName[JF_LOGGER_MAX_CALLER_NAME_LEN];

} logger_stdout_log_location_t;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 logToStdout(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_stdout_log_location_t * plsll = pLocation;
    olchar_t strBanner[256];

    if (bBanner)
    {
        /*Banner is required.*/
        u32Ret = getCommonLogBanner(plsll->lsll_strCallerName, strBanner, sizeof(strBanner));

        if (u32Ret == JF_ERR_NO_ERROR)
            ol_fprintf(stdout, "%s%s\n", strBanner, pstrLog);
    }
    else
    {
        /*No banner, output the message directly.*/
        ol_fprintf(stdout, "%s\n", pstrLog);
    }

    fflush(stdout);

    return u32Ret;
}

u32 saveLogToStdout(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    const olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strStamp[32];

    getSaveLogTimeStamp(u64Time, strStamp, sizeof(strStamp));

    ol_fprintf(stdout, "%s %s %s\n", strStamp, pstrSource, pstrLog);

    return u32Ret;
}

u32 destroyStdoutLogLocation(jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    logger_stdout_log_location_t * plsll = *ppLocation;

    jf_mem_free(ppLocation);

    return u32Ret;
}

u32 createStdoutLogLocation(
    create_stdout_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_stdout_log_location_t * plsll = NULL;

    u32Ret = jf_mem_alloc((void **)&plsll, sizeof(*plsll));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(plsll, sizeof(*plsll));
        if (pParam->csllp_pstrCallerName != NULL)
            ol_strcpy(plsll->lsll_strCallerName, pParam->csllp_pstrCallerName);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLocation = plsll;
    else if (plsll != NULL)
        destroyStdoutLogLocation((void **)&plsll);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
