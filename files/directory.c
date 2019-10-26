/**
 *  @file directory.c
 *
 *  @brief Files implementation files, provide common routines to manipulate
 *   directory
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
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <sys/types.h>
    #include <dirent.h>
    #include <stdlib.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_dir.h"
#include "jf_err.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

#if defined(LINUX)
typedef jf_dir_t    internal_dir_t;
#elif defined(WINDOWS)
typedef struct
{
    HANDLE id_hDir;
    HANDLE id_hFind;
    olchar_t id_strDirName[JF_LIMIT_MAX_PATH_LEN];
} internal_dir_t;
#endif

/* --- private routine section ------------------------------------------------------------------ */
static u32 _getFirstDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct dirent * pdirent;

    pdirent = readdir(pDir);
    if (pdirent == NULL)
        u32Ret = JF_ERR_DIR_ENTRY_NOT_FOUND;
    else
    {
        memset(pEntry, 0, sizeof(jf_dir_entry_t));
        pEntry->jde_sName = (olsize_t)pdirent->d_reclen;
        ol_strncpy(
            pEntry->jde_strName, pdirent->d_name, JF_LIMIT_MAX_PATH_LEN - 1);
    }
#elif defined(WINDOWS)
    internal_dir_t * pid = (internal_dir_t *)pDir;
    WIN32_FIND_DATA FindFileData;
    boolean_t bRet;
    olchar_t strDirName[JF_LIMIT_MAX_PATH_LEN];

    memset(strDirName, 0, JF_LIMIT_MAX_PATH_LEN);
    ol_snprintf(
        strDirName, JF_LIMIT_MAX_PATH_LEN - 1, "%s%c*", pid->id_strDirName,
        PATH_SEPARATOR);
    pid->id_hFind = FindFirstFile(strDirName, &FindFileData);
    if (pid->id_hFind == INVALID_HANDLE_VALUE) 
        u32Ret = JF_ERR_DIR_ENTRY_NOT_FOUND;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pEntry, 0, sizeof(jf_dir_entry_t));
        pEntry->jde_sName = ol_strlen(FindFileData.cFileName);
        ol_strncpy(
            pEntry->jde_strName, FindFileData.cFileName, JF_LIMIT_MAX_PATH_LEN - 1);
    }
#endif

    return u32Ret;
}

static u32 _getNextDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    struct dirent * pdirent;

    pdirent = readdir(pDir);
    if (pdirent == NULL)
        u32Ret = JF_ERR_DIR_ENTRY_NOT_FOUND;
    else
    {
        memset(pEntry, 0, sizeof(jf_dir_entry_t));
        pEntry->jde_sName = (olsize_t)pdirent->d_reclen;
        ol_strncpy(
            pEntry->jde_strName, pdirent->d_name, JF_LIMIT_MAX_PATH_LEN - 1);
    }
#elif defined(WINDOWS)
    internal_dir_t * pid = (internal_dir_t *)pDir;
    WIN32_FIND_DATA FindFileData;
    boolean_t bRet;

    bRet = FindNextFile(pid->id_hFind, &FindFileData);
    if (bRet)
    {
        memset(pEntry, 0, sizeof(jf_dir_entry_t));
        pEntry->jde_sName = ol_strlen(FindFileData.cFileName);
        ol_strncpy(
            pEntry->jde_strName, FindFileData.cFileName, JF_LIMIT_MAX_PATH_LEN - 1);
    }
    else
    {
        if (GetLastError() == ERROR_NO_MORE_FILES)
            u32Ret = JF_ERR_DIR_ENTRY_NOT_FOUND;
        else
            u32Ret = JF_ERR_FAIL_GET_ENTRY;
    }
#endif

    return u32Ret;
}

static boolean_t _isIgnoreEntry(olchar_t * pstrEntryName)
{
    boolean_t bRet = FALSE;

    if ((strcmp(pstrEntryName, ".") == 0) ||
        (strcmp(pstrEntryName, "..") == 0))
        bRet = TRUE;

    return bRet;
}

static u32 _traversalDirectory(
    olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_stat_t filestat;
    jf_dir_t * pDir = NULL;
    jf_dir_entry_t direntry;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN * 2];

    u32Ret = jf_dir_open(pstrDirName, &pDir);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getFirstDirEntry(pDir, &direntry);
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            if (! _isIgnoreEntry(direntry.jde_strName))
            {
                memset(strFullname, 0, sizeof(strFullname));
                ol_snprintf(
                    strFullname, sizeof(strFullname) - 1, "%s%c%s", pstrDirName,
                    PATH_SEPARATOR, direntry.jde_strName);

                u32Ret = jf_file_getStat(strFullname, &filestat);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = fnHandleFile(strFullname, &filestat, pArg);
                }

                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    if (jf_file_isDirFile(filestat.jfs_u32Mode))
                    {
                        u32Ret = jf_dir_traversal(strFullname, fnHandleFile, pArg);
                    }
                }
            }

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getNextDirEntry(pDir, &direntry);
        }

        if (u32Ret == JF_ERR_DIR_ENTRY_NOT_FOUND)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (pDir != NULL)
        jf_dir_close(&pDir);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_dir_open(const olchar_t * pstrDirName, jf_dir_t ** ppDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dir_t * pDir = NULL;

    assert((pstrDirName != NULL) && (ppDir != NULL));

#if defined(LINUX)
    pDir = opendir(pstrDirName);
    if (pDir != NULL)
        *ppDir = pDir;
    else
        u32Ret = JF_ERR_FAIL_OPEN_DIR;
#elif defined(WINDOWS)
    u32Ret = xmalloc(&pDir, sizeof(internal_dir_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pDir, 0, sizeof(internal_dir_t));
        pDir->id_hDir = CreateFile(
            pstrDirName, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (pDir->id_hDir == INVALID_HANDLE_VALUE)
        {
            u32Ret = JF_ERR_FAIL_OPEN_DIR;
            xfree(&pDir);
        }
        else
        {
            pDir->id_hFind = INVALID_HANDLE_VALUE;
            ol_strncpy(
                pDir->id_strDirName, pstrDirName, JF_LIMIT_MAX_PATH_LEN - 1);
            *ppDir = (jf_dir_t *)pDir;
        }
    }
#endif

    return u32Ret;
}

u32 jf_dir_close(jf_dir_t ** ppDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dir_t * pDir;

    assert(ppDir != NULL);

#if defined(LINUX)
    pDir = *ppDir;
    closedir(pDir);
    *ppDir = NULL;
#elif defined(WINDOWS)
    pDir = (internal_dir_t *)*ppDir;
    CloseHandle(pDir->id_hDir);
    if (pDir->id_hFind != INVALID_HANDLE_VALUE)
        FindClose(pDir->id_hFind);
    xfree(ppDir);
    *ppDir = NULL;
#endif

    return u32Ret;
}

u32 jf_dir_getFirstDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry)
{
    return _getFirstDirEntry(pDir, pEntry);
}

u32 jf_dir_getNextDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry)
{
    return _getNextDirEntry(pDir, pEntry);
}

u32 jf_dir_create(const olchar_t * pstrDirName, jf_file_mode_t mode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t ret;

    ret = mkdir(pstrDirName, mode);
    if (ret != 0)
    {
        if (errno == EEXIST)
            u32Ret = JF_ERR_DIR_ALREADY_EXIST;
        else
            u32Ret = JF_ERR_FAIL_CREATE_DIR;
    }
#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = CreateDirectory(pstrDirName, NULL);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_CREATE_DIR;
#endif

    return u32Ret;
}

u32 jf_dir_remove(const olchar_t * pstrDirName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t ret;

    ret = rmdir(pstrDirName);
    if (ret != 0)
        u32Ret = JF_ERR_FAIL_REMOVE_DIR;
#elif defined(WINDOWS)
    boolean_t bRet;

    bRet = RemoveDirectory(pstrDirName);
    if (! bRet)
        u32Ret = JF_ERR_FAIL_REMOVE_DIR;
#endif

    return u32Ret;
}

u32 jf_dir_traversal(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strName[JF_LIMIT_MAX_PATH_LEN];

    ol_strncpy(strName, pstrDirName, JF_LIMIT_MAX_PATH_LEN - 1);
    strName[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

    u32Ret = _traversalDirectory(strName, fnHandleFile, pArg);

    return u32Ret;
}

u32 jf_dir_parse(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strName[JF_LIMIT_MAX_PATH_LEN];
    jf_file_stat_t filestat;
    jf_dir_t * pDir = NULL;
    jf_dir_entry_t direntry;
    olchar_t strFullname[JF_LIMIT_MAX_PATH_LEN * 2];

    ol_strncpy(strName, pstrDirName, JF_LIMIT_MAX_PATH_LEN - 1);
    strName[JF_LIMIT_MAX_PATH_LEN - 1] = '\0';

    u32Ret = jf_dir_open(strName, &pDir);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getFirstDirEntry(pDir, &direntry);
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            if (! _isIgnoreEntry(direntry.jde_strName))
            {
                ol_memset(strFullname, 0, sizeof(strFullname));
                ol_snprintf(strFullname, sizeof(strFullname) - 1, "%s%c%s", strName,
                         PATH_SEPARATOR, direntry.jde_strName);

                u32Ret = jf_file_getStat(strFullname, &filestat);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    u32Ret = fnHandleFile(strFullname, &filestat, pArg);
                }
            }

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getNextDirEntry(pDir, &direntry);
        }

        if (u32Ret == JF_ERR_DIR_ENTRY_NOT_FOUND)
            u32Ret = JF_ERR_NO_ERROR;
    }

    if (pDir != NULL)
        jf_dir_close(&pDir);

    return u32Ret;
}

FILESAPI u32 FILESCALL jf_dir_scan(
    const olchar_t * pstrDirName, jf_dir_entry_t * entry, olint_t * numofentry,
    jf_dir_fnFilterDirEntry_t fnFilter, jf_dir_fnCompareDirEntry_t fnCompare)
{
	u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t num = 0;
    jf_dir_t * pDir = NULL;
    jf_dir_entry_t direntry, *start = entry;

    u32Ret = jf_dir_open(pstrDirName, &pDir);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getFirstDirEntry(pDir, &direntry);
        while (u32Ret == JF_ERR_NO_ERROR)
        {
            if ((! _isIgnoreEntry(direntry.jde_strName)) &&
				((fnFilter == NULL) || (! fnFilter(&direntry))))
			{
				memcpy(start, &direntry, sizeof(jf_dir_entry_t));
				start ++;
				num++;

				if (num >= *numofentry)
					break;
            }

            if (u32Ret == JF_ERR_NO_ERROR)
                u32Ret = _getNextDirEntry(pDir, &direntry);
        }

        if (u32Ret == JF_ERR_DIR_ENTRY_NOT_FOUND)
            u32Ret = JF_ERR_NO_ERROR;
    }

	if ((u32Ret == JF_ERR_NO_ERROR) && (num != 0) && (fnCompare != NULL))
	{
		qsort(entry, num, sizeof(jf_dir_entry_t), fnCompare);
	}

    if (pDir != NULL)
    {
        jf_dir_close(&pDir);
    }
    
	*numofentry = num;

	return u32Ret;
}

olint_t jf_dir_compareDirEntry(const void * a, const void * b)
{
    jf_dir_entry_t * e1 = (jf_dir_entry_t *)a;
    jf_dir_entry_t * e2 = (jf_dir_entry_t *)b;

	return strcmp(e1->jde_strName, e2->jde_strName);
}
/*------------------------------------------------------------------------------------------------*/


