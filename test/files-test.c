/**
 *  @file files-test.c
 *
 *  @brief Test file for file function defined in jf_files library.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_thread.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrDirNameOfFt = NULL;

static boolean_t ls_bTestLockFileInFt = FALSE;

static boolean_t ls_bTestAppendFileInFt = FALSE;

static boolean_t ls_bTestFileFuncInFt = FALSE;

static boolean_t ls_bTestFileStreamFuncInFt = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printFilesTestUsage(void)
{
    ol_printf("\
Usage: files-test [-d directory] [-f] [-l] [-s] [-a]\n\
  -d: test directory list.\n\
  -a: test file append.\n\
  -f: test file functions.\n\
  -l: test file lock in multi-thread environment.\n\
  -s: test file stream functions.\n");

    ol_printf("\n");

}

static u32 _parseFilesTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lfad:sh?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printFilesTestUsage();
            exit(0);
            break;
        case 'a':
            ls_bTestAppendFileInFt = TRUE;
            break;
        case 'f':
            ls_bTestFileFuncInFt = TRUE;
            break;
        case 'l':
            ls_bTestLockFileInFt = TRUE;
            break;
        case 's':
            ls_bTestFileStreamFuncInFt = TRUE;
            break;
        case 'd':
            ls_pstrDirNameOfFt = jf_option_getArg();
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_OPTION_ARG;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _handleFile(const olchar_t * pstrFilename, jf_file_stat_t * pStat, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("handling file %s ...\n", pstrFilename);

    return u32Ret;
}

static u32 _listDir(const olchar_t * pstrDir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_stat_t filestat;

    u32Ret = jf_file_getStat(pstrDir, &filestat);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (jf_file_isDirFile(filestat.jfs_u32Mode))
        {
            u32Ret = jf_dir_traverse(ls_pstrDirNameOfFt, _handleFile, NULL);
        }
        else
        {
            u32Ret = JF_ERR_NOT_A_DIR;
        }
    }

    return u32Ret;
}

static u32 _testFileStreamWriteInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrFile = "file-stream-write-file";
    jf_filestream_t * pjf = NULL;

    ol_printf("----------------------------------------------\n");
    ol_printf("Test file stream write\n");

    u32Ret = jf_filestream_open(pstrFile, "w", &pjf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_filestream_seek(pjf, SEEK_SET, 0);
        jf_filestream_writen(pjf, "hello", 5);

        jf_filestream_close(&pjf);
    }

    return u32Ret;
}

static u32 _testFileStreamReadLineInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_filestream_t * pjf = NULL;
    olchar_t * pstrFile = "file-stream-read-line-file";
    olchar_t buf[8];
    olsize_t sBuf = sizeof(buf);

    ol_printf("----------------------------------------------\n");
    ol_printf("Test file stream read line\n");

    u32Ret = jf_filestream_open(pstrFile, "r", &pjf);

    if (u32Ret != JF_ERR_NO_ERROR)
        ol_printf("Failed to open file: %s\n", pstrFile);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        sBuf = sizeof(buf);
        u32Ret = jf_filestream_readLine(pjf, buf, &sBuf);

        ol_printf("Data (%d): %s, return: 0x%X\n", sBuf, buf, u32Ret);
    }

    if (pjf != NULL)
        jf_filestream_close(&pjf);

    return u32Ret;
}

static u32 _testFileStreamFuncInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _testFileStreamWriteInFt();

    _testFileStreamReadLineInFt();

    return u32Ret;
}

static olchar_t * ls_pstrLockFile = "lockfile";

JF_THREAD_RETURN_VALUE _testLockFileInFtThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t id = *(olint_t *)pArg;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;

    ol_printf("thread %d starts\n", id);

    u32Ret = jf_file_open(ls_pstrLockFile, O_RDONLY, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("lock file\n");
        u32Ret = jf_file_lock(fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("thread %d acquire the lock\n", id);
            ol_sleep(10);

            jf_file_unlock(fd);
            ol_printf("thread %d release the lock\n", id);
        }

        jf_file_close(&fd);
    }

    ol_printf("thread %d quits\n", id);

    JF_THREAD_RETURN(u32Ret);
}

static u32 _testFileFuncInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t * pstrFile = "file-test-read-line-file";
    olchar_t buf[8];
    olsize_t sBuf = sizeof(buf);

    u32Ret = jf_file_openWithMode(
        pstrFile, O_RDWR | O_CREAT, JF_FILE_MODE_RUSR | JF_FILE_MODE_WUSR, &fd);

    while (u32Ret == JF_ERR_NO_ERROR)
    {
        sBuf = sizeof(buf);
        u32Ret = jf_file_readLine(fd, buf, &sBuf);

        ol_printf("Data (%d): %s, return: 0x%X\n", sBuf, buf, u32Ret);
    }

    if (fd != JF_FILE_INVALID_FILE_VALUE)
        jf_file_close(&fd);

    return u32Ret;
}

static u32 _testLockFileInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olint_t index1 = 1, index2 = 2;

    u32Ret = jf_file_openWithMode(
        ls_pstrLockFile, O_WRONLY | O_CREAT, JF_FILE_MODE_RUSR | JF_FILE_MODE_WUSR, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_file_writen(fd, "12345678", 8);
        jf_file_close(&fd);

        u32Ret = jf_thread_create(NULL, NULL, _testLockFileInFtThread, (void *)&index1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_create(NULL, NULL, _testLockFileInFtThread, (void *)&index2);

    ol_sleep(30);
    jf_file_remove(ls_pstrLockFile);

    return u32Ret;
}

static u32 _testAppendFileInFt(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_file_t fd = JF_FILE_INVALID_FILE_VALUE;
    olchar_t * pstrFile = "appendfile.txt";

    u32Ret = jf_file_openWithMode(
        pstrFile, O_WRONLY | O_APPEND | O_CREAT, JF_FILE_DEFAULT_CREATE_MODE, &fd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_file_writen(fd, "12345678", 8);
        jf_file_close(&fd);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseFilesTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_pstrDirNameOfFt != NULL)
            u32Ret = _listDir(ls_pstrDirNameOfFt);
        else if (ls_bTestFileStreamFuncInFt)
            u32Ret = _testFileStreamFuncInFt();
        else if (ls_bTestLockFileInFt)
            u32Ret = _testLockFileInFt();
        else if (ls_bTestAppendFileInFt)
            u32Ret = _testAppendFileInFt();
        else if (ls_bTestFileFuncInFt)
            u32Ret = _testFileFuncInFt();
        else
            _printFilesTestUsage();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    exit(0);
}

/*------------------------------------------------------------------------------------------------*/
