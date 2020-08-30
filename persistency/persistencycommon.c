/**
 *  @file persistencycommon.c
 *
 *  @brief Implementation file for persistency common routine.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

#include "persistencycommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

/** Define the string of persistency type.
 */
static olchar_t * ls_pstrPersistencyType[JF_PERSISTENCY_TYPE_MAX] =
{
    "unknown",
    "sqlite",
    "file"
};

/* --- public routine section ------------------------------------------------------------------- */

const olchar_t * getStringPersistencyType(u8 u8Type)
{
    if (u8Type >= JF_PERSISTENCY_TYPE_MAX)
        u8Type = 0;

    return ls_pstrPersistencyType[u8Type];
}    

/*------------------------------------------------------------------------------------------------*/


