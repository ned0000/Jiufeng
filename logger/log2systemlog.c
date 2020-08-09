/**
 *  @file logger/log2systemlog.c
 *
 *  @brief Implementation file for logging to system log.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)
    #include <sys/errno.h>
    #include <syslog.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_mem.h"

#include "log2systemlog.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the systemlog log location data type.
 */
typedef struct
{
    u8 lsll_u8Reserved[64];

} logger_systemlog_log_location_t;

/* --- private routine section ------------------------------------------------------------------ */

#if defined(LINUX)
static olint_t _convertLogLevelForSyslog(u8 u8LogLevel)
{
    olint_t nLevel = LOG_INFO;

    switch (u8LogLevel)
    {
    case JF_LOGGER_TRACE_LEVEL_ERROR:
        nLevel = LOG_ERR;
        break;
    case JF_LOGGER_TRACE_LEVEL_WARN:
        nLevel = LOG_WARNING;
        break;
    case JF_LOGGER_TRACE_LEVEL_INFO:
        nLevel = LOG_INFO;
        break;
    case JF_LOGGER_TRACE_LEVEL_DEBUG:
    case JF_LOGGER_TRACE_LEVEL_DATA:
        nLevel = LOG_DEBUG;
        break;
    default:
        break;
    }

    return nLevel;
}
#endif

/* --- public routine section ------------------------------------------------------------------- */

u32 logToSystemlog(
    jf_logger_log_location_t * pLocation, u8 u8LogLevel, boolean_t bBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(WINDOWS)

    u32Ret = JF_ERR_NOT_IMPLEMENTED;

#elif defined(LINUX)
    olint_t nLevel = _convertLogLevelForSyslog(u8LogLevel);

    if (bBanner)
        syslog(nLevel, "[%lu] %s", jf_thread_getCurrentId(), pstrLog);
    else
        syslog(nLevel, "%s", pstrLog);
#endif

    return u32Ret;
}

u32 destroySystemlogLogLocation(jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mem_free(ppLocation);

    return u32Ret;
}

u32 createSystemlogLogLocation(
    create_systemlog_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_systemlog_log_location_t * plsll = NULL;

    u32Ret = jf_mem_alloc((void **)&plsll, sizeof(*plsll));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(plsll, sizeof(*plsll));

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLocation = plsll;
    else if (plsll != NULL)
        destroySystemlogLogLocation((void **)&plsll);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
