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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "stringparse.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/** Determines if a buffer offset is a delimiter
 *
 *  - Notes:  
 *    -# Used by string parsing methods
 *
 *  @param pstrBuf : olchar_t * <BR> 
 *     @b [in] The buffer to check 
 *  @param sOffset : olsize_t <BR> 
 *     @b [in] The offset of the buffer to check 
 *  @param sBuf : olsize_t <BR> 
 *     @b [in] The size of the buffer to check 
 *  @param pstrDelimiter : olchar_t * <BR> 
 *     @b [in] The delimiter we are looking for 
 *  @param sDelimiter : olsize_t <BR> 
 *     @b [in] The length of the delimiter
  
 *  @return return 0 if no match;
 *          return nonzero otherwise   
 */
static boolean_t _isDelimiter(olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    // For simplicity sake, we'll assume a match unless proven otherwise
    boolean_t bRet = TRUE;
    olint_t i = 0;

    if (sOffset + sDelimiter > sBuf)
    {
        // If the offset plus delimiter length is greater than the sBuf
        // There can't possible be a match, so don't bother looking
        return FALSE;
    }

    for (i = 0; i < sDelimiter; ++i)
    {
        if (pstrBuf[sOffset + i] != pstrDelimiter[i])
        {
            // Uh oh! Can't possibly be a match now!
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

/** Parses a string into a linked list of tokens.
 *
 *  - Notes:  
 *    -# Differs from parseString, in that this method 
 *       ignores characters contained within quotation marks, 
 *       whereas parseString does not.
 *
 *  @param pstrBuf : olchar_t * <BR> 
 *     @b [in] The buffer to parse 
 *  @param sOffset : olsize_t <BR> 
 *     @b [in] The offset of the buffer to start parsing 
 *  @param sBuf : olsize_t <BR> 
 *     @b [in] The size of the buffer to parse 
 *  @param pstrDelimiter : olchar_t * <BR> 
 *     @b [in] The delimiter 
 *  @param sDelimiter : olsize_t <BR> 
 *     @b [in] The length of the delimiter 
 *
 *  @return return A list of tokens   
 */
u32 parseStringAdv(parse_result_t ** ppResult, olchar_t * pstrBuf,
    olsize_t sOffset, olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * ppr;
    olint_t i = 0;
    olchar_t * token = NULL;
    olint_t tokenlength = 0;
    parse_result_field_t *pprf;
    olint_t ignore = 0;
    olchar_t cDelimiter = 0;

    u32Ret = xmalloc((void **)&ppr, sizeof(parse_result_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(ppr, 0, sizeof(parse_result_t));

        // By default we will always return at least one token, which will be the
        // entire string if the delimiter is not found.

        // Iterate through the string to find delimiters
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < (sBuf + sOffset)) && (u32Ret == OLERR_NO_ERROR); ++i)
        {
            if (cDelimiter == 0)
            {
                if (pstrBuf[i] == '"')
                {
                    // ignore everything inside double quotes
                    cDelimiter = '"';
                    ignore = 1;
                }
                else
                {
                    if (pstrBuf[i] == '\'')
                    {
                        // Ignore everything inside single quotes
                        cDelimiter = '\'';
                        ignore = 1;
                    }
                }
            }
            else
            {
                // Once we isolated everything inside double or single quotes, 
                // we can get on with the real parsing
                if (pstrBuf[i] == cDelimiter)
                {
                    ignore = ((ignore == 0) ? 1 : 0);
                }
            }

            if (ignore == 0 &&
                _isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                // We found a delimiter in the string
                u32Ret = xmalloc((void **)&pprf, sizeof(parse_result_field_t));
                if (u32Ret == OLERR_NO_ERROR)
                {
                    memset(pprf, 0, sizeof(parse_result_field_t));
                    pprf->prf_pstrData = token;
                    pprf->prf_sData = tokenlength;
                    pprf->prf_pprfNext = NULL;
                    if (ppr->pr_pprfFirst != NULL)
                    {
                        ppr->pr_pprfLast->prf_pprfNext = pprf;
                        ppr->pr_pprfLast = pprf;
                    }
                    else
                    {
                        ppr->pr_pprfFirst = pprf;
                        ppr->pr_pprfLast = pprf;
                    }

                    // After we populate the values, we advance the token to after 
                    // the delimiter to prep for the next token
                    ++ppr->pr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                // No match yet, so just increment this counter
                ++tokenlength;
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        // Create a result for the last token, since it won't be caught in 
        // the above loop. because if there are no more delimiters, than 
        // the entire last portion of the string since the 
        // last delimiter is the token
        u32Ret = xmalloc((void **)&pprf, sizeof(parse_result_field_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(pprf, 0, sizeof(parse_result_field_t));

        pprf->prf_pstrData = token;
        pprf->prf_sData = tokenlength;
        pprf->prf_pprfNext = NULL;
        if (ppr->pr_pprfFirst != NULL)
        {
            ppr->pr_pprfLast->prf_pprfNext = pprf;
            ppr->pr_pprfLast = pprf;
        }
        else
        {
            ppr->pr_pprfFirst = pprf;
            ppr->pr_pprfLast = pprf;
        }
        ++ppr->pr_u32NumOfResult;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppResult = ppr;
    }
    else if (ppr != NULL)
    {
        destroyParseResult(&ppr);
    }

    return u32Ret;
}

/** Parse a string into a linked list of tokens.
 *
 *  - Notes:  
 *    -# Differs from parseStringAdv, in that this method 
 *       does not ignore characters contained within
 *       quotation marks, whereas parseStringAdv does.
 *
 *  @param pstrBuf : olchar_t * <BR> 
 *     @b [in] The buffer to parse 
 *  @param sOffset : olsize_t <BR> 
 *     @b [in] The offset of the buffer to start parsing 
 *  @param sBuf : olsize_t <BR> 
 *     @b [in] The length of the buffer to parse 
 *  @param pstrDelimiter : olchar_t * <BR> 
 *     @b [in] The delimiter 
 *  @param sDelimiter : olsize_t <BR> 
 *     @b [in] The length of the delimiter 
 * 
 *  @return return A list of tokens   
 */
u32 parseString(parse_result_t ** ppResult, olchar_t * pstrBuf,
    olsize_t sOffset, olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter)
{
    u32 u32Ret = OLERR_NO_ERROR;
    parse_result_t * ppr;
    olint_t i = 0;
    olchar_t * token = NULL;
    olsize_t tokenlength = 0;
    parse_result_field_t *pprf;

    u32Ret = xmalloc((void **)&ppr, sizeof(parse_result_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(ppr, 0, sizeof(parse_result_t));

        /*By default we will always return at least one token, which will be the
          entire string if the delimiter is not found.
          Iterate through the string to find delimiters*/
        token = pstrBuf + sOffset;
        for (i = sOffset; (i < sBuf) && (u32Ret == OLERR_NO_ERROR); ++i)
        {
            if (_isDelimiter(pstrBuf, i, sBuf, pstrDelimiter, sDelimiter))
            {
                /*We found a delimiter in the string*/
                u32Ret = xmalloc((void **)&pprf, sizeof(parse_result_field_t));
                if (u32Ret == OLERR_NO_ERROR)
                {
                    memset(pprf, 0, sizeof(parse_result_field_t));

                    pprf->prf_pstrData = token;
                    pprf->prf_sData = tokenlength;
                    pprf->prf_pprfNext = NULL;
                    if (ppr->pr_pprfFirst != NULL)
                    {
                        ppr->pr_pprfLast->prf_pprfNext = pprf;
                        ppr->pr_pprfLast = pprf;
                    }
                    else
                    {
                        ppr->pr_pprfFirst = pprf;
                        ppr->pr_pprfLast = pprf;
                    }

                    // After we populate the values, we advance the token to 
                    // after the delimiter to prep for the next token
                    ++ppr->pr_u32NumOfResult;
                    i = i + sDelimiter - 1;
                    token = token + tokenlength + sDelimiter;
                    tokenlength = 0;
                }
            }
            else
            {
                // No match yet, so just increment this counter
                ++tokenlength;
            }
        }
    }

    if ((u32Ret == OLERR_NO_ERROR) && (tokenlength > 0))
    {
        // Create a result for the last token, since it won't be caught 
        // in the above loop because if there are no more delimiters, 
        // than the entire last portion of the string since the 
        // last delimiter is the token
        u32Ret = xmalloc((void **)&pprf, sizeof(parse_result_field_t));
        if (u32Ret == OLERR_NO_ERROR)
        {
            memset(pprf, 0, sizeof(parse_result_field_t));

            pprf->prf_pstrData = token;
            pprf->prf_sData = tokenlength;
            pprf->prf_pprfNext = NULL;
            if (ppr->pr_pprfFirst != NULL)
            {
                ppr->pr_pprfLast->prf_pprfNext = pprf;
                ppr->pr_pprfLast = pprf;
            }
            else
            {
                ppr->pr_pprfFirst = pprf;
                ppr->pr_pprfLast = pprf;
            }
            ++ppr->pr_u32NumOfResult;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        *ppResult = ppr;
    }
    else if (ppr != NULL)
    {
        destroyParseResult(&ppr);
    }

    return u32Ret;
}

/** Frees resources associated with the list of tokens returned 
 *  from parseString and parseStringAdv.
 *
 *  @param result : parse_result_t * <BR> 
 *     @b [in] The list of tokens to free 
 *
 *  @return void
 */
u32 destroyParseResult(parse_result_t ** ppResult)
{
    u32 u32Ret = OLERR_NO_ERROR;
    // All of these nodes only contain pointers
    // so we just need to iterate through all the nodes and free them
    parse_result_field_t *node = (*ppResult)->pr_pprfFirst;
    parse_result_field_t *temp;

    while (node != NULL)
    {
        temp = node->prf_pprfNext;
        xfree((void **)&node);
        node = temp;
    }

    xfree((void **)ppResult);

    return u32Ret;
}

/** remove the blank space(s) from the left and the right of the string
 *
 *  @param
 *       [out] pu8Dest, the output string after removing the blank space(s)
 *       [in] pu8Source, the input string to be removed the blank space(s)
 *  Return: None.
 *  Remarks: None.     
 */
void skipBlank(olchar_t * pstrDest, const olchar_t * pstrSource)
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

boolean_t isBlankLine(const olchar_t * pstrLine)
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

u32 freeString(olchar_t ** ppstrStr)
{
    u32 u32Ret = OLERR_NO_ERROR;

    xfree((void **)ppstrStr);

    return u32Ret;
}

u32 dupStringWithLen(olchar_t ** ppstrDest, const olchar_t * pstrSource,
    const olsize_t sSource)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pstr;

    *ppstrDest = NULL;
    if (sSource > 0)
    {
        u32Ret = xmalloc((void **)&pstr, sSource + 1);
        if (u32Ret == OLERR_NO_ERROR)
        {
            memcpy(pstr, pstrSource, sSource);
            pstr[sSource] = '\0';
            *ppstrDest = pstr;
        }
    }

    return u32Ret;
}

u32 dupString(olchar_t ** ppstrDest, const olchar_t * pstrSource)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pStr;
    olint_t nLen;

    *ppstrDest = NULL;
    nLen = ol_strlen(pstrSource);

    u32Ret = xmalloc((void **)&pStr, nLen + 1);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (nLen > 0)
            ol_strcpy(pStr, pstrSource);
        else
            pStr[0] = '\0';

        *ppstrDest = pStr;
    }

    return u32Ret;
}

void filterComment(olchar_t * pstrDest, const olchar_t * pstrComment)
{
    olchar_t *p;

    assert((pstrDest != NULL) && (pstrComment != NULL));

    p = strstr(pstrDest, pstrComment);
    if (p != NULL)
    {
        *p = '\0';
    }
}

void lowerString(olchar_t * pstr)
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

void upperString(olchar_t * pstr)
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

void removeLeadingSpace(olchar_t * pstr)
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

void removeTailingSpace(olchar_t * pstr)
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

/** Removes all leading and tailing blankspaces, and 
 *  replaces all multiple occurences of blank spaces
 *  with a single one.
 *  eg:
 *       "  this   is  a   test string"
 *  =>    "this is a test string"
 *
 *  @param str The string that should be trimmed.
 *
 *  @returns The trimmed string.
 */
void trimBlankOfString(olchar_t * pstr)
{
    u32 u32Index = 0, sBuf, u32SpaceLen = 0;

    assert(pstr != NULL);

    removeLeadingSpace(pstr);
    removeTailingSpace(pstr);

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

/** break string to line with width, the line terminator is at the best
 *  suitable position. 
 *
 *  @param str   The string that should be wrapped.
 *  @param width The maximal column count of the wrapped string.
 *  @returns The wrapped string.
 */
char * breakStringToLine(olchar_t * pstr, olsize_t sWidth)
{
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

    return NULL;
}

u32 locateSubString(const olchar_t * pstr, const olchar_t * pstrSubStr, olchar_t ** ppstrLoc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t * pstrLoc;

    pstrLoc = strstr(pstr, pstrSubStr);
    if (pstrLoc == NULL)
        u32Ret = OLERR_SUBSTRING_NOT_FOUND;
    else
    {
        if (ppstrLoc != NULL)
            *ppstrLoc = pstrLoc;
    }

    return u32Ret;
}

/** replaceString() replaces the first occurence of needle
 *  in the string src with the string subst. If no occurence
 *  of needle could be found in src, NULL is returned, otherwise
 *  the starting index of needle inside src. src needs to be
 *  big enough to store the resulting string.
 *
 *  @param pstrSrc     The string that should be modified.
 *  @param u32BufLen  the size of the buffer containing the source string
 *  @param pstrNeedle  The pattern that should be replaced.
 *  @param pstrSubst   The pattern that should be used for
 *                replacing.
 *
 *  @returns NULL if no occurence of needle could be found
 *  in src, otherwise the starting idx of needle inside src.
 */
char * replaceString(olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle,
    olchar_t * pstrSubst)
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

