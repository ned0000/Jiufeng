/**
 *  @file file.c
 *
 *  @brief Implementation file of file API.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <errno.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_file.h"
#include "jf_err.h"
#include "jf_string.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

#if defined(LINUX)

u32 _setFileStat(jf_file_stat_t * pStat, struct stat * pFileInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pStat, sizeof(jf_file_stat_t));

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

    pStat->jfs_u32Mode |= (pFileInfo->st_mode & 0x0FFF);

    pStat->jfs_u64Size = (u64)pFileInfo->st_size;
    pStat->jfs_u32Dev = (u32)pFileInfo->st_dev;
    pStat->jfs_u32RDev = (u32)pFileInfo->st_rdev;
    pStat->jfs_u32INode = (u32)pFileInfo->st_ino;
    pStat->jfs_u16Link = (u16)pFileInfo->st_nlink;
    pStat->jfs_u16UserId = (u16)pFileInfo->st_uid;
    pStat->jfs_u16GroupId = (u16)pFileInfo->st_gid;
    pStat->jfs_u64AccessTime = (u64)pFileInfo->st_atime;
    pStat->jfs_u64ModifyTime = (u64)pFileInfo->st_mtime;
    pStat->jfs_u64Createtime  = (u64)pFileInfo->st_ctime;

    return u32Ret;
}

#elif defined(WINDOWS)

u32 _setFileStat(jf_file_stat_t * pStat, WIN32_FILE_ATTRIBUTE_DATA * pFileInfo)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_bzero(pStat, sizeof(jf_file_stat_t));

    if (pFileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pStat->jfs_u32Mode |= JF_FILE_MODE_TDIR;
    else
        pStat->jfs_u32Mode |= JF_FILE_MODE_TREG;

    pStat->jfs_u64Size = (((u64)pFileInfo->nFileSizeHigh) << 32) + pFileInfo->nFileSizeLow;

    pStat->jfs_u64Createtime = jf_time_fileTimeToSecondsSince1970(&(pFileInfo->ftCreationTime));
    pStat->jfs_u64AccessTime = jf_time_fileTimeToSecondsSince1970(&(pFileInfo->ftLastAccessTime));
    pStat->jfs_u64ModifyTime = jf_time_fileTimeToSecondsSince1970(&(pFileInfo->ftLastWriteTime));

    return u32Ret;
}

#endif

u32 jf_file_getStat(const olchar_t * pstrFilename, jf_file_stat_t * pStat)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet = 0;
    struct stat filestat;

    nRet = lstat(pstrFilename, &filestat);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_STAT_FILE;

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setFileStat(pStat, &filestat);

#elif defined(WINDOWS)
    boolean_t bRet = FALSE;
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;

    bRet = GetFileAttributesEx(pstrFilename, GetFileExInfoStandard, (void *)&FileInfo);

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
    pstrDirName[sDir - 1] = JF_STRING_NULL_CHAR;

    /*Locate the last occurrence of the character.*/
    pu8Name = strrchr(pstrDirName, PATH_SEPARATOR);
    if (pu8Name != NULL)
        *pu8Name = JF_STRING_NULL_CHAR;
    else /*path separator is not found, clear the buffer*/
        pstrDirName[0] = JF_STRING_NULL_CHAR;

    /*Remove the trailing path separator.*/
    jf_file_removeTrailingPathSeparator(pstrDirName);
}

void jf_file_getFileName(
    olchar_t * pstrFileName, olsize_t sFileName, const olchar_t * pstrFullpath)
{
    olchar_t * pstrName = NULL;

    assert((pstrFileName != NULL) && (pstrFullpath != NULL));

    /*Locate the last occurrence of the character.*/
    pstrName = strrchr(pstrFullpath, PATH_SEPARATOR);
    if (pstrName == NULL)
    {
        /*Path separator is not found, copy the full path to file name buffer.*/
        ol_strncpy(pstrFileName, pstrFullpath, sFileName - 1);
        pstrFileName[sFileName - 1] = JF_STRING_NULL_CHAR;
    }
    else
    {
        /*Path separator is found.*/
        pstrName++;
        if (*pstrName == JF_STRING_NULL_CHAR)
        {
            /*No file name.*/
            *pstrFileName = JF_STRING_NULL_CHAR;
        }
        else
        {
            /*Found the file name.*/
            ol_strncpy(pstrFileName, pstrName, sFileName - 1);
            pstrFileName[sFileName - 1] = JF_STRING_NULL_CHAR;
        }
    }
}

u32 jf_file_removeTrailingPathSeparator(olchar_t * pstrFullpath)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sPath = 0;

    assert(pstrFullpath != NULL);

    sPath = ol_strlen(pstrFullpath);
    if (sPath > 0)
    {
        sPath --;
        /*Remove all path separators.*/
        while ((sPath >= 0) && (pstrFullpath[sPath] == PATH_SEPARATOR))
            pstrFullpath[sPath --] = JF_STRING_NULL_CHAR;
    }

    return u32Ret;
}

u32 jf_file_create(const olchar_t * pstrFilename, jf_file_mode_t mode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
#if defined(LINUX)

    fd = creat(pstrFilename, mode);
    if (fd == JF_FILE_INVALID_FILE_VALUE)
        u32Ret = JF_ERR_FAIL_CREATE_FILE;
    else
        jf_file_close(&fd);

#elif defined(WINDOWS)
    fd = CreateFile(
        pstrFilename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
        NULL);
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
    if (*pFile == JF_FILE_INVALID_FILE_VALUE)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;

#elif defined(WINDOWS)
    u32 u32Flags = 0, u32CreateFlags = 0, u32ShareMode = 0;

    assert((pstrFilename != NULL) && (pFile != NULL));

    /*Convert the flags.*/
    if (flags & O_WRONLY)
    {
        u32Flags |= GENERIC_WRITE;
    }
    else if (flags & O_RDONLY)
    {
        u32Flags |= GENERIC_READ;
    }
    else if (flags & O_RDWR)
    {
        u32Flags |= GENERIC_WRITE | GENERIC_READ;
    }
    else
    {
        u32Flags |= GENERIC_READ;
        u32ShareMode |= FILE_SHARE_READ;
    }

    /*Some flags convert to create flags.*/
    if ((flags & O_CREAT) && (flags & O_TRUNC))
        u32CreateFlags = CREATE_ALWAYS;
    else if (flags & O_CREAT)
        u32CreateFlags = CREATE_NEW;
    else if (flags & O_TRUNC)
        u32CreateFlags = TRUNCATE_EXISTING;
    else
        u32CreateFlags = OPEN_EXISTING;

    *pFile = CreateFileA(
        pstrFilename, u32Flags, u32ShareMode, NULL, u32CreateFlags, FILE_ATTRIBUTE_NORMAL, NULL);
    if (*pFile == INVALID_HANDLE_VALUE)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;
#endif

    return u32Ret;
}

u32 jf_file_openWithMode(
    const olchar_t * pstrFilename, olint_t flags, jf_file_mode_t mode, jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pstrFilename != NULL) && (pFile != NULL));

#if defined(LINUX)
    *pFile = open(pstrFilename, flags, mode);
    if (*pFile == -1)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;
#elif defined(WINDOWS)
    u32Ret = jf_file_open(pstrFilename, flags, pFile);
#endif

    return u32Ret;
}

u32 jf_file_close(jf_file_t * pFile)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pFile != NULL);

#if defined(LINUX)
    close(*pFile);
#elif defined(WINDOWS)
    CloseHandle(*pFile);
#endif

    *pFile = JF_FILE_INVALID_FILE_VALUE;

    return u32Ret;
}

u32 jf_file_remove(const olchar_t * pstrFilename)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet = 0;

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
    olint_t nRet = 0;

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
    olint_t nRet = 0;

    nRet = flock(fd, LOCK_EX);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_LOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet = FALSE;
    OVERLAPPED overlap;

    bRet = LockFileEx(fd, LOCKFILE_EXCLUSIVE_LOCK, 0, 0xFFFFFFFF, 0xFFFFFFFF, &overlap);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_LOCK_FILE;

#endif

    return u32Ret;
}

u32 jf_file_unlock(jf_file_t fd)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet = 0;

    nRet = flock(fd, LOCK_UN);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_UNLOCK_FILE;

#elif defined(WINDOWS)
    boolean_t bRet = FALSE;

    bRet = UnlockFileEx(fd, 0, 0xFFFFFFFF, 0xFFFFFFFF, NULL);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_UNLOCK_FILE;

#endif

    return u32Ret;
}

u32 jf_file_readn(jf_file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    ssize_t nleft = 0, nread = 0;
    u8 * pData = NULL;

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
                /*Interrupted by a signal, call read() again.*/
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
            /*EOF.*/
            /*Return error if nothing is read.*/
            if (nleft == *psRead)
                u32Ret = JF_ERR_END_OF_FILE;
            break;
        }

        nleft -= nread;
        pData += nread;
    }

    *psRead = (*psRead) - nleft;

#elif defined(WINDOWS)
    boolean_t bRet = FALSE;
    olsize_t sread = 0;

    bRet = ReadFile(fd, pBuffer, *psRead, &sread, NULL);
    if (bRet)
    {
        if (sread == 0)
            u32Ret = JF_ERR_END_OF_FILE;
        else
            *psRead = sread;
    }
    else
    {
        u32Ret = JF_ERR_FAIL_READ_FILE;
    }
#endif
    return u32Ret;
}

u32 jf_file_readWithTimeout(
    jf_file_t fd, void * pBuffer, olsize_t * psRead, struct timeval * timeout)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    fd_set fset;
    olint_t ret = 0;
    ssize_t nleft = 0, nread = 0;
    u8 * pData = NULL;

    nleft = *psRead;
    pData = pBuffer;
    do
    {
        FD_ZERO(&fset);
        FD_SET(fd, &fset);

        /*Use select to control the timeout.*/
        ret = select(fd + 1, &fset, NULL, NULL, timeout);
        if (ret > 0)
        {
            /*File is readable.*/
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
            /*Timeout.*/
            break;
        }
        else
        {
            /*Error*/
            /*Continue if interrupted.*/
            if (errno == EINTR)
                continue;
            else /*Return error.*/
                u32Ret = JF_ERR_FAIL_READ_FILE;
        }
    } while ((u32Ret == JF_ERR_NO_ERROR) && (nleft > 0));

    *psRead = (*psRead) - nleft;

#elif defined(WINDOWS)

    u32Ret = JF_ERR_NOT_IMPLEMENTED;

#endif
    return u32Ret;
}

u32 jf_file_writen(jf_file_t fd, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olsize_t nleft = 0;
    ssize_t nwritten = 0;
    const olchar_t * p = NULL;

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
                /*Interrupted by a signal, call write() again.*/
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
    boolean_t bRet = FALSE;
    olsize_t nwrite = 0;

    bRet = WriteFile(fd, pBuffer, sWrite, &nwrite, NULL);
    if (bRet)
    {
        if (nwrite != sWrite)
            u32Ret = JF_ERR_FAIL_WRITE_FILE;
    }
    else
    {
        u32Ret = JF_ERR_FAIL_WRITE_FILE;
    }
#endif
    return u32Ret;
}

u32 jf_file_readLine(jf_file_t fd, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 n = 0, maxlen = 0;
    olsize_t nread = 0;
    olchar_t c = 0, * p = NULL;

    assert((pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 0; n < maxlen - 1; n ++)
    {
        /*Read 1 character.*/
        nread = 1;
        u32Ret = jf_file_readn(fd, (void *)&c, &nread);

        /*Save the character.*/
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (nread == 1) 
            {
                *p++ = c;
                if (c == JF_STRING_LINE_FEED_CHAR)
                    break; /*Newline is stored.*/
            } 
            /*No need to check the case (nread == 0), as JF_ERR_END_OF_FILE is returned.*/
        }

        if (u32Ret != JF_ERR_NO_ERROR)
            break;
    }

    /*Null terminated is always appended.*/
    *p = JF_STRING_NULL_CHAR;

    *psRead = n;

    return u32Ret;
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

boolean_t jf_file_isTypedFile(
    const olchar_t * pstrName, const olchar_t * pstrPrefix, const olchar_t * pstrFileExt)
{
    boolean_t bRet = FALSE;
    olsize_t namesize = 0, extsize = 0;

    if (pstrPrefix != NULL)
    {
        /*Compare the prefix*/
        if (ol_strncmp(pstrName, pstrPrefix, ol_strlen(pstrPrefix)) != 0)
            return bRet;
    }

    if (pstrFileExt != NULL)
    {
        /*Compare the size.*/
        namesize = ol_strlen(pstrName);
        extsize = ol_strlen(pstrFileExt);
        if (namesize <= extsize)
            return bRet;

        /*Compare the extension.*/
        if (ol_strcmp(pstrName + namesize - extsize, pstrFileExt) != 0)
            return bRet;
    }

    return TRUE;
}

/*------------------------------------------------------------------------------------------------*/
