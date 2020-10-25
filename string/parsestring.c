/**
 *  @file parsestring.c
 *
 *  @brief The string parse implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <ctype.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_jiukun.h"

#include "stringcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/** Determines if a buffer offset is a delimiter.
 *
 *  @param pstrBuf [in] The buffer to check.
 *  @param sOffset [in] The offset of the buffer to check.
 *  @param sBuf [in] The size of the buffer to check.
 *  @param pstrDelimiter [in] The delimiter we are looking for.
 *  @param sDelimiter [in] The length of the delimiter.
  
 *  @return The status if the delimiter is found.
 *  @retval TRUE The delimiter is found.
 *  @retval FALSE The delimiter is not found.
 */
static boolean_t _isDelimiter(
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, olchar_t * pstrDelimiter,
    olsize_t sDelimiter)
{
    /*For simplicity sake, assume a match unless proven otherwise.*/
    boolean_t bRet = TRUE;
    olint_t i = 0;

    if (sOffset + sDelimiter > sBuf)
    {
        /*If the offset plus delimiter length is greater than the buffer size, there can't possible
          be a match.*/
        return FALSE;
    }

    for (i = 0; i < sDelimiter; ++i)
    {
        if (pstrBuf[sOffset + i] != pstrDelimiter[i])
        {
            /*Can't possibly be a match now.*/
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

static boolean_t _isblank(olchar_t c)
{
    boolean_t bRet = FALSE;

    if  ((c == JF_STRING_SPACE_CHAR) || (c == JF_STRING_HORIZONTAL_TAB_CHAR))
        bRet = TRUE;

    return bRet;
}

/* --- public routine section ------------------------------------------------------------------- */

/* String Parsing Methods */

u32 jf_string_parseAdv(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf,
    olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pjspr = NULL;
    olint_t i = 0;
    olchar_t * token = NULL;
    olint_t tokenlength = 0;
    jf_string_parse_result_field_t * pjsprf = NULL;
    olint_t ignore = 0;
    olchar_t cDelimiter = 0;

    /*Allocate memory for the parse result.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pjspr, sizeof(jf_string_parse_result_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pjspr, sizeof(*pjspr));

        /*By default we will always return at least one token, which will be the entire string if
          the delimiter is not found. Iterate through the string to find delimiters.*/
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < (sBuf + sOffset)) && (u32Ret == JF_ERR_NO_ERROR); ++i)
        {
            if (cDelimiter == 0)
            {
                if (pstrBuf[i] == JF_STRING_DOUBLE_QUOTES_CHAR)
                {
                    /*Ignore everything inside double quotes.*/
                    cDelimiter = JF_STRING_DOUBLE_QUOTES_CHAR;
                    ignore = 1;
                }
                else
                {
                    if (pstrBuf[i] == JF_STRING_SINGLE_QUOTE_CHAR)
                    {
                        /*Ignore everything inside single quotes.*/
                        cDelimiter = JF_STRING_SINGLE_QUOTE_CHAR;
                        ignore = 1;
                    }
                }
            }
            else
            {
                /*Once we isolated everything inside double or single quotes, we can go on with the
                  real parsing.*/
                if (pstrBuf[i] == cDelimiter)
                {
                    ignore = ((ignore == 0) ? 1 : 0);
                }
            }

            if ((ignore == 0) && _isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                /*Found a delimiter in the string.*/
                u32Ret = jf_jiukun_allocMemory((void **)&pjsprf, sizeof(*pjsprf));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    ol_bzero(pjsprf, sizeof(*pjsprf));
                    pjsprf->jsprf_pstrData = token;
                    pjsprf->jsprf_sData = tokenlength;
                    pjsprf->jsprf_pjsprfNext = NULL;

                    /*Add parse field to list.*/
                    if (pjspr->jspr_pjsprfFirst != NULL)
                    {
                        pjspr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                        pjspr->jspr_pjsprfLast = pjsprf;
                    }
                    else
                    {
                        pjspr->jspr_pjsprfFirst = pjsprf;
                        pjspr->jspr_pjsprfLast = pjsprf;
                    }

                    /*Advance the token to after the delimiter to prepare for the next token.*/
                    ++pjspr->jspr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                /*No match yet, so just increment this counter.*/
                ++tokenlength;
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Create a result for the last token, since it won't be caught in the above loop. because if
          there are no more delimiters.*/
        u32Ret = jf_jiukun_allocMemory((void **)&pjsprf, sizeof(*pjsprf));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pjsprf, sizeof(*pjsprf));

        pjsprf->jsprf_pstrData = token;
        pjsprf->jsprf_sData = tokenlength;
        pjsprf->jsprf_pjsprfNext = NULL;

        /*Add parse field to list.*/
        if (pjspr->jspr_pjsprfFirst != NULL)
        {
            pjspr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
            pjspr->jspr_pjsprfLast = pjsprf;
        }
        else
        {
            pjspr->jspr_pjsprfFirst = pjsprf;
            pjspr->jspr_pjsprfLast = pjsprf;
        }
        ++pjspr->jspr_u32NumOfResult;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppResult = pjspr;
    }
    else if (pjspr != NULL)
    {
        jf_string_destroyParseResult(&pjspr);
    }

    return u32Ret;
}

u32 jf_string_parse(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf,
    olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_t * pjspr = NULL;
    olint_t i = 0;
    olchar_t * token = NULL;
    olsize_t tokenlength = 0;
    jf_string_parse_result_field_t * pjsprf = NULL;

    /*Allocate memory for the parse result.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pjspr, sizeof(jf_string_parse_result_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pjspr, sizeof(*pjspr));

        /*By default we will always return at least one token, which will be the entire string if
          the delimiter is not found. Iterate through the string to find delimiters.*/
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < sBuf) && (u32Ret == JF_ERR_NO_ERROR); ++i)
        {
            if (_isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                /*Found a delimiter in the string.*/
                u32Ret = jf_jiukun_allocMemory((void **)&pjsprf, sizeof(*pjsprf));
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    /*Set parse result field.*/
                    ol_bzero(pjsprf, sizeof(*pjsprf));

                    pjsprf->jsprf_pstrData = token;
                    pjsprf->jsprf_sData = tokenlength;
                    pjsprf->jsprf_pjsprfNext = NULL;
                    /*Add the field to list.*/
                    if (pjspr->jspr_pjsprfFirst != NULL)
                    {
                        pjspr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                        pjspr->jspr_pjsprfLast = pjsprf;
                    }
                    else
                    {
                        pjspr->jspr_pjsprfFirst = pjsprf;
                        pjspr->jspr_pjsprfLast = pjsprf;
                    }

                    /*Advance the token to after the delimiter to prepare for the next token.*/
                    ++pjspr->jspr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                /*No match yet, so just increment this counter.*/
                ++tokenlength;
            }
        }
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (tokenlength >= 0))
    {
        /*Create a result for the last token, since it won't be caught in the above loop because
          if there are no more delimiters. The last token is counted in even the length is 0.*/
        u32Ret = jf_jiukun_allocMemory((void **)&pjsprf, sizeof(*pjsprf));
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(pjsprf, sizeof(*pjsprf));

            pjsprf->jsprf_pstrData = token;
            pjsprf->jsprf_sData = tokenlength;
            pjsprf->jsprf_pjsprfNext = NULL;

            /*Add the field to list.*/
            if (pjspr->jspr_pjsprfFirst != NULL)
            {
                pjspr->jspr_pjsprfLast->jsprf_pjsprfNext = pjsprf;
                pjspr->jspr_pjsprfLast = pjsprf;
            }
            else
            {
                pjspr->jspr_pjsprfFirst = pjsprf;
                pjspr->jspr_pjsprfLast = pjsprf;
            }
            ++pjspr->jspr_u32NumOfResult;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppResult = pjspr;
    }
    else if (pjspr != NULL)
    {
        jf_string_destroyParseResult(&pjspr);
    }

    return u32Ret;
}

u32 jf_string_destroyParseResult(jf_string_parse_result_t ** ppResult)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_string_parse_result_field_t * node = (*ppResult)->jspr_pjsprfFirst;
    jf_string_parse_result_field_t * temp = NULL;

    /*All of these nodes only contain pointers, so we just need to iterate through all the nodes
      and free them.*/
    while (node != NULL)
    {
        temp = node->jsprf_pjsprfNext;
        jf_jiukun_freeMemory((void **)&node);
        node = temp;
    }

    jf_jiukun_freeMemory((void **)ppResult);

    return u32Ret;
}

void jf_string_skipBlank(olchar_t * pstrDest, const olchar_t * pstrSource)
{
    olsize_t right = 0, left = 0;

    /*Move the index to the first character which is not blank.*/
    while ((pstrSource[left] != JF_STRING_NULL_CHAR) &&
           (pstrSource[left] == JF_STRING_SPACE_CHAR))
    {
        left++;
    }

    right = ol_strlen(pstrSource);

    if (pstrSource[left] != JF_STRING_NULL_CHAR)
    {
        /*Move the index to the last character which is not blank.*/
        while (pstrSource[right - 1] == JF_STRING_SPACE_CHAR)
        {
            right--;
        }

        /*Copy the string from source to destination.*/
        ol_strcpy(pstrDest, &pstrSource[left]);
    }

    /*Add a null-terminator.*/
    pstrDest[right - left] = JF_STRING_NULL_CHAR;
}

boolean_t jf_string_isBlankLine(const olchar_t * pstrLine)
{
    /*By default, it's a blank line.*/
    boolean_t bRet = TRUE;

    while (*pstrLine)
    {
        if ((*pstrLine != JF_STRING_LINE_FEED_CHAR) && (! _isblank(*pstrLine)))
        {
            /*Found a character which is not blank.*/
            bRet = FALSE;
            break;
        }
        else
        {
            pstrLine ++;
        }
    }

    return bRet;
}

u32 jf_string_free(olchar_t ** ppstrStr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_jiukun_freeMemory((void **)ppstrStr);

    return u32Ret;
}

u32 jf_string_duplicateWithLen(
    olchar_t ** ppstrDest, const olchar_t * pstrSource, const olsize_t sSource)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstr = NULL;

    *ppstrDest = NULL;
    if (sSource > 0)
    {
        /*Allocate 1 more characters for the null-terminator.*/
        u32Ret = jf_jiukun_allocMemory((void **)&pstr, sSource + 1);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_memcpy(pstr, pstrSource, sSource);
            pstr[sSource] = JF_STRING_NULL_CHAR;
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

    /*Allocate 1 more characters for the null-terminator.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pStr, nLen + 1);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (nLen > 0)
            ol_strcpy(pStr, pstrSource);
        else
            pStr[0] = JF_STRING_NULL_CHAR;

        *ppstrDest = pStr;
    }

    return u32Ret;
}

void jf_string_filterComment(olchar_t * pstrDest, const olchar_t * pstrComment)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * p = NULL;

    assert((pstrDest != NULL) && (pstrComment != NULL));

    /*Find the comment.*/
    u32Ret = jf_string_locateSubString(pstrDest, pstrComment, &p);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *p = JF_STRING_NULL_CHAR;
    }
}

void jf_string_lower(olchar_t * pstr)
{
    olsize_t index = 0, sBuf = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);
    while (index < sBuf)
    {
        pstr[index] = tolower(pstr[index]);

        index ++;
    }
}

void jf_string_upper(olchar_t * pstr)
{
    olsize_t index = 0, sBuf = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);
    while (index < sBuf)
    {
        pstr[index] = toupper(pstr[index]);

        index ++;
    }
}

void jf_string_removeLeadingSpace(olchar_t * pstr)
{
    olsize_t index = 0, sBuf = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

    while (index < sBuf)
    {
        /*Find the first character which is not blank.*/
        if (pstr[index] != JF_STRING_SPACE_CHAR)
            break;

        index ++;
    }

    if (index > 0)
    {
        /*Include the trailing null-terminator.*/
        ol_memmove(pstr, pstr + index, sBuf - index + 1);
    }
}

void jf_string_removeTailingSpace(olchar_t * pstr)
{
    olsize_t sBuf = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

    while (sBuf > 0)
    {
        if (pstr[sBuf - 1] != JF_STRING_SPACE_CHAR)
            break;

        sBuf --;
    }

    pstr[sBuf] = JF_STRING_NULL_CHAR;
}

void jf_string_trimBlank(olchar_t * pstr)
{
    olsize_t index = 0, sBuf = 0, sSpace = 0;

    assert(pstr != NULL);

    jf_string_removeLeadingSpace(pstr);
    jf_string_removeTailingSpace(pstr);

    sBuf = ol_strlen(pstr);

    while (index < sBuf)
    {
        sSpace = 0;

        /*Count number of space.*/
        if (pstr[index] == JF_STRING_SPACE_CHAR)
        {
            sSpace ++;

            while (pstr[index + sSpace] == JF_STRING_SPACE_CHAR)
                sSpace ++;
        }

        if (sSpace > 1)
            /*Include the traling null-terminator.*/
            ol_memmove(pstr + index + 1, pstr + index + sSpace, sBuf - index - sSpace + 1);

        index ++;
    }
}

u32 jf_string_breakToLine(olchar_t * pstr, olsize_t sWidth)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
	olsize_t sIndex = 0, sBuf = 0;
	olsize_t sLastSpace = 0;
	olsize_t sCol = 0;

    assert(pstr != NULL);

    sBuf = ol_strlen(pstr);

	while (sIndex < sBuf)
	{
        /*Save the position of the last space.*/
		if (pstr[sIndex] == JF_STRING_SPACE_CHAR)
		{
			sLastSpace = sIndex;
		}

		if (++ sCol == sWidth)
		{
            /*Reach the width, break to line.*/
			pstr[sLastSpace] = JF_STRING_LINE_FEED_CHAR;
			sCol = 0;
		}

        /*Clear the counter for the new line.*/
		if (pstr[sIndex] == JF_STRING_LINE_FEED_CHAR)
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
    olchar_t * pstrLoc = NULL;

    pstrLoc = strstr(pstr, pstrSubStr);
    if (pstrLoc == NULL)
    {
        u32Ret = JF_ERR_SUBSTRING_NOT_FOUND;
    }
    else
    {
        if (ppstrLoc != NULL)
            *ppstrLoc = pstrLoc;
    }

    return u32Ret;
}

olchar_t * jf_string_replace(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle, olchar_t * pstrSubst)
{
    /* "The string NEEDLE will be substituted"
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
    u32 u32NeedleLen = 0, u32SubstLen = 0;

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
        ol_memmove(end, beg, ol_strlen(beg) + 1);
        /* now put the substitution string on place.
         * "The string SUBST will be substituted"
         *         pos-^    ^-end
         */
        ol_memmove(pos, pstrSubst, ol_strlen(pstrSubst));
    }

    return pos;
}

/*------------------------------------------------------------------------------------------------*/
