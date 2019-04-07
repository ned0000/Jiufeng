/**
 *  @file parsestring.c
 *
 *  @brief The string parse implementation file
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
#include <errno.h>
#include <ctype.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "stringparse.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/** Determines if a buffer offset is a delimiter
 *
 *  @param pstrBuf [in] The buffer to check 
 *  @param sOffset [in] The offset of the buffer to check 
 *  @param sBuf [in] The size of the buffer to check 
 *  @param pstrDelimiter [in] The delimiter we are looking for 
 *  @param sDelimiter [in] The length of the delimiter
  
 *  @return if the delimiter is found
 *  @retval TRUE the delimiter is found
 *  @retval FALSE the delimiter is not found
 */
static boolean_t _isDelimiter(olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    /* for simplicity sake, we'll assume a match unless proven otherwise */
    boolean_t bRet = TRUE;
    olint_t i = 0;

    if (sOffset + sDelimiter > sBuf)
    {
        /* If the offset plus delimiter length is greater than the sBuf
           There can't possible be a match, so don't bother looking */
        return FALSE;
    }

    for (i = 0; i < sDelimiter; ++i)
    {
        if (pstrBuf[sOffset + i] != pstrDelimiter[i])
        {
            /* Can't possibly be a match now */
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

static boolean_t _isblank(olchar_t c)
{
    boolean_t bRet = FALSE;

    if  (c == ' ' || c == '\t')
        bRet = TRUE;

    return bRet;
}

/* --- public routine section ---------------------------------------------- */

/* String Parsing Methods */

u32 jf_string_parseAdv(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * ppr;
    olint_t i = 0;
    olchar_t * token = NULL;
    olint_t tokenlength = 0;
    jf_string_parse_result_field_t *pjsprf;
    olint_t ignore = 0;
    olchar_t cDelimiter = 0;

    u32Ret = jf_mem_alloc((void **)&ppr, sizeof(jf_string_parse_result_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(ppr, 0, sizeof(jf_string_parse_result_t));

        /* By default we will always return at least one token, which will be the
           entire string if the delimiter is not found.
           Iterate through the string to find delimiters  */
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < (sBuf + sOffset)) && (u32Ret == JF_ERR_NO_ERROR); ++i)
        {
            if (cDelimiter == 0)
            {
                if (pstrBuf[i] == '"')
                {
                    /* ignore everything inside double quotes */
                    cDelimiter = '"';
                    ignore = 1;
                }
                else
                {
                    if (pstrBuf[i] == '\'')
                    {
                        /* ignore everything inside single quotes */
                        cDelimiter = '\'';
                        ignore = 1;
                    }
                }
            }
            else
            {
                /* once we isolated everything inside double or single quotes, 
                   we can get on with the real parsing */
                if (pstrBuf[i] == cDelimiter)
                {
                    ignore = ((ignore == 0) ? 1 : 0);
                }
            }

            if (ignore == 0 &&
                _isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                /* we found a delimiter in the string */
                u32Ret = jf_mem_alloc((void **)&pjsprf, sizeof(jf_string_parse_result_field_t));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    memset(pjsprf, 0, sizeof(jf_string_parse_result_field_t));
                    pjsprf->jsprf_pstrData = token;
                    pjsprf->jsprf_sData = tokenlength;
                    pjsprf->jsprf_pjsprfNext = NULL;
                    if (ppr->jspr_pjsprfFirst != NULL)
                    {
                        ppr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                        ppr->jspr_pjsprfLast = pjsprf;
                    }
                    else
                    {
                        ppr->jspr_pjsprfFirst = pjsprf;
                        ppr->jspr_pjsprfLast = pjsprf;
                    }

                    /* after we populate the values, we advance the token to after 
                       the delimiter to prep for the next token */
                    ++ppr->jspr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                /* no match yet, so just increment this counter */
                ++tokenlength;
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* create a result for the last token, since it won't be caught in 
           the above loop. because if there are no more delimiters */
        u32Ret = jf_mem_alloc((void **)&pjsprf, sizeof(jf_string_parse_result_field_t));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pjsprf, 0, sizeof(jf_string_parse_result_field_t));

        pjsprf->jsprf_pstrData = token;
        pjsprf->jsprf_sData = tokenlength;
        pjsprf->jsprf_pjsprfNext = NULL;
        if (ppr->jspr_pjsprfFirst != NULL)
        {
            ppr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
            ppr->jspr_pjsprfLast = pjsprf;
        }
        else
        {
            ppr->jspr_pjsprfFirst = pjsprf;
            ppr->jspr_pjsprfLast = pjsprf;
        }
        ++ppr->jspr_u32NumOfResult;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppResult = ppr;
    }
    else if (ppr != NULL)
    {
        jf_string_destroyParseResult(&ppr);
    }

    return u32Ret;
}

u32 jf_string_parse(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * ppr;
    olint_t i = 0;
    olchar_t * token = NULL;
    olsize_t tokenlength = 0;
    jf_string_parse_result_field_t *pjsprf;

    u32Ret = jf_mem_alloc((void **)&ppr, sizeof(jf_string_parse_result_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(ppr, 0, sizeof(jf_string_parse_result_t));

        /*By default we will always return at least one token, which will be the
          entire string if the delimiter is not found.
          Iterate through the string to find delimiters*/
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < sBuf) && (u32Ret == JF_ERR_NO_ERROR); ++i)
        {
            if (_isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                /*We found a delimiter in the string*/
                u32Ret = jf_mem_alloc((void **)&pjsprf, sizeof(jf_string_parse_result_field_t));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    memset(pjsprf, 0, sizeof(jf_string_parse_result_field_t));

                    pjsprf->jsprf_pstrData = token;
                    pjsprf->jsprf_sData = tokenlength;
                    pjsprf->jsprf_pjsprfNext = NULL;
                    if (ppr->jspr_pjsprfFirst != NULL)
                    {
                        ppr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                        ppr->jspr_pjsprfLast = pjsprf;
                    }
                    else
                    {
                        ppr->jspr_pjsprfFirst = pjsprf;
                        ppr->jspr_pjsprfLast = pjsprf;
                    }

                    /* After we populate the values, we advance the token to 
                       after the delimiter to prep for the next token */
                    ++ppr->jspr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                /* No match yet, so just increment this counter */
                ++tokenlength;
            }
        }
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (tokenlength >= 0))
    {
        /* Create a result for the last token, since it won't be caught in the
           above loop because if there are no more delimiters. The last token
           is counted in even the length is 0 */
        u32Ret = jf_mem_alloc((void **)&pjsprf, sizeof(jf_string_parse_result_field_t));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            memset(pjsprf, 0, sizeof(jf_string_parse_result_field_t));

            pjsprf->jsprf_pstrData = token;
            pjsprf->jsprf_sData = tokenlength;
            pjsprf->jsprf_pjsprfNext = NULL;
            if (ppr->jspr_pjsprfFirst != NULL)
            {
                ppr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                ppr->jspr_pjsprfLast = pjsprf;
            }
            else
            {
                ppr->jspr_pjsprfFirst = pjsprf;
                ppr->jspr_pjsprfLast = pjsprf;
            }
            ++ppr->jspr_u32NumOfResult;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppResult = ppr;
    }
    else if (ppr != NULL)
    {
        jf_string_destroyParseResult(&ppr);
    }

    return u32Ret;
}

u32 jf_string_destroyParseResult(jf_string_parse_result_t ** ppResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    /* all of these nodes only contain pointers
       so we just need to iterate through all the nodes and free them */
    jf_string_parse_result_field_t *node = (*ppResult)->jspr_pjsprfFirst;
    jf_string_parse_result_field_t *temp;

    while (node != NULL)
    {
        temp = node->jsprf_pjsprfNext;
        jf_mem_free((void **)&node);
        node = temp;
    }

    jf_mem_free((void **)ppResult);

    return u32Ret;
}

void jf_string_skipBlank(olchar_t * pstrDest, const olchar_t * pstrSource)
{
    olsize_t right, left = 0;

    while ((pstrSource[left] != 0) && (pstrSource[left] == ' '))
    {
        left++;
    }

    right = ol_strlen(pstrSource);

    if (pstrSource[left] != 0)
    {
        while (pstrSource[right-1] == ' ')
        {
            right--;
        }

        ol_strcpy(pstrDest, &pstrSource[left]);
    }

    pstrDest[right - left] = 0;
}

boolean_t jf_string_isBlankLine(const olchar_t * pstrLine)
{
    boolean_t bRet = TRUE;

    while (*pstrLine)
    {
        if ((*pstrLine != '\n') && (! _isblank(*pstrLine)))
        {
            bRet = FALSE;
            break;
        }
        else
        {
            pstrLine++;
        }
    }

    return bRet;
}

u32 jf_string_free(olchar_t ** ppstrStr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_mem_free((void **)ppstrStr);

    return u32Ret;
}

u32 jf_string_duplicateWithLen(
    olchar_t ** ppstrDest, const olchar_t * pstrSource, const olsize_t sSource)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstr;

    *ppstrDest = NULL;
    if (sSource > 0)
    {
        u32Ret = jf_mem_alloc((void **)&pstr, sSource + 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            memcpy(pstr, pstrSource, sSource);
            pstr[sSource] = '\0';
            *ppstrDest = pstr;
        }
    }

    return u32Ret;
}

u32 jf_string_duplicate(olchar_t ** ppstrDest, const olchar_t * pstrSource)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pStr;
    olint_t nLen;

    *ppstrDest = NULL;
    nLen = ol_strlen(pstrSource);

    u32Ret = jf_mem_alloc((void **)&pStr, nLen + 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (nLen > 0)
            ol_strcpy(pStr, pstrSource);
        else
            pStr[0] = '\0';

        *ppstrDest = pStr;
    }

    return u32Ret;
}

void jf_string_filterComment(olchar_t * pstrDest, const olchar_t * pstrComment)
{
    olchar_t *p;

    assert((pstrDest != NULL) && (pstrComment != NULL));

    p = strstr(pstrDest, pstrComment);
    if (p != NULL)
    {
        *p = '\0';
    }
}

void jf_string_lower(olchar_t * pstr)
{
    u32 u32Index, sBuf;

    assert(pstr != NULL);

    u32Index = 0;
    sBuf = ol_strlen(pstr);
    while (u32Index < sBuf)
    {
        pstr[u32Index] = tolower(pstr[u32Index]);

        u32Index ++;
    }
}

void jf_string_upper(olchar_t * pstr)
{
    u32 u32Index, sBuf;

    assert(pstr != NULL);

    u32Index = 0;
    sBuf = ol_strlen(pstr);
    while (u32Index < sBuf)
    {
        pstr[u32Index] = toupper(pstr[u32Index]);

        u32Index ++;
    }
}

void jf_string_removeLeadingSpace(olchar_t * pstr)
{
    u32 u32Index = 0, sBuf;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

    while (u32Index < sBuf)
    {
        if (pstr[u32Index] != ' ')
            break;

        u32Index ++;
    }

    if (u32Index != 0)
    {
        memmove(pstr, pstr + u32Index, sBuf - u32Index + 1);
    }
}

void jf_string_removeTailingSpace(olchar_t * pstr)
{
    u32 sBuf;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

    while (sBuf > 0)
    {
        if (pstr[sBuf - 1] != ' ')
            break;

        sBuf --;
    }

    pstr[sBuf] = '\0';
}

void jf_string_trimBlank(olchar_t * pstr)
{
    u32 u32Index = 0, sBuf, u32SpaceLen = 0;

    assert(pstr != NULL);

    jf_string_removeLeadingSpace(pstr);
    jf_string_removeTailingSpace(pstr);

    sBuf = ol_strlen(pstr);

    while (u32Index < sBuf)
    {
        u32SpaceLen = 0;

        if (pstr[u32Index] == ' ')
        {
            u32SpaceLen ++;

            while (pstr[u32Index + u32SpaceLen] != ' ')
                u32SpaceLen ++;
        }

        if (u32SpaceLen > 1)
        {
            memmove(pstr + u32Index, pstr + u32Index + u32SpaceLen,
                sBuf - u32Index - u32SpaceLen + 1);

            u32Index += u32SpaceLen;
        }
        else
            u32Index ++;
    }
}

u32 jf_string_breakToLine(olchar_t * pstr, olsize_t sWidth)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	olsize_t sIndex = 0, sBuf;
	olsize_t sLastSpace = 0;
	olsize_t sCol = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

	while (sIndex < sBuf)
	{
		if (pstr[sIndex] == ' ')
		{
			sLastSpace = sIndex;
		}

		if (++ sCol == sWidth)
		{
			pstr[sLastSpace] = '\n';
			sCol = 0;
		}

		if (pstr[sIndex] == '\n')
		{
			sCol = 0;
		}

		sIndex ++;
	}

    return u32Ret;
}

u32 jf_string_locateSubString(
    const olchar_t * pstr, const olchar_t * pstrSubStr, olchar_t ** ppstrLoc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrLoc;

    pstrLoc = strstr(pstr, pstrSubStr);
    if (pstrLoc == NULL)
        u32Ret = JF_ERR_SUBSTRING_NOT_FOUND;
    else
    {
        if (ppstrLoc != NULL)
            *ppstrLoc = pstrLoc;
    }

    return u32Ret;
}

olchar_t * jf_string_replace(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle, olchar_t * pstrSubst)
{    /* "The string NEEDLE will be substituted"
      *                   ^- beg
      */
    olchar_t * beg = NULL;
    /* "The string SUBST will be substituted"
     *                  ^- end
     */
    olchar_t * end = NULL;
    /* "The string NEEDLE will be substituted"
     *             ^- pos
     */
    olchar_t * pos = NULL;
    u32 u32NeedleLen, u32SubstLen;

    assert((pstrSrc != NULL) && (sBuf > 0));
    assert((pstrNeedle != NULL) && (pstrSubst != NULL));

    u32NeedleLen = ol_strlen(pstrNeedle);
    u32SubstLen = ol_strlen(pstrSubst);

    if ((u32NeedleLen == 0) || (u32SubstLen == 0))
        return NULL;

    if ((u32SubstLen > u32NeedleLen) &&
        (u32SubstLen - u32NeedleLen + ol_strlen(pstrSrc) >= sBuf))
        return NULL;

    if ((pos = strstr(pstrSrc, pstrNeedle)) != NULL)
    {
        beg = pos + ol_strlen(pstrNeedle);
        end = pos + ol_strlen(pstrSubst);

        /* move the latter part of the string to index end:
         * "The string NEEDL will be substituted"
         *              end-^
         */
        memmove(end, beg, ol_strlen(beg) + 1);
        /* now put the substitution string on place.
         * "The string SUBST will be substituted"
         *         pos-^    ^-end
         */
        memmove(pos, pstrSubst, ol_strlen(pstrSubst));
    }

    return pos;
}

/*---------------------------------------------------------------------------*/

