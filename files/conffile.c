/**
 *  @file conffile.c
 *
 *  @brief Configuration file implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_conffile.h"
#include "jf_filestream.h"
#include "jf_array.h"
#include "jf_option.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** The comment indicator.
 */
#define COMMENT_INDICATOR                 '#'

/** Define the internal configuration file data type.
 */
typedef struct
{
    /**Configuration file.*/
    olchar_t ijc_strConfFile[JF_LIMIT_MAX_PATH_LEN];
    /**File stream handle pointing to the configuration file.*/
    jf_filestream_t * ijc_pjfConfFile;
    u8 ijc_u8Reserved[8];
} internal_jf_conffile_t;

/** Define the data type for setting value to configuration file..
 */
typedef struct
{
    /**File stream handle pointing to the configuration file.*/
    jf_filestream_t * jcsv_pjfFile;
    /**The tag string to be set.*/
    const olchar_t * jcsv_pstrTag;
    /**The value string to be set.*/
    const olchar_t * jcsv_pstrValue;
    /**The tag is found and value is saved if TRUE.*/
    boolean_t jcsv_bSet;
    u8 jcsv_u8Reserved[7];
} jf_conffile_set_value_t;

/* --- private routine section ------------------------------------------------------------------ */

/** Read a line from the file, and trim line comment.
 *
 *  @param fp [in] The file stream handle.
 *  @param strLine [out] the line will be returned here. 
 *
 *  @return The last character read from file.
 *  @retval 0xA Line feed, a line has been read.
 *  @retval EOF Reach the end of the file.
 */
static olint_t _readLineFromFile(
    jf_filestream_t * fp, olchar_t strLine[JF_CONFFILE_MAX_LINE_LEN])
{
    olint_t nChar = 0;
    olint_t nLength = 0;
    olint_t nComment = 0;

    do
    {
        nChar = jf_filestream_getChar(fp);

        /*Check comment.*/
        if ((nComment == 0) && (nChar == '#'))
        {
            /*Check the character before '#'.*/
            if (nLength == 0)
            {
                /*No character before '#', this is real comment.*/
                nComment = 1;
            }
            else
            {
                /*'\' character before '#', not a real comment. "\#" = "#" */
                if (strLine[nLength - 1] == '\\')
                    nLength--;
                else
                    nComment = 1;
            }
        }

        if ((nChar != EOF) && (nChar != '\n') && (nComment == 0))
        {
            /*Save the character to the buffer.*/
            strLine[nLength] = (olchar_t)nChar;
            nLength++;
        }
    } while ((nChar != EOF) && (nChar != '\n') && (nLength < JF_CONFFILE_MAX_LINE_LEN - 1));

    strLine[nLength] = '\0';

    return nChar;
}

/** Remove the blank spaces from the left and the right of the string.
 *
 *  @param pstrBufOut [out] The output string after removing the blank spaces.
 *  @param pstrBufIn [in] The input string to be removed the blank spaces.
 *
 *  @return Void.
 */
static void _skipBlank(olchar_t * pstrBufOut, const olchar_t * pstrBufIn)
{
    olint_t nright, nleft = 0;

    /*Find the first character which is not space.*/
    while ((pstrBufIn[nleft] != 0) && (pstrBufIn[nleft] == ' '))
    {
        nleft++;
    }

    nright = ol_strlen(pstrBufIn);

    if (pstrBufIn[nleft] != 0)
    {
        /*Find the last character which is not space.*/
        while (pstrBufIn[nright-1] == ' ')
        {
            nright--;
        }

        ol_strcpy(pstrBufOut, &pstrBufIn[nleft]);
    }

    pstrBufOut[nright - nleft] = '\0';
}

/** Get the value string of a option of the specified tag name.
 *
 *  @param pijc [in] The configuration file object.
 *  @param pstrTag [in] The tag name of the config.
 *  @param strBuf [out] The value string of the config.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _getValueStringByTag(
    internal_jf_conffile_t * pijc, const olchar_t * pstrTag, olchar_t strBuf[JF_CONFFILE_MAX_LINE_LEN])
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olchar_t strLine[JF_CONFFILE_MAX_LINE_LEN];
    olchar_t * pcEqual = NULL, * pstrLine = NULL, * pstrValue = NULL;
    olint_t nChar = 0;
    olint_t nLength = 0, nLengthTag = 0;

    nLengthTag = ol_strlen(pstrTag);

    do
    {
        ol_memset(strLine, 0, JF_CONFFILE_MAX_LINE_LEN);
        pstrLine = strLine;
        nChar = _readLineFromFile(pijc->ijc_pjfConfFile, strLine);

        /*Skip the space before tag name string.*/
        pstrLine = jf_option_skipSpaceBeforeString(pstrLine);

        if (ol_strlen(pstrLine) > 0)
        {
            /*Found the tag, search for "=".*/
            pcEqual = strstr(&(pstrLine[nLengthTag]), (const olchar_t *)"=");
            if (pcEqual != NULL)
            {
                pstrValue = pcEqual + 1;

                /*Skip the spaces after tag name string.*/
                pcEqual --;
                while (*pcEqual == ' ')
                    pcEqual --;

                nLength = pcEqual - pstrLine + 1;
                if ((nLength == nLengthTag) && (ol_strncmp(pstrLine, pstrTag, nLengthTag) == 0))
                {
                    _skipBlank(strBuf, pstrValue);
                    nChar = EOF;
                    u32Ret = JF_ERR_NO_ERROR;
                }
            }
        }
    } while (nChar != EOF);

    return u32Ret;
}

static u32 _traverseConfFile(
    internal_jf_conffile_t * pijc, jf_conffile_fnHandleConfig_t fnHandleConfig, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[JF_CONFFILE_MAX_LINE_LEN];
    olchar_t * pstrLine = NULL, * pstrValue = NULL, * pstrEqual = NULL;
    olint_t nChar = 0;
    olint_t nLength = 0;

    jf_filestream_seek(pijc->ijc_pjfConfFile, 0L, SEEK_SET);

    do
    {
        ol_bzero(strLine, JF_CONFFILE_MAX_LINE_LEN);
        pstrLine = strLine;

        /*Read a line from file.*/
        nChar = _readLineFromFile(pijc->ijc_pjfConfFile, strLine);

        /*Skip the space before tag name string.*/
        pstrLine = jf_option_skipSpaceBeforeString(pstrLine);

        nLength = ol_strlen(pstrLine);
        if (nLength > 0)
        {
            /*Found the tag, search for "=".*/
            pstrEqual = strstr(pstrLine, "=");
            if (pstrEqual != NULL)
            {
                pstrValue = pstrEqual + 1;
                *pstrEqual = '\0';
                /*Remove the space after tag name string.*/
                jf_option_removeSpaceAfterString(pstrLine);

                /*Skip the space before value string.*/
                pstrValue = jf_option_skipSpaceBeforeString(pstrValue);
                /*Remove the space after value string.*/
                jf_option_removeSpaceAfterString(pstrValue);

                /*Call the callback function with tag and value string.*/
                u32Ret = fnHandleConfig(pstrLine, pstrValue, pArg);
            }
        }
    } while ((nChar != EOF) && (u32Ret == JF_ERR_NO_ERROR));

    return u32Ret;
}

/** Write config to configuration file.
 */
static u32 _writeConfigToConfFile(
    jf_filestream_t * pjf, const olchar_t * pstrTag, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[JF_CONFFILE_MAX_LINE_LEN];
    olsize_t sLine = 0;

    ol_snprintf(strLine, sizeof(strLine), "%s=%s\n", pstrTag, pstrValue);
    strLine[JF_CONFFILE_MAX_LINE_LEN - 1] = '\0';
    sLine = ol_strlen(strLine);

    u32Ret = jf_filestream_writen(pjf, strLine, sLine);

    return u32Ret;
}

/** Callback function to set config to temporary file.
 */
static u32 _fnSaveConfigToTempFile(olchar_t * pstrTag, olchar_t * pstrValue, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_conffile_set_value_t * pjcsv = pArg;
    const olchar_t * value = pstrValue;

    /*Use the new value If the tag is found.*/
    if (ol_strcmp(pstrTag, pjcsv->jcsv_pstrTag) == 0)
    {
        value = pjcsv->jcsv_pstrValue;
        pjcsv->jcsv_bSet = TRUE;
    }

    u32Ret = _writeConfigToConfFile(pjcsv->jcsv_pjfFile, pstrTag, value);

    return u32Ret;
}

/** Open configuration file.
 */
static u32 _openConfFile(olchar_t * pstrFile, internal_jf_conffile_t * pijc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Try to open it for reading or writing, create the file if it's not existing.*/
    u32Ret = jf_filestream_open(pstrFile, "a+", &pijc->ijc_pjfConfFile);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_conffile_open(
    jf_conffile_open_param_t * pParam, jf_conffile_t ** ppConffile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = NULL;

    assert(ppConffile != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pijc, sizeof(*pijc));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pijc, sizeof(*pijc));
        ol_snprintf(
            pijc->ijc_strConfFile, sizeof(pijc->ijc_strConfFile), "%s", pParam->jcop_pstrFile);

        /*Open the config file.*/
        u32Ret = _openConfFile(pijc->ijc_strConfFile, pijc);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppConffile = pijc;
    else if (pijc != NULL)
        jf_conffile_close((jf_conffile_t **)&pijc);

    return u32Ret;
}

u32 jf_conffile_close(jf_conffile_t ** ppConffile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = *ppConffile;

    assert(ppConffile != NULL);

    if (pijc->ijc_pjfConfFile != NULL)
    {
        jf_filestream_close(&pijc->ijc_pjfConfFile);
    }

    jf_jiukun_freeMemory(ppConffile);

    return u32Ret;
}

u32 jf_conffile_write(jf_conffile_t * pConffile, const olchar_t * pstrData, olsize_t sData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = (internal_jf_conffile_t *)pConffile;

    u32Ret = jf_filestream_writen(pijc->ijc_pjfConfFile, pstrData, sData);

    return u32Ret;
}

u32 jf_conffile_getInt(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, olint_t nDefault, olint_t * pnValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = (internal_jf_conffile_t *)pConffile;
    char strLine[JF_CONFFILE_MAX_LINE_LEN];
    olint_t nRet = 0;

    assert((pConffile != NULL) && (pstrTag != NULL) && (pnValue != NULL));

    jf_filestream_seek(pijc->ijc_pjfConfFile, 0L, SEEK_SET);

    u32Ret = _getValueStringByTag(pijc, pstrTag, strLine);
    while (u32Ret == JF_ERR_NO_ERROR)
    {
        nRet = ol_sscanf(strLine, "%d", pnValue);
        if (nRet == 1)
            break;
        else /*Continue looking for the tag if the found one is not right.*/
            u32Ret = _getValueStringByTag(pijc, pstrTag, strLine);
    }
        
    if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /*Set the option value to default.*/
        u32Ret = JF_ERR_NO_ERROR;
        *pnValue = nDefault;
    }
    
    return u32Ret;
}

u32 jf_conffile_get(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, const olchar_t * pstrDefault,
    olchar_t * pstrValueBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = (internal_jf_conffile_t *)pConffile;
    olchar_t strValue[JF_CONFFILE_MAX_LINE_LEN];
    olsize_t size = 0;
    
    assert((pConffile != NULL) && (pstrTag != NULL) && (pstrValueBuf != NULL));

    jf_filestream_seek(pijc->ijc_pjfConfFile, 0L, SEEK_SET);

    u32Ret = _getValueStringByTag(pijc, pstrTag, strValue);
    while (u32Ret == JF_ERR_NO_ERROR)
    {
        size = ol_strlen(strValue);
        if (size > 0)
        {
            /*The value string is availble, check the buffer size.*/
            if (size < sBuf)
            {
                /*Copy the value string to buffer.*/
                ol_strcpy(pstrValueBuf, strValue);
                break;
            }
            else
            {
                /*Buffer is too small.*/
                u32Ret = JF_ERR_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            /*Continue looking for the tag if the found one is not right.*/
            u32Ret = _getValueStringByTag(pijc, pstrTag, strValue);
        }
    }
        
    if ((u32Ret == JF_ERR_NOT_FOUND) && (pstrDefault != NULL))
    {
        /*Set the option value to default.*/
        ol_snprintf(pstrValueBuf, sBuf, "%s", pstrDefault);
        pstrValueBuf[sBuf - 1] = '\0';
        u32Ret = JF_ERR_NO_ERROR;
    }
    
    return u32Ret;
}

u32 jf_conffile_set(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = (internal_jf_conffile_t *)pConffile;
    olchar_t strTempFile[JF_LIMIT_MAX_PATH_LEN];
    jf_conffile_set_value_t jcsv;

    JF_LOGGER_DEBUG("tag: %s, value: %s", pstrTag, pstrValue);

    ol_bzero(&jcsv, sizeof(jcsv));
    jcsv.jcsv_pstrTag = pstrTag;
    jcsv.jcsv_pstrValue = pstrValue;

    ol_snprintf(strTempFile, sizeof(strTempFile), "%s.temp", pijc->ijc_strConfFile);

    /*Open a temperory file to save all configs.*/
    u32Ret = jf_filestream_open(strTempFile, "w", &jcsv.jcsv_pjfFile);

    /*Traverse the source config file.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_conffile_traverse(pConffile, _fnSaveConfigToTempFile, &jcsv);

    /*The tag is not found, append the value to temp file.*/
    if ((u32Ret == JF_ERR_NO_ERROR) && (! jcsv.jcsv_bSet))
        u32Ret = _writeConfigToConfFile(jcsv.jcsv_pjfFile, pstrTag, pstrValue);

    if (jcsv.jcsv_pjfFile != NULL)
        jf_filestream_close(&jcsv.jcsv_pjfFile);

    /*Remove the original config file and rename the new one.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Close the original config file.*/
        jf_filestream_close(&pijc->ijc_pjfConfFile);

        /*Remove the original config file.*/
        jf_file_remove(pijc->ijc_strConfFile);

        /*Rename the temporary file.*/
        jf_file_rename(strTempFile, pijc->ijc_strConfFile);

        /*Reopen the config file.*/
        u32Ret = _openConfFile(pijc->ijc_strConfFile, pijc);
    }

    return u32Ret;
}

u32 jf_conffile_traverse(
    jf_conffile_t * pConffile, jf_conffile_fnHandleConfig_t fnHandleConfig, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_conffile_t * pijc = (internal_jf_conffile_t *)pConffile;

    u32Ret = _traverseConfFile(pijc, fnHandleConfig, pArg);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
