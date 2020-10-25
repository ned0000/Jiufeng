/**
 *  @file log2tty.c
 *
 *  @brief Implementation file for logging to tty.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)
    #include <sys/errno.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h" 
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_limit.h"
#include "jf_mem.h"

#include "log2tty.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the file log location data type.
 */
typedef struct
{
    /**TTY file name.*/
    olchar_t ltll_strTtyFileName[JF_LIMIT_MAX_PATH_LEN];

    /**FILE descriptor to the TTY file.*/
    olint_t ltll_nTtyFd;

    /**The name of the calling module.*/
    olchar_t ltll_strCallerName[JF_LOGGER_MAX_CALLER_NAME_LEN];

} logger_tty_log_location_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _logToTty(
    logger_tty_log_location_t * pltll, boolean_t bBanner, olchar_t * pstrBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = 0;
    olint_t retry = 0, count = JF_LOGGER_RETRY_COUNT;

    while (retry < count)
    {
        if (bBanner)
            nRet = ol_dprintf(pltll->ltll_nTtyFd, "%s%s\n", pstrBanner, pstrLog);
        else
            nRet = ol_dprintf(pltll->ltll_nTtyFd, "%s\n", pstrLog);

        if (nRet > 0)
            break;

        /*Reopen the tty and retry writing.*/
        close(pltll->ltll_nTtyFd);

        pltll->ltll_nTtyFd = open(pltll->ltll_strTtyFileName, O_WRONLY);
        if (pltll->ltll_nTtyFd < 0)
            break;

        retry ++;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 createTtyLogLocation(
    create_tty_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_tty_log_location_t * pltll = NULL;

    assert(pParam->ctllp_pstrTtyFile != NULL);

    u32Ret = jf_mem_alloc((void **)&pltll, sizeof(*pltll));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pltll, sizeof(*pltll));
        if (pParam->ctllp_pstrCallerName != NULL)
            ol_strcpy(pltll->ltll_strCallerName, pParam->ctllp_pstrCallerName);

        ol_strncpy(
            pltll->ltll_strTtyFileName, pParam->ctllp_pstrTtyFile,
            sizeof(pltll->ltll_strTtyFileName) - 1);

        /*Open the tty device file.*/
        pltll->ltll_nTtyFd = open(pltll->ltll_strTtyFileName, O_WRONLY);
        if (pltll->ltll_nTtyFd < 0)
        {
            /*Failed to open device file.*/
            olchar_t strDesc[JF_ERR_MAX_DESCRIPTION_SIZE];

            u32Ret = JF_ERR_FAIL_OPEN_FILE;

            jf_err_readDescription(u32Ret, strDesc, sizeof(strDesc));

            ol_fprintf(
                stderr, "Failed to Initialize logger, cannot open tty \"%s\" - %s\n",
                pltll->ltll_strTtyFileName, strDesc);

            ol_fflush(stderr);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLocation = pltll;
    else if (pltll != NULL)
        destroyTtyLogLocation((void **)&pltll);

    return u32Ret;
}

u32 destroyTtyLogLocation(jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_tty_log_location_t * pltll = *ppLocation;

    if (pltll->ltll_nTtyFd >= 0)
    {
        close(pltll->ltll_nTtyFd);
        pltll->ltll_nTtyFd = -1;
    }

    jf_mem_free(ppLocation);

    return u32Ret;
}

u32 logToTty(jf_logger_log_location_t * pLocation, boolean_t bBanner, olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_tty_log_location_t * pltll = pLocation;
    olchar_t strBanner[256];

    if (bBanner)
        u32Ret = getCommonLogBanner(pltll->ltll_strCallerName, strBanner, sizeof(strBanner));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _logToTty(pltll, bBanner, strBanner, pstrLog);

    return u32Ret;
}

u32 saveLogToTty(
    jf_logger_log_location_t * pLocation, u64 u64Time, const olchar_t * pstrSource,
    olchar_t * pstrLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_tty_log_location_t * pltll = pLocation;
    olchar_t strBanner[256];

    getSaveLogBanner(u64Time, pstrSource, strBanner, sizeof(strBanner));

    u32Ret = _logToTty(pltll, TRUE, strBanner, pstrLog);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
