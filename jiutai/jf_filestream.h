/**
 *  @file jf_filestream.h
 *
 *  @brief Provide common routines to manipulate file with stream operation.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_files library.
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

/** Define the file stream data type.
 */
typedef FILE  jf_filestream_t;

/* --- data structures -------------------------------------------------------------------------- */



/* --- functional routines ---------------------------------------------------------------------- */

/** Open a file and associates a stream with it.
 *
 *  @par Mode
 *  <table>
 *  <tr><th>Mode  <th>Description                                                              </tr>
 *  <tr><td>r     <td>Open for reading. The stream is positioned at the beginning of the file. </tr>
 *  <tr><td>r+    <td>Open for reading and writing. The stream is positioned at the beginning 
 *    of the file. </tr>
 *  <tr><td>w     <td>Truncate file to zero length or create file for writing. The stream is 
 *    positioned at the beginning of the file. </tr>
 *  <tr><td>w+    <td>Open for reading and writing. The file is created if it does not exist, 
 *    otherwise it is truncated.  The stream is positioned at the beginning of the file. </tr>
 *  <tr><td>a     <td>Open for appending (writing at end of file). The file is created if it does 
 *    not exist.  The stream is positioned at the end of the file. </tr>
 *  <tr><td>a+    <td>Open for reading and appending (writing at end of file). The file is created 
 *    if it does not exist. The initial file position for reading is at the beginning of the file, 
 *    but output is always appended to the end of the file. </tr>
 *  </table>
 *
 *  @param pstrFilename [in] The file to be opened.
 *  @param mode [in] String specify how to open the file.
 *  @param ppjf [out] The file stream created. 
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_open(
    const olchar_t * pstrFilename, const olchar_t * mode, jf_filestream_t ** ppjf);

/** Close a file stream.
 *
 *  @param pjf [in] The file stream to be closed.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_close(jf_filestream_t ** pjf);

/** Reposition a file stream.
 *
 *  @par Whence
 *  <table>
 *  <tr><th>whence    <th>Description                                              </tr>
 *  <tr><td>SEEK_SET  <td>The offset is relative to the start of the file.         </tr>
 *  <tr><td>SEEK_CUR  <td>The offset is the current position indicator.            </tr>
 *  <tr><td>SEEK_END  <td>The offset is relative to the end of the file.           </tr>
 *  </table>
 *
 *  @param pjf [in] The file stream to be closed.
 *  @param offset [in] The new position measured in byte.
 *  @param whence [in] Specify where to reposition.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_seek(jf_filestream_t * pjf, long offset, olint_t whence);

/** Read a file stream.
 *
 *  @param pjf [in] The file stream.
 *  @param pBuffer [out] The data buffer.
 *  @param psRead [in/out] The data buffer size as in parameter and size actually read as out
 *   parameter.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_readn(
    jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead);

/** Write a file stream.
 *
 *  @param pjf [in] The file stream.
 *  @param pBuffer [in] The data buffer.
 *  @param sWrite [in] The data buffer size.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_writen(
    jf_filestream_t * pjf, const void * pBuffer, olsize_t sWrite);

/** Read a file stream, the data is saved to data vector.
 *
 *  @param pjf [in] The file stream.
 *  @param pjdData [out] The data vector.
 *  @param psRead [in/out] The data buffer size as in parameter and size actually read as out
 *   parameter.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_readVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psRead);

/** Write a file stream, the data is in a data vector.
 *
 *  @param pjf [in] The file stream.
 *  @param pjdData [in] The data vector.
 *  @param sWrite [in] The data vector size.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_writeVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sWrite);

/** Read a file stream, the data is saved to data vector with offset.
 *
 *  @note
 *  -# Data is read from stream and saved to the vector from the offsest.
 *
 *  @param pjf [in] The file stream.
 *  @param pjdData [out] The data vector.
 *  @param sVecOffset [in] The offset of the data vector.
 *  @param psRead [in/out] The data buffer size as in parameter and size actually read as out
 *   parameter.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_readVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset, olsize_t * psRead);

/** Write a file stream, the data is in a data vector with offset.
 *
 *  @note
 *  -# Data is read from data vector from the offsest and write to stream.
 *
 *  @param pjf [in] The file stream.
 *  @param pjdData [in] The data vector.
 *  @param sVecOffset [in] The offset of the data vector.
 *  @param sWrite [in] The data vector size.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_writeVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset, olsize_t sWrite);

/** Read line from a file stream.
 *
 *  @param pjf [in] The file stream.
 *  @param ptr [out] The data buffer.
 *  @param psRead [in/out] The data buffer size as in parameter and size actually read as out
 *   parameter.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_readLine(
    jf_filestream_t * pjf, void * ptr, olsize_t * psRead);

/** Copy file content from source file to destination file, the buffer is provided by caller.
 *
 *  @param fpDest [in] The destination file stream.
 *  @param pstrSourceFile [in] The source file.
 *  @param u8Buffer [in] The data buffer.
 *  @param sBuf [in/out] The data buffer size.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_copyFile(
    jf_filestream_t * fpDest, const olchar_t * pstrSourceFile, u8 * u8Buffer, olsize_t sBuf);

/** Flush a file stream.
 *
 *  @param pjf [in] The file stream.
 *
 *  @return The error code.
 */
FILESAPI u32 FILESCALL jf_filestream_flush(jf_filestream_t * pjf);

/** Formatted output conversion and write the output to a file stream.
 *
 *  @param pjf [in] The file stream.
 *  @param format [in] The format string.
 *  @param ... [in] The parameters.
 *
 *  @return Number of characters printed.
 */
FILESAPI olsize_t FILESCALL jf_filestream_printf(
    jf_filestream_t * pjf, const olchar_t * format, ...);

/** Test the end-of-file indicator for the stream.
 *
 *  @param pjf [in] The file stream.
 *
 *  @return The end-of-file indicator status.
 *  @retval TRUE The end-of-file indicator is set.
 *  @retval FALSE The end-of-file indicator is not set.
 */
FILESAPI boolean_t FILESCALL jf_filestream_isEndOfFile(jf_filestream_t * pjf);

/** Get a character from file stream.
 *
 *  @param pjf [in] The file stream.
 *
 *  @return The character.
 */
FILESAPI olint_t FILESCALL jf_filestream_getChar(jf_filestream_t * pjf);

#endif /*JIUFENG_FILESTREAM_H*/

/*------------------------------------------------------------------------------------------------*/

