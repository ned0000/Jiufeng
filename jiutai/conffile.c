/**
 *  @file conffile.c
 *
 *  @brief configuration file implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "conffile.h"
#include "array.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */

#define COMMENT_INDICATOR '#'

/* --- private routine section ------------------------------------------------ */

/** Read a line from the file, and trim line comment.
 *
 *  @param fp [in] the file descriptor
 *  @param strLine [out] the line will be returned here. 
 *
 *  @return the last character read from file 
 *  @retval 0xA line feed, a line has been read
 *  @retval EOF reach the end of the file

 *  Notes
 *   - fp MUST NOT be NULL and it MUST be opend before this function is called
 */
static olint_t _readLineFromFile(FILE * fp, olchar_t strLine[MAX_CONFFILE_LINE_LEN])
{
    olint_t nChar;
    olint_t nLength = 0;
    olint_t nComment = 0;

    do
    {
        nChar = fgetc(fp);

        /* check comment */
        if ((nComment == 0) && (nChar == '#'))
        {
            if (nLength == 0)
            {
                nComment = 1;
            }
            else
            {
                if (strLine[nLength - 1] == '\\')
                {
                    nLength--; /* "\#" = "#" */
                }
                else
                {
                    nComment = 1;
                }
            }
        }

        if ((nChar != EOF) && (nChar != '\n') && (nComment == 0))
        {
            strLine[nLength] = (char)nChar;
            nLength++;
        }
    } while ((nChar != EOF) && (nChar != '\n') &&
             (nLength < MAX_CONFFILE_LINE_LEN - 1));

    strLine[nLength] = 0;

    return nChar;
}

/** Remove the blank space(s) from the left and the right of the string
 *
 *  @param pstrBufOut [out] the output string after removing the blank space(s)
 *  @param pstrBufIn [in] the input string to be removed the blank space(s)
 *
 *  @return void
 */
static void _skipBlank(olchar_t * pstrBufOut, const olchar_t * pstrBufIn)
{
    olint_t nright, nleft = 0;

    while ((pstrBufIn[nleft] != 0) && (pstrBufIn[nleft] == ' '))
    {
        nleft++;
    }

    nright = ol_strlen(pstrBufIn);

    if (pstrBufIn[nleft] != 0)
    {
        while (pstrBufIn[nright-1] == ' ')
        {
            nright--;
        }

        ol_strcpy(pstrBufOut, &pstrBufIn[nleft]);
    }

    pstrBufOut[nright - nleft] = 0;
}

/** Get the value string of a option of the specified tag name.
 *
 *  @param pcf [in] the configuration file object.
 *  @param pstrTag [in] the tag name of the option.
 *  @param strBuf [out] the value string of the option. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _getValueStringByTag(
    conf_file_t * pcf, 
    const olchar_t * pstrTag, olchar_t strBuf[MAX_CONFFILE_LINE_LEN])
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    olchar_t strLine[MAX_CONFFILE_LINE_LEN];
    olchar_t * pcEqual, * pstrLine;
    olint_t nChar;
    olint_t nLength, nLengthTag;

    nLengthTag = ol_strlen(pstrTag);

    do
    {
        memset(strLine, 0, MAX_CONFFILE_LINE_LEN);
        pstrLine = strLine;
        nChar = _readLineFromFile(pcf->cf_pfConfFile, strLine);

        while (*pstrLine == ' ')
            pstrLine ++;

        nLength = ol_strlen(pstrLine);
        if (nLength > 0)
        {
            if (strncmp(pstrLine, pstrTag, nLengthTag) == 0)
            {
                /* found the tag, search for "=" */
                pcEqual = strstr(&(pstrLine[nLengthTag]), (const olchar_t *)"=");
                if (pcEqual != NULL)
                {
                    _skipBlank(strBuf, &(pcEqual[1]));
                    nChar = EOF;
                    u32Ret = JF_ERR_NO_ERROR;
                }
            }
        }
    } while (nChar != EOF);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 openConfFile(conf_file_t * pcf, const olchar_t * pstrPath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pcf != NULL);

    memset(pcf, 0, sizeof(conf_file_t));

    pcf->cf_pfConfFile = fopen(pstrPath, "r");
    if (pcf->cf_pfConfFile == NULL)
    {
        u32Ret = JF_ERR_FILE_NOT_FOUND;
    }

    return u32Ret;
}

u32 closeConfFile(conf_file_t * pcf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
    assert(pcf != NULL);
        
    if (pcf->cf_pfConfFile != NULL)
    {
        fclose(pcf->cf_pfConfFile);
    }
            
    memset(pcf, 0, sizeof(conf_file_t));

    return u32Ret;
}

u32 getConfFileInt(conf_file_t * pcf, 
    const olchar_t * pstrTag, olint_t nDefault, olint_t * pnValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    char strLine[MAX_CONFFILE_LINE_LEN];
    olint_t nRet = 0;
    
    assert((pcf != NULL) && (pstrTag != NULL) && (pnValue != NULL));

    rewind(pcf->cf_pfConfFile); 
    u32Ret = _getValueStringByTag(pcf, pstrTag, strLine);
    while (u32Ret == JF_ERR_NO_ERROR)
    {
        nRet = sscanf(strLine, "%d", pnValue);
        if (nRet == 1)
        {
            /*printf("found olint_t option %s = %d\n", pstrTag, *pnValue);*/
            break;
        }
        else
        {
            u32Ret = _getValueStringByTag(pcf, pstrTag, strLine);
        }
    }
        
    if (u32Ret == JF_ERR_NOT_FOUND)
    {
        /* set the option value to default */
        u32Ret = JF_ERR_NO_ERROR;
        *pnValue = nDefault;
    }
    
    return u32Ret;
}

u32 getConfFileString(conf_file_t * pcf,
    const olchar_t * pstrTag, const olchar_t * pstrDefault, 
    olchar_t * pstrValueBuf, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[MAX_CONFFILE_LINE_LEN];
    olsize_t size = 0;
    
    assert(pcf != NULL && pstrTag != NULL && pstrValueBuf != NULL);

    rewind(pcf->cf_pfConfFile); 
    u32Ret = _getValueStringByTag(pcf, pstrTag, strLine);
    while (u32Ret == JF_ERR_NO_ERROR)
    {
        size = ol_strlen(strLine);
        if (size > 0)
        {
            if(size < sBuf)
            {
                ol_strcpy(pstrValueBuf, strLine);
                break;
            }
            else
            {
                u32Ret = JF_ERR_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            u32Ret = _getValueStringByTag(pcf, pstrTag, strLine);
        }
    }
        
    if (u32Ret == JF_ERR_NOT_FOUND && pstrDefault != NULL)
    {
        /* set the option value to default */
        size = ol_strlen(pstrDefault);
        if (size < sBuf)
        {
            ol_strcpy(pstrValueBuf, pstrDefault);
            u32Ret = JF_ERR_NO_ERROR;
        }
        else
        {
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
        }
    }
    
    return u32Ret;
}

/*---------------------------------------------------------------------------*/


