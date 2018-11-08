/**
 *  @file files-test.c
 *
 *  @brief the test file for files library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "menu.h"
#include "files.h"
#include "process.h"

/* --- private data/data structure section --------------------------------- */
static olchar_t * ls_pstrDirName = NULL;
static olchar_t * ls_pstrFileName = NULL;
static boolean_t ls_bLockFile = FALSE;
static boolean_t ls_bAppendFile = FALSE;

/* --- private routine section---------------------------------------------- */
static void _printUsage(void)
{
    ol_printf("\
Usage: files-test [-d <directory>] [-l] [-s filename] [-a]\n\
    -a append file\n\
    -l lock file test\n\
    -s test stream file write\n");

    ol_printf("\n");

    exit(0);
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "lad:s:h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            break;
        case 'a':
            ls_bAppendFile = TRUE;
            break;
        case 'l':
            ls_bLockFile = TRUE;
            break;
        case 's':
            ls_pstrFileName = optarg;
            break;
        case 'd':
            ls_pstrDirName = optarg;
            break;
        case ':':
            u32Ret = OLERR_MISSING_PARAM;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _handleFile(const olchar_t * pstrFilename, file_stat_t * pStat,
    void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;

    ol_printf("handling file %s ...\n", pstrFilename);

    return u32Ret;
}

static u32 _listDir(const olchar_t * pstrDir)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_stat_t filestat;

    u32Ret = getFileStat(pstrDir, &filestat);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (isDirFile(filestat.fs_u32Mode))
        {
            u32Ret = traversalDirectory(ls_pstrDirName, _handleFile, NULL);
        }
        else
            u32Ret = OLERR_NOT_A_DIR;
    }

    return u32Ret;
}

static u32 _testFpWrite(olchar_t * file)
{
    u32 u32Ret = OLERR_NO_ERROR;
    FILE * fp = NULL;

    u32Ret = fpOpenFile(file, "r+", &fp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        fpSeekFile(fp, SEEK_SET, 0);
        fpWriten(fp, "hello", 5);

        fpCloseFile(&fp);
    }

    return u32Ret;
}

static olchar_t * ls_pstrLockFile = "lockfile";

THREAD_RETURN_VALUE _testLockFileThread(void * pArg)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t id = (olint_t)(ulong)pArg;
    file_t fd = INVALID_FILE_VALUE;

    ol_printf("thread %d\n", id);

    u32Ret = openFile(ls_pstrLockFile, O_RDONLY, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = lockFile(fd);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("thread %d acquire the lock\n", id);
            sleep(10);

            unlockFile(fd);
            ol_printf("thread %d release the lock\n", id);
        }

        closeFile(&fd);
    }

    THREAD_RETURN(u32Ret);
}

static u32 _testLockFile(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_t fd = INVALID_FILE_VALUE;

    u32Ret = openFile(ls_pstrLockFile, O_CREAT, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        writen(fd, "12345678", 8);
        closeFile(&fd);

        u32Ret = createThread(NULL, NULL, _testLockFileThread, (void *)1);
    }

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = createThread(NULL, NULL, _testLockFileThread, (void *)2);

    sleep(30);
    removeFile(ls_pstrLockFile);

    return u32Ret;
}

static u32 _testAppendFile(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    file_t fd = INVALID_FILE_VALUE;
    olchar_t * pstrFile = "appendfile.txt";

    u32Ret = openFile2(
        pstrFile, O_WRONLY | O_APPEND | O_CREAT,
        DEFAULT_CREATE_FILE_MODE, &fd);
//    u32Ret = openFile(pstrFile, O_WRONLY | O_APPEND, &fd);
    if (u32Ret == OLERR_NO_ERROR)
    {
        writen(fd, "12345678", 8);
        closeFile(&fd);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (ls_pstrDirName != NULL)
            u32Ret = _listDir(ls_pstrDirName);
        else if (ls_pstrFileName != NULL)
            u32Ret = _testFpWrite(ls_pstrFileName);
        else if (ls_bLockFile)
            u32Ret = _testLockFile();
        else if (ls_bAppendFile)
            u32Ret = _testAppendFile();
        else
            _printUsage();
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    exit(0);
}

/*--------------------------------------------------------------------------*/

