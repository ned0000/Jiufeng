/**
 *  @file
 *
 *  @brief The common routine shared in the logger library
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
#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */
boolean_t isSysErrorCode(u32 u32ErrCode)
{
    boolean_t bRet = FALSE;

    if (u32ErrCode == JF_ERR_OPERATION_FAIL)
    {
        bRet = TRUE;
    }
    else
    {
        if (u32ErrCode & JF_ERR_CODE_FLAG_SYSTEM)
            bRet = TRUE;
    }

    return bRet;
}

/*------------------------------------------------------------------------------------------------*/


