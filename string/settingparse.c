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


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_option.h"

#include "stringcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

/** Trim the setting value and copy the trimmed setting value to value buffer.
 *
 *  @param pstrSetting [in] The setting value.
 *  @param pstrValue [out] Setting value buffer.
 *  @param sValue [in] Length of value buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_SETTING_TOO_LONG Setting value is too long, buffer cannot hold it.
 *  @retval JF_ERR_SETTING_EMPTY Setting value is empty.
 */
static u32 _trimSetting(olchar_t * pstrSetting, olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrBegin = pstrSetting;
    olsize_t sEndIndex = 0;

    /*Skip extra space at the head of the string.*/
    pstrBegin = jf_option_skipSpaceBeforeString(pstrBegin);

    /*Skip the trailing spaces.*/
    sEndIndex = ol_strlen(pstrBegin);
    while (sEndIndex > 0)
    {
        sEndIndex--;
        if (pstrBegin[sEndIndex] != JF_STRING_SPACE_CHAR)
        {
            break;
        }
    }

    if ((pstrBegin[sEndIndex] != 0) && (pstrBegin[sEndIndex] != JF_STRING_SPACE_CHAR))
    {
        /*The setting value is not empty.*/
        sEndIndex++;
        if (sEndIndex > sValue)
        {
            u32Ret = JF_ERR_SETTING_TOO_LONG;
        }
        else
        {
            /*No error, copy the value to buffer.*/
            ol_strncpy(pstrValue, pstrBegin, sEndIndex);
            pstrValue[sEndIndex] = JF_STRING_NULL_CHAR;
        }
    }
    else
    {
        /*The setting value is empty.*/
        pstrValue[0] = JF_STRING_NULL_CHAR;
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
    olchar_t cEqual = JF_STRING_EQUAL_SIGN_CHAR;
    olchar_t * psubStr = NULL, * pstrPos = NULL;
    olsize_t sTagLength = ol_strlen(pstrName);
    olsize_t index = 0;

    ol_bzero(pstrValue, sValue);

    /*Iterate the setting array.*/
    for (index = 0; index < sArray; index ++)
    {
        /*Compare the setting name.*/
        if (ol_strncasecmp(pstrArray[index], pstrName, sTagLength) == 0)
        {
            /*Found.*/
            pstrPos = &(pstrArray[index][sTagLength]);
            /*Find the equal sign.*/
            psubStr = strchr(pstrPos, cEqual);
            if (psubStr != NULL)
            {
                /*Equal sign is Found.*/
                u32Ret = JF_ERR_NO_ERROR;
                /*Only space is allowed between setting name and equal sign.*/
                while ((pstrPos < psubStr) && (u32Ret == JF_ERR_NO_ERROR))
                {
                    if (*pstrPos != JF_STRING_SPACE_CHAR)
                    {
                        /*Not space, it means the setting name is not exactly as expected.*/
                        u32Ret = JF_ERR_NOT_FOUND;
                    }

                    pstrPos++;
                }
            }
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /*Copy the value to buffer.*/
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
    olchar_t u8Equal = JF_STRING_EQUAL_SIGN_CHAR;
    olsize_t sLen = 0, sTagLen = 0, i = 0, j = 0;

    /*Iterate the setting array.*/
    for (i = 0; (i < sArray) && (u32Ret == JF_ERR_NO_ERROR); i++)
    {
        u32Ret = JF_ERR_INVALID_SETTING;
        /*Find the delimiter '=' to get setting name.*/
        psubStr = ol_strchr(pstrArray[i], u8Equal);
        if ((psubStr != NULL) && (psubStr != pstrArray[i]))
        {
            /*Found.*/
            sTagLen = psubStr - pstrArray[i];
            while (sTagLen > 0)
            {
                /*Skip the space after the name.*/
                if (pstrArray[i][sTagLen - 1] == JF_STRING_SPACE_CHAR)
                    sTagLen--;
                else
                    break;
            }

            /*Try to find the setting name in name array.*/
            for (j = 0; j < sNameArray; j++)
            {
                sLen = ol_strlen(pstrNameArray[j]);
                if (ol_strncasecmp(pstrArray[i], pstrNameArray[j], sLen) == 0)
                {
                    /*Compare the Length.*/
                    if (sTagLen == sLen)
                    {
                        u32Ret = JF_ERR_NO_ERROR;
                        break;
                    }
                }
            }
        }

        /*For invalid setting, save the index.*/
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
    olchar_t * firstChar = NULL;
    olsize_t sIndex = 0, sInd = 0, sArrayIndex = 0;
    olsize_t sArray = 0;

    assert(pu8Settings != NULL);

    sArray = *psArray;
    *psArray = 0;

    firstChar = (olchar_t *)pu8Settings;

    while (sIndex < sSettings)
    {
        /*Move the index to the next setting.*/
        sIndex = sIndex + ol_strlen(firstChar) + 1;

        /*Skip extra space at the head of the string.*/
        firstChar = jf_option_skipSpaceBeforeString(firstChar);

        sInd = ol_strlen(firstChar);
        if (sInd != 0)
        {
            /*Trim the ending space.*/
            jf_option_removeSpaceAfterString(firstChar);

            /*At least 3 characters, otherwise error.*/
            if (ol_strlen(firstChar) < 3)
                return JF_ERR_INVALID_SETTING;

            /*Save the result to the array.*/
            pstrArray[sArrayIndex] = firstChar;

            sArrayIndex ++;

            if (sArrayIndex >= sArray)
                break;
        }

        firstChar = (olchar_t *)pu8Settings + sIndex;
    }

    *psArray = sArrayIndex;

    return u32Ret;
}

u32 jf_string_processSettings(
    olchar_t * pstrSettings, olchar_t * pstrArray[], olsize_t * psArray)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * firstChar, * psubStr;
    olsize_t sIndex = 0, sLength = 0;

    if ((pstrSettings == NULL) || (*psArray <= 0))
        return JF_ERR_INVALID_PARAM;

    sLength = ol_strlen(pstrSettings);

    /*Skip the Quote.*/
    if (pstrSettings[0] == JF_STRING_DOUBLE_QUOTES_CHAR)
    {
        if (pstrSettings[sLength - 1] != JF_STRING_DOUBLE_QUOTES_CHAR)
            return JF_ERR_MISSING_QUOTE;

        firstChar = pstrSettings + 1;
        pstrSettings[sLength - 1] = 0;
    }
    else
    {
        firstChar = pstrSettings;
    }

    /*Skip extra space.*/
    firstChar = jf_option_skipSpaceBeforeString(firstChar);

    psubStr = firstChar;
    while (psubStr != NULL)
    {
        psubStr = ol_strchr(firstChar, JF_STRING_COMMA_CHAR);
        if (psubStr != NULL)
        {
            /*Trim the ending space.*/
            *psubStr = JF_STRING_NULL_CHAR;
            jf_option_removeSpaceAfterString(firstChar);

            /*At least 3 characters, otherwise error.*/
            if (ol_strlen(firstChar) < 3)
                return JF_ERR_INVALID_SETTING;

            /*Save the result to the array.*/
            pstrArray[sIndex] = firstChar;

            /*Skip the devider and extra space.*/
            firstChar = jf_option_skipSpaceBeforeString(psubStr + 1);

            sIndex ++;
        }
        else  /*Last setting param.*/
        {
            /*Trim the ending space.*/
            jf_option_removeSpaceAfterString(firstChar);

            /*At least 3 characters, otherwise error.*/
            if (ol_strlen(firstChar) < 3)
                return JF_ERR_INVALID_SETTING;

            /*Save the result to the array.*/
            pstrArray[sIndex] = firstChar;

            sIndex++;
        }

        if (sIndex == *psArray)
            break;
    }

    *psArray = sIndex;

    return u32Ret;
}

u32 jf_string_getSettingsString(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const olchar_t * pstrDefaultValue, olchar_t * pstrValue, olsize_t sValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Try to find the setting in the array.*/
    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, pstrValue, sValue);
    if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Not found, use default.*/
        ol_strncpy(pstrValue, pstrDefaultValue, sValue);
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

    /*Try to find the setting in the array.*/
    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Found, parse the setting string.*/
        u32Ret = jf_option_getU32FromString(strValue, pu32Value);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            /*The value is invalid, use default.*/
            *pu32Value = u32DefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Not found, use default.*/
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

    /*Try to find the setting in the array.*/
    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Found, parse the setting string.*/
        u32Ret = jf_option_getU64FromString(strValue, pu64Value);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            /*The value is invalid, use default.*/
            *pu64Value = u64DefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Not found, use default.*/
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

    /*Try to find the setting in the array.*/
    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Found, parse the setting string.*/
        u32Ret = jf_option_getDoubleFromString(strValue, pdbValue);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            /*The value is invalid, use default.*/
            *pdbValue = dbDefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Not found, use default.*/
        *pdbValue = dbDefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

u32 jf_string_getSettingsBoolean(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const boolean_t bDefaultValue, boolean_t * pbValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strValue[200];

    /*Try to find the setting in the array.*/
    u32Ret = jf_string_retrieveSettings(
        pstrArray, sArray, pstrSettingName, strValue, sizeof(strValue));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Found, parse the setting string.*/
        u32Ret = jf_string_getBooleanFromString(strValue, ol_strlen(strValue), pbValue);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            /*The value is invalid, use default.*/
            *pbValue = bDefaultValue;
        }
    }
    else if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Not found, use default.*/
        *pbValue = bDefaultValue;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

u32 jf_string_processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrSubStr = NULL;

    assert((ppstrName != NULL) && (ppstrValue != NULL));

    *ppstrName = NULL;
    *ppstrValue = NULL;

    /*Find the delimiter '='.*/
    pstrSubStr = strchr(pstrSetting, JF_STRING_EQUAL_SIGN_CHAR);
    if (pstrSubStr != NULL)
    {
        *ppstrName = pstrSetting;
        *ppstrValue = pstrSubStr + 1;
        *pstrSubStr = JF_STRING_NULL_CHAR;
    }
    else
    {
        u32Ret = JF_ERR_INVALID_SETTING;
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
