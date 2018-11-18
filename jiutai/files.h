/**
 *  @file files.h
 *
 *  @brief Files header file. Provide common routines to manipulate files
 *
 *  @author Min Zhang
 *
 *  @note For windows, when using function fopen() with "r+" and "w+", fseek()
 *   is required between fread() and fwrite(). Eg.
 *  @note fread(fp, buf, sbuf);
 *  @note fseek(fp, 0, SEEK_END);
 *  @note fwrite(buf, 1, sbuf, fp)
 *
 */

#ifndef JIUFENG_FILES_H
#define JIUFENG_FILES_H

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)
    #include <dirent.h>
    #include <sys/file.h>
#elif defined(WINDOWS)
    #include <io.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "datavec.h"

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

/* --- constant definitions ------------------------------------------------ */
typedef FILE     olfile_t;

#if defined(LINUX)
    typedef olint_t  file_t;
    typedef DIR      dir_t;
    #define INVALID_FILE_VALUE  -1
#elif defined(WINDOWS)
    typedef HANDLE   file_t;
    typedef void     dir_t;
    #define INVALID_FILE_VALUE  INVALID_HANDLE_VALUE

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
#define FS_MODE_TYPE_MASK   0x001F0000

#define FS_MODE_TSOCK  0x00070000   /* socket */
#define FS_MODE_TLNK   0x00060000   /* symbolic link */
#define FS_MODE_TREG   0x00050000   /* regular file */
#define FS_MODE_TBLK   0x00040000   /* block device */
#define FS_MODE_TDIR   0x00030000   /* directory */
#define FS_MODE_TCHR   0x00020000   /* character device */
#define FS_MODE_TFIFO  0x00010000   /* fifo */

#define FS_MODE_SUID   0x00004000
#define FS_MODE_SGID   0x00002000
#define FS_MODE_SVTX   0x00001000

#define FS_MODE_PERM_MASK   0x00000FFF

#define FS_MODE_RUSR   0x00000400
#define FS_MODE_WUSR   0x00000200
#define FS_MODE_XUSR   0x00000100

#define FS_MODE_RGRP   0x00000040
#define FS_MODE_WGRP   0x00000020
#define FS_MODE_XGRP   0x00000010

#define FS_MODE_ROTH   0x00000004
#define FS_MODE_WOTH   0x00000002
#define FS_MODE_XOTH   0x00000001

typedef struct
{
    u32 fs_u32Mode;
    u32 fs_u32Dev;
    u32 fs_u32RDev;
    u32 fs_u32INode;
    u64 fs_u64Size;
    u16 fs_u16Link;
    u16 fs_u16UserId;
    u16 fs_u16GroupId;
    u16 fs_u16Reserved;
    u32 fs_u32AccessTime; /* 00:00:00 UTC, January 1, 1970, in seconds*/
    u32 fs_u32ModifyTime;
    u32 fs_u32Createtime;
    u32 fs_u32Reserved2;
    u32 fs_u32Reserved3[8];
} file_stat_t;

FILESAPI boolean_t FILESCALL isDirFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL isRegFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL isCharDevice(u32 u32Mode);
FILESAPI boolean_t FILESCALL isBlockDevice(u32 u32Mode);
FILESAPI boolean_t FILESCALL isFifoFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL isSockFile(u32 u32Mode);
FILESAPI boolean_t FILESCALL isLinkFile(u32 u32Mode);

#if defined(WINDOWS)
    typedef u16 mode_t;
#endif


/* --- data structures ----------------------------------------------------- */

typedef struct
{
    olchar_t de_strName[MAX_PATH_LEN];
    olsize_t de_sName;
    u8 de_u8Reserved[60];
} dir_entry_t;

/* --- functional routines ------------------------------------------------- */

FILESAPI u32 FILESCALL getFileStat(
    const olchar_t * pstrFilename, file_stat_t * pStat);

FILESAPI void FILESCALL getDirectoryName(
    olchar_t * pstrDirName, olsize_t sDir, const olchar_t * pstrFullpath);

FILESAPI void FILESCALL getFileName(
    olchar_t * pstrFileName, olsize_t sFileName, const olchar_t * pstrFullpath);

FILESAPI u32 FILESCALL removeTrailingPathSeparator(olchar_t * pstrFullpath);



#define DEFAULT_CREATE_FILE_MODE  0644

FILESAPI u32 FILESCALL createFile(const olchar_t * pstrFilename, mode_t mode);

FILESAPI u32 FILESCALL openFile(
    const olchar_t * pstrFilename, olint_t flags, file_t * pFile);

/*for Linux, mode is used when flag O_CREAT is set
 for Windows, the routine is the same as openFile */
FILESAPI u32 FILESCALL openFile2(
    const olchar_t * pstrFilename, olint_t flags, mode_t mode, file_t * pFile);

FILESAPI u32 FILESCALL closeFile(file_t * pFile);

FILESAPI u32 FILESCALL removeFile(const olchar_t * pstrFilename);

FILESAPI u32 FILESCALL renameFile(
    const olchar_t *oldpath, const olchar_t *newpath);

FILESAPI u32 FILESCALL lockFile(file_t fd);

FILESAPI u32 FILESCALL unlockFile(file_t fd);

FILESAPI u32 FILESCALL readn(file_t fd, void * pBuffer, olsize_t * psRead);

FILESAPI u32 FILESCALL writen(file_t fd, const void * pBuffer, olsize_t sWrite);

FILESAPI u32 FILESCALL readLine(file_t fd, void * pBuffer, olsize_t * psRead);

/*stream operation*/
FILESAPI u32 FILESCALL fpOpenFile(
    const olchar_t * pstrFilename, const olchar_t * mode, olfile_t ** pfp);

FILESAPI u32 FILESCALL fpCloseFile(olfile_t ** pfp);

FILESAPI u32 FILESCALL fpSeekFile(olfile_t * fp, long offset, olint_t whence);

FILESAPI u32 FILESCALL fpReadn(olfile_t * fp, void * pBuffer, olsize_t * psRead);

FILESAPI u32 FILESCALL fpWriten(
    olfile_t * fp, const void * pBuffer, olsize_t sWrite);

FILESAPI u32 FILESCALL fpReadVec(olfile_t * fp, data_vec_t * pdvData,
           olsize_t * psRead);

FILESAPI u32 FILESCALL fpWriteVec(olfile_t * fp, data_vec_t * pdvData,
            olsize_t sWrite);

FILESAPI u32 FILESCALL fpReadVecOffset(olfile_t * fp, data_vec_t * pdvData,
           olsize_t sVecOffset, olsize_t * psRead);

FILESAPI u32 FILESCALL fpWriteVecOffset(olfile_t * fp, data_vec_t * pdvData,
           olsize_t sVecOffset, olsize_t sWrite);

FILESAPI u32 FILESCALL fpReadLine(olfile_t * fp, void * ptr, olsize_t * psRead);

/*copy file content from pstrSourceFile to fpDest, pu8Buffer is a buffer
 provided by caller for the function*/
FILESAPI u32 FILESCALL fpCopyFile(olfile_t * fpDest, const olchar_t * pstrSourceFile,
    u8 * u8Buffer, olsize_t sBuf);

FILESAPI u32 FILESCALL fpFlushFile(olfile_t * fp);

FILESAPI boolean_t FILESCALL fpEndOfFile(olfile_t * fp);

#define DEFAULT_CREATE_DIR_MODE  0755
/*'mode' is for Linux only, it specifies the permission to use*/
FILESAPI u32 FILESCALL createDir(const olchar_t * pstrDirName, mode_t mode);

FILESAPI u32 FILESCALL removeDir(const olchar_t * pstrDirName);

FILESAPI u32 FILESCALL openDir(const olchar_t * pstrDirName, dir_t ** ppDir);

FILESAPI u32 FILESCALL closeDir(dir_t ** ppDir);

FILESAPI u32 FILESCALL getFirstDirEntry(dir_t * pDir, dir_entry_t * pEntry);

FILESAPI u32 FILESCALL getNextDirEntry(dir_t * pDir, dir_entry_t * pEntry);

typedef u32 (* fnHandleFile_t)(const olchar_t * pstrFullpath,
    file_stat_t * pStat, void * pArg);

FILESAPI u32 FILESCALL traversalDirectory(const olchar_t * pstrDirName,
    fnHandleFile_t fnHandleFile, void * pArg);

FILESAPI u32 FILESCALL parseDirectory(const olchar_t * pstrDirName,
    fnHandleFile_t fnHandleFile, void * pArg);

/*true, the entry is ignored*/
typedef boolean_t (* fnFilterDirEntry_t)(dir_entry_t * entry);
typedef olint_t (* fnCompareDirEntry_t)(const void * a, const void * b);

FILESAPI u32 FILESCALL scanDirectory(
    const olchar_t * pstrDirName, dir_entry_t * entry, olint_t * numofentry,
    fnFilterDirEntry_t fnFilter, fnCompareDirEntry_t fnCompare);

FILESAPI olint_t FILESCALL compareDirEntry(const void * a, const void * b);


#endif /*JIUFENG_FILES_H*/

/*---------------------------------------------------------------------------*/

