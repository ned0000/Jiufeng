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

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(LINUX)
    #include <errno.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_date.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_string_getS32FromString(
    const olchar_t * pstrInteger, const olsize_t size, s32 * ps32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getS32FromString(temp_buf, ps32Value);

        jf_string_free(&temp_buf);
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
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU32FromString(temp_buf, pu32Value);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU16FromString(temp_buf, pu16Value);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU8FromString(temp_buf, pu8Value);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, slong * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getLongFromString(temp_buf, numeric);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, ulong * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getUlongFromString(temp_buf, numeric);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU64FromString(temp_buf, pu64Value);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getS64FromString(
    const olchar_t * pstrInteger, const olsize_t size, s64 * ps64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrInteger, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getS64FromString(temp_buf, ps64Value);

        jf_string_free(&temp_buf);
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

u32 jf_string_getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrFloat, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getFloatFromString(temp_buf, pflValue);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

u32 jf_string_getDoubleFromString(
    const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstrDouble, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getDoubleFromString(temp_buf, pdbValue);

        jf_string_free(&temp_buf);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

