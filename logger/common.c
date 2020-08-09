/**
 *  @file logger/common.c
 *
 *  @brief The common routine shared in the logger library.
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
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_time.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

static olsize_t _getStringZeroTimeStamp(olchar_t * pstrStamp, olsize_t sStamp)
{
    olsize_t sStr = 0;

    sStr = ol_snprintf(pstrStamp, sStamp, "00/00/0000 00:00:00.000000");
    pstrStamp[sStamp - 1] = '\0';

    return sStr;
}

static olsize_t _getStringTimeStamp(
    struct tm * tmLocal, u32 microsecond, olchar_t * pstrStamp, olsize_t sStamp)
{
    olsize_t sStr = 0;

    sStr = ol_snprintf(
        pstrStamp, sStamp, "%02d/%02d/%04d %02d:%02d:%02d.%06u", tmLocal->tm_mon + 1,
        tmLocal->tm_mday, tmLocal->tm_year + 1900, tmLocal->tm_hour, tmLocal->tm_min,
        tmLocal->tm_sec, microsecond);

    pstrStamp[sStamp - 1] = '\0';

    return sStr;
}

/** Get time stamp string of the current time.
 *
 *  @note
 *  -# The time stamp is in the format of "mm/dd/yyyy hh:mm:ss".
 *
 *  @param pstrStamp [out] The string buffer where the time stamp will be returned.
 *  @param sStamp [in] Size of the string buffer.
 *
 *  @return Size of the time stamp string.
 */
static olsize_t _getLogTimeStamp(olchar_t * pstrStamp, olsize_t sStamp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sStr = 0;
    struct timeval tv;
    struct tm * tmLocal = NULL;

    u32Ret = jf_time_getTimeOfDay(&tv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        tmLocal = localtime(&tv.tv_sec);

        if (tmLocal != NULL)
            sStr = _getStringTimeStamp(tmLocal, (u32)tv.tv_usec, pstrStamp, sStamp);
        else
            u32Ret = JF_ERR_INVALID_TIME;
    }

    if (u32Ret != JF_ERR_NO_ERROR)
        sStr = _getStringZeroTimeStamp(pstrStamp, sStamp);

    return sStr;
}

/* --- public routine section ------------------------------------------------------------------- */

boolean_t isSysErrorCode(u32 u32ErrCode)
{
    boolean_t bRet = FALSE;

    if (u32ErrCode == JF_ERR_OPERATION_FAIL)
    {
        bRet = TRUE;
    }
    else
    {
        if (u32ErrCode & JF_ERR_CODE_FLAG_SYSTEM)
            bRet = TRUE;
    }

    return bRet;
}

u32 getCommonLogBanner(const olchar_t * pstrCallerName, olchar_t * pstrBanner, olsize_t sBanner)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strStamp[32];

    assert(sBanner >= JF_LOGGER_MAX_COMMON_BANNER_LEN);

    _getLogTimeStamp(strStamp, sizeof(strStamp));

    ol_snprintf(
        pstrBanner, sBanner, "%s [%s:%lu] ", strStamp, pstrCallerName, jf_thread_getCurrentId());
    pstrBanner[sBanner - 1] = '\0';

    return u32Ret;
}

olsize_t getSaveLogTimeStamp(const u64 u64Time, olchar_t * pstrStamp, olsize_t sStamp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sStr = 0;
    time_t tCurrent = 0;
    u32 msecond = 0;
    struct tm * tmLocal = NULL;

    /*Get second from micro-second.*/
    tCurrent = (time_t)(u64Time / JF_TIME_SECOND_TO_MICROSECOND);
    msecond = (u32)(u64Time % JF_TIME_SECOND_TO_MICROSECOND);

    tmLocal = localtime(&tCurrent);

    if (tmLocal != NULL)
        sStr = _getStringTimeStamp(tmLocal, msecond, pstrStamp, sStamp);
    else
        u32Ret = JF_ERR_INVALID_TIME;

    if (u32Ret != JF_ERR_NO_ERROR)
        sStr = _getStringZeroTimeStamp(pstrStamp, sStamp);

    return sStr;
}

u32 getSaveLogBanner(
    const u64 u64Time, const olchar_t * pstrSource, olchar_t * pstrBanner, olsize_t sBanner)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strStamp[32];

    assert(sBanner >= JF_LOGGER_MAX_COMMON_BANNER_LEN);

    getSaveLogTimeStamp(u64Time, strStamp, sizeof(strStamp));

    ol_snprintf(pstrBanner, sBanner, "%s %s ", strStamp, pstrSource);
    pstrBanner[sBanner - 1] = '\0';

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
