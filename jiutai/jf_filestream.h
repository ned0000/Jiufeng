/**
 *  @file jf_filestream.h
 *
 *  @brief Provide common routines to manipulate file with stream operation.
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_files library
 */

#ifndef JIUFENG_FILESTREAM_H
#define JIUFENG_FILESTREAM_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)
    #include <dirent.h>
    #include <sys/file.h>
#elif defined(WINDOWS)
    #include <io.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_datavec.h"
#include "jf_file.h"

/* --- constant definitions --------------------------------------------------------------------- */

typedef FILE     jf_filestream_t;

/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

FILESAPI u32 FILESCALL jf_filestream_open(
    const olchar_t * pstrFilename, const olchar_t * mode, jf_filestream_t ** ppjf);

FILESAPI u32 FILESCALL jf_filestream_close(jf_filestream_t ** pjf);

FILESAPI u32 FILESCALL jf_filestream_seek(
    jf_filestream_t * pjf, long offset, olint_t whence);

FILESAPI u32 FILESCALL jf_filestream_readn(
    jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead);

FILESAPI u32 FILESCALL jf_filestream_writen(
    jf_filestream_t * pjf, const void * pBuffer, olsize_t sWrite);

FILESAPI u32 FILESCALL jf_filestream_readVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psRead);

FILESAPI u32 FILESCALL jf_filestream_writeVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sWrite);

FILESAPI u32 FILESCALL jf_filestream_readVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset,
    olsize_t * psRead);

FILESAPI u32 FILESCALL jf_filestream_writeVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset,
    olsize_t sWrite);

FILESAPI u32 FILESCALL jf_filestream_readLine(
    jf_filestream_t * pjf, void * ptr, olsize_t * psRead);

/*copy file content from pstrSourceFile to fpDest, pu8Buffer is a buffer
 provided by caller for the function*/
FILESAPI u32 FILESCALL jf_filestream_copyFile(
    jf_filestream_t * fpDest, const olchar_t * pstrSourceFile,
    u8 * u8Buffer, olsize_t sBuf);

FILESAPI u32 FILESCALL jf_filestream_flush(jf_filestream_t * pjf);

FILESAPI boolean_t FILESCALL jf_filestream_isEndOfFile(jf_filestream_t * pjf);

FILESAPI olint_t FILESCALL jf_filestream_getChar(jf_filestream_t * pjf);

#endif /*JIUFENG_FILESTREAM_H*/

/*------------------------------------------------------------------------------------------------*/

