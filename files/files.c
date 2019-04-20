/**
 *  @file files.c
 *
 *  @brief Files implementation files, provide common routines to manipulate
 *   files
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <sys/types.h>
    #include <dirent.h>
    #include <sys/file.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_err.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */
static u32 _writeVec(
    jf_filestream_t * pjf, olsize_t vecoffset, jf_datavec_t * pjdData,
    olsize_t * psData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t total = *psData, ndata = 0, size;
    jf_datavec_entry_t * entry;
    olint_t index = 0;

    while (total > 0 && u32Ret == JF_ERR_NO_ERROR &&
           index <= pjdData->jd_u16CurEntry)
    {
        entry = &pjdData->jd_pjdeEntry[index ++];

        if (entry->jde_sOffset == 0)
            break;

        if (vecoffset >= entry->jde_sOffset)
        {
            vecoffset -= entry->jde_sOffset;
            continue;
        }

        size = entry->jde_sOffset - vecoffset;
        if (size > total)
            size = total;

        u32Ret = jf_filestream_writen(pjf, entry->jde_pu8Buf + vecoffset, size);

        ndata += size;
        total -= size;
        vecoffset = 0;
    }

    *psData = ndata;

    return u32Ret;
}

static u32 _readVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t total = *psData, ndata = 0, size;
    jf_datavec_entry_t * entry;

    while ((total > 0) && (! jf_filestream_isEndOfFile(pjf)) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        entry = &pjdData->jd_pjdeEntry[pjdData->jd_u16CurEntry];

        size = entry->jde_sBuf - entry->jde_sOffset;
        if (size > total)
            size = total;

        u32Ret = jf_filestream_readn(pjf, entry->jde_pu8Buf + entry->jde_sOffset, &size);

        ndata += size;
        total -= size;

        entry->jde_sOffset += size;
        if (entry->jde_sOffset == entry->jde_sBuf)
        {
            pjdData->jd_u16CurEntry ++;
            if (pjdData->jd_u16CurEntry == pjdData->jd_u16MaxEntry)
                break;
        }
    }

    *psData = ndata;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
#if defined(LINUX)
u32 _setFileStat(jf_file_stat_t * pStat, struct stat * pFileInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pStat, 0, sizeof(jf_file_stat_t));

    if (S_ISREG(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TREG;
    else if (S_ISDIR(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TDIR;
    else if (S_ISCHR(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TCHR;
    else if (S_ISBLK(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TBLK;
    else if (S_ISFIFO(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TFIFO;
    else if (S_ISLNK(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TLNK;
    else if (S_ISSOCK(pFileInfo->st_mode))
        pStat->jfs_u32Mode |= JF_FILE_MODE_TSOCK;

    pStat->jfs_u32Mode |= (pFileInfo->st_mode | 0x0FFF);

    pStat->jfs_u64Size = (u64)pFileInfo->st_size;
    pStat->jfs_u32Dev = (u32)pFileInfo->st_dev;
    pStat->jfs_u32RDev = (u32)pFileInfo->st_rdev;
    pStat->jfs_u32INode = (u32)pFileInfo->st_ino;
    pStat->jfs_u16Link = (u16)pFileInfo->st_nlink;
    pStat->jfs_u16UserId = (u16)pFileInfo->st_uid;
    pStat->jfs_u16GroupId = (u16)pFileInfo->st_gid;
    pStat->jfs_u32AccessTime = (u32)pFileInfo->st_atime;
    pStat->jfs_u32ModifyTime = (u32)pFileInfo->st_mtime;
    pStat->jfs_u32Createtime  = (u32)pFileInfo->st_ctime;

    return u32Ret;
}
#elif defined(WINDOWS)
u32 _setFileStat(jf_file_stat_t * pStat, WIN32_FILE_ATTRIBUTE_DATA * pFileInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    memset(pStat, 0, sizeof(jf_file_stat_t));

    if (pFileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pStat->jfs_u32Mode |= JF_FILE_MODE_TDIR;
    else
        pStat->jfs_u32Mode |= JF_FILE_MODE_TREG;

    pStat->jfs_u64Size = (((u64)pFileInfo->nFileSizeHigh) << 32) + pFileInfo->nFileSizeLow;

    pStat->jfs_u32Createtime = fileTimeToSecondsSince1970(&(pFileInfo->ftCreationTime));
    pStat->jfs_u32AccessTime = fileTimeToSecondsSince1970(&(pFileInfo->ftLastAccessTime));
    pStat->jfs_u32ModifyTime = fileTimeToSecondsSince1970(&(pFileInfo->ftLastWriteTime));

    return u32Ret;
}

#endif

u32 jf_file_getStat(const olchar_t * pstrFilename, jf_file_stat_t * pStat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;
    struct stat filestat;

    nRet = lstat(pstrFilename, &filestat);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_STAT_FILE;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setFileStat(pStat, &filestat);

#elif defined(WINDOWS)
    boolean_t bRet;
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;

    bRet = GetFileAttributesEx(pstrFilename, GetFileExInfoStandard,
        (void *)&FileInfo);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_STAT_FILE;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setFileStat(pStat, &FileInfo);
#endif

    return u32Ret;
}

void jf_file_getDirectoryName(
    olchar_t * pstrDirName, olsize_t sDir, const olchar_t * pstrFullpath)
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

    jf_file_removeTrailingPathSeparator(pstrDirName);
}

void jf_file_getFileName(
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

u32 jf_file_removeTrailingPathSeparator(olchar_t * pstrFullpath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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

u32 jf_file_create(const olchar_t * pstrFilename, jf_file_mode_t mode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd;
#if defined(LINUX)

    fd = creat(pstrFilename, mode);
    if (fd == -1)
        u32Ret = JF_ERR_FAIL_CREATE_FILE;
    else
        jf_file_close(&fd);

#elif defined(WINDOWS)
    fd = CreateFile(
        pstrFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd == INVALID_HANDLE_VALUE)
        u32Ret = JF_ERR_FAIL_CREATE_FILE;
    else
        CloseHandle(fd);

#endif

    return u32Ret;
}

u32 jf_file_open(const olchar_t * pstrFilename, olint_t flags, jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)

    assert((pstrFilename != NULL) && (pFile != NULL));

    *pFile = open(pstrFilename, flags);
    if (*pFile == -1)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;
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
        u32Ret = JF_ERR_FAIL_OPEN_FILE;
#endif

    return u32Ret;
}

u32 jf_file_openWithMode(
    const olchar_t * pstrFilename, olint_t flags, jf_file_mode_t mode,
    jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)

    assert((pstrFilename != NULL) && (pFile != NULL));

    *pFile = open(pstrFilename, flags, mode);
    if (*pFile == -1)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;
#elif defined(WINDOWS)
    u32Ret = openFile(pstrFilename, flags, pFile);
#endif

    return u32Ret;
}

u32 jf_file_close(jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

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

u32 jf_file_remove(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    assert(pstrFilename != NULL);

    nRet = unlink(pstrFilename);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_REMOVE_FILE;

#elif defined(WINDOWS)

    assert(pstrFilename != NULL);
    if (DeleteFile(pstrFilename) == 0)
        u32Ret = JF_ERR_FAIL_REMOVE_FILE;

#endif
    return u32Ret;
}

u32 jf_file_rename(const olchar_t *oldpath, const olchar_t *newpath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    assert((oldpath != NULL) && (newpath != NULL));

    nRet = rename(oldpath, newpath);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_RENAME_FILE;

#elif defined(WINDOWS)
    assert((oldpath != NULL) && (newpath != NULL));

    if (rename(oldpath, newpath) != 0)
        u32Ret = JF_ERR_FAIL_RENAME_FILE;

#endif
    return u32Ret;
}

u32 jf_file_lock(jf_file_t fd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = flock(fd, LOCK_EX);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_LOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = LockFileEx(fd, LOCKFILE_EXCLUSIVE_LOCK, 0,
       0xFFFFFFFF, 0xFFFFFFFF, NULL);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_LOCK_FILE;

#endif

    return u32Ret;
}

u32 jf_file_unlock(jf_file_t fd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = flock(fd, LOCK_UN);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_UNLOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = UnlockFileEx(fd, 0, 0xFFFFFFFF, 0xFFFFFFFF, NULL);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_UNLOCK_FILE;

#endif

    return u32Ret;
}

/* Read "n" bytes from a descriptor */
u32 jf_file_readn(jf_file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    ssize_t nleft, nread;
    u8 * pData;

    assert((pBuffer != NULL) && (*psRead > 0));

    pData = pBuffer;
    nleft = *psRead;
    while (nleft > 0) 
    {
        nread = read(fd, pData, nleft);
        if (nread < 0) 
        {
            if (errno == EINTR)
            {
                /* and call read() again */
                nread = 0;
            }
            else
            {
                u32Ret = JF_ERR_FAIL_READ_FILE;
                break;
            }
        } 
        else if (nread == 0)
        {
            /* EOF */
            if (nleft == *psRead)
                u32Ret = JF_ERR_END_OF_FILE;
            break;
        }

        nleft -= nread;
        pData += nread;
    }

    *psRead = (*psRead) - nleft;

#elif defined(WINDOWS)
    boolean_t bRet;
    olsize_t sread;

    bRet = ReadFile(fd, pBuffer, *psRead, &sread, NULL);
    if (bRet)
    {
        if (sread == 0)
            u32Ret = JF_ERR_END_OF_FILE;
        else
            *psRead = sread;

    }
    else
        u32Ret = JF_ERR_FAIL_READ_FILE;
#endif
    return u32Ret;
}

u32 jf_file_readWithTimeout(
    jf_file_t fd, void * pBuffer, olsize_t * psRead, struct timeval * timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    fd_set fset;
    olint_t ret;
    ssize_t nleft, nread;
    u8 * pData;

    nleft = *psRead;
    pData = pBuffer;
    do
    {
        FD_ZERO(&fset);
        FD_SET(fd, &fset);

        ret = select(fd + 1, &fset, NULL, NULL, timeout);
        if (ret > 0)
        {
            if (FD_ISSET(fd, &fset))
            {
                nread = read(fd, pData, nleft);
                if (nread > 0)
                {
                    pData += nread;
                    nleft -= nread;
                }
            }
        }
        else if (ret == 0)
        {
            /* timeout */
            break;
        }
        else
        {
            /* error */
            if (errno == EINTR)
                continue;
            else
                u32Ret = JF_ERR_FAIL_READ_FILE;
        }
    } while ((u32Ret == JF_ERR_NO_ERROR) && (nleft > 0));

    *psRead = (*psRead) - nleft;

#elif defined(WINDOWS)

#endif
    return u32Ret;
}

/* Write "n" bytes to a descriptor. */
u32 jf_file_writen(jf_file_t fd, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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
                u32Ret = JF_ERR_FAIL_WRITE_FILE;
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
            u32Ret = JF_ERR_FAIL_WRITE_FILE;
    }
    else
        u32Ret = JF_ERR_FAIL_WRITE_FILE;
#endif
    return u32Ret;
}

u32 jf_file_readLine(jf_file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 n, maxlen;
    olsize_t nread;
    olchar_t c, * p;

    assert((pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 1; (n < maxlen) && (u32Ret == JF_ERR_NO_ERROR); n++)
    {
        nread = 1;
        u32Ret = jf_file_readn(fd, (void *)&c, &nread);
        if (u32Ret == JF_ERR_NO_ERROR)
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

u32 jf_filestream_open(
    const olchar_t * pstrFilename, const olchar_t * mode, jf_filestream_t ** ppjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pstrFilename != NULL) && (mode != NULL) && (ppjf != NULL));

    *ppjf = fopen(pstrFilename, mode);
    if (*ppjf == NULL)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;

    return u32Ret;
}

u32 jf_filestream_flush(jf_filestream_t * pjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet;

    nRet = fflush(pjf);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_FLUSH_FILE;

    return u32Ret;
}

olsize_t jf_filestream_printf(jf_filestream_t * pjf, const olchar_t * format, ...)
{
    olsize_t sRet = 0;
    va_list vlParam;

    va_start(vlParam, format);
    sRet = vfprintf(pjf, format, vlParam);
    va_end(vlParam);

    return sRet;
}

u32 jf_filestream_seek(jf_filestream_t * pjf, long offset, olint_t whence)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t ret;

    assert(pjf != NULL);

    ret = fseek(pjf, offset, whence);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_SEEK_FILE;

    return u32Ret;
}

u32 jf_filestream_close(jf_filestream_t ** pjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pjf != NULL) && (*pjf != NULL));

    fclose(*pjf);
    *pjf = NULL;

    return u32Ret;
}

boolean_t jf_filestream_isEndOfFile(jf_filestream_t * pjf)
{
    boolean_t bRet = FALSE;
    olint_t ret;

    ret = feof(pjf);
    if (ret != 0)
        bRet = TRUE;

    return bRet;
}

/* Read "n" bytes from a file */
u32 jf_filestream_readn(
    jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t nread;

    assert((pjf != NULL) && (pBuffer != NULL) && (*psRead > 0));

    nread = fread(pBuffer, 1, *psRead, pjf);
    if ((u32)nread != *psRead)
    {
        if (! feof(pjf))
            u32Ret = JF_ERR_FAIL_READ_FILE;
        else
        {
            if (nread == 0)
                u32Ret = JF_ERR_END_OF_FILE;
        }
    }

    *psRead = nread;

    return u32Ret;
}

u32 jf_filestream_readVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_datavec_init(pjdData);

    u32Ret = _readVec(pjf, pjdData, psRead);

    return u32Ret;
}

u32 jf_filestream_writeVec(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = sWrite;

    u32Ret = _writeVec(pjf, 0, pjdData, &size);

    return u32Ret;
}

u32 jf_filestream_readVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData,
    olsize_t sVecOffset, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_datavec_set(pjdData, sVecOffset);

    u32Ret = _readVec(pjf, pjdData, psRead);

    return u32Ret;
}

u32 jf_filestream_writeVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData,
    olsize_t sVecOffset, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = sWrite;

    u32Ret = _writeVec(pjf, sVecOffset, pjdData, &size);

    return u32Ret;
}

/* Write "n" bytes to a file */
u32 jf_filestream_writen(
    jf_filestream_t * pjf, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t nwritten;

    assert((pjf != NULL) && (pBuffer != NULL) && (sWrite > 0));

    nwritten = fwrite(pBuffer, 1, sWrite, pjf);
    if ((u32)nwritten != sWrite)
        u32Ret = JF_ERR_FAIL_WRITE_FILE;

    return u32Ret;
}

u32 jf_filestream_readLine(
    jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 n, maxlen;
    olsize_t nread;
    olchar_t c, * p;

    assert((pjf != NULL) && (pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 1; n < maxlen; n++)
    {
        nread = 1;
        u32Ret = jf_filestream_readn(pjf, (void *)&c, &nread);
        if (u32Ret == JF_ERR_NO_ERROR)
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
u32 jf_filestream_copyFile(
    jf_filestream_t * pjfDest, const olchar_t * pstrSourceFile,
    u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_filestream_t * sourcejf = NULL;
    olsize_t size;

    u32Ret = jf_filestream_open(pstrSourceFile, "rb", &sourcejf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        while ((u32Ret == JF_ERR_NO_ERROR) && (! feof(sourcejf)))
        {
            size = sBuf;
            u32Ret = jf_filestream_readn(sourcejf, pu8Buffer, &size);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_filestream_writen(pjfDest, pu8Buffer, size);
            }
        }

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;

        jf_filestream_close(&sourcejf);
    }

    return u32Ret;
}

olint_t jf_filestream_getChar(jf_filestream_t * pjf)
{
    return fgetc(pjf);
}

boolean_t jf_file_isDirFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TDIR)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isRegFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TREG)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isCharDevice(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TCHR)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isBlockDevice(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TBLK)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isFifoFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TFIFO)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isSockFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TSOCK)
        bRet = TRUE;

    return bRet;
}

boolean_t jf_file_isLinkFile(u32 u32Mode)
{
    boolean_t bRet = FALSE;

    if ((u32Mode & JF_FILE_MODE_TYPE_MASK) == JF_FILE_MODE_TLNK)
        bRet = TRUE;

    return bRet;
}

/*------------------------------------------------------------------------------------------------*/


