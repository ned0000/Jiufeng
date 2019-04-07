/**
 *  @file scanstring.c
 *
 *  @brief routines for scanning string 
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(LINUX)
    #include <errno.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "stringparse.h"
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static boolean_t _isDayInRange(olint_t year, olint_t month, olint_t day)
{
    olint_t dm = jf_date_getDaysOfMonth(year, month);

    if (day <= 0 || day > dm)
        return FALSE;

    return TRUE;
}

/** Get date from the string with the format yyyy%mm%dd where '%' is the
 *  seperator
 */
static u32 _getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay,
    olchar_t sep)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olsize_t size = 0;
    olchar_t * firstChar, * psubStr;
    olchar_t u8Data[100];
    u32 u32Value;

    memset(u8Data, 0, 100);
    ol_strncpy(u8Data, pstrDate, 99);
    firstChar = u8Data;

    /* year */
    psubStr = strchr(firstChar, sep);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (sscanf(firstChar, "%04d", &u32Value) == 1)
        {
            if (u32Value >= 1970 && u32Value <= 2037)
            {
                firstChar = psubStr + 1;
                *pYear = u32Value;
                u32Ret = JF_ERR_NO_ERROR;
            }
        }
    }
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        return u32Ret;
    }

    /* Month */
    u32Ret = JF_ERR_INVALID_DATE;
    psubStr = strchr(firstChar, sep);
    if(psubStr != NULL)
    {
        u32Ret = JF_ERR_INVALID_DATE;
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (sscanf(firstChar, "%02d", &u32Value) == 1)
        {
            if (u32Value <= 12 && u32Value >= 1)
            {
                firstChar = psubStr + 1;
                *pMon = u32Value;
                u32Ret = JF_ERR_NO_ERROR;
            }
        }
    }
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        return u32Ret;
    }

    /* Day */
    u32Ret = JF_ERR_INVALID_DATE;
    if (sscanf(firstChar, "%02d", &u32Value) == 1)
    {
        if (_isDayInRange(*pYear, *pMon, u32Value) == TRUE)
        {
            *pDay = u32Value;
            u32Ret = JF_ERR_NO_ERROR;
        }
    }

    return u32Ret;
}


/* --- public routine section ---------------------------------------------- */

u32 jf_string_getS32FromString(
    const olchar_t * pstrInteger, const olsize_t size, s32 * ps32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    s32 s32Value;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateIntegerString(pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%d", &s32Value) == 1))
            {
                *ps32Value = s32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getS32FromHexString(
    const olchar_t * pstrHex, const olsize_t size, s32 * ps32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    s32 s32Value;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateHexString(pstrHex, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrHex, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%x", &s32Value) == 1))
            {
                *ps32Value = s32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getU32FromString(
    const olchar_t * pstrInteger, const olsize_t size, u32 * pu32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateIntegerString(pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%u", &u32Value) == 1))
            {
                *pu32Value = u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateIntegerString(pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%u", &u32Value) == 1))
            {
                if (u32Value > 65535)
                    u32Ret = JF_ERR_INTEGER_OUT_OF_RANGE;
                else
                    *pu16Value = (u16)u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateIntegerString(pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%u", &u32Value) == 1))
            {
                if (u32Value > 255)
                    u32Ret = JF_ERR_INTEGER_OUT_OF_RANGE;
                else
                    *pu8Value = (u8)u32Value;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, long * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *numeric = strtol(temp_buf, (olchar_t **)&stop_str, 10);
        if (*stop_str != '\0')
        {
            // If strtol stopped somewhere other than the end, there was an error
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
        else
        {
            // Now just check errno to see if there was an error reported
#if defined(LINUX)
            if (errno == ERANGE)
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }
#endif
        }

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *numeric = strtoul(temp_buf, &stop_str, 10);
        if (*stop_str != '\0')
        {
            jf_string_free(&temp_buf);
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
        else
        {
            jf_string_free(&temp_buf);
#if defined(LINUX)
            if (errno != ERANGE)
            {
                if (memcmp(pstrInteger, "-", 1) == 0)
                {
                    u32Ret = JF_ERR_INVALID_INTEGER;
                }
            }
            else
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }
#endif
        }
    }

    return u32Ret;
}

u32 jf_string_getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
#if defined(WINDOWS)
        *pu64Value = _strtoui64(temp_buf, &stop_str, 10);
#elif defined(LINUX)
        *pu64Value = strtoull(temp_buf, &stop_str, 10);
#endif
        if (*stop_str != '\0')
        {
            jf_string_free(&temp_buf);
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
        else
        {
            jf_string_free(&temp_buf);
#if defined(LINUX)
            if (errno == ERANGE)
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }
#endif
        }
    }

    return u32Ret;
}

u32 jf_string_getS64FromString(
    const olchar_t * pstrInteger, const olsize_t size, s64 * ps64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
#if defined(WINDOWS)
        *ps64Value = _strtoi64(temp_buf, &stop_str, 10);
#elif defined(LINUX)
        *ps64Value = strtoll(temp_buf, &stop_str, 10);
#endif
        if (*stop_str != '\0')
        {
            jf_string_free(&temp_buf);
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
        else
        {
            jf_string_free(&temp_buf);
#if defined(LINUX)
            if (errno == ERANGE)
            {
                u32Ret = JF_ERR_INVALID_INTEGER;
            }
#endif
        }
    }

    return u32Ret;
}

u32 jf_string_getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary,
    olsize_t * psBinary)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value, i;
    olsize_t sLen = 0;
    olchar_t buffer[10], * pend;

    assert((pu8Binary != NULL) && (psBinary != NULL) && 
           (pstr != NULL) && (size != 0));

    u32Ret = jf_string_validateHexString(pstr, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        while (sLen < size)
        {
            ol_strncpy(buffer, &pstr[sLen], 8);
            buffer[8] = '\0';

            u32Value = strtoul(buffer, (olchar_t **)&pend, 16);
            for (i = 0; i < 4; i++)
            {
                * pu8Binary ++ = (u8) (u32Value);
                u32Value = u32Value >> 8;
            }
            sLen += 8;
        }

        * psBinary = (size + 1) / 2;
    }

    return u32Ret;
}

#ifdef WINDOWS
    #define ONE_TEGABYTE    0x10000000000i64
#else
    #define ONE_TEGABYTE    0x10000000000LL
#endif
#define ONE_GIGABYTE    0x40000000
#define ONE_MEGABYTE    0x100000
#define ONE_KILOBYTE    0x400

u32 jf_string_getSizeFromString(const olchar_t * pstrSize, u64 * pu64Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u64 u64Base, u64Integer, u64Fraction;
    olsize_t size = ol_strlen(pstrSize);
    olchar_t strInteger[8], strFraction[8];
    olchar_t * pstrDot = NULL;
    olint_t i, nCount = 0;

    assert((pstrSize != NULL) && (pu64Size != NULL));

    if ((toupper(pstrSize[size - 1]) != 'B') ||
        (size > 10) || (size < 2))
    {
        u32Ret = JF_ERR_INVALID_SIZE;
    }
    else
    {
        switch (toupper(pstrSize[size - 2]))
        {
        case 'T':
            u64Base = ONE_TEGABYTE; /* 1TB */
            size -= 2;
            break;
        case 'G':
            u64Base = ONE_GIGABYTE; /* 1GB */
            size -= 2;
            break;
        case 'M':
            u64Base = ONE_MEGABYTE; /* 1MB */
            size -= 2;
            break;
        case 'K':
            u64Base = ONE_KILOBYTE; /* 1KB */
            size -= 2;
            break;
        default:
            u64Base = 1; /* 1Byte */
            size -= 1;
            break;
        }

        // Allow one or more spaces between value and unit
        while (pstrSize[size - 1] == ' ')
            size--;

        for (i = 0; i < size; i ++)
            if (isdigit(pstrSize[i]) == 0)
            {
                if (pstrSize[i] == '.')
                    nCount++;
                else
                    return JF_ERR_INVALID_SIZE;
            }

        if (nCount > 1)  /* There can't be more than 1 dot */
            return JF_ERR_INVALID_SIZE;

        memset(strInteger, 0, 8);
        memset(strFraction, 0, 8);
        u64Integer = 0;
        u64Fraction = 0;
        pstrDot = strchr(pstrSize, '.');
        if (pstrDot != NULL)
        {
            ol_strncpy(strInteger, pstrSize, (pstrDot-pstrSize));
            if ((size - (pstrDot - pstrSize + 1)) >= 2)
            {
                ol_strncpy(strFraction, (pstrDot + 1), 2);
            }
            else if ((size - (pstrDot- pstrSize + 1)) == 1)
            {
                ol_strncpy(strFraction, (pstrDot + 1), 1);
                ol_strcat(strFraction, "0");
            }
            else
            {
                return JF_ERR_INVALID_SIZE;
            }
        }
        else
        {
            ol_strncpy(strInteger, pstrSize, size);
            ol_strcpy(strFraction, "0");
        }

#ifdef WINDOWS
        if ((sscanf(strInteger, "%I64u", &u64Integer) == 1) &&
            (sscanf(strFraction, "%I64u", &u64Fraction) == 1))
#else
        if ((sscanf(strInteger, "%lld", &u64Integer) == 1) &&
            (sscanf(strFraction, "%lld", &u64Fraction) == 1))
#endif
        {
            *pu64Size = u64Base * u64Integer;
            if (u64Fraction != 0)
            {
                if (u64Base < ONE_KILOBYTE)
                {
                    u32Ret = JF_ERR_INVALID_SIZE;
                }
                else
                {
                    *pu64Size += (u64Fraction * u64Base / 100);
                }
            }
        }
        else
        {
            u32Ret = JF_ERR_INVALID_SIZE;
        }
    }

    return u32Ret;
}

/** Get the time from the string with the format hour:minute:second
 *
 */
u32 jf_string_getTimeFromString(
    const olchar_t * pstrTime, olint_t * pHour, olint_t * pMin, olint_t * pSec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * firstChar, * psubStr, cCol = ':';
    olchar_t strTime[100];
    u32 u32Value;
    olsize_t size;

    memset(strTime, 0, sizeof(strTime));
    ol_strncpy(strTime, pstrTime, sizeof(strTime) - 1);
    firstChar = strTime;

    /* hour */
    psubStr = strchr(firstChar, cCol);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (sscanf(firstChar, "%02d", &u32Value) != 1)
        {
            return JF_ERR_INVALID_TIME;
        }
        else if (u32Value > 23)
        {
            return JF_ERR_INVALID_TIME;
        }
        else
        {
            firstChar = psubStr + 1;
            *pHour = u32Value;
        }
    }
    else
    {
        return JF_ERR_INVALID_TIME;
    }

    /* Minute */
    psubStr = strchr(firstChar, cCol);
    if(psubStr != NULL)
    {
        size = (u32)(psubStr - firstChar);
        firstChar[size] = 0;
        if (sscanf(firstChar, "%02d", &u32Value) != 1)
        {
            return JF_ERR_INVALID_TIME;
        }
        else if (u32Value >= 60)
        {
            return JF_ERR_INVALID_TIME;
        }
        else
        {
            firstChar = psubStr + 1;
            *pMin = u32Value;
        }
    }
    else
    {
        return JF_ERR_INVALID_TIME;
    }

    /* Second */
    if (sscanf(firstChar, "%02d", &u32Value) != 1)
    {
        return JF_ERR_INVALID_TIME;
    }
    else if (u32Value >= 60)
    {
        return JF_ERR_INVALID_TIME;
    }
    else
    {
        *pSec = u32Value;
    }

    return u32Ret;
}

/** Get date from the string with the format year/month/date like 2005/10/20
 */
u32 jf_string_getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olchar_t cSlash = '/';

    u32Ret = _getDateFromString(pstrDate, pYear, pMon, pDay, cSlash);

    return u32Ret;
}

/** Get date from the string with the format year-month-date like 2005-10-20
 */
u32 jf_string_getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay)
{
    u32 u32Ret = JF_ERR_INVALID_DATE;
    olchar_t cSlash = '-';

    u32Ret = _getDateFromString(pstrDate, pYear, pMon, pDay, cSlash);

    return u32Ret;
}

u32 jf_string_getMACAddressFromString(const olchar_t * pMACString, u8 * pu8Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size;
    olchar_t strTemp[32];
    olchar_t * firstChar, * psubStr;
    olchar_t cCo1 = ':';
    olint_t i, j, count = 0;
    u32 u32Value[6];

    j = 6;
    if (strlen(pMACString) < 17)
        return JF_ERR_INVALID_SETTING;

    memset(strTemp, 0, sizeof(strTemp));
    ol_strncpy(strTemp, pMACString, sizeof(strTemp) - 1);
    firstChar = strTemp;
    for (i = 0; i < j; i ++)
    {
        psubStr = strchr(firstChar, cCo1);
        if(psubStr != NULL)
        {
           size = (u32)(psubStr - firstChar);
           firstChar[size] = 0;
           if (sscanf(firstChar, "%x", &u32Value[i]) != 1)
           {
               return JF_ERR_INVALID_SETTING;
           }
           else
           {
               if (u32Value[i] <= 255)
               {
                   firstChar = psubStr + 1;
                   pu8Value[i] = u32Value[i];
                   count ++;
               }
               else
               {
                   return JF_ERR_INVALID_SETTING;
               }
            }
        }
        else
        {
            if (count == 5)
            {
                if (sscanf(firstChar, "%x", &u32Value[i]) != 1)
                {
                    return JF_ERR_INVALID_SETTING;
                }
                if (u32Value[i] <= 255)
                {
                    pu8Value[i] = u32Value[i];
                }
                else
                {
                    return JF_ERR_INVALID_SETTING;
                }
            }
            else
            {
                return JF_ERR_INVALID_SETTING;
            }
        }
    }

    if (pu8Value[0] == 0 && pu8Value[1] == 0 &&
        pu8Value[2] == 0 && pu8Value[3] == 0 && pu8Value[4] && pu8Value[5])
    {
        return JF_ERR_NULL_IP_ADDRESS;
    }

    return u32Ret;
}

olsize_t jf_string_getHexFromString(
    const olchar_t * pstr, const olsize_t size,
    u8 * pu8Hex, olsize_t sHexLen)
{
    olsize_t sLen = 0, start;
    u32 u32Hex;
    olchar_t u8Temp[8];
    
    assert((pstr != NULL) && (size > 0) &&
           (pu8Hex != NULL) && (sHexLen > 0));

    start = 0;
    ol_bzero(u8Temp, sizeof(u8Temp));
    while ((sLen < sHexLen) && (start + 2 <= size))
    {
        ol_strncpy(u8Temp, &(pstr[start]), 2);

        ol_sscanf(u8Temp, "%x", &u32Hex);
        pu8Hex[sLen] = (u8)u32Hex;

        start += 2;

        sLen ++;
    }

    return sLen;
}

u32 jf_string_getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olfloat_t flValue;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateFloatString(pstrFloat, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrFloat, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%f", &flValue) == 1))
            {
                *pflValue = flValue;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_FLOAT;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

u32 jf_string_getDoubleFromString(
    const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbValue;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_validateFloatString(pstrDouble, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrDouble, size);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((sscanf(temp_buf, "%lf", &dbValue) == 1))
            {
                *pdbValue = dbValue;
            }
            else
            {
                u32Ret = JF_ERR_INVALID_FLOAT;
            }

            jf_string_free(&temp_buf);
        }
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

