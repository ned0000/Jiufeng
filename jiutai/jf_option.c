/**
 *  @file jf_option.c
 *
 *  @brief routines for handling option 
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
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_option_validateIntegerString(const olchar_t * pstrInteger, const olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t i;

    for (i = 0; ((i < size) && (u32Ret == JF_ERR_NO_ERROR)); i++)
    {
        if (isdigit(pstrInteger[i]) == 0)
        {
            return JF_ERR_INVALID_INTEGER;
        }
    }

    return u32Ret;
}

u32 jf_option_validateFloatString(const olchar_t * pstrFloat, const olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t i;
    olchar_t * str = (olchar_t *)pstrFloat;
    olsize_t sizes = size;

    if (size < 1)
        return JF_ERR_INVALID_FLOAT;

    if (str[0] == '-')
    {
        str ++;
        sizes --;
    }

    for (i = 0; ((i < sizes) && (u32Ret == JF_ERR_NO_ERROR)); i++)
    {
        if (str[i] == 'e')
            break;

        if ((isdigit(str[i]) == 0) && (str[i] != '.'))
        {
            return JF_ERR_INVALID_FLOAT;
        }
    }

    return u32Ret;
}

u32 jf_option_getS32FromString(const olchar_t * pstrInteger, s32 * ps32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    s32 s32Value;

    u32Ret = jf_option_validateIntegerString(pstrInteger, ol_strlen(pstrInteger));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrInteger, "%d", &s32Value) == 1))
        {
            *ps32Value = s32Value;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
    }

    return u32Ret;
}

u32 jf_option_getU32FromString(const olchar_t * pstrInteger, u32 * pu32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    u32Ret = jf_option_validateIntegerString(pstrInteger, ol_strlen(pstrInteger));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrInteger, "%u", &u32Value) == 1))
        {
            *pu32Value = u32Value;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
    }

    return u32Ret;
}

u32 jf_option_getU16FromString(const olchar_t * pstrInteger, u16 * pu16Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    u32Ret = jf_option_validateIntegerString(pstrInteger, ol_strlen(pstrInteger));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrInteger, "%u", &u32Value) == 1))
        {
            if (u32Value > U16_MAX)
                u32Ret = JF_ERR_INTEGER_OUT_OF_RANGE;
            else
                *pu16Value = (u16)u32Value;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
    }

    return u32Ret;
}

u32 jf_option_getU8FromString(const olchar_t * pstrInteger, u8 * pu8Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Value;

    u32Ret = jf_option_validateIntegerString(pstrInteger, ol_strlen(pstrInteger));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrInteger, "%u", &u32Value) == 1))
        {
            if (u32Value > U8_MAX)
                u32Ret = JF_ERR_INTEGER_OUT_OF_RANGE;
            else
                *pu8Value = (u8)u32Value;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
    }

    return u32Ret;
}

u32 jf_option_getLongFromString(const olchar_t * pstrInteger, slong * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;

    *numeric = ol_strtol(pstrInteger, (olchar_t **)&stop_str, 10);
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

    return u32Ret;
}

u32 jf_option_getUlongFromString(const olchar_t * pstrInteger, ulong * numeric)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;

    *numeric = ol_strtoul(pstrInteger, &stop_str, 10);
    if (*stop_str != '\0')
    {
        u32Ret = JF_ERR_INVALID_INTEGER;
    }
    else
    {
#if defined(LINUX)
        if (errno != ERANGE)
        {
            if (ol_memcmp(pstrInteger, "-", 1) == 0)
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

    return u32Ret;
}

u32 jf_option_getU64FromString(const olchar_t * pstrInteger, u64 * pu64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;

    *pu64Value = ol_strtoull(pstrInteger, &stop_str, 10);
    if (*stop_str != '\0')
    {
        u32Ret = JF_ERR_INVALID_INTEGER;
    }
    else
    {
#if defined(LINUX)
        if (errno == ERANGE)
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
#endif
    }

    return u32Ret;
}

u32 jf_option_getS64FromString(const olchar_t * pstrInteger, s64 * ps64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * stop_str;

    *ps64Value = ol_strtoll(pstrInteger, &stop_str, 10);
    if (*stop_str != '\0')
    {
        u32Ret = JF_ERR_INVALID_INTEGER;
    }
    else
    {
#if defined(LINUX)
        if (errno == ERANGE)
        {
            u32Ret = JF_ERR_INVALID_INTEGER;
        }
#endif
    }

    return u32Ret;
}

u32 jf_option_getFloatFromString(const olchar_t * pstrFloat, olfloat_t * pflValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olfloat_t flValue;

    u32Ret = jf_option_validateFloatString(pstrFloat, ol_strlen(pstrFloat));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrFloat, "%f", &flValue) == 1))
        {
            *pflValue = flValue;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_FLOAT;
        }
    }

    return u32Ret;
}

u32 jf_option_getDoubleFromString(const olchar_t * pstrDouble, oldouble_t * pdbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    oldouble_t dbValue;

    u32Ret = jf_option_validateFloatString(pstrDouble, ol_strlen(pstrDouble));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((ol_sscanf(pstrDouble, "%lf", &dbValue) == 1))
        {
            *pdbValue = dbValue;
        }
        else
        {
            u32Ret = JF_ERR_INVALID_FLOAT;
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

