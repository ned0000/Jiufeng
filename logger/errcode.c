/**
 *  @file
 *
 *  @brief The error code implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_err.h"
#include "common.h"

/* --- private data/data structure section --------------------------------- */
#define ERR_MSG_FORMAT      "ERR - (0x%X) %s"
#define SYS_ERR_MSG_FORMAT  "ERR - (0x%X) %s\n      %d, %s"

/* error code descriptions */
typedef struct
{
    u32 iecd_u32ErrorCode;
    olchar_t * iecd_pstrDesc;
} internal_error_code_desc_t;

/* the data in ls_iecdErrorCodeDesc must be sorted in the ascending order of error code */
/* in order to support binary search */
static internal_error_code_desc_t ls_iecdErrorCodeDesc[] = 
{
    {JF_ERR_NO_ERROR, "No error."},
    {JF_ERR_UNKNOWN_ERROR, "Unknown error."},
/* generic errors */
    {JF_ERR_NOT_FOUND, "Not found."},
    {JF_ERR_TIMEOUT, "Operation has timed out."},
    {JF_ERR_NULL_POINTER, "Null pointer."},
    {JF_ERR_FILE_NOT_FOUND, "File is not found."},
    {JF_ERR_NOT_SUPPORTED, "Not supported."},
    {JF_ERR_OPERATION_FAIL, "Operation failed due to some system error."},
    {JF_ERR_PROGRAM_ERROR, "Program error. Please contact the technical support."},
    {JF_ERR_BUFFER_TOO_SMALL, "Buffer is too small."},
    {JF_ERR_WRONG_PRODUCT_TYPE, "Operation is not supported by this product type."},
    {JF_ERR_NOT_IMPLEMENTED, "Not implemented."},
    {JF_ERR_OUT_OF_RANGE, "Out of range."},
    {JF_ERR_INVALID_VERSION, "Invalid version."},
    {JF_ERR_PENDING, "Pending."},
    {JF_ERR_INVALID_DATA, "Invalid data."},
    {JF_ERR_BUSY, "Busy."},
    {JF_ERR_INVALID_PARAM, "Invalid parameter."},
    {JF_ERR_MISSING_PARAM, "Missing parameter."},
    {JF_ERR_ALREADY_EXIST, "Already exist."},
    {JF_ERR_INVALID_FILE, "Invalid file."},
    {JF_ERR_INVALID_INPUT, "Invalid input."},
    {JF_ERR_SERVICE_UNAVAILABLE, "Service is unavailable."},
    {JF_ERR_SERVICE_BUSY, "Service is busy."},
    {JF_ERR_NOT_READY, "Not ready."},
    {JF_ERR_NOT_MATCH, "Not match."},
/* queue error */
    {JF_ERR_FAIL_CREATE_QUEUE, "Failed to creat queue."},
/* mem error */
    {JF_ERR_OUT_OF_MEMORY, "Out of memory."},
/* array error */

/* mutex error */
    {JF_ERR_FAIL_CREATE_MUTEX, "Failed to create mutex."},
    {JF_ERR_FAIL_DESTROY_MUTEX, "Failed to destroy mutex."},
    {JF_ERR_FAIL_ACQUIRE_MUTEX, "Failed to acquire mutex."},
    {JF_ERR_FAIL_RELEASE_MUTEX, "Failed to release mutex."},
/* respool error */
    {JF_ERR_RESOURCE_BUSY, "The requested resource is busy."},
/* hash error */

/* conffile error */

/* host error */

/* menu error */

/* process error */
    {JF_ERR_PROCESS_CREATED, "Process has been created. The program continues in background."},
    {JF_ERR_ALREADY_RUNNING, "The program is already running."},
    {JF_ERR_FAIL_CREATE_PROCESS, "Failed to create process."},
/* thread error */
    {JF_ERR_FAIL_CREATE_THREAD, "Failed to create thread."},
/* shared memory error */
    {JF_ERR_INVALID_SHAREDMEMORY_ID, "Invalid shared memory identifier."},
    {JF_ERR_FAIL_CREATE_SHAREDMEMORY, "Failed to create shared memory."},
    {JF_ERR_FAIL_ATTACH_SHAREDMEMORY, "Failed to attach shared memory."},
    {JF_ERR_FAIL_DETACH_SHAREDMEMORY, "Failed to detach shared memory."},
    {JF_ERR_FAIL_DESTROY_SHAREDMEMORY, "Failed to detroy shared memory."},
/* time error */

/* date error */

/* resouce error */

/* syncsem error */
    {JF_ERR_FAIL_CREATE_SEM, "Failed to create semaphore."},
    {JF_ERR_FAIL_DESTROY_SEM, "Failed to destroy semaphore."},
    {JF_ERR_FAIL_ACQUIRE_SEM, "Failed to acquire semaphore."},
    {JF_ERR_FAIL_RELEASE_SEM, "Failed to release semaphore."},
/* dynlib error */
    {JF_ERR_FAIL_LOAD_DYNLIB, "Failed to load dynamic library."},
    {JF_ERR_FAIL_FREE_DYNLIB, "Failed to free dynamic library."},
    {JF_ERR_FAIL_GET_SYMBOL_ADDR, "Failed to get symbol's address."},
/* logger error */
    {JF_ERR_CALLER_NAME_TOO_LONG, "Length of the caller name is too long."},
/* archive error */
    {JF_ERR_ARCHIVE_CORRUPTED, "Archive file is corrupted."},
/* network error */
    {JF_ERR_FAIL_INIT_NETWORK_LIB, "Failed to initialize network library."},
    {JF_ERR_SOCKET_PEER_CLOSED, "Connection is closed by peer."},
    {JF_ERR_SOCKET_ALREADY_CLOSED, "Connection is already closed."},
    {JF_ERR_HOST_NOT_FOUND, "The specified host is unknown."},
    {JF_ERR_HOST_NO_ADDRESS, "The requested host name is valid but does not have an IP address."},
    {JF_ERR_NAME_SERVER_NO_RECOVERY, "A non-recoverable name server error occurred."},
    {JF_ERR_RESOLVE_TRY_AGAIN, "A temporary error occurred on an authoritative name server. Try again later."},

    {JF_ERR_FAIL_SEND_DATA, "Failed to send data."},
    {JF_ERR_FAIL_RECV_DATA, "Failed to receive data."},
    {JF_ERR_FAIL_INITIATE_CONNECTION, "Failed to initiate connection."},
    {JF_ERR_FAIL_ACCEPT_CONNECTION, "Failed to accept connection."},
/* encrypt error */

/* encode error */

/* clieng error */
    {JF_ERR_INVALID_CMD_GROUP, "Invalid command or request group."},
    {JF_ERR_INVALID_COMMAND, "Invalid command or request."},
    {JF_ERR_CMD_TOO_LONG, "Command line is too long."},
    {JF_ERR_BLANK_CMD, "Command line is blank" },
    {JF_ERR_INVALID_COUNT, "Invalid count."},
    {JF_ERR_COMMENT_CMD, "Command line is for comments."},
    {JF_ERR_LINE_TOO_LONG, "Output line is too long."},
/* string error */
    {JF_ERR_INVALID_STRING, "Invalid string."},
    {JF_ERR_INVALID_SIZE, "Invalid size."},
    {JF_ERR_INVALID_IP, "Invalid IP address."},
    {JF_ERR_INVALID_IP_ADDR_TYPE, "Invalid IP address type."},
    {JF_ERR_NULL_IP_ADDRESS, "IP address or subnet mask cannot be all zero."},
    {JF_ERR_INVALID_DATE, "Invalid setting for date."},
    {JF_ERR_INVALID_TIME, "Invalid setting for time."},
    {JF_ERR_INVALID_ALIAS, "Invalid character in alias."},
    {JF_ERR_INVALID_INTEGER, "Invalid input for integer parameter(s)."},
    {JF_ERR_SUBSTRING_NOT_FOUND, "Substring is not found."},
    {JF_ERR_INVALID_FLOAT, "Invalid input for float parameter(s)."},

    {JF_ERR_INVALID_SETTING, "Invalid setting parameters."},
    {JF_ERR_SETTING_TOO_LONG, "Setting parameters too long."},
    {JF_ERR_MISSING_QUOTE, "Missing quote in parameters."},
    {JF_ERR_SETTING_EMPTY, "Empty value is not allowed for the setting option."},
/* files error */
    {JF_ERR_NOT_A_DIR, "Not a directory."},
    {JF_ERR_DIR_ALREADY_EXIST, "Directory already exists."},

    {JF_ERR_END_OF_FILE, "End of file."},
    {JF_ERR_FILE_ACCESS_VIOLOATION, "Invalid file access permission."},
    {JF_ERR_FILE_PATH_TOO_LONG, "The file path is too long."},

    {JF_ERR_DIR_ENTRY_NOT_FOUND, "Cannot find the directory entry."},
    {JF_ERR_FAIL_CREATE_DIR, "Failed to create directory."},
    {JF_ERR_FAIL_REMOVE_DIR, "Failed to remove directory."},
    {JF_ERR_FAIL_OPEN_DIR, "Failed to open directory."},

    {JF_ERR_FAIL_CREATE_FILE, "Failed to create file."},
    {JF_ERR_FAIL_OPEN_FILE, "Failed to open file."},
    {JF_ERR_FAIL_READ_FILE, "Failed to read file."},
    {JF_ERR_FAIL_WRITE_FILE, "Failed to write file."},
    {JF_ERR_FAIL_STAT_FILE, "Failed to get file status."},
    {JF_ERR_FAIL_SEEK_FILE, "Failed to seek file."},
    {JF_ERR_FAIL_FLUSH_FILE, "Failed to flush file."},
    {JF_ERR_FAIL_LOCK_FILE, "Failed to lock file."},
    {JF_ERR_FAIL_UNLOCK_FILE, "Failed to unlock file."},
    {JF_ERR_FAIL_REMOVE_FILE, "Failed to remove file."},
    {JF_ERR_FAIL_CLOSE_FILE, "Failed to close file."},
    {JF_ERR_FAIL_IOCTL_FILE, "Failed to ioctl file."},
/* uuid error */
    {JF_ERR_INVALID_UUID_FORMAT, "Invalid uuid format."},
/* xml parser error */
    {JF_ERR_INCOMPLETE_XML, "Incomplete XML file."},
    {JF_ERR_ILLEGAL_CLOSE_TAG, "Illegal close tag."},
    {JF_ERR_UNMATCHED_CLOSE_TAG, "Unmatched close tag found."},
    {JF_ERR_CORRUPTED_XML_FILE, "Corrupted XML file."},
/* httpparser error */
    {JF_ERR_HTTP_STATUS_NOT_OK, "HTTP status is not OK."},
/* webclient error */

/* cghash error */
    {JF_ERR_SHA1_STATE_ERROR, "SHA1 state error."},
/* prng error */
    {JF_ERR_PRNG_NOT_SEEDED, "Prng is not seeded"},
/* jiukun error */
    {JF_ERR_JIUKUN_OUT_OF_MEMORY, "Out of user memory"},
/* ifmgmt error */

/* matrix error */
    {JF_ERR_MATRIX_SINGULAR, "Matrix is singular."},
/* jiuhua error */

/* cli error */
    {JF_ERR_LOGOUT_REQUIRED, "Command cannot be processed in an active session. Please logout first."},
    {JF_ERR_MORE_CANCELED, "More has been canceled by the user."},
    {JF_ERR_CLI_TOO_MANY_OPTIONS, "Too many options."},
    {JF_ERR_CLI_TERMINATED, "CLI is terminated."},

    {JF_ERR_INVALID_ACTION, "Invalid action."},
    {JF_ERR_ACTION_NOT_APPLY, "The specified action does not apply to the command."},
    {JF_ERR_INVALID_OPTION, "Invalid option for the command or the action."},
};

static u32 ls_u32NumberOfErrorCodes = sizeof(ls_iecdErrorCodeDesc) / \
    sizeof(internal_error_code_desc_t);

static internal_error_code_desc_t \
    ls_iecdVendorSpecErrorCodeDesc[JF_ERR_MAX_VENDOR_SPEC_ERROR];

/* --- private routine section---------------------------------------------- */
static void _getSysErrMsg(u32 u32Ret, olchar_t * pstrBuf, olsize_t sBuf)
{
    olchar_t strMsg[128 + 1];
#if defined(WINDOWS)
    DWORD dwErrorCode = 0, dwRet = 0;
#elif defined(LINUX)
    olint_t errno_save = 0;
#endif

#if defined(LINUX)
    errno_save = errno;
#elif defined(WINDOWS)
    dwErrorCode = GetLastError();
#endif

    memset(strMsg, 0, 128 + 1);
    pstrBuf[0] = '\0';
#if defined(LINUX)
    ol_snprintf(pstrBuf, sBuf - 1, SYS_ERR_MSG_FORMAT, u32Ret,
		jf_err_getDescription(u32Ret), errno_save, strerror(errno_save));
#elif defined(WINDOWS)
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, 
        strMsg, 128, NULL);
    ol_snprintf(pstrBuf, sBuf - 1, SYS_ERR_MSG_FORMAT, u32Ret,
		jf_err_getDescription(u32Ret), dwErrorCode, strMsg);
#endif
}

void _getErrMsg(u32 u32Ret, olchar_t * pstrBuf, olsize_t sBuf)
{
    memset(pstrBuf, 0, sBuf);

#if defined(LINUX)
    ol_snprintf(pstrBuf, sBuf - 1, ERR_MSG_FORMAT, u32Ret,
        jf_err_getDescription(u32Ret));
#elif defined(WINDOWS)
    ol_snprintf(pstrBuf, sBuf - 1, ERR_MSG_FORMAT, u32Ret,
        jf_err_getDescription(u32Ret));
#endif
}

/* --- public routine section ---------------------------------------------- */
olchar_t * jf_err_getDescription(u32 u32ErrCode)
{
    olchar_t * pstrDesc = ls_iecdErrorCodeDesc[1].iecd_pstrDesc;
    u32 u32Begin = 0, u32End = 0, u32Index = 0;
    u32 u32Code;

    if ((u32ErrCode >> JF_ERR_CODE_MODULE_SHIFT) == JF_ERR_VENDOR_SPEC_ERROR)
    {
        u32Code = u32ErrCode & JF_ERR_CODE_CODE_MASK;
        if (ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_u32ErrorCode != 0)
            return ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_pstrDesc;
        else
            return pstrDesc;
    }
    
    u32Begin = 0;
    u32End = ls_u32NumberOfErrorCodes - 1;
    
    /*binary search*/
    while (u32Begin <= u32End)
    {
        u32Index = (u32Begin + u32End) / 2;
        if (ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode == u32ErrCode)
        {
            pstrDesc = ls_iecdErrorCodeDesc[u32Index].iecd_pstrDesc;
            u32Begin = u32End + 1;
        }
        else
        {
            if (ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode < u32ErrCode)
            {
                u32Begin = u32Index + 1;
            }
            else
            {
                u32End = u32Index - 1;
            }
        }
    }

    return pstrDesc;
}

void jf_err_getMsg(u32 u32Err, olchar_t * pstrBuf, olsize_t sBuf)
{
    assert((pstrBuf != NULL) && (sBuf > 0));

    memset(pstrBuf, 0, sBuf);

    if (isSysErrorCode(u32Err))
        _getSysErrMsg(u32Err, pstrBuf, sBuf);
    else
        _getErrMsg(u32Err, pstrBuf, sBuf);
}

u32 jf_err_addCode(u32 u32Err, olchar_t * pstrDesc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Code;

    assert((u32Err >> JF_ERR_CODE_MODULE_SHIFT) == JF_ERR_VENDOR_SPEC_ERROR);
    assert((u32Err & JF_ERR_CODE_CODE_MASK) < JF_ERR_MAX_VENDOR_SPEC_ERROR);

    u32Code = u32Err & JF_ERR_CODE_CODE_MASK;
    ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_u32ErrorCode = u32Err;
    ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_pstrDesc = pstrDesc;

    return u32Ret;
} 

#if DEBUG
void checkErrCode(void)
{
    u32 u32Index;
    u32 u32Min = 0;

    ol_printf("0x%X %s\n", ls_iecdErrorCodeDesc[0].iecd_u32ErrorCode,
            ls_iecdErrorCodeDesc[0].iecd_pstrDesc);

    for (u32Index = 1; u32Index < ls_u32NumberOfErrorCodes; u32Index ++)
    {
        ol_printf("0x%X %s\n", ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode,
            ls_iecdErrorCodeDesc[u32Index].iecd_pstrDesc);

        if (ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode < u32Min)
        {
            ol_printf("Error detected\n");
            break;
        }
        else
            u32Min = ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode;

    }
}
#endif

/*---------------------------------------------------------------------------*/


