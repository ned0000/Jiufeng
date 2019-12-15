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

/*------------------------------------------------------------------------------------------------*/

