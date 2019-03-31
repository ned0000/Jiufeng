/**
 *  @file create.c
 *
 *  @brief functions to create archive
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "archive.h"
#include "errcode.h"
#include "archivecommon.h"
#include "files.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

static u32 _alignData(ar_file_t * paf, file_stat_t * pFilestat, u8 * pu8Buffer)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olsize_t append;

    append = (olsize_t)(pFilestat->fs_u64Size % ARCHIVE_BLOCK_SIZE);
    if (append != 0)
    {
        memset(pu8Buffer, 0, ARCHIVE_BLOCK_SIZE - append);
        u32Ret = writeArFile(paf, (u8 *)pu8Buffer, ARCHIVE_BLOCK_SIZE - append);
    }

    return u32Ret;
}

static u32 _setName(archive_header_t * pah, const olchar_t * pstrFullpath,
    file_stat_t * pFilestat)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strFilename[MAX_PATH_LEN];
    archive_block_t * pab;

    getFileName(strFilename, MAX_PATH_LEN, pstrFullpath);
    if (strlen(strFilename) < AH_NAME_LEN)
    {
        ol_snprintf((olchar_t *)pah->ah_u8Name, AH_NAME_LEN - 1, "%s", strFilename);
    }
    else
    {
        pah->ah_u8NameLen = AH_NAME_LEN_LONG;
        pab = (archive_block_t *)((u8 *)pah + ARCHIVE_BLOCK_SIZE);
        memset(pab, 0, ARCHIVE_BLOCK_SIZE);
        ol_snprintf((olchar_t *)pab->ab_u8Buffer, ARCHIVE_BLOCK_SIZE - 1, "%s", strFilename);
    }

    return u32Ret;
}

static u32 _countDirDepth(const olchar_t * pstrNewPath, olchar_t * pstrSavedPath,
    u8 * pu8Depth)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strNewPath[MAX_PATH_LEN], strSavedPath[MAX_PATH_LEN];
    u32 u32Depth;

    getDirectoryName(strNewPath, MAX_PATH_LEN, pstrNewPath);
    u32Depth = 0;

    while (pstrSavedPath[0] != '\0')
    {
        if (strcmp(strNewPath, pstrSavedPath) == 0)
        {
            break;
        }
        else
        {
            u32Depth ++;
            getDirectoryName(strSavedPath, MAX_PATH_LEN - 1, pstrSavedPath);
            if (strSavedPath[0] == '\0')
                break;
            else
                ol_strcpy(pstrSavedPath, strSavedPath);
        }
    }

    if (u32Depth >= AH_DIR_DEPTH_NULL)
        u32Ret = OLERR_REACH_MAX_DIR_DEPTH;
    else
        *pu8Depth = (u8)u32Depth;

    return u32Ret;
}

static u32 _setDirDepth(archive_header_t * pah, const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (pfh->fh_strFullpath[0] == '\0')
    {
        pah->ah_u8DirDepth = AH_DIR_DEPTH_NULL;
    }
    else
    {
        u32Ret = _countDirDepth(pstrFullpath, pfh->fh_strFullpath,
            &(pah->ah_u8DirDepth));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strcpy(pfh->fh_strFullpath, pstrFullpath);
    }

    return u32Ret;
}

static u32 _setFileMode(archive_header_t * pah, file_stat_t * pFilestat)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Mode = 0;

    u32Mode = ((pFilestat->fs_u32Mode & FS_MODE_SUID ? AH_MODE_SUID : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_SGID ? AH_MODE_SGID : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_SVTX ? AH_MODE_SVTX : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_RUSR ? AH_MODE_UREAD : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_WUSR ? AH_MODE_UWRITE : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_XUSR ? AH_MODE_UEXEC : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_RGRP ? AH_MODE_GREAD : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_WGRP ? AH_MODE_GWRITE : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_XGRP ? AH_MODE_GEXEC : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_ROTH ? AH_MODE_OREAD : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_WOTH ? AH_MODE_OWRITE : 0) |
               (pFilestat->fs_u32Mode & FS_MODE_XOTH ? AH_MODE_OEXEC : 0));

    ol_snprintf((olchar_t *)pah->ah_u8Mode, AH_MODE_LEN - 1, "%u", u32Mode);

    return u32Ret;
}

static u32 _setCommonFields(archive_header_t * pah, const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _setName(pah, pstrFullpath, pFilestat);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setDirDepth(pah, pstrFullpath, pFilestat, pfh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setFileMode(pah, pFilestat);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_snprintf((olchar_t *)pah->ah_u8UserId, AH_USER_ID_LEN - 1, "%d",
            pFilestat->fs_u16UserId);
        ol_snprintf((olchar_t *)pah->ah_u8GroupId, AH_USER_GROUP_ID_LEN - 1, "%d",
            pFilestat->fs_u16GroupId);
        ol_snprintf((olchar_t *)pah->ah_u8ModifyTime, AH_TIME_LEN - 1, "%u",
            pFilestat->fs_u32ModifyTime);
        ol_snprintf((olchar_t *)pah->ah_u8Magic, AH_MAGIC_LEN - 1, "%s", AH_MAGIC);
        /* version */
        memcpy(pah->ah_u8Version, AH_VERSION, AH_VERSION_LEN);
    }

    return u32Ret;
}

static u32 _saveDir(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
    pah->ah_u8Type = AH_TYPE_DIR;
    u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    return u32Ret;
}

static u32 _copyRegularFile(ar_file_t * paf, const olchar_t * pstrSourceFile,
    u8 * u8Buffer, olsize_t sBuf)
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
            u32Ret = fpReadn(sourcefp, u8Buffer, &size);
            if (u32Ret == OLERR_NO_ERROR)
            {
                u32Ret = writeArFile(paf, u8Buffer, size);
            }
        }

        if (u32Ret == OLERR_END_OF_FILE)
            u32Ret = OLERR_NO_ERROR;

        fpCloseFile(&sourcefp);
    }

    return u32Ret;
}

static u32 _saveRegularFile(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
    pah->ah_u8Type = AH_TYPE_REGULAR;
    u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    if (u32Ret == OLERR_NO_ERROR)
    {
		snprintf((olchar_t *)pah->ah_u8Size, AH_SIZE_LEN - 1, "%llu",
            pFilestat->fs_u64Size);
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _copyRegularFile(pfh->fh_pafArchive, pstrFullpath,
            pfh->fh_pu8Buffer, pfh->fh_sBuf);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _alignData(pfh->fh_pafArchive, pFilestat, pfh->fh_pu8Buffer);
    }

    return u32Ret;
}

static u32 _saveDeviceFile(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
	if (isCharDevice(pFilestat->fs_u32Mode))
        pah->ah_u8Type = AH_TYPE_CHAR;
    else if (isBlockDevice(pFilestat->fs_u32Mode))
        pah->ah_u8Type = AH_TYPE_BLOCK;
    else
        u32Ret = OLERR_INVALID_FILE_TYPE;

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _saveLinkFile(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
    pah->ah_u8Type = AH_TYPE_SYM_LINK;

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _saveFifoFile(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
    pah->ah_u8Type = AH_TYPE_FIFO;

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _saveSockFile(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;
    archive_header_t * pah = (archive_header_t *)pfh->fh_pu8Buffer;

    memset(pah, 0, sizeof(archive_block_t));
    pah->ah_u8Type = AH_TYPE_SOCK;

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _setCommonFields(pah, pstrFullpath, pFilestat, pfh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = writeArFile(pfh->fh_pafArchive, (u8 *)pah, ARCHIVE_BLOCK_SIZE);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {

    }

    return u32Ret;
}

static u32 _addFileToArchive(const olchar_t * pstrFullpath,
    file_stat_t * pFilestat, file_handler_t * pfh)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (isDirFile(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveDir(pstrFullpath, pFilestat, pfh);
    }
    else if (isRegFile(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveRegularFile(pstrFullpath, pFilestat, pfh);
    }
	else if (isCharDevice(pFilestat->fs_u32Mode) ||
             isBlockDevice(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveDeviceFile(pstrFullpath, pFilestat, pfh);
    }
    else if (isLinkFile(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveLinkFile(pstrFullpath, pFilestat, pfh);
    }
    else if (isFifoFile(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveFifoFile(pstrFullpath, pFilestat, pfh);
    }
    else if (isSockFile(pFilestat->fs_u32Mode))
    {
        u32Ret = _saveSockFile(pstrFullpath, pFilestat, pfh);
    }
    else
    {
        logInfoMsg("unrecognized file type: %s",
            pFilestat->fs_u32Mode);
        u32Ret = OLERR_UNRECOGNIZED_FILE_TYPE;
    }

    return u32Ret;
}

static u32 _handleMemberFile(const olchar_t * pstrFullpath, file_stat_t * pFilestat, void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_handler_t * pfh = (file_handler_t *)pArg;

    if (pfh->fh_bVerbose)
        ol_printf("add file: %s\n", pstrFullpath);

    logInfoMsg("add file: %s", pstrFullpath);

    u32Ret = _addFileToArchive(pstrFullpath, pFilestat, pfh);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 writeToArchive(
    ar_file_t * pafArchive, const olchar_t * pstrFullpath,
    u8 * pu8Buffer, olsize_t sBuf, jf_archive_create_param_t * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_stat_t filestat;
    file_handler_t fh;
    olchar_t strName[MAX_PATH_LEN];

    u32Ret = getFileStat(pstrFullpath, &filestat);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_strncpy(strName, pstrFullpath, MAX_PATH_LEN - 1);
        strName[MAX_PATH_LEN - 1] = '\0';
        removeTrailingPathSeparator(strName);

        memset(&fh, 0, sizeof(file_handler_t));
        fh.fh_pafArchive = pafArchive;
        fh.fh_pu8Buffer = pu8Buffer;
        fh.fh_sBuf = sBuf;
        fh.fh_bVerbose = pParam->jacp_bVerbose;

        u32Ret = _handleMemberFile(strName, &filestat, (void *)&fh);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (isDirFile(filestat.fs_u32Mode))
        {
            ol_strcpy(fh.fh_strFullpath, strName);
            u32Ret = traversalDirectory(strName, _handleMemberFile, (void *)&fh);
        }
    }

    return u32Ret;
} 

/*---------------------------------------------------------------------------*/


