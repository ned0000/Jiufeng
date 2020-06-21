/**
 *  @file jf_file.h
 *
 *  @brief Header file declares routines to manipulate file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_files library.
 */

#ifndef JIUFENG_FILE_H
#define JIUFENG_FILE_H

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

#undef FILESAPI
#undef FILESCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_FILES_DLL)
        #define FILESAPI  __declspec(dllexport)
        #define FILESCALL
    #else
        #define FILESAPI
        #define FILESCALL __cdecl
    #endif
#else
    #define FILESAPI
    #define FILESCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the file handle data type.
 */
#if defined(LINUX)
    typedef olint_t  jf_file_t;
    #define JF_FILE_INVALID_FILE_VALUE  (-1)
#elif defined(WINDOWS)
    typedef HANDLE   jf_file_t;
    #define JF_FILE_INVALID_FILE_VALUE  (INVALID_HANDLE_VALUE)

    #ifndef O_RDONLY
        #define O_RDONLY         (0x0000)
    #endif

    #ifndef O_WRONLY
        #define O_WRONLY         (0x0001)
    #endif

    #ifndef O_RDWR
        #define O_RDWR           (0x0002)
    #endif

    #ifndef O_APPEND
        #define O_APPEND         (0x0008)
    #endif

    #ifndef O_CREAT
        #define O_CREAT          (0x0100)
    #endif

    #ifndef O_TRUNC
        #define O_TRUNC          (0x0200)
    #endif

    #ifndef O_EXCL
        #define O_EXCL           (0x0400)
    #endif

#endif

/** Possible value for file mode.
 */
#define JF_FILE_MODE_TYPE_MASK   (0x001F0000)

#define JF_FILE_MODE_TSOCK       (0x00070000)   /* socket */
#define JF_FILE_MODE_TLNK        (0x00060000)   /* symbolic link */
#define JF_FILE_MODE_TREG        (0x00050000)   /* regular file */
#define JF_FILE_MODE_TBLK        (0x00040000)   /* block device */
#define JF_FILE_MODE_TDIR        (0x00030000)   /* directory */
#define JF_FILE_MODE_TCHR        (0x00020000)   /* character device */
#define JF_FILE_MODE_TFIFO       (0x00010000)   /* fifo */

#define JF_FILE_MODE_PERM_MASK   (0x00000FFF)

#define JF_FILE_MODE_SUID        (04000)
#define JF_FILE_MODE_SGID        (02000)
#define JF_FILE_MODE_SVTX        (01000)

#define JF_FILE_MODE_RWXU        (00700)
#define JF_FILE_MODE_RUSR        (00400)
#define JF_FILE_MODE_WUSR        (00200)
#define JF_FILE_MODE_XUSR        (00100)

#define JF_FILE_MODE_RWXG        (00070)
#define JF_FILE_MODE_RGRP        (00040)
#define JF_FILE_MODE_WGRP        (00020)
#define JF_FILE_MODE_XGRP        (00010)

#define JF_FILE_MODE_RWXO        (00007)
#define JF_FILE_MODE_ROTH        (00004)
#define JF_FILE_MODE_WOTH        (00002)
#define JF_FILE_MODE_XOTH        (00001)

/** Define the file status data type.
 */
typedef struct
{
    /**File type and mode.*/
    u32 jfs_u32Mode;
    /**ID of device containing file.*/
    u32 jfs_u32Dev;
    /**Device ID (if special file)*/
    u32 jfs_u32RDev;
    /**Inode number.*/
    u32 jfs_u32INode;
    /**Total size, in bytes.*/
    u64 jfs_u64Size;
    /**Number of hard links.*/
    u16 jfs_u16Link;
    /**User ID of owner.*/
    u16 jfs_u16UserId;
    /**Group ID of owner.*/
    u16 jfs_u16GroupId;
    u16 jfs_u16Reserved;
    /**Last access time in second from 00:00:00 UTC, January 1, 1970.*/
    u64 jfs_u64AccessTime;
    /**Last modify time in second from 00:00:00 UTC, January 1, 1970.*/
    u64 jfs_u64ModifyTime;
    /**Create time in second from 00:00:00 UTC, January 1, 1970.*/
    u64 jfs_u64Createtime;
    u32 jfs_u32Reserved3[8];
} jf_file_stat_t;

/** Define the file mode data type.
 */
#if defined(LINUX)
    typedef mode_t    jf_file_mode_t;
#elif defined(WINDOWS)
    typedef u16       jf_file_mode_t;
#endif

/** Define the default create mode for creating file.
 */
#define JF_FILE_DEFAULT_CREATE_MODE    (0644)

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Check if it's directory.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The directory type.
 *  @retval TRUE The file is directory.
 *  @retval FALSE The file is not directory.
 */
FILESAPI boolean_t FILESCALL jf_file_isDirFile(u32 u32Mode);

/** Check if it's regular file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The regular file type.
 *  @retval TRUE The file is regular file.
 *  @retval FALSE The file is not regular file.
 */
FILESAPI boolean_t FILESCALL jf_file_isRegFile(u32 u32Mode);

/** Check if it's char file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The char file type.
 *  @retval TRUE The file is char file.
 *  @retval FALSE The file is not char file.
 */
FILESAPI boolean_t FILESCALL jf_file_isCharDevice(u32 u32Mode);

/** Check if it's block file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The block file type.
 *  @retval TRUE The file is block file.
 *  @retval FALSE The file is not block file.
 */
FILESAPI boolean_t FILESCALL jf_file_isBlockDevice(u32 u32Mode);

/** Check if it's fifo file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The fifo file type.
 *  @retval TRUE The file is fifo file.
 *  @retval FALSE The file is not fifo file.
 */
FILESAPI boolean_t FILESCALL jf_file_isFifoFile(u32 u32Mode);

/** Check if it's socket file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The socket file type.
 *  @retval TRUE The file is socket file.
 *  @retval FALSE The file is not socket file.
 */
FILESAPI boolean_t FILESCALL jf_file_isSockFile(u32 u32Mode);

/** Check if it's link file.
 *
 *  @param u32Mode [in] The file mode.
 *
 *  @return The link file type.
 *  @retval TRUE The file is link file.
 *  @retval FALSE The file is not link file.
 */
FILESAPI boolean_t FILESCALL jf_file_isLinkFile(u32 u32Mode);

/** Get status of the file.
 *
 *  @param pstrFilename [in] The file name.
 *  @param pStat [out] The file status.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_STAT_FILE Failed to get status of file.
 */
FILESAPI u32 FILESCALL jf_file_getStat(const olchar_t * pstrFilename, jf_file_stat_t * pStat);

/** Get directory name of the file path.
 *
 *  @param pstrDirName [out] The buffer for the directory name.
 *  @param sDir [in] The size of the directory name buffer.
 *  @param pstrFullpath [in] The full file path.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI void FILESCALL jf_file_getDirectoryName(
    olchar_t * pstrDirName, olsize_t sDir, const olchar_t * pstrFullpath);

/** Get file name of the file path.
 *
 *  @param pstrFileName [out] The buffer for the file name.
 *  @param sFileName [in] The size of the file name buffer.
 *  @param pstrFullpath [in] The full file path.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI void FILESCALL jf_file_getFileName(
    olchar_t * pstrFileName, olsize_t sFileName, const olchar_t * pstrFullpath);

/** Remove trailing path separator of the file path.
 *
 *  @param pstrFullpath [in] The full file path.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_FILE Failed to create file.
 */
FILESAPI u32 FILESCALL jf_file_removeTrailingPathSeparator(olchar_t * pstrFullpath);

/** Create file.
 *
 *  @param pstrFilename [in] The file name to be created.
 *  @param mode [in] File mode for creating file.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_FILE Failed to create file.
 */
FILESAPI u32 FILESCALL jf_file_create(const olchar_t * pstrFilename, jf_file_mode_t mode);

/** Open file and return the file handle.
 *
 *  @param pstrFilename [in] The name of the file to be opened.
 *  @param flags [in] The access modes, it's one of O_RDONLY, O_WRONLY, or O_RDWR.
 *  @param pFile [out] The file handle returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_FILE Failed to open file.
 */
FILESAPI u32 FILESCALL jf_file_open(
    const olchar_t * pstrFilename, olint_t flags, jf_file_t * pFile);

/** Open file with mode and return the file handle.
 *
 *  @note
 *  -# For Linux, mode is used when flag O_CREAT is set. For Windows, the routine is the same as
 *     openFile().
 *
 *  @param pstrFilename [in] The name of the file to be opened.
 *  @param flags [in] The access modes, it's one of O_RDONLY, O_WRONLY, or O_RDWR.
 *  @param mode [in] File mode for creating new file.
 *  @param pFile [out] The file handle returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_FILE Failed to open file.
 */
FILESAPI u32 FILESCALL jf_file_openWithMode(
    const olchar_t * pstrFilename, olint_t flags, jf_file_mode_t mode, jf_file_t * pFile);

/** Close file.
 *
 *  @param pFile [in] The pointer to file handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_file_close(jf_file_t * pFile);

/** Ramove file.
 *
 *  @param pstrFilename [in] The name of the file to be removed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_REMOVE_FILE Failed to remove file.
 */
FILESAPI u32 FILESCALL jf_file_remove(const olchar_t * pstrFilename);

/** Raname file.
 *
 *  @param oldpath [in] The old file name.
 *  @param newpath [in] The new file name.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RENAME_FILE Failed to rename file.
 */
FILESAPI u32 FILESCALL jf_file_rename(const olchar_t *oldpath, const olchar_t *newpath);

/** Lock file.
 *
 *  @param file [in] The file handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_LOCK_FILE Failed to lock file.
 */
FILESAPI u32 FILESCALL jf_file_lock(jf_file_t file);

/** Unlock file.
 *
 *  @param file [in] The file handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_UNLOCK_FILE Failed to unlock file.
 */
FILESAPI u32 FILESCALL jf_file_unlock(jf_file_t file);

/** Read data with specified size from file.
 *
 *  @note
 *  -# The function read specified size unless end of file.
 *
 *  @param file [in] The file handle.
 *  @param pBuffer [in] The buffer for data.
 *  @param psRead [in/out] The size of buffer before reading. After reading, it's the size actually
 *   read.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_END_OF_FILE End of file.
 *  @retval JF_ERR_FAIL_READ_FILE Failed to read file.
 */
FILESAPI u32 FILESCALL jf_file_readn(jf_file_t file, void * pBuffer, olsize_t * psRead);

/** Read data from file with timeout.
 *
 *  @note
 *  -# The function read data until timeout.
 *
 *  @param file [in] The file handle.
 *  @param pBuffer [in] The buffer for data.
 *  @param psRead [in/out] The size of buffer before reading. After reading, it's the size actually
 *   read.
 *  @param timeout [in] The timeout value.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_END_OF_FILE End of file.
 *  @retval JF_ERR_FAIL_READ_FILE Failed to read file.
 */
FILESAPI u32 FILESCALL jf_file_readWithTimeout(
    jf_file_t file, void * pBuffer, olsize_t * psRead, struct timeval * timeout);

/** Write data to file.
 *
 *  @param file [in] The file handle.
 *  @param pBuffer [in] The buffer for data.
 *  @param sWrite [in] The size of buffer to write.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_WRITE_FILE Failed to write file.
 */
FILESAPI u32 FILESCALL jf_file_writen(jf_file_t file, const void * pBuffer, olsize_t sWrite);

/** Read one line of the file.
 *
 *  @note
 *  -# The line is ended with '\n'.
 *
 *  @param file [in] The file handle.
 *  @param pBuffer [in] The buffer for data.
 *  @param psRead [in/out] The size of buffer before reading. After reading, it's the size actually
 *   read.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_END_OF_FILE End of file.
 *  @retval JF_ERR_FAIL_READ_FILE Failed to read file.
 */
FILESAPI u32 FILESCALL jf_file_readLine(jf_file_t file, void * pBuffer, olsize_t * psRead);

/** Check if the file is the typed file.
 *
 *  @note
 *  -# The prefix string can be NULL. Prefix is not checked in this case.
 *  -# The file extenstion can be NULL. File extension is not checked in this case.
 *
 *  @param pstrName [in] The name of the file, no path in the string.
 *  @param pstrPrefix [in] The prefix string.
 *  @param pstrFileExt [in] The file extension.
 *
 *  @return The status of the file.
 *  @retval TRUE The file is typed file.
 *  @retval FALSE The file is not typed file.
 */
FILESAPI boolean_t FILESCALL jf_file_isTypedFile(
    const olchar_t * pstrName, const olchar_t * pstrPrefix, const olchar_t * pstrFileExt);

#endif /*JIUFENG_FILE_H*/

/*------------------------------------------------------------------------------------------------*/

