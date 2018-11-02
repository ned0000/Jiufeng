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
#include "errcode.h"
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
    {OLERR_NO_ERROR, "No error."},
    {OLERR_UNKNOWN_ERROR, "Unknown error."},
/* generic errors */
    {OLERR_NOT_FOUND, "Not found."},
    {OLERR_TIMEOUT, "Operation has timed out."},
    {OLERR_NULL_POINTER, "Null pointer."},
    {OLERR_FILE_NOT_FOUND, "File is not found."},
    {OLERR_NOT_SUPPORTED, "Not supported."},
    {OLERR_OPERATION_FAIL, "Operation failed due to some system error."},
    {OLERR_PROGRAM_ERROR, "Program error. Please contact the technical support."},
    {OLERR_BUFFER_TOO_SMALL, "Buffer is too small."},
    {OLERR_WRONG_PRODUCT_TYPE, "Operation is not supported by this product type."},
    {OLERR_NOT_IMPLEMENTED, "Not implemented."},
    {OLERR_OUT_OF_RANGE, "Out of range."},
    {OLERR_INVALID_VERSION, "Invalid version."},
    {OLERR_PENDING, "Pending."},
    {OLERR_INVALID_DATA, "Invalid data."},
    {OLERR_BUSY, "Busy."},
    {OLERR_INVALID_PARAM, "Invalid parameter."},
    {OLERR_MISSING_PARAM, "Missing parameter."},
    {OLERR_ALREADY_EXIST, "Already exist."},
    {OLERR_INVALID_FILE, "Invalid file."},
    {OLERR_INVALID_INPUT, "Invalid input."},
    {OLERR_SERVICE_UNAVAILABLE, "Service is unavailable."},
    {OLERR_SERVICE_BUSY, "Service is busy."},
    {OLERR_NOT_READY, "Not ready."},
    {OLERR_NOT_MATCH, "Not match."},
/* bases error */
    {OLERR_FAIL_CREATE_QUEUE, "Failed to creat queue."},
/* xmalloc error */
    {OLERR_OUT_OF_MEMORY, "Out of memory."},
/* array error */

/* synobj error */
    {OLERR_FAIL_CREATE_MUTEX, "Failed to create mutex."},
    {OLERR_FAIL_DESTROY_MUTEX, "Failed to destroy mutex."},
    {OLERR_FAIL_ACQUIRE_MUTEX, "Failed to acquire mutex."},
    {OLERR_FAIL_RELEASE_MUTEX, "Failed to release mutex."},
/* respool error */
    {OLERR_RESOURCE_BUSY, "The requested resource is busy."},
/* hash error */

/* conffile error */

/* hostinfo error */

/* menu error */

/* process error */
    {OLERR_PROCESS_CREATED, "Process has been created. The program continues in background."},
    {OLERR_ALREADY_RUNNING, "The program is already running."},
    {OLERR_FAIL_CREATE_PROCESS, "Failed to create process."},
    {OLERR_FAIL_CREATE_THREAD, "Failed to create thread."},
/* cksum error */

/* shared memory error */
    {OLERR_INVALID_SHAREDMEMORY_ID, "Invalid shared memory identifier."},
    {OLERR_FAIL_CREATE_SHAREDMEMORY, "Failed to create shared memory."},
    {OLERR_FAIL_ATTACH_SHAREDMEMORY, "Failed to attach shared memory."},
    {OLERR_FAIL_DETACH_SHAREDMEMORY, "Failed to detach shared memory."},
    {OLERR_FAIL_DESTROY_SHAREDMEMORY, "Failed to detroy shared memory."},
/* xtime error */

/* resouce error */

/* syncsem error */
    {OLERR_FAIL_CREATE_SEM, "Failed to create semaphore."},
    {OLERR_FAIL_DESTROY_SEM, "Failed to destroy semaphore."},
    {OLERR_FAIL_ACQUIRE_SEM, "Failed to acquire semaphore."},
    {OLERR_FAIL_RELEASE_SEM, "Failed to release semaphore."},
/* dynlib error */
    {OLERR_FAIL_LOAD_DYNLIB, "Failed to load dynamic library."},
    {OLERR_FAIL_FREE_DYNLIB, "Failed to free dynamic library."},
    {OLERR_FAIL_GET_SYMBOL_ADDR, "Failed to get symbol's address."},
/* logger error */
    {OLERR_CALLER_NAME_TOO_LONG, "Length of the caller name is too long."},
/* archive error */
    {OLERR_ARCHIVE_CORRUPTED, "Archive file is corrupted."},
/* smtp error */
    {OLERR_FAIL_SEND_EMAIL, "Failed to sent email."},
    {OLERR_EMAIL_SUBJECT_TOO_LONG, "Maximum email subject length is reached."},
    {OLERR_SMTP_FATAL_ERROR, "Server return a fatal error"},
    {OLERR_INVALID_SMTP_RECIPIENTS, "No valid recipient was given."},
    {OLERR_UNSUPPORTED_SMTP_AUTH_METHOD, "Unsupported SMTP authentication method."},
/* network error */
    {OLERR_FAIL_INIT_NETWORK_LIB, "Failed to initialize network library."},
    {OLERR_SOCKET_PEER_CLOSED, "Connection is closed by peer."},
    {OLERR_SOCKET_ALREADY_CLOSED, "Connection is already closed."},
    {OLERR_HOST_NOT_FOUND, "The specified host is unknown."},
    {OLERR_HOST_NO_ADDRESS, "The requested host name is valid but does not have an IP address."},
    {OLERR_NAME_SERVER_NO_RECOVERY, "A non-recoverable name server error occurred."},
    {OLERR_RESOLVE_TRY_AGAIN, "A temporary error occurred on an authoritative name server. Try again later."},

    {OLERR_FAIL_SEND_DATA, "Failed to send data."},
    {OLERR_FAIL_RECV_DATA, "Failed to receive data."},
    {OLERR_FAIL_INITIATE_CONNECTION, "Failed to initiate connection."},
    {OLERR_FAIL_ACCEPT_CONNECTION, "Failed to accept connection."},
/* encrypt error */

/* encode error */

/* netsend error */

/* clieng error */
    {OLERR_INVALID_CMD_GROUP, "Invalid command or request group."},
    {OLERR_INVALID_COMMAND, "Invalid command or request."},
    {OLERR_CMD_TOO_LONG, "Command line is too long."},
    {OLERR_BLANK_CMD, "Command line is blank" },
    {OLERR_INVALID_COUNT, "Invalid count."},
    {OLERR_COMMENT_CMD, "Command line is for comments."},
    {OLERR_LINE_TOO_LONG, "Output line is too long."},
/* event */
    {OLERR_EVENT_NOT_FOUND, "Event not found."},
    {OLERR_INVALID_EVENT_LOCATION, "Invalid event location."},
/* stringparse error */
    {OLERR_INVALID_STRING, "Invalid string."},
    {OLERR_INVALID_SIZE, "Invalid size."},
    {OLERR_INVALID_IP, "Invalid IP address."},
    {OLERR_INVALID_IP_ADDR_TYPE, "Invalid IP address type."},
    {OLERR_NULL_IP_ADDRESS, "IP address or subnet mask cannot be all zero."},
    {OLERR_INVALID_DATE, "Invalid setting for date."},
    {OLERR_INVALID_TIME, "Invalid setting for time."},
    {OLERR_INVALID_ALIAS, "Invalid character in alias."},
    {OLERR_INVALID_INTEGER, "Invalid input for integer parameter(s)."},
    {OLERR_SUBSTRING_NOT_FOUND, "Substring is not found."},
    {OLERR_INVALID_FLOAT, "Invalid input for float parameter(s)."},

    {OLERR_INVALID_SETTING, "Invalid setting parameters."},
    {OLERR_SETTING_TOO_LONG, "Setting parameters too long."},
    {OLERR_MISSING_QUOTE, "Missing quote in parameters."},
    {OLERR_SETTING_EMPTY, "Empty value is not allowed for the setting option."},
/* files error */
    {OLERR_NOT_A_DIR, "Not a directory."},
    {OLERR_DIR_ALREADY_EXIST, "Directory already exists."},

    {OLERR_END_OF_FILE, "End of file."},
    {OLERR_FILE_ACCESS_VIOLOATION, "Invalid file access permission."},
    {OLERR_FILE_PATH_TOO_LONG, "The file path is too long."},

    {OLERR_DIR_ENTRY_NOT_FOUND, "Cannot find the directory entry."},
    {OLERR_FAIL_CREATE_DIR, "Failed to create directory."},
    {OLERR_FAIL_REMOVE_DIR, "Failed to remove directory."},
    {OLERR_FAIL_OPEN_DIR, "Failed to open directory."},

    {OLERR_FAIL_CREATE_FILE, "Failed to create file."},
    {OLERR_FAIL_OPEN_FILE, "Failed to open file."},
    {OLERR_FAIL_READ_FILE, "Failed to read file."},
    {OLERR_FAIL_WRITE_FILE, "Failed to write file."},
    {OLERR_FAIL_STAT_FILE, "Failed to get file status."},
    {OLERR_FAIL_SEEK_FILE, "Failed to seek file."},
    {OLERR_FAIL_FLUSH_FILE, "Failed to flush file."},
    {OLERR_FAIL_LOCK_FILE, "Failed to lock file."},
    {OLERR_FAIL_UNLOCK_FILE, "Failed to unlock file."},
    {OLERR_FAIL_REMOVE_FILE, "Failed to remove file."},
    {OLERR_FAIL_CLOSE_FILE, "Failed to close file."},
    {OLERR_FAIL_IOCTL_FILE, "Failed to ioctl file."},
/* uuid error */
    {OLERR_INVALID_UUID_FORMAT, "Invalid uuid format."},
/* xml parser error */
    {OLERR_INCOMPLETE_XML, "Incomplete XML file."},
    {OLERR_ILLEGAL_CLOSE_TAG, "Illegal close tag."},
    {OLERR_UNMATCHED_CLOSE_TAG, "Unmatched close tag found."},
    {OLERR_CORRUPTED_XML_FILE, "Corrupted XML file."},
/* http parser error */
    {OLERR_HTTP_STATUS_NOT_OK, "HTTP status is not OK."},
/* web client error */

/* web server error */

/* slp error */
    {OLERR_INVALID_SLP_MSG, "Invalid SLP message."},
/* cghash error */
    {OLERR_SHA1_STATE_ERROR, "SHA1 state error."},
/* prng error */
    {OLERR_PRNG_NOT_SEEDED, "Prng is not seeded"},
/* aether error */
    {OLERR_AETHER_OUT_OF_MEMORY, "Out of user memory"},
/* ifmgmt error */

/* matrix error */
    {OLERR_MATRIX_SINGULAR, "Matrix is singular."},
/* jiuhua error */

/* cli error */
    {OLERR_LOGOUT_REQUIRED, "Command cannot be processed in an active session. Please logout first."},
    {OLERR_MORE_CANCELED, "More has been canceled by the user."},
    {OLERR_CLI_TOO_MANY_OPTIONS, "Too many options."},
    {OLERR_CLI_TERMINATED, "CLI is terminated."},

    {OLERR_INVALID_ACTION, "Invalid action."},
    {OLERR_ACTION_NOT_APPLY, "The specified action does not apply to the command."},
    {OLERR_INVALID_OPTION, "Invalid option for the command or the action."},
};

static u32 ls_u32NumberOfErrorCodes = sizeof(ls_iecdErrorCodeDesc) / \
    sizeof(internal_error_code_desc_t);

static internal_error_code_desc_t \
    ls_iecdVendorSpecErrorCodeDesc[MAX_VENDOR_SPEC_ERROR];

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
		getErrorDescription(u32Ret), errno_save, strerror(errno_save));
#elif defined(WINDOWS)
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, 
        strMsg, 128, NULL);
    ol_snprintf(pstrBuf, sBuf - 1, SYS_ERR_MSG_FORMAT, u32Ret,
		getErrorDescription(u32Ret), dwErrorCode, strMsg);
#endif
}

void _getErrMsg(u32 u32Ret, olchar_t * pstrBuf, olsize_t sBuf)
{
    memset(pstrBuf, 0, sBuf);

#if defined(LINUX)
    ol_snprintf(pstrBuf, sBuf - 1, ERR_MSG_FORMAT, u32Ret,
        getErrorDescription(u32Ret));
#elif defined(WINDOWS)
    ol_snprintf(pstrBuf, sBuf - 1, ERR_MSG_FORMAT, u32Ret,
        getErrorDescription(u32Ret));
#endif
}

/* --- public routine section ---------------------------------------------- */
char * getErrorDescription(u32 u32ErrCode)
{
    olchar_t * pstrDesc = ls_iecdErrorCodeDesc[1].iecd_pstrDesc;
    u32 u32Begin = 0, u32End = 0, u32Index = 0;
    u32 u32Code;

    if ((u32ErrCode >> ERRCODE_MODULE_SHIFT) == OLERR_VENDOR_SPEC_ERROR)
    {
        u32Code = u32ErrCode & ERRCODE_CODE_MASK;
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

void getErrMsg(u32 u32Err, olchar_t * pstrBuf, olsize_t sBuf)
{
    assert((pstrBuf != NULL) && (sBuf > 0));

    memset(pstrBuf, 0, sBuf);

    if (isSysErrorCode(u32Err))
        _getSysErrMsg(u32Err, pstrBuf, sBuf);
    else
        _getErrMsg(u32Err, pstrBuf, sBuf);
}

u32 addErrCode(u32 u32Err, olchar_t * pstrDesc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Code;

    assert((u32Err >> ERRCODE_MODULE_SHIFT) == OLERR_VENDOR_SPEC_ERROR);
    assert((u32Err & ERRCODE_CODE_MASK) < MAX_VENDOR_SPEC_ERROR);

    u32Code = u32Err & ERRCODE_CODE_MASK;
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


