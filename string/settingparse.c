/**
 *  @file settingparse.c
 *
 *  @brief Implementation file of routines for parsing the setting string.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

/** Trim the setting pointed by pstrSetting and copy the trimmed setting to pstrValue
 */
static u32 _trimSetting(olchar_t * pstrSetting, olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrBegin = pstrSetting;
    olsize_t sEndIndex = 0;

    while (*pstrBegin == ' ')
    {
        pstrBegin++;
    }

    sEndIndex = ol_strlen(pstrBegin);
    while (sEndIndex > 0)
    {
        sEndIndex--;
        if (pstrBegin[sEndIndex] != ' ')
        {
            break;
        }
    }

    if ((pstrBegin[sEndIndex] != 0) && (pstrBegin[sEndIndex] != ' '))
    {
        sEndIndex++;
        if (sEndIndex > sValue)
        {
            u32Ret = JF_ERR_SETTING_TOO_LONG;
        }
        else
        {
            ol_strncpy(pstrValue, pstrBegin, sEndIndex);
            pstrValue[sEndIndex] = '\0';
        }
    }
    else
    {
        pstrValue[0] = '\0';
        u32Ret = JF_ERR_SETTING_EMPTY;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_string_retrieveSettings(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrName, olchar_t * pstrValue,
    olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olchar_t cEqual = '=';
    olchar_t * psubStr = NULL, * pstrPos = NULL;
    olsize_t sTagLength = ol_strlen(pstrName);
    olsize_t index = 0;

    ol_bzero(pstrValue, sValue);

    for (index = 0; index < sArray; index ++)
    {
        if (ol_strncasecmp(pstrArray[index], pstrName, sTagLength) == 0)
        {
            pstrPos = &(pstrArray[index][sTagLength]);
            psubStr = strchr(pstrPos, cEqual);
            if (psubStr != NULL)
            {
                u32Ret = JF_ERR_NO_ERROR;
                while ((pstrPos < psubStr) && (u32Ret == JF_ERR_NO_ERROR))
                {
                    if (*pstrPos != ' ')
                    {
                        u32Ret = JF_ERR_NOT_FOUND;
                    }

                    pstrPos++;
                }
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _trimSetting((psubStr + 1), pstrValue, sValue);
            break;
        }
    }

    return u32Ret;
}

u32 jf_string_validateSettings(
    olchar_t * pstrNameArray[], olsize_t sNameArray, olchar_t * pstrArray[], olsize_t sArray,
    olindex_t * piArray)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * psubStr = NULL;
    olchar_t u8Equal = '=';
    olsize_t sLen = 0, sTagLen = 0, i = 0, j = 0;

    for (i = 0; (i < sArray) && (u32Ret == JF_ERR_NO_ERROR); i++)
    {
        u32Ret = JF_ERR_INVALID_SETTING;
        psubStr = ol_strchr(pstrArray[i], u8Equal);
        if ((psubStr != NULL) && (psubStr != pstrArray[i]))
        {
            sTagLen = psubStr - pstrArray[i];
            while (sTagLen > 0)
            {
                /* skip the space after the tag */
                if (pstrArray[i][sTagLen-1] == ' ')
                {
                    sTagLen--;
                }
                else
                {
                    break;
                }
            }

            for (j = 0; j < sNameArray; j++)
            {
                sLen = ol_strlen(pstrNameArray[j]);
                if (ol_strncasecmp(pstrArray[i], pstrNameArray[j], sLen) == 0)
                {
                    if (sTagLen == sLen)
                    {
                        u32Ret = JF_ERR_NO_ERROR;
                        break;
                    }
                }
            }
        }

        if (u32Ret == JF_ERR_INVALID_SETTING)
        {
            *piArray = i;
        }
    }

    return u32Ret;
}

u32 jf_string_processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings, olchar_t * pstrArray[], olsize_t * psArray)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t u8Space = ' ';
    u8 * firstChar;
    olsize_t sIndex=0, sInd = 0, sArrayIndex = 0;
    olsize_t sArray = 0;

    assert(pu8Settings != NULL);

    sArray = *psArray;
    *psArray = 0;

    firstChar = pu8Settings;

    while (sIndex < sSettings)
    {
        /* skip extra space */
        while (firstChar[0] == u8Space)
            firstChar++;

        sInd = ol_strlen((olchar_t *)firstChar);
        if (sInd != 0)
        {
            /* trim the ending space */
            while (sInd > 0)
            {
                sInd--;
                if (firstChar[sInd] != u8Space)
                {
                    sInd++;
                    break;
                }
            }

            if (sInd < 3)  /* *=* */
            {
                return JF_ERR_INVALID_SETTING;
            }

            firstChar[sInd] = 0;
            pstrArray[sArrayIndex] = (olchar_t *)firstChar;

            sArrayIndex ++;

            if (sArrayIndex >= sArray)
                break;
        }

        sIndex = sIndex + sInd + 1;
        firstChar = firstChar + sInd + 1;
    }

    * psArray = sArrayIndex;

    return u32Ret;
}

u32 jf_string_processSettings(
    olchar_t * pstrSettings, olchar_t * pstrArray[], olsize_t * psArray)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t u8Devider = ',', u8Quote = '"', u8Space = ' ';
    olchar_t * firstChar, * psubStr;
    olsize_t sIndex = 0, sLength, sInd = 0;

    if (pstrSettings == NULL)
        return JF_ERR_INVALID_SETTING;

    sLength = ol_strlen(pstrSettings);

    /* skip the Quote */
    if (pstrSettings[0] == u8Quote)
    {
        if (pstrSettings[sLength - 1] != u8Quote)
            return JF_ERR_MISSING_QUOTE;

        firstChar = pstrSettings + 1;
        pstrSettings[sLength - 1] = 0;
    }
    else
    {
        firstChar = pstrSettings;
    }

    /* skip extra space */
    while (firstChar[0] == u8Space)
        firstChar++;

    psubStr = firstChar;
    while (psubStr != NULL)
    {
        psubStr = strchr(firstChar, u8Devider);
        if (psubStr != NULL)
        {
            sLength = psubStr - firstChar;
            sInd = sLength;
            /* trim the ending space */
            while (sInd > 0)
            {
                sInd--;
                if (firstChar[sInd] != u8Space)
                {
                    sInd++;
                    break;
                }
            }

            if (sInd < 3)  /* *=* */
            {
                return JF_ERR_INVALID_SETTING;
            }

            firstChar[sInd] = '\0';
            pstrArray[sIndex] = firstChar;

            /* skip the Devider */
            firstChar = psubStr + 1;

            /* skip extra space */
            while (firstChar[0] == u8Space)
                firstChar++;

            sIndex ++;
        }
        else
        /* last setting param */
        {
            sLength = ol_strlen(firstChar);
            sInd = sLength;
            /* trim the ending space */
            while (sInd > 0)
            {
                sInd--;
                if (firstChar[sInd] != u8Space)
                {
                    sInd++;
                    break;
                }
            }

            if (sInd < 3)  /* *=* */
            {
                return JF_ERR_INVALID_SETTING;
            }

            firstChar[sInd] = '\0';
            pstrArray[sIndex] = firstChar;
            sIndex++;
        }
    }

    * psArray = sIndex;

    return u32Ret;
}

u32 jf_string_getSettingsString(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const olchar_t * pstrDefaultValue, olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, pstrValue, sValue);
    if (u32Ret == JF_ERR_NOT_FOUND)
    {
        ol_strncpy(pstrValue, pstrDefaultValue, sValue - 1);
        pstrValue[sValue - 1] = 0;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

u32 jf_string_getSettingsU32(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const u32 u32DefaultValue, u32 * pu32Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strValue[200];

    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU32FromString(strValue, pu32Value);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            *pu32Value = u32DefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        *pu32Value = u32DefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

u32 jf_string_getSettingsU64(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const u64 u64DefaultValue, u64 * pu64Value)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strValue[200];

    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getU64FromString(strValue, pu64Value);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            *pu64Value = u64DefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        *pu64Value = u64DefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

u32 jf_string_getSettingsDouble(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const oldouble_t dbDefaultValue, oldouble_t * pdbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strValue[200];

    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_option_getDoubleFromString(strValue, pdbValue);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            *pdbValue = dbDefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        *pdbValue = dbDefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

/** If setting name is not found in the string array, the default value is
 *  set and return JF_ERR_NO_ERROR. if setting name is found and the value 
 *  is incorrect, the default value is set and return JF_ERR_INVALID_SETTING
 */
u32 jf_string_getSettingsBoolean(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const boolean_t bDefaultValue, boolean_t * pbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strValue[200];

    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_string_getBooleanFromString(strValue, ol_strlen(strValue), pbValue);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            *pbValue = bDefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        *pbValue = bDefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

/** Process setting string with format "name=value", the string is null-terminated.
 *
 */
u32 jf_string_processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t cEqual = '=';
    olchar_t * pstrSubStr = NULL;

    assert((ppstrName != NULL) && (ppstrValue != NULL));

    *ppstrName = NULL;
    *ppstrValue = NULL;

    pstrSubStr = strchr(pstrSetting, cEqual);
    if (pstrSubStr != NULL)
    {
        *ppstrName = pstrSetting;
        *ppstrValue = pstrSubStr + 1;
        *pstrSubStr = '\0';
    }
    else
    {
        u32Ret = JF_ERR_INVALID_SETTING;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
