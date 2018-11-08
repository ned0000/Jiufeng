/**
 *  @file files.c
 *
 *  @brief files implementation files
 *  provide common routines to manipulate files
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <sys/types.h>
    #include <dirent.h>
    #include <sys/file.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "files.h"
#include "errcode.h"
#include "common.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
static u32 _writeVec(FILE * fp, olsize_t vecoffset, data_vec_t * pdvData,
    olsize_t * psData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t total = *psData, ndata = 0, size;
    data_vec_entry_t * entry;
    olint_t index = 0;

    while (total > 0 && u32Ret == OLERR_NO_ERROR &&
           index <= pdvData->dv_u16CurEntry)
    {
        entry = &pdvData->dv_pdveEntry[index ++];

        if (entry->dve_sOffset == 0)
            break;

        if (vecoffset >= entry->dve_sOffset)
        {
            vecoffset -= entry->dve_sOffset;
            continue;
        }

        size = entry->dve_sOffset - vecoffset;
        if (size > total)
            size = total;

        u32Ret = fpWriten(fp, entry->dve_pu8Buf + vecoffset, size);

        ndata += size;
        total -= size;
        vecoffset = 0;
    }

    *psData = ndata;

    return u32Ret;
}

static u32 _readVec(FILE * fp, data_vec_t * pdvData,
       olsize_t * psData)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t total = *psData, ndata = 0, size;
    data_vec_entry_t * entry;

    while (total > 0 && ! fpEndOfFile(fp) && u32Ret == OLERR_NO_ERROR)
    {
        entry = &pdvData->dv_pdveEntry[pdvData->dv_u16CurEntry];

        size = entry->dve_sBuf - entry->dve_sOffset;
        if (size > total)
            size = total;

        u32Ret = fpReadn(fp, entry->dve_pu8Buf + entry->dve_sOffset, &size);

        ndata += size;
        total -= size;

        entry->dve_sOffset += size;
        if (entry->dve_sOffset == entry->dve_sBuf)
        {
            pdvData->dv_u16CurEntry ++;
            if (pdvData->dv_u16CurEntry == pdvData->dv_u16MaxEntry)
                break;
        }
    }

    *psData = ndata;

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
#if defined(LINUX)
u32 _setFileStat(file_stat_t * pStat, struct stat * pFileInfo)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pStat, 0, sizeof(file_stat_t));

    if (S_ISREG(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TREG;
    else if (S_ISDIR(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TDIR;
    else if (S_ISCHR(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TCHR;
    else if (S_ISBLK(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TBLK;
    else if (S_ISFIFO(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TFIFO;
    else if (S_ISLNK(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TLNK;
    else if (S_ISSOCK(pFileInfo->st_mode))
        pStat->fs_u32Mode |= FS_MODE_TSOCK;

    pStat->fs_u32Mode |= (pFileInfo->st_mode | 0x0FFF);

    pStat->fs_u64Size = (u64)pFileInfo->st_size;
    pStat->fs_u32Dev = (u32)pFileInfo->st_dev;
    pStat->fs_u32RDev = (u32)pFileInfo->st_rdev;
    pStat->fs_u32INode = (u32)pFileInfo->st_ino;
    pStat->fs_u16Link = (u16)pFileInfo->st_nlink;
    pStat->fs_u16UserId = (u16)pFileInfo->st_uid;
    pStat->fs_u16GroupId = (u16)pFileInfo->st_gid;
    pStat->fs_u32AccessTime = (u32)pFileInfo->st_atime;
    pStat->fs_u32ModifyTime = (u32)pFileInfo->st_mtime;
    pStat->fs_u32Createtime  = (u32)pFileInfo->st_ctime;

    return u32Ret;
}
#elif defined(WINDOWS)
u32 _setFileStat(file_stat_t * pStat, WIN32_FILE_ATTRIBUTE_DATA * pFileInfo)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pStat, 0, sizeof(file_stat_t));

    if (pFileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pStat->fs_u32Mode |= FS_MODE_TDIR;
    else
        pStat->fs_u32Mode |= FS_MODE_TREG;

    pStat->fs_u64Size = (((u64)pFileInfo->nFileSizeHigh) << 32) + pFileInfo->nFileSizeLow;

    pStat->fs_u32Createtime = fileTimeToSecondsSince1970(&(pFileInfo->ftCreationTime));
    pStat->fs_u32AccessTime = fileTimeToSecondsSince1970(&(pFileInfo->ftLastAccessTime));
    pStat->fs_u32ModifyTime = fileTimeToSecondsSince1970(&(pFileInfo->ftLastWriteTime));

    return u32Ret;
}

#endif

u32 getFileStat(const olchar_t * pstrFilename, file_stat_t * pStat)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;
    struct stat filestat;

    nRet = lstat(pstrFilename, &filestat);
    if (nRet != 0)
        u32Ret = OLERR_FAIL_STAT_FILE;

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _setFileStat(pStat, &filestat);

#elif defined(WINDOWS)
    boolean_t bRet;
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;

    bRet = GetFileAttributesEx(pstrFilename, GetFileExInfoStandard,
        (void *)&FileInfo);
    if (! bRet)
        u32Ret = OLERR_FAIL_STAT_FILE;

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _setFileStat(pStat, &FileInfo);
#endif

    return u32Ret;
}

void getDirectoryName(olchar_t * pstrDirName, olsize_t sDir,
    const olchar_t * pstrFullpath)
{
    olchar_t * pu8Name = NULL;

    assert((pstrDirName != NULL) && (pstrFullpath != NULL));

    ol_strncpy(pstrDirName, pstrFullpath, sDir - 1);
    pstrDirName[sDir - 1] = '\0';

    pu8Name = strrchr(pstrDirName, PATH_SEPARATOR);
    if (pu8Name != NULL)
        *pu8Name = '\0';
    else /*path separator is not found, clear the buffer*/
        pstrDirName[0] = '\0';

    removeTrailingPathSeparator(pstrDirName);
}

void getFileName(
    olchar_t * pstrFileName, olsize_t sFileName, const olchar_t * pstrFullpath)
{
    olchar_t * pstrName = NULL;

    assert((pstrFileName != NULL) && (pstrFullpath != NULL));

    pstrName = strrchr(pstrFullpath, PATH_SEPARATOR);
    if (pstrName == NULL)
    {
        ol_strncpy(pstrFileName, pstrFullpath, sFileName - 1);
        pstrFileName[sFileName - 1] = '\0';
    }
    else
    {
        pstrName++;
        if (*pstrName == '\0')
        {
            *pstrFileName = '\0';
        }
        else
        {
            ol_strncpy(pstrFileName, pstrName, sFileName - 1);
            pstrFileName[sFileName - 1] = '\0';
        }
    }
}

u32 removeTrailingPathSeparator(olchar_t * pstrFullpath)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t sPath;

    assert(pstrFullpath != NULL);

    sPath = ol_strlen(pstrFullpath);
    if (sPath > 0)
    {
        sPath --;
        while ((sPath >= 0) && (pstrFullpath[sPath] == PATH_SEPARATOR))
            pstrFullpath[sPath --] = '\0';
    }

    return u32Ret;
}

u32 createFile(const olchar_t * pstrFilename, mode_t mode)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_t fd;
#if defined(LINUX)

    fd = creat(pstrFilename, mode);
    if (fd == -1)
        u32Ret = OLERR_FAIL_CREATE_FILE;
    else
        closeFile(&fd);

#elif defined(WINDOWS)
    fd = CreateFile(pstrFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_HANDLE_VALUE)
        u32Ret = OLERR_FAIL_CREATE_FILE;
    else
        CloseHandle(fd);

#endif

    return u32Ret;
}

u32 openFile(const olchar_t * pstrFilename, olint_t flags, file_t * pFile)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)

    assert((pstrFilename != NULL) && (pFile != NULL));

    *pFile = open(pstrFilename, flags);
    if (*pFile == -1)
        u32Ret = OLERR_FAIL_OPEN_FILE;
#elif defined(WINDOWS)
    u32 u32Flags = 0, u32CreateFlags = 0, u32ShareMode = 0;

    assert((pstrFilename != NULL) && (pFile != NULL));

    if (flags & O_WRONLY)
        u32Flags |= GENERIC_WRITE;
    else if (flags & O_RDONLY)
        u32Flags |= GENERIC_READ;
    else if (flags & O_RDWR)
        u32Flags |= GENERIC_WRITE | GENERIC_READ;
    else
    {
        u32Flags |= GENERIC_READ;
        u32ShareMode |= FILE_SHARE_READ;
    }

    if ((flags & O_CREAT) && (flags & O_TRUNC))
        u32CreateFlags = CREATE_ALWAYS;
    else if (flags & O_CREAT)
        u32CreateFlags = CREATE_NEW;
    else if (flags & O_TRUNC)
        u32CreateFlags = TRUNCATE_EXISTING;
    else
        u32CreateFlags = OPEN_EXISTING;

    *pFile = CreateFile(pstrFilename, u32Flags, u32ShareMode, NULL,
        u32CreateFlags, FILE_ATTRIBUTE_NORMAL, NULL);
    if (*pFile == INVALID_HANDLE_VALUE)
        u32Ret = OLERR_FAIL_OPEN_FILE;
#endif

    return u32Ret;
}

u32 openFile2(const olchar_t * pstrFilename, olint_t flags, mode_t mode, file_t * pFile)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)

    assert((pstrFilename != NULL) && (pFile != NULL));

    *pFile = open(pstrFilename, flags, mode);
    if (*pFile == -1)
        u32Ret = OLERR_FAIL_OPEN_FILE;
#elif defined(WINDOWS)
    u32Ret = openFile(pstrFilename, flags, pFile);
#endif

    return u32Ret;
}

u32 closeFile(file_t * pFile)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert(pFile != NULL);

#if defined(LINUX)
    close(*pFile);
    *pFile = -1;
#elif defined(WINDOWS)
    CloseHandle(*pFile);
    *pFile = INVALID_HANDLE_VALUE;
#endif

    return u32Ret;
}

u32 removeFile(const olchar_t * pstrFilename)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    assert(pstrFilename != NULL);

    nRet = unlink(pstrFilename);
    if (nRet == -1)
        u32Ret = OLERR_FAIL_REMOVE_FILE;

#elif defined(WINDOWS)

    assert(pstrFilename != NULL);
    if (DeleteFile(pstrFilename) == 0)
        u32Ret = OLERR_FAIL_REMOVE_FILE;

#endif
    return u32Ret;
}

u32 renameFile(const olchar_t *oldpath, const olchar_t *newpath)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    assert((oldpath != NULL) && (newpath != NULL));

    nRet = rename(oldpath, newpath);
    if (nRet == -1)
        u32Ret = OLERR_FAIL_RENAME_FILE;

#elif defined(WINDOWS)
    assert((oldpath != NULL) && (newpath != NULL));

    if (rename(oldpath, newpath) != 0)
        u32Ret = OLERR_FAIL_RENAME_FILE;

#endif
    return u32Ret;
}

u32 lockFile(file_t fd)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = flock(fd, LOCK_EX);
    if (nRet == -1)
        u32Ret = OLERR_FAIL_LOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = LockFileEx(fd, LOCKFILE_EXCLUSIVE_LOCK, 0,
       0xFFFFFFFF, 0xFFFFFFFF, NULL);
    if (! bRet)
        u32Ret = OLERR_FAIL_LOCK_FILE;

#endif

    return u32Ret;
}

u32 unlockFile(file_t fd)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = flock(fd, LOCK_UN);
    if (nRet == -1)
        u32Ret = OLERR_FAIL_UNLOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = UnlockFileEx(fd, 0, 0xFFFFFFFF, 0xFFFFFFFF, NULL);
    if (! bRet)
        u32Ret = OLERR_FAIL_UNLOCK_FILE;

#endif

    return u32Ret;
}

/* Read "n" bytes from a descriptor */
u32 readn(file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    ssize_t nleft, nread;
    olchar_t * p;

    assert((pBuffer != NULL) && (*psRead > 0));

    p = pBuffer;
    nleft = *psRead;
    while (nleft > 0) 
    {
        nread = read(fd, p, nleft);
        if (nread < 0) 
        {
            if (errno == EINTR)
            {
                /* and call read() again */
                nread = 0;
            }
            else
            {
                u32Ret = OLERR_FAIL_READ_FILE;
                break;
            }
        } 
        else if (nread == 0)
        {
            /* EOF */
            if (nleft == *psRead)
                u32Ret = OLERR_END_OF_FILE;
            break;
        }

        nleft -= nread;
        p += nread;
    }

    *psRead = (*psRead) - (u32)nleft;
#elif defined(WINDOWS)
    boolean_t bRet;
    olsize_t sread;

    bRet = ReadFile(fd, pBuffer, *psRead, &sread, NULL);
    if (bRet)
    {
        if (sread == 0)
            u32Ret = OLERR_END_OF_FILE;
        else
            *psRead = sread;

    }
    else
        u32Ret = OLERR_FAIL_READ_FILE;
#endif
    return u32Ret;
}

/* Write "n" bytes to a descriptor. */
u32 writen(file_t fd, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olsize_t nleft;
    ssize_t nwritten;
    const olchar_t * p;

    assert((pBuffer != NULL) && (sWrite > 0));

    p = pBuffer;
    nleft = sWrite;
    while (nleft > 0)
    {
        nwritten = write(fd, p, nleft);
        if (nwritten <= 0)
        {
            if (errno == EINTR)
            {
                /* and call write() again */
                nwritten = 0;
            }
            else
            {
                u32Ret = OLERR_FAIL_WRITE_FILE;
                break;
            }
        }

        nleft -= nwritten;
        p += nwritten;
    }
#elif defined(WINDOWS)
    boolean_t bRet;
    olsize_t nwrite;

    bRet = WriteFile(fd, pBuffer, sWrite, &nwrite, NULL);
    if (bRet)
    {
        if (nwrite != sWrite)
            u32Ret = OLERR_FAIL_WRITE_FILE;
    }
    else
        u32Ret = OLERR_FAIL_WRITE_FILE;
#endif
    return u32Ret;
}

u32 readLine(file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 n, maxlen;
    olsize_t nread;
    olchar_t c, * p;

    assert((pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 1; (n < maxlen) && (u32Ret == OLERR_NO_ERROR); n++)
    {
        nread = 1;
        u32Ret = readn(fd, (void *)&c, &nread);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (nread == 1) 
            {
                *p++ = c;
                if (c == '\n')
                    break; /* newline is stored, like fgets() */
            } 
            else if (nread == 0) 
            {
                /* EOF, some data was read */
                break;
            }
        }
    }

    *p = 0;/* null terminate like fgets() */

    *psRead = n;

    return u32Ret;
}

u32 fpOpenFile(const olchar_t * pstrFilename, const olchar_t * mode, FILE ** pfp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert((pstrFilename != NULL) && (mode != NULL) && (pfp != NULL));

    *pfp = fopen(pstrFilename, mode);
    if (*pfp == NULL)
        u32Ret = OLERR_FAIL_OPEN_FILE;

    return u32Ret;
}

u32 fpFlushFile(FILE * fp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nRet;

    nRet = fflush(fp);
    if (nRet != 0)
        u32Ret = OLERR_FAIL_FLUSH_FILE;

    return u32Ret;
}

u32 fpSeekFile(FILE * fp, long offset, olint_t whence)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t ret;

    assert(fp != NULL);

    ret = fseek(fp, offset, whence);
    if (ret != 0)
        u32Ret = OLERR_FAIL_SEEK_FILE;

    return u32Ret;
}

u32 fpCloseFile(FILE ** pfp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert((pfp != NULL) && (*pfp != NULL));

    fclose(*pfp);
    *pfp = NULL;

    return u32Ret;
}

boolean_t fpEndOfFile(FILE * fp)
{
    boolean_t bRet = FALSE;
    olint_t ret;

    ret = feof(fp);
    if (ret != 0)
        bRet = TRUE;

    return bRet;
}

/* Read "n" bytes from a file */
u32 fpReadn(FILE * fp, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t nread;

    assert((fp != NULL) && (pBuffer != NULL) && (*psRead > 0));

    nread = fread(pBuffer, 1, *psRead, fp);
    if ((u32)nread != *psRead)
    {
        if (! feof(fp))
            u32Ret = OLERR_FAIL_READ_FILE;
        else
        {
            if (nread == 0)
                u32Ret = OLERR_END_OF_FILE;
        }
    }

    *psRead = nread;

    return u32Ret;
}

u32 fpReadVec(FILE * fp, data_vec_t * pdvData, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;

    initDataVec(pdvData);

    u32Ret = _readVec(fp, pdvData, psRead);

    return u32Ret;
}

u32 fpWriteVec(FILE * fp, data_vec_t * pdvData, olsize_t sWrite)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t size = sWrite;

    u32Ret = _writeVec(fp, 0, pdvData, &size);

    return u32Ret;
}

u32 fpReadVecOffset(FILE * fp, data_vec_t * pdvData,
    olsize_t sVecOffset, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;

    setDataVec(pdvData, sVecOffset);

    u32Ret = _readVec(fp, pdvData, psRead);

    return u32Ret;
}

u32 fpWriteVecOffset(FILE * fp, data_vec_t * pdvData,
    olsize_t sVecOffset, olsize_t sWrite)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t size = sWrite;

    u32Ret = _writeVec(fp, sVecOffset, pdvData, &size);

    return u32Ret;
}

/* Write "n" bytes to a file */
u32 fpWriten(FILE * fp, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t nwritten;

    assert((fp != NULL) && (pBuffer != NULL) && (sWrite > 0));

    nwritten = fwrite(pBuffer, 1, sWrite, fp);
    if ((u32)nwritten != sWrite)
        u32Ret = OLERR_FAIL_WRITE_FILE;

    return u32Ret;
}

u32 fpReadLine(FILE * fp, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 n, maxlen;
    olsize_t nread;
    olchar_t c, * p;

    assert((fp != NULL) && (pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 1; n < maxlen; n++)
    {
        nread = 1;
        u32Ret = fpReadn(fp, (void *)&c, &nread);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if (nread == 1) 
            {
                *p++ = c;
                if (c == '\n')
                    break; /* newline is stored, like fgets() */
            } 
            else if (nread == 0) 
            {
                /* EOF, some data was read */
                break;
            }
        }
    }

    *p = 0;/* null terminate like fgets() */

    *psRead = n;

    return u32Ret;
}

/*copy file content from pstrSourceFile to fpDest, pu8Buffer is a buffer
 provided by caller for the function*/
u32 fpCopyFile(FILE * fpDest, const olchar_t * pstrSourceFile,
    u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = OLERR_NO_ERROR;
    FILE * sourcefp = NULL;
    olsize_t size;

    u32Ret = fpOpenFile(pstrSourceFile, "rb", &sourcefp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        while ((u32Ret == OLERR_NO_ERROR) && (! feof(sourcefp)))
        {
            size = sBuf;
            u32Ret = fpReadn(sourcefp, pu8Buffer, &size);
            if (u32Ret == OLERR_NO_ERROR)
            {
                u32Ret = fpWriten(fpDest, pu8Buffer, size);
            }
        }

        if (u32Ret == OLERR_END_OF_FILE)
            u32Ret = OLERR_NO_ERROR;

        fpCloseFile(&sourcefp);
    }

    return u32Ret;
}

boolean_t isDirFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TDIR)
        bRet = TRUE;

    return bRet;
}

boolean_t isRegFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TREG)
        bRet = TRUE;

    return bRet;
}

boolean_t isCharDevice(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TCHR)
        bRet = TRUE;

    return bRet;
}

boolean_t isBlockDevice(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TBLK)
        bRet = TRUE;

    return bRet;
}

boolean_t isFifoFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TFIFO)
        bRet = TRUE;

    return bRet;
}

boolean_t isSockFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TSOCK)
        bRet = TRUE;

    return bRet;
}

boolean_t isLinkFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & FS_MODE_TYPE_MASK) == FS_MODE_TLNK)
        bRet = TRUE;

    return bRet;
}

/*---------------------------------------------------------------------------*/


