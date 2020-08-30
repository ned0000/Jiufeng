/**
 *  @file jf_option.c
 *
 *  @brief Implementation file for handling option. 
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
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

/** End of option array.
 */
#define JF_OPTION_END_OF_ARGV                     (-1)

/** Option returned when there are unknown option.
 */
#define JF_OPTION_UNKNOWN                         '?'

/** Option returned when options has a missing argument.
 */
#define JF_OPTION_MISSING_ARG                     ':'

/** Option prefix.
 */
#define JF_OPTION_PREFIX                          '-'

/** Index of the option array, ignore the first entry in array which is command itself.
 */
static olint_t ls_nArgvIndex = 1;

/** The argument of the option.
 */
static olchar_t * ls_pstrOptionArg = NULL;

/** Index of the currently parsed option, ignore the first character '-'. For the options like
 *  "-xyz".
 */
static olint_t ls_nOptionIndex = 1;

/* --- private routine section ------------------------------------------------------------------ */

/** Print error message to stderr for the option.
 */
static void _printErrorOption(const olchar_t * pstrCmd, olchar_t * pstrError, int nOpt)
{
    olchar_t erropt[2];

    erropt[0] = (olchar_t)nOpt;
    erropt[1] = '\0';

    ol_fprintf(stderr, "%s: %s -- '%s'\n", pstrCmd, pstrError, erropt);
}

/** Move option index to the next.
 */
static void _moveOptionIndex(olint_t argc, olchar_t ** const argv)
{
    if (ls_nArgvIndex >= argc)
        return;

    ++ ls_nOptionIndex;
    if (argv[ls_nArgvIndex][ls_nOptionIndex] == '\0')
    {
        /*End of the option, move to the next entry in array.*/
        ++ ls_nArgvIndex;
        ls_nOptionIndex = 1;
    }
}

/** Reset the variable of option parse.
 */
static void _resetOptionVar(void)
{
    ls_nArgvIndex = 1;
    ls_pstrOptionArg = NULL;
    ls_nOptionIndex = 1;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_option_validateIntegerString(const olchar_t * pstrInteger, const olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t i;

    for (i = 0; ((i < size) && (u32Ret == JF_ERR_NO_ERROR)); i++)
    {
        if (isdigit(pstrInteger[i]) == 0)
        {
            return JF_ERR_INVALID_STRING;
        }
    }

    return u32Ret;
}

u32 jf_option_validateFloatString(const olchar_t * pstrFloat, const olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t i = 0;
    olchar_t * str = (olchar_t *)pstrFloat;
    olsize_t sizes = size;
    olint_t nCount = 0;

    if (size < 1)
        u32Ret = JF_ERR_INVALID_STRING;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (str[0] == '-')
        {
            str ++;
            sizes --;
        }
    }

    for (i = 0; ((i < sizes) && (u32Ret == JF_ERR_NO_ERROR)); ++ i)
    {
        if (str[i] == 'e')
            break;

        if (isdigit(str[i]) == 0)
        {
            if (str[i] == '.')
                nCount++;
            else
                u32Ret = JF_ERR_INVALID_STRING;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (nCount > 1)  /*There can't be more than 1 dot.*/
            u32Ret = JF_ERR_INVALID_STRING;
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
    olchar_t * stop_str = NULL;

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

olchar_t * jf_option_skipSpaceBeforeString(olchar_t * pstr)
{
    if (*pstr != '\0')
    {
        while (*pstr == ' ')
            pstr ++;
    }

    return pstr;
}

void jf_option_removeSpaceAfterString(olchar_t * pstr)
{
    olchar_t * end = NULL;

    if (*pstr != '\0')
    {
        end = pstr + ol_strlen(pstr) - 1;
        while ((end >= pstr) && (*end == ' '))
            end --;
        *(end + 1) = '\0';
    }
}

olint_t jf_option_get(olint_t argc, olchar_t ** const argv, const olchar_t * stropt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt = JF_OPTION_END_OF_ARGV;
    olchar_t * arg = NULL;

    /*End of argument array.*/
    if (ls_nArgvIndex >= argc)
        u32Ret = JF_ERR_INVALID_OPTION;

    if ((u32Ret == JF_ERR_NO_ERROR) && (ls_nOptionIndex == 1))
    {
        /*The option should be started with '-'.*/
        if ((argv[ls_nArgvIndex][0] != JF_OPTION_PREFIX) || (ol_strlen(argv[ls_nArgvIndex]) <= 1))
        {
            _printErrorOption(argv[0], "illegal option", argv[ls_nArgvIndex][0]);
            nOpt = JF_OPTION_UNKNOWN;
            u32Ret = JF_ERR_INVALID_OPTION;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*':' cannot be an option.*/
        nOpt = argv[ls_nArgvIndex][ls_nOptionIndex];
        if (nOpt == ':')
        {
            _printErrorOption(argv[0], "illegal option", nOpt);
            nOpt = JF_OPTION_UNKNOWN;
            u32Ret = JF_ERR_INVALID_OPTION;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Not specified option.*/
        arg = ol_strchr(stropt, nOpt);
        if (arg == NULL)
        {
            _printErrorOption(argv[0], "illegal option", nOpt);
            nOpt = JF_OPTION_UNKNOWN;
            u32Ret = JF_ERR_INVALID_OPTION;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ++ arg;
        if (*arg == ':')
        {
            /*The option has an argument.*/
            ++ ls_nArgvIndex;
            if (ls_nArgvIndex >= argc)
            {
                _printErrorOption(argv[0], "option requires an argument", nOpt);
                nOpt = JF_OPTION_MISSING_ARG;
                u32Ret = JF_ERR_MISSING_OPTION_ARG;
            }
            else
            {
                ls_pstrOptionArg = argv[ls_nArgvIndex];
                ++ ls_nArgvIndex;
                ls_nOptionIndex = 1;
            }
        }
        else
        {
            /*The option has no argument*/
            _moveOptionIndex(argc, argv);
        }
    }
    else
    {
        _moveOptionIndex(argc, argv);
    }

    return nOpt;
}

olchar_t * jf_option_getArg(void)
{
    return ls_pstrOptionArg;
}

u32 jf_option_reset(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _resetOptionVar();

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
