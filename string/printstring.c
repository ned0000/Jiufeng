/**
 *  @file printstring.c
 *
 *  @brief Implementation file for routines printing string.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_string.h"
#include "jf_err.h"

#include "stringcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

const olchar_t * jf_string_getStringNotApplicable(void)
{
    return JF_STRING_NA;
}

const olchar_t * jf_string_getStringNotSupported(void)
{
    return JF_STRING_NOT_SUPPORTED;
}

u32 jf_string_getStringWWN(olchar_t * pstrWwn, olsize_t sWwn, const u64 u64WWN)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pByte = NULL;

    assert(pstrWwn != NULL);

    pByte = (u8 *)&u64WWN;
    ol_snprintf(
        pstrWwn, sWwn, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
        pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5], pByte[6], pByte[7]);

    pstrWwn[sWwn - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getStringU64(olchar_t * pstrInteger, olsize_t sInteger, const u64 u64Integer)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pstrInteger != NULL);

#ifdef WINDOWS
    ol_snprintf(pstrInteger, sInteger, "%I64u", u64Integer);
#else
    ol_snprintf(pstrInteger, sInteger, "%lld", u64Integer);
#endif

    pstrInteger[sInteger - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getStringMacAddress(olchar_t * pstrMacAddr, olsize_t sMacAddr, const u8 * pu8Mac)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_snprintf(
        pstrMacAddr, sMacAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
        pu8Mac[0], pu8Mac[1], pu8Mac[2], pu8Mac[3], pu8Mac[4], pu8Mac[5]);
    pstrMacAddr[sMacAddr - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getByteStringSize(olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Size1 = u64Size, u64Mod1 = 0;

    if (u64Size <= 1)
    {
        /*size <= 1 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_KILOBYTE)
    {
        /*size < 1024 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE)
    {
        /*size < 1048576 Byte*/
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE);
        /*2 digits after decimal point, so 100 is used here, rounding is required.*/
        u64Mod1 = ((u64Mod1 * 100) + (ONE_KILOBYTE / 2)) / ONE_KILOBYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "KB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE)
    {
        /*size < 1073741824 Byte*/
        u64Size1 = u64Size / ONE_MEGABYTE;
        u64Mod1 = u64Size % ONE_MEGABYTE;
        u64Mod1 = (((u64Mod1 * 100) + (ONE_MEGABYTE / 2)) / ONE_MEGABYTE);/* adjust rounding */
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "MB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE)
    {
        /*size < 1099511627776 Byte*/
        u64Size1 = u64Size / ONE_GIGABYTE;
        u64Mod1 = u64Size % ONE_GIGABYTE;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_GIGABYTE / 2)) / ONE_GIGABYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "GB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        /*size >= 1099511627776 Byte*/
        u64Size1 = u64Size / ONE_TEGABYTE;
        u64Mod1 = u64Size % ONE_TEGABYTE;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_TEGABYTE / 2)) / ONE_TEGABYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "TB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }

    pstrSize[sStrSize - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getByteStringSizeMax(olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Size1 = u64Size, u64Mod1 = 0;

    if (u64Size <= 1)
    {
        /*size <= 1 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_KILOBYTE)
    {
        /*size < 1024 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE)
    {
        /*size < 1048576 Byte*/
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE);
        /*2 digits after decimal point, so 100 is used here, no rounding.*/
        u64Mod1 = (u64Mod1 * 100) / ONE_KILOBYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "KB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE)
    {
        /*size < 1073741824 Byte*/
        u64Size1 = u64Size / ONE_MEGABYTE;
        u64Mod1 = u64Size % ONE_MEGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_MEGABYTE;/* adjust rounding */
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "MB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE)
    {
        /*size < 1099511627776 Byte*/
        u64Size1 = u64Size / ONE_GIGABYTE;
        u64Mod1 = u64Size % ONE_GIGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_GIGABYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        /*size >= 1099511627776 Byte*/
        u64Size1 = u64Size / ONE_TEGABYTE;
        u64Mod1 = u64Size % ONE_TEGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_TEGABYTE;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "TB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }

    pstrSize[sStrSize - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getByteStringSize1000Based(olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Size1 = u64Size, u64Mod1 = 0;

    if (u64Size <= 1)
    {
        /*size <= 1 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_KILOBYTE_1000_BASED)
    {
        /*size < 1,000 Byte*/
        ol_snprintf(pstrSize, sStrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE_1000_BASED)
    {
        /*size < 1,000,000 Byte*/
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE_1000_BASED);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE_1000_BASED);
        /*2 digits after decimal point, so 100 is used here, rounding is required.*/
        u64Mod1 = ((u64Mod1 * 100) + (ONE_KILOBYTE_1000_BASED / 2)) / ONE_KILOBYTE_1000_BASED;
        if (u64Mod1 == 0 )
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "KB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE_1000_BASED)
    {
        /*size < 1,000,000,000 Byte*/
        u64Size1 = u64Size / ONE_MEGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_MEGABYTE_1000_BASED;
        u64Mod1 = (((u64Mod1 * 100) + (ONE_MEGABYTE_1000_BASED / 2)) / ONE_MEGABYTE_1000_BASED);/* adjust rounding */
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "MB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE_1000_BASED)
    {
        /*size < 1,000,000,000,000 Byte*/
        u64Size1 = u64Size / ONE_GIGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_GIGABYTE_1000_BASED;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_GIGABYTE_1000_BASED / 2)) / ONE_GIGABYTE_1000_BASED;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "GB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        /*size >= 1,000,000,000,000 Byte*/
        u64Size1 = u64Size / ONE_TEGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_TEGABYTE_1000_BASED;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_TEGABYTE_1000_BASED / 2)) / ONE_TEGABYTE_1000_BASED;
        if (u64Mod1 == 0)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_snprintf(pstrSize, sStrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
            ol_snprintf(pstrSize, sStrSize, "%u%s", (u32)(u64Size1 + 1), "TB");
        else
            ol_snprintf(pstrSize, sStrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }

    pstrSize[sStrSize - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

u32 jf_string_getStringVersion(
    olchar_t * pstrVersion, olsize_t sVersion, const u8 u8Major, const u8 u8Minor,
    const u32 u32OEMCode, const u8 u8BuildNo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_snprintf(
        pstrVersion, sVersion, "%d.%02d.%04d.%02d", u8Major, u8Minor, u32OEMCode, u8BuildNo);
    /*Append a null-terminator any way.*/
    pstrVersion[sVersion - 1] = JF_STRING_NULL_CHAR;

    return u32Ret;
}

const olchar_t * jf_string_getStringEnable(const boolean_t bEnable)
{
    if (bEnable)
    {
        return JF_STRING_ENABLED;
    }
    else
    {
        return JF_STRING_DISABLED;
    }
}

const olchar_t * jf_string_getStringUnknown(void)
{
    return JF_STRING_UNKNOWN;
}

const olchar_t * jf_string_getStringPositive(const boolean_t bPositive)
{
    if (bPositive)
    {
        return JF_STRING_YES;
    }
    else
    {
        return JF_STRING_NO;
    }
}

const olchar_t * jf_string_getStringTrue(const boolean_t bTrue)
{
    if (bTrue)
    {
        return JF_STRING_TRUE;
    }
    else
    {
        return JF_STRING_FALSE;
    }
}

/*------------------------------------------------------------------------------------------------*/
