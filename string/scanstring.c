/**
 *  @file scanstring.c
 *
 *  @brief Implementation file of routines for scanning string. 
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

#include "stringcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Maximum ID length, ol_strlen(olid_t string)
 */
#define MAX_ID_LENGTH                         (32)


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

u32 jf_string_getBooleanFromString(
    const olchar_t * pstr, const olsize_t size, boolean_t * pbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;

    *pbValue = FALSE;

    if (size == 0)
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_string_duplicateWithLen(&temp_buf, pstr, size);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_strcasecmp(temp_buf, JF_STRING_ENABLED) == 0) ||
            (ol_strcasecmp(temp_buf, JF_STRING_YES) == 0) ||
            (ol_strcasecmp(temp_buf, JF_STRING_TRUE) == 0))
            *pbValue = TRUE;
        else if ((ol_strcasecmp(temp_buf, JF_STRING_DISABLED) == 0) ||
                 (ol_strcasecmp(temp_buf, JF_STRING_NO) == 0) ||
                 (ol_strcasecmp(temp_buf, JF_STRING_FALSE) == 0))
            *pbValue = FALSE;
        else
            u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (temp_buf != NULL)
        jf_string_free(&temp_buf);

    return u32Ret;
}

u32 jf_string_getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary, olsize_t * psBinary)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value, i;
    olsize_t sLen = 0;
    olchar_t buffer[10], * pend;

    assert((pu8Binary != NULL) && (psBinary != NULL) && (pstr != NULL) && (size != 0));

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

u32 jf_string_getSizeFromByteString(const olchar_t * pstr, const olsize_t size, u64 * pu64Size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * temp_buf = NULL;
    u64 u64Base = 0, u64Integer = 0, u64Fraction = 0;
    olchar_t * pstrInteger = NULL, * pstrFraction = NULL;
    olsize_t sBuf = (olsize_t)size;

    assert((pstr != NULL) && (pu64Size != NULL));

    u32Ret = jf_string_duplicateWithLen(&temp_buf, pstr, sBuf);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((toupper(temp_buf[sBuf - 1]) != 'B') || (sBuf > 10) || (sBuf < 2))
            u32Ret = JF_ERR_INVALID_STRING;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        switch (toupper(temp_buf[sBuf - 2]))
        {
        case 'T':
            u64Base = ONE_TEGABYTE; /* 1TB */
            sBuf -= 2;
            break;
        case 'G':
            u64Base = ONE_GIGABYTE; /* 1GB */
            sBuf -= 2;
            break;
        case 'M':
            u64Base = ONE_MEGABYTE; /* 1MB */
            sBuf -= 2;
            break;
        case 'K':
            u64Base = ONE_KILOBYTE; /* 1KB */
            sBuf -= 2;
            break;
        default:
            u64Base = 1; /* 1Byte */
            sBuf -= 1;
            break;
        }

        if (sBuf == 0)
            u32Ret = JF_ERR_INVALID_STRING;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Allow one or more spaces between value and unit.*/
        temp_buf[sBuf] = '\0';
        jf_option_removeSpaceAfterString(temp_buf);
        sBuf = ol_strlen(temp_buf);
        
        u32Ret = jf_option_validateFloatString(temp_buf, sBuf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pstrInteger = temp_buf;
        pstrFraction = ol_strchr(temp_buf, '.');
        if (pstrFraction != NULL)
        {
            *pstrFraction = '\0';
            pstrFraction ++;
        }

        u32Ret = jf_option_getU64FromString(pstrInteger, &u64Integer);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (pstrFraction != NULL))
        u32Ret = jf_option_getU64FromString(pstrFraction, &u64Fraction);

    if (u32Ret == JF_ERR_NO_ERROR)
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

    if (temp_buf != NULL)
        jf_string_free(&temp_buf);

    return u32Ret;
}

u32 jf_string_getMacAddressFromString(const olchar_t * pMacString, u8 * pu8Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = 0;
    olchar_t strTemp[32];
    olchar_t * firstChar = NULL, * psubStr = NULL;
    olchar_t cCo1 = ':';
    olint_t i = 0, j = 0, count = 0;
    u32 u32Value[6];

    j = 6;
    if (strlen(pMacString) < 17)
        return JF_ERR_INVALID_SETTING;

    ol_bzero(strTemp, sizeof(strTemp));
    ol_strncpy(strTemp, pMacString, sizeof(strTemp) - 1);
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

u32 jf_string_getIdListFromString(const olchar_t * pstrIdList, olid_t * pids, olsize_t * psId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strId[MAX_ID_LENGTH];
    const olchar_t cDelimit = ',', cRange = '~';
    olchar_t * pstrList = NULL, * pstrRange = NULL, * pstrDelimit;
    olsize_t sLength = 0, sCount = 0;
    olsize_t sValue1 = 0, sValue2 = 0;
    olsize_t i = 0;

    assert((pids != NULL) && (psId != NULL));

    if (pstrIdList == NULL)
    {
        *psId = 0;
        return u32Ret;
    }
    else
    {
        /* check for illegal input */
        sLength = ol_strlen(pstrIdList);

        for(i = 0; i < (olint_t)sLength; i++)
        {
            if (isdigit(pstrIdList[i]))
                continue;
            else if (pstrIdList[i] == '~')
                continue;
            else if (pstrIdList[i] == ',')
                continue;
            else if (pstrIdList[i] == ' ')
                continue;
            else
            {
                u32Ret = JF_ERR_INVALID_SETTING;
                i = sLength;
            }
        }
    }

    sCount = 0;
    pstrList = (olchar_t *)pstrIdList;
    while ((pstrList != NULL) && (u32Ret == JF_ERR_NO_ERROR))
    {
        sLength = ol_strlen(pstrList);

        pstrDelimit = strchr(pstrList, cDelimit);
        if (pstrDelimit != NULL)
        {
            sLength = pstrDelimit - pstrList;
            ol_strncpy(strId, pstrList, sLength);
            strId[sLength] = 0;
            pstrList = pstrDelimit + 1;
            if (ol_strlen(pstrList) == 0)
            {
                pstrList = NULL;
            }
        }
        else
        {
            ol_strcpy(strId, pstrList);
            pstrList = NULL;
        }

        for (i = 0; i < (olint_t)strlen(strId); i++)
        {
            if ((isdigit(strId[i]) == 0) &&
                (strId[i] != cRange) &&
                (strId[i] != cDelimit))
                return JF_ERR_INVALID_SETTING;
        }

        pstrRange = strchr(strId, cRange);
        if (pstrRange != NULL)
        {
            if (sscanf(strId, "%d~%d", &sValue1, &sValue2) != 2)
            {
                u32Ret = JF_ERR_INVALID_SETTING;
            }
        }
        else
        {
            if (sscanf(strId, "%d", &sValue1) != 1)
            {
                u32Ret = JF_ERR_INVALID_SETTING;
            }
            else
            {
                sValue2 = sValue1;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (sValue2 < sValue1)
            {
                u32Ret = JF_ERR_INVALID_SETTING;
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /* fill the id array */
            while ((sCount < *psId) && (sValue1 <= sValue2))
            {
                pids[sCount] = (olid_t)sValue1;
                sCount++;
                sValue1++;
            }
        }

    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *psId = sCount;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
