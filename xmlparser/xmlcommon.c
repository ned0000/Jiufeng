/**
 *  @file xmlcommon.c
 *
 *  @brief Implementation file for common routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_xmlparser.h"

#include "xmlcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t ls_strXmlErrMsg[256];

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

boolean_t isAllSpaceInXmlBuffer(olchar_t * pstr, olsize_t size)
{
    boolean_t bRet = TRUE;
    olsize_t index = 0;

    for (index = 0; index < size; index ++)
    {
        if (pstr[index] != ' ')
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

void initXmlErrMsg(void)
{
    ol_bzero(ls_strXmlErrMsg, sizeof(ls_strXmlErrMsg));
}

void genXmlErrMsg(u32 u32Err, const olchar_t * pData, olsize_t sData)
{
    olchar_t str[64];
    olsize_t size = sizeof(str) - 1;

    if (sData == 0)
    {
        ol_snprintf(
            ls_strXmlErrMsg, sizeof(ls_strXmlErrMsg) - 1, "%s", jf_err_getDescription(u32Err));
    }
    else
    {
        if (size > sData)
            size = sData;

        ol_strncpy(str, pData, size);

        ol_snprintf(
            ls_strXmlErrMsg, sizeof(ls_strXmlErrMsg) - 1, "%s \"%s\".",
            jf_err_getDescription(u32Err), str);
    }
}

void tryGenXmlErrMsg(u32 u32Err, const olchar_t * pData, olsize_t sData)
{
    if (ls_strXmlErrMsg[0] == '\0')
        genXmlErrMsg(u32Err, pData, sData);
}

const olchar_t * jf_xmlparser_getErrMsg(void)
{
    return ls_strXmlErrMsg;
}

/*------------------------------------------------------------------------------------------------*/

