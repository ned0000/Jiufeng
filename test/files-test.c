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
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_file.h"
#include "jf_filestream.h"
#include "jf_dir.h"
#include "jf_thread.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static olchar_t * ls_pstrDirName = NULL;
static olchar_t * ls_pstrFileName = NULL;
static boolean_t ls_bLockFile = FALSE;
static boolean_t ls_bAppendFile = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _printFilesTestUsage(void)
{
    ol_printf("\
Usage: files-test [-d directory] [-l] [-s filename] [-a]\n\
    -d list directory.\n\
    -a append file.\n\
    -l lock file test.\n\
    -s test stream file write.\n");

    ol_printf("\n");

}

static u32 _parseFilesTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "lad:s:h?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printFilesTestUsage();
            exit(0);
            break;
        case 'a':
            ls_bAppendFile = TRUE;
            break;
        case 'l':
            ls_bLockFile = TRUE;
            break;
        case 's':
            ls_pstrFileName = jf_option_getArg();
            break;
        case 'd':
            ls_pstrDirName = jf_option_getArg();
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
            u32Ret = jf_dir_traversal(ls_pstrDirName, _handleFile, NULL);
        }
        else
        {
            u32Ret = JF_ERR_NOT_A_DIR;
        }
    }

    return u32Ret;
}

static u32 _testFpWrite(olchar_t * file)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_filestream_t * pjf = NULL;

    u32Ret = jf_filestream_open(file, "r+", &pjf);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_filestream_seek(pjf, SEEK_SET, 0);
        jf_filestream_writen(pjf, "hello", 5);

        jf_filestream_close(&pjf);
    }

    return u32Ret;
}

static olchar_t * ls_pstrLockFile = "lockfile";

JF_THREAD_RETURN_VALUE _testLockFileThread(void * pArg)
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

static u32 _testLockFile(void)
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

        u32Ret = jf_thread_create(NULL, NULL, _testLockFileThread, (void *)&index1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_thread_create(NULL, NULL, _testLockFileThread, (void *)&index2);

    ol_sleep(30);
    jf_file_remove(ls_pstrLockFile);

    return u32Ret;
}

static u32 _testAppendFile(void)
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
        if (ls_pstrDirName != NULL)
            u32Ret = _listDir(ls_pstrDirName);
        else if (ls_pstrFileName != NULL)
            u32Ret = _testFpWrite(ls_pstrFileName);
        else if (ls_bLockFile)
            u32Ret = _testLockFile();
        else if (ls_bAppendFile)
            u32Ret = _testAppendFile();
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
