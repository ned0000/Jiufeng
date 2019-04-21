/**
 *  @file jf_file.h
 *
 *  @brief Provide common routines to manipulate file.
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_files library
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
#include "jf_datavec.h"

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

#if defined(LINUX)
    typedef olint_t  jf_file_t;
    #define JF_FILE_INVALID_FILE_VALUE  (-1)
#elif defined(WINDOWS)
    typedef HANDLE   jf_file_t;
    #define JF_FILE_INVALID_FILE_VALUE  (INVALID_HANDLE_VALUE)

    #ifndef O_RDONLY
        #define O_RDONLY          0x0000
    #endif

    #ifndef O_WRONLY
        #define O_WRONLY          0x0001
    #endif

    #ifndef O_RDWR
        #define O_RDWR            0x0002
    #endif

    #ifndef O_APPEND
        #define O_APPEND          0x0008
    #endif

    #ifndef O_CREAT
        #define O_CREAT           0x0100
    #endif

    #ifndef O_TRUNC
        #define O_TRUNC           0x0200
    #endif

    #ifndef O_EXCL
        #define O_EXCL            0x0400
    #endif

#endif

/* Possible value for fs_u32Mode */
#define JF_FILE_MODE_TYPE_MASK   0x001F0000

#define JF_FILE_MODE_TSOCK  0x00070000   /* socket */
#define JF_FILE_MODE_TLNK   0x00060000   /* symbolic link */
#define JF_FILE_MODE_TREG   0x00050000   /* regular file */
#define JF_FILE_MODE_TBLK   0x00040000   /* block device */
#define JF_FILE_MODE_TDIR   0x00030000   /* directory */
#define JF_FILE_MODE_TCHR   0x00020000   /* character device */
#define JF_FILE_MODE_TFIFO  0x00010000   /* fifo */

#define JF_FILE_MODE_SUID   0x00004000
#define JF_FILE_MODE_SGID   0x00002000
#define JF_FILE_MODE_SVTX   0x00001000

#define JF_FILE_MODE_PERM_MASK   0x00000FFF

#define JF_FILE_MODE_RUSR   0x00000400
#define JF_FILE_MODE_WUSR   0x00000200
#define JF_FILE_MODE_XUSR   0x00000100

#define JF_FILE_MODE_RGRP   0x00000040
#define JF_FILE_MODE_WGRP   0x00000020
#define JF_FILE_MODE_XGRP   0x00000010

#define JF_FILE_MODE_ROTH   0x00000004
#define JF_FILE_MODE_WOTH   0x00000002
#define JF_FILE_MODE_XOTH   0x00000001

typedef struct
{
    u32 jfs_u32Mode;
    u32 jfs_u32Dev;
    u32 jfs_u32RDev;
    u32 jfs_u32INode;
    u64 jfs_u64Size;
    u16 jfs_u16Link;
    u16 jfs_u16UserId;
    u16 jfs_u16GroupId;
    u16 jfs_u16Reserved;
    u32 jfs_u32AccessTime; /* 00:00:00 UTC, January 1, 1970, in seconds*/
    u32 jfs_u32ModifyTime;
    u32 jfs_u32Createtime;
    u32 jfs_u32Reserved2;
    u32 jfs_u32Reserved3[8];
} jf_file_stat_t;

FILESAPI boolean_t FILESCALL jf_file_isDirFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isRegFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isCharDevice(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isBlockDevice(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isFifoFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isSockFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL jf_file_isLinkFile(u32 u32Mode);

#if defined(LINUX)
    typedef mode_t    jf_file_mode_t;
#elif defined(WINDOWS)
    typedef u16       jf_file_mode_t;
#endif

#define JF_FILE_DEFAULT_CREATE_MODE    (0644)

/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

FILESAPI u32 FILESCALL jf_file_getStat(
    const olchar_t * pstrFilename, jf_file_stat_t * pStat);

FILESAPI void FILESCALL jf_file_getDirectoryName(
    olchar_t * pstrDirName, olsize_t sDir, const olchar_t * pstrFullpath);

FILESAPI void FILESCALL jf_file_getFileName(
    olchar_t * pstrFileName, olsize_t sFileName, const olchar_t * pstrFullpath);

FILESAPI u32 FILESCALL jf_file_removeTrailingPathSeparator(olchar_t * pstrFullpath);

FILESAPI u32 FILESCALL jf_file_create(
    const olchar_t * pstrFilename, jf_file_mode_t mode);

FILESAPI u32 FILESCALL jf_file_open(
    const olchar_t * pstrFilename, olint_t flags, jf_file_t * pFile);

/*for Linux, mode is used when flag O_CREAT is set
 for Windows, the routine is the same as openFile */
FILESAPI u32 FILESCALL jf_file_openWithMode(
    const olchar_t * pstrFilename, olint_t flags, jf_file_mode_t mode,
    jf_file_t * pFile);

FILESAPI u32 FILESCALL jf_file_close(jf_file_t * pFile);

FILESAPI u32 FILESCALL jf_file_remove(const olchar_t * pstrFilename);

FILESAPI u32 FILESCALL jf_file_rename(
    const olchar_t *oldpath, const olchar_t *newpath);

FILESAPI u32 FILESCALL jf_file_lock(jf_file_t fd);

FILESAPI u32 FILESCALL jf_file_unlock(jf_file_t fd);

FILESAPI u32 FILESCALL jf_file_readn(
    jf_file_t fd, void * pBuffer, olsize_t * psRead);

FILESAPI u32 FILESCALL jf_file_readWithTimeout(
    jf_file_t fd, void * pBuffer, olsize_t * psRead, struct timeval * timeout);

FILESAPI u32 FILESCALL jf_file_writen(
    jf_file_t fd, const void * pBuffer, olsize_t sWrite);

FILESAPI u32 FILESCALL jf_file_readLine(
    jf_file_t fd, void * pBuffer, olsize_t * psRead);

/** Check if the file is the typed file
 *  
 *  @param pstrName [in] the name of the file, no path in the string
 *  @param pstrPrefex [in] the prefix string
 *  @param pstrFileExt [in] the file extension
 *
 *  @return the status of the file
 *  @retval TRUE the file is typed file
 *  @retval FALSE the file is not typed file
 *
 *  @note fnCreateResource_t must be called successfully before this func is
 *   called.
 */
FILESAPI boolean_t FILESCALL jf_file_isTypedFile(
    const olchar_t * pstrName, const olchar_t * pstrPrefex, const olchar_t * pstrFileExt);

#endif /*JIUFENG_FILE_H*/

/*------------------------------------------------------------------------------------------------*/

