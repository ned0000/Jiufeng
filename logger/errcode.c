/**
 *  @file logger/errcode.c
 *
 *  @brief The error code implementation file.
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
#include "jf_err.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the error message format for normal error code.
 */
#define ERR_MSG_FORMAT      "(0x%X) %s"

/** Define the error message format for error code related to system call.
 */
#define SYS_ERR_MSG_FORMAT  "(0x%X) %s\n      %d, %s"

/** Define the error code description data type.
 */
typedef struct
{
    /**The error code.*/
    u32 iecd_u32ErrorCode;
    /**The error description.*/
    olchar_t * iecd_pstrDesc;
} internal_error_code_desc_t;

/** Define the error code description.
 *
 *  @note
 *  -# the data in ls_iecdErrorCodeDesc must be sorted in the ascending order of error code in
 *   order to support binary search.
 */
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
    {JF_ERR_PROGRAM_ERROR, "Program error."},
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
    {JF_ERR_BUFFER_IS_FULL, "Buffer is full."},
    {JF_ERR_NOT_INITIALIZED, "Not initialized."},
    {JF_ERR_INVALID_NAME, "Invalid name."},
    {JF_ERR_INVALID_CALLBACK_FUNCTION, "Invalid callback function."},
    {JF_ERR_INCOMPLETE_DATA, "Incomplete data."},
    {JF_ERR_TERMINATED, "Terminated."},
    {JF_ERR_INVALID_MESSAGE, "Invalid message."},
    {JF_ERR_REACH_MAX_TRANSACTION, "Maximum number of transaction is reached."},
    {JF_ERR_TRANSACTION_NOT_FOUND, "Transaction is not found."},
/* stack error */

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
    {JF_ERR_FAIL_GET_CWD, "Failed to get current working directory."},
    {JF_ERR_FAIL_SET_CWD, "Failed to set current working directory."},
    {JF_ERR_FAIL_TERMINATE_PROCESS, "Failed to terminate process."},
    {JF_ERR_FAIL_WAIT_PROCESS_TERMINATION, "Failed to wait process termination."},
    {JF_ERR_FAIL_INIT_SOCKET, "Failed to initialize socket."},
    {JF_ERR_FAIL_FINI_SOCKET, "Failed to initialize socket."},
/* thread error */
    {JF_ERR_FAIL_CREATE_THREAD, "Failed to create thread."},
    {JF_ERR_FAIL_STOP_THREAD, "Failed to stop thread."},
    {JF_ERR_FAIL_WAIT_THREAD_TERMINATION, "Failed to wait thread termination."},
    {JF_ERR_FAIL_TERMINATE_THREAD, "Failed to terminate thread."},
/* shared memory error */
    {JF_ERR_INVALID_SHAREDMEMORY_ID, "Invalid shared memory identifier."},
    {JF_ERR_FAIL_CREATE_SHAREDMEMORY, "Failed to create shared memory."},
    {JF_ERR_FAIL_SET_SHAREDMEMORY_ATTR, "Failed to set shared memory attribute."},
    {JF_ERR_FAIL_ATTACH_SHAREDMEMORY, "Failed to attach shared memory."},
    {JF_ERR_FAIL_DETACH_SHAREDMEMORY, "Failed to detach shared memory."},
    {JF_ERR_FAIL_DESTROY_SHAREDMEMORY, "Failed to detroy shared memory."},
/* time error */

/* date error */

/* sem error */
    {JF_ERR_FAIL_CREATE_SEM, "Failed to create semaphore."},
    {JF_ERR_FAIL_DESTROY_SEM, "Failed to destroy semaphore."},
    {JF_ERR_FAIL_DOWN_SEM, "Failed to down semaphore."},
    {JF_ERR_FAIL_UP_SEM, "Failed to up semaphore."},
/* dynlib error */
    {JF_ERR_FAIL_LOAD_DYNLIB, "Failed to load dynamic library."},
    {JF_ERR_FAIL_FREE_DYNLIB, "Failed to free dynamic library."},
    {JF_ERR_FAIL_GET_SYMBOL_ADDR, "Failed to get symbol's address."},
/* attask error */
    {JF_ERR_ATTASK_ITEM_NOT_FOUND, "Attask item is not found."},
/* hsm error */
    {JF_ERR_HSM_STATE_NOT_FOUND, "HSM state is not found"},
/* rwlock error */
    {JF_ERR_FAIL_CREATE_RWLOCK, "Failed to create read-write lock."},
    {JF_ERR_FAIL_DESTROY_RWLOCK, "Failed to destroy read-write lock."},
    {JF_ERR_FAIL_ACQUIRE_RWLOCK, "Failed to acquire read-write lock."},
    {JF_ERR_FAIL_RELEASE_RWLOCK, "Failed to release read-write lock."},
/* user error */
    {JF_ERR_FAIL_GET_USER_INFO, "Failed to get user information."},
/* ptree error */
    {JF_ERR_MAX_PTREE_NODE_FOUND, "Maximum property tree nodes have been found."},
    {JF_ERR_PTREE_NODE_NOT_FOUND, "Property tree node is not found."},
/* option error */
    {JF_ERR_INVALID_OPTION, "Invalid option for the command."},
    {JF_ERR_MISSING_OPTION_ARG, "Option argument is missing."},
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
    {JF_ERR_MORE_CANCELED, "More has been canceled by the user."},
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
    {JF_ERR_INCOMPLETE_XML_DOCUMENT, "Incomplete XML document."},
    {JF_ERR_ILLEGAL_XML_CLOSE_TAG, "Illegal XML close tag."},
    {JF_ERR_UNMATCHED_XML_CLOSE_TAG, "Unmatched XML close tag."},
    {JF_ERR_CORRUPTED_XML_DOCUMENT, "Corrupted XML document."},
    {JF_ERR_INVALID_XML_DECLARATION, "Invalid XML declaration."},
    {JF_ERR_INVALID_XML_FILE, "Invalid XML file."},
    {JF_ERR_NOT_UNIQUE_XML_ROOT_ELEMENT, "Not unique XML root element."},
/* httpparser error */
    {JF_ERR_HTTP_STATUS_NOT_OK, "HTTP status is not OK."},
    {JF_ERR_CORRUPTED_HTTP_MSG, "Corrupted HTTP message."},
    {JF_ERR_INVALID_HTTP_URI, "Invalid HTTP URI."},
    {JF_ERR_INVALID_HTTP_HEADER_LINE, "Invalid line in HTTP header."},
    {JF_ERR_INVALID_HTTP_HEADER_START_LINE, "Invalid start line in HTTP header."},
/* webclient error */

/* cghash error */
    {JF_ERR_SHA1_STATE_ERROR, "SHA1 state error."},
/* prng error */
    {JF_ERR_PRNG_NOT_SEEDED, "Prng is not seeded."},
/* jiukun error */
    {JF_ERR_JIUKUN_OUT_OF_MEMORY, "Out of user memory."},
    {JF_ERR_FAIL_REAP_JIUKUN, "Failed to reap jiukun memory."},
    {JF_ERR_JIUKUN_FREE_UNALLOCATED, "Free an unallocated jiukun memory."},
    {JF_ERR_JIUKUN_DOUBLE_FREE, "Double free jiukun memory is detected."},
    {JF_ERR_INVALID_JIUKUN_ADDRESS, "Invalid jiukun memory address."},
    {JF_ERR_JIUKUN_MEMORY_LEAK, "Jiukun memory leak is detected."},
    {JF_ERR_JIUKUN_MEMORY_CORRUPTED, "Jiukun memory is corrupted."},
    {JF_ERR_JIUKUN_MEMORY_OUT_OF_BOUND, "Jiukun memory access is out of bound."},
/* ifmgmt error */

/* matrix error */
    {JF_ERR_MATRIX_SINGULAR, "Matrix is singular."},
/* persistency error */
    {JF_ERR_SQL_EVAL_ERROR, "SQL statement evaluation error."},
    {JF_ERR_PERSISTENCY_INIT_ERROR, "Persistency initialization error."},
    {JF_ERR_SQL_COMPILE_ERROR, "SQL statement compile error."},
    {JF_ERR_UNSUPPORTED_PERSISTENCY_TYPE, "Unsupported persistency type."},
/* servmgmt error */
    {JF_ERR_SERV_NOT_FOUND, "Service is not found."},
/* dispatcher error */
    {JF_ERR_INVALID_DISPATCHER_SERVICE_CONFIG, "Invalid dispatcher service configuration."},
    {JF_ERR_DISPATCHER_UNAUTHORIZED_USER, "Unauthorized user for service in dispatcher."},
/* cli error */

};

/** Number of error code in the array.
 */
static u32 ls_u32NumberOfErrorCodes = \
    sizeof(ls_iecdErrorCodeDesc) / sizeof(internal_error_code_desc_t);

/** The vendor specific error code description array.
 */
static internal_error_code_desc_t ls_iecdVendorSpecErrorCodeDesc[JF_ERR_MAX_VENDOR_SPEC_ERROR];

/* --- private routine section ------------------------------------------------------------------ */

static void _getSysErrMsg(u32 u32Ret, olchar_t * pstrBuf, olsize_t sBuf)
{
#if defined(WINDOWS)
    olchar_t strMsg[128];
    DWORD dwErrorCode = 0;

    /*Get error code from OS.*/
    dwErrorCode = GetLastError();

    /*Get the error message based on the error code.*/
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, strMsg, sizeof(strMsg), NULL);
    strMsg[sizeof(strMsg) - 1] = '\0';

    /*Generate the message.*/
    ol_snprintf(
        pstrBuf, sBuf, SYS_ERR_MSG_FORMAT, u32Ret, jf_err_getDescription(u32Ret), dwErrorCode,
        strMsg);

#elif defined(LINUX)
    olint_t errno_save = 0;

    /*Save error code from OS.*/
    errno_save = errno;

    /*Generate the message.*/
    ol_snprintf(
        pstrBuf, sBuf, SYS_ERR_MSG_FORMAT, u32Ret, jf_err_getDescription(u32Ret), errno_save,
        strerror(errno_save));

#endif

    pstrBuf[sBuf - 1] = '\0';
}

static void _getErrMsg(u32 u32Ret, olchar_t * pstrBuf, olsize_t sBuf)
{
    /*Generate message for the error code.*/
    ol_snprintf(
        pstrBuf, sBuf, ERR_MSG_FORMAT, u32Ret, jf_err_getDescription(u32Ret));

    pstrBuf[sBuf - 1] = '\0';
}

/* --- public routine section ------------------------------------------------------------------- */

olchar_t * jf_err_getDescription(u32 u32ErrCode)
{
    olchar_t * pstrDesc = ls_iecdErrorCodeDesc[1].iecd_pstrDesc;
    u32 u32Begin = 0, u32End = 0, u32Index = 0;
    u32 u32Code = 0;

    /*For vendor specific error code.*/
    if ((u32ErrCode >> JF_ERR_CODE_MODULE_SHIFT) == JF_ERR_VENDOR_SPEC_ERROR)
    {
        u32Code = u32ErrCode & JF_ERR_CODE_CODE_MASK;
        if (ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_u32ErrorCode != 0)
            return ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_pstrDesc;
        else
            return pstrDesc;
    }
    
    /*Binary search for the error codes.*/
    u32Begin = 0;
    u32End = ls_u32NumberOfErrorCodes - 1;

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

void jf_err_readDescription(u32 u32ErrCode, olchar_t * pstrBuf, olsize_t sBuf)
{
    assert((pstrBuf != NULL) && (sBuf > 0));

    ol_bzero(pstrBuf, sBuf);

    /*Normal error code or related to system call.*/
    if (isSysErrorCode(u32ErrCode))
        _getSysErrMsg(u32ErrCode, pstrBuf, sBuf);
    else
        _getErrMsg(u32ErrCode, pstrBuf, sBuf);
}

u32 jf_err_addCode(u32 u32ErrCode, olchar_t * pstrDesc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Code;

    assert((u32ErrCode >> JF_ERR_CODE_MODULE_SHIFT) == JF_ERR_VENDOR_SPEC_ERROR);
    assert((u32ErrCode & JF_ERR_CODE_CODE_MASK) < JF_ERR_MAX_VENDOR_SPEC_ERROR);

    u32Code = u32ErrCode & JF_ERR_CODE_CODE_MASK;
    ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_u32ErrorCode = u32ErrCode;
    ls_iecdVendorSpecErrorCodeDesc[u32Code].iecd_pstrDesc = pstrDesc;

    return u32Ret;
} 

#if defined(DEBUG_LOGGER)
void jf_err_checkErrCode(void)
{
    u32 u32Index = 0;
    u32 u32Min = 0;

    ol_printf(
        "0x%X %s\n", ls_iecdErrorCodeDesc[0].iecd_u32ErrorCode,
        ls_iecdErrorCodeDesc[0].iecd_pstrDesc);

    for (u32Index = 1; u32Index < ls_u32NumberOfErrorCodes; u32Index ++)
    {
        ol_printf(
            "0x%X %s\n", ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode,
            ls_iecdErrorCodeDesc[u32Index].iecd_pstrDesc);

        /*Error codes should be sorted in the ascending order.*/
        if (ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode < u32Min)
        {
            ol_printf("Error detected\n");
            break;
        }
        else
        {
            /*Save the minimum error code.*/
            u32Min = ls_iecdErrorCodeDesc[u32Index].iecd_u32ErrorCode;
        }
    }
}
#endif

/*------------------------------------------------------------------------------------------------*/
