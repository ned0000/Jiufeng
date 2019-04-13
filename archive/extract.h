/**
 *  @file extract.h
 *
 *  @brief extract archive file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef ARCHIVE_EXTRACT_H
#define ARCHIVE_EXTRACT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 extractFromArchive(
    FILE * fpArchive, u8 * pu8Buffer, u32 u32BufLen,
    jf_archive_extract_param_t * pParam);

#endif /*ARCHIVE_EXTRACT_H*/

/*------------------------------------------------------------------------------------------------*/


