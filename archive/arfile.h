/**
 *  @file arfile.h
 *
 *  @brief archive file header file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef ARCHIVE_ARFILE_H
#define ARCHIVE_ARFILE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */
typedef void ar_file_t;

typedef struct
{
    boolean_t afp_bExtract;
    u8 afp_u8Reserved[7];
    u32 afp_u32Reserved1[4];
} ar_file_param_t;

/* --- functional routines ---------------------------------------------------------------------- */
u32 createArFile(olchar_t * pstrArchiveName, ar_file_param_t * pafp,
    ar_file_t ** ppaf);

u32 destroyArFile(ar_file_t ** ppaf);

u32 readArFile(ar_file_t * paf, u8 * pu8Buffer, olsize_t * psBuf);

u32 writeArFile(ar_file_t * paf, u8 * pu8Buffer, olsize_t sBuf);

boolean_t isEndOfArFile(ar_file_t * paf);

u32 seekArFile(ar_file_t * paf, long offset, olint_t whence);

#endif /*ARCHIVE_ARFILE_H*/

/*------------------------------------------------------------------------------------------------*/


