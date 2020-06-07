/**
 *  @file jf_err.h
 *
 *  @brief Error code header file which defines the interface of error codes and the related
 *   external routines.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_logger library.
 *  -# Vendor specific error code and description can be added to the library.
 *
 *  @par Error Code Format
 *  @code
 *       3                   2                   1
 *     1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      module   |S|   reserved  |     code 1    |   code 2      |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  @endcode
 */

#ifndef JIUFENG_ERR_H
#define JIUFENG_ERR_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_logger.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** System error code if this flag is set.
 */
#define JF_ERR_CODE_FLAG_SYSTEM       (0x800000)

#define JF_ERR_CODE_MODULE_MASK       (0xFF000000)
#define JF_ERR_CODE_MODULE_SHIFT      (24)
#define JF_ERR_CODE_CODE_MASK         (0xFFFF)

/* error code definition */
/** No error.
 */
#define JF_ERR_NO_ERROR            (0x0)

/* module definition */
#define JF_ERR_UNKNOWN_ERROR       (0x1)
#define JF_ERR_GENERIC_ERROR       (0x2)

/* jiutai error */
#define JF_ERR_JIUTAI_ERROR        (0x3)
#define JF_ERR_STACK_ERROR         (JF_ERR_JIUTAI_ERROR + 0x0)
#define JF_ERR_QUEUE_ERROR         (JF_ERR_JIUTAI_ERROR + 0x1)
#define JF_ERR_HASHTREE_ERROR      (JF_ERR_JIUTAI_ERROR + 0x2)
#define JF_ERR_LISTHEAD_ERROR      (JF_ERR_JIUTAI_ERROR + 0x3)
#define JF_ERR_HLISTHEAD_ERROR     (JF_ERR_JIUTAI_ERROR + 0x4)
#define JF_ERR_LINKLIST_ERROR      (JF_ERR_JIUTAI_ERROR + 0x5)
#define JF_ERR_DLINKLIST_ERROR     (JF_ERR_JIUTAI_ERROR + 0x6)
#define JF_ERR_LISTARRAY_ERROR     (JF_ERR_JIUTAI_ERROR + 0x7)
#define JF_ERR_MEM_ERROR           (JF_ERR_JIUTAI_ERROR + 0x8)
#define JF_ERR_ARRAY_ERROR         (JF_ERR_JIUTAI_ERROR + 0x9)
#define JF_ERR_MUTEX_ERROR         (JF_ERR_JIUTAI_ERROR + 0xA)
#define JF_ERR_RESPOOL_ERROR       (JF_ERR_JIUTAI_ERROR + 0xB)
#define JF_ERR_HASHTABLE_ERROR     (JF_ERR_JIUTAI_ERROR + 0xC)
#define JF_ERR_CONFFILE_ERROR      (JF_ERR_JIUTAI_ERROR + 0xD)
#define JF_ERR_HOST_ERROR          (JF_ERR_JIUTAI_ERROR + 0xE)
#define JF_ERR_MENU_ERROR          (JF_ERR_JIUTAI_ERROR + 0xF)
#define JF_ERR_PROCESS_ERROR       (JF_ERR_JIUTAI_ERROR + 0x10)
#define JF_ERR_THREAD_ERROR        (JF_ERR_JIUTAI_ERROR + 0x11)
#define JF_ERR_SHAREDMEMORY_ERROR  (JF_ERR_JIUTAI_ERROR + 0x12)
#define JF_ERR_TIME_ERROR          (JF_ERR_JIUTAI_ERROR + 0x13)
#define JF_ERR_DATE_ERROR          (JF_ERR_JIUTAI_ERROR + 0x14)
#define JF_ERR_SEM_ERROR           (JF_ERR_JIUTAI_ERROR + 0x15)
#define JF_ERR_DYNLIB_ERROR        (JF_ERR_JIUTAI_ERROR + 0x16)
#define JF_ERR_ATTASK_ERROR        (JF_ERR_JIUTAI_ERROR + 0x17)
#define JF_ERR_BITARRAY_ERROR      (JF_ERR_JIUTAI_ERROR + 0x18)
#define JF_ERR_HSM_ERROR           (JF_ERR_JIUTAI_ERROR + 0x19)
#define JF_ERR_RWLOCK_ERROR        (JF_ERR_JIUTAI_ERROR + 0x1A)
#define JF_ERR_USER_ERROR          (JF_ERR_JIUTAI_ERROR + 0x1B)
#define JF_ERR_PTREE_ERROR         (JF_ERR_JIUTAI_ERROR + 0x1C)

#define JF_ERR_STATIC_LIB_ERROR    (0x50)

#define JF_ERR_DYNAMIC_LIB_ERROR   (0x60)
#define JF_ERR_LOGGER_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x0)
#define JF_ERR_ARCHIVE_ERROR       (JF_ERR_DYNAMIC_LIB_ERROR + 0x1)
#define JF_ERR_NETWORK_ERROR       (JF_ERR_DYNAMIC_LIB_ERROR + 0x3)
#define JF_ERR_ENCRYPT_ERROR       (JF_ERR_DYNAMIC_LIB_ERROR + 0x4)
#define JF_ERR_ENCODE_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x5)
#define JF_ERR_CLIENG_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x7)
#define JF_ERR_STRINGPARSE_ERROR   (JF_ERR_DYNAMIC_LIB_ERROR + 0x9)
#define JF_ERR_FILES_ERROR         (JF_ERR_DYNAMIC_LIB_ERROR + 0xA)
#define JF_ERR_UUID_ERROR          (JF_ERR_DYNAMIC_LIB_ERROR + 0xB)
#define JF_ERR_XMLPARSER_ERROR     (JF_ERR_DYNAMIC_LIB_ERROR + 0xC)
#define JF_ERR_HTTPPARSER_ERROR    (JF_ERR_DYNAMIC_LIB_ERROR + 0xD)
#define JF_ERR_WEBCLIENT_ERROR     (JF_ERR_DYNAMIC_LIB_ERROR + 0xE)
#define JF_ERR_CGHASH_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x12)
#define JF_ERR_CGMAC_ERROR         (JF_ERR_DYNAMIC_LIB_ERROR + 0x13)
#define JF_ERR_PRNG_ERROR          (JF_ERR_DYNAMIC_LIB_ERROR + 0x14)
#define JF_ERR_JIUKUN_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x15)
#define JF_ERR_IFMGMT_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x16)
#define JF_ERR_MATRIX_ERROR        (JF_ERR_DYNAMIC_LIB_ERROR + 0x17)
#define JF_ERR_PERSISTENCY_ERROR   (JF_ERR_DYNAMIC_LIB_ERROR + 0x18)

#define JF_ERR_SERVICE_ERROR       0xA0
#define JF_ERR_SERVMGMT_ERROR      (JF_ERR_SERVICE_ERROR + 0x0)
#define JF_ERR_DISPATCHER_ERROR    (JF_ERR_SERVICE_ERROR + 0x1)
#define JF_ERR_CONFIGMGR_ERROR     (JF_ERR_SERVICE_ERROR + 0x2)

#define JF_ERR_APPLICATION_ERROR   0xC0
#define JF_ERR_CLI_ERROR           (JF_ERR_APPLICATION_ERROR + 0x0)

#define JF_ERR_VENDOR_SPEC_ERROR   0xE0

/* generic errors */
#define JF_ERR_GENERIC_ERROR_START (JF_ERR_GENERIC_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_NOT_FOUND (JF_ERR_GENERIC_ERROR_START + 0x0)
#define JF_ERR_TIMEOUT (JF_ERR_GENERIC_ERROR_START + 0x1)
#define JF_ERR_NULL_POINTER (JF_ERR_GENERIC_ERROR_START + 0x2)
#define JF_ERR_FILE_NOT_FOUND (JF_ERR_GENERIC_ERROR_START + 0x3)
#define JF_ERR_NOT_SUPPORTED (JF_ERR_GENERIC_ERROR_START + 0x4)
#define JF_ERR_OPERATION_FAIL (JF_ERR_GENERIC_ERROR_START + 0x5)
#define JF_ERR_PROGRAM_ERROR (JF_ERR_GENERIC_ERROR_START + 0x6)
#define JF_ERR_BUFFER_TOO_SMALL (JF_ERR_GENERIC_ERROR_START + 0x7)
#define JF_ERR_WRONG_PRODUCT_TYPE (JF_ERR_GENERIC_ERROR_START + 0x8)
#define JF_ERR_NOT_IMPLEMENTED (JF_ERR_GENERIC_ERROR_START + 0x9)
#define JF_ERR_OUT_OF_RANGE (JF_ERR_GENERIC_ERROR_START + 0xA)
#define JF_ERR_INVALID_VERSION (JF_ERR_GENERIC_ERROR_START + 0xB)
#define JF_ERR_PENDING (JF_ERR_GENERIC_ERROR_START + 0xC)
#define JF_ERR_INVALID_DATA (JF_ERR_GENERIC_ERROR_START + 0xD)
#define JF_ERR_BUSY (JF_ERR_GENERIC_ERROR_START + 0xE)
#define JF_ERR_INVALID_PARAM (JF_ERR_GENERIC_ERROR_START + 0xF)
#define JF_ERR_MISSING_PARAM (JF_ERR_GENERIC_ERROR_START + 0x10)
#define JF_ERR_ALREADY_EXIST (JF_ERR_GENERIC_ERROR_START + 0x11)
#define JF_ERR_INVALID_FILE (JF_ERR_GENERIC_ERROR_START + 0x12)
#define JF_ERR_INVALID_INPUT (JF_ERR_GENERIC_ERROR_START + 0x13)
#define JF_ERR_SERVICE_UNAVAILABLE (JF_ERR_GENERIC_ERROR_START + 0x14)
#define JF_ERR_SERVICE_BUSY (JF_ERR_GENERIC_ERROR_START + 0x15)
#define JF_ERR_NOT_READY (JF_ERR_GENERIC_ERROR_START + 0x16)
#define JF_ERR_NOT_MATCH (JF_ERR_GENERIC_ERROR_START + 0x17)
#define JF_ERR_BUFFER_IS_FULL (JF_ERR_GENERIC_ERROR_START + 0x18)
#define JF_ERR_NOT_INITIALIZED (JF_ERR_GENERIC_ERROR_START + 0x19)
#define JF_ERR_INVALID_NAME (JF_ERR_GENERIC_ERROR_START + 0x1A)
#define JF_ERR_INVALID_CALLBACK_FUNCTION (JF_ERR_GENERIC_ERROR_START + 0x1B)
#define JF_ERR_INCOMPLETE_DATA (JF_ERR_GENERIC_ERROR_START + 0x1C)
#define JF_ERR_TERMINATED (JF_ERR_GENERIC_ERROR_START + 0x1D)
#define JF_ERR_INVALID_MESSAGE (JF_ERR_GENERIC_ERROR_START + 0x1E)

/* stack error */
#define JF_ERR_STACK_ERROR_START (JF_ERR_STACK_ERROR << JF_ERR_CODE_MODULE_SHIFT)

/* queue error */
#define JF_ERR_QUEUE_ERROR_START (JF_ERR_QUEUE_ERROR << JF_ERR_CODE_MODULE_SHIFT)
#define JF_ERR_FAIL_CREATE_QUEUE (JF_ERR_QUEUE_ERROR_START + 0x0)

/* hashtree error */
#define JF_ERR_HASHTREE_ERROR_START (JF_ERR_HASHTREE_ERROR << JF_ERR_CODE_MODULE_SHIFT)
#define JF_ERR_END_OF_HASHTREE (JF_ERR_HASHTREE_ERROR_START + 0x0)
#define JF_ERR_HASHTREE_ENTRY_NOT_FOUND (JF_ERR_HASHTREE_ERROR_START + 0x1)

/* mem error */
#define JF_ERR_MEM_ERROR_START (JF_ERR_MEM_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_OUT_OF_MEMORY (JF_ERR_MEM_ERROR_START + 0x0)

/* array error */
#define JF_ERR_ARRAY_ERROR_START (JF_ERR_ARRAY_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define xxx (JF_ERR_ARRAY_ERROR_START + 0x0)

/* mutex error */
#define JF_ERR_MUTEX_ERROR_START (JF_ERR_MUTEX_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_CREATE_MUTEX (JF_ERR_MUTEX_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_DESTROY_MUTEX (JF_ERR_MUTEX_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_ACQUIRE_MUTEX (JF_ERR_MUTEX_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_RELEASE_MUTEX (JF_ERR_MUTEX_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)

/* respool error */
#define JF_ERR_RESPOOL_ERROR_START (JF_ERR_RESPOOL_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_RESOURCE_BUSY (JF_ERR_RESPOOL_ERROR_START + 0x0)
#define JF_ERR_REACH_MAX_RESOURCES (JF_ERR_RESPOOL_ERROR_START + 0x1)

/* hashtable error */
#define JF_ERR_HASHTABLE_ERROR_START (JF_ERR_HASHTABLE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_HASH_ENTRY_NOT_FOUND (JF_ERR_HASHTABLE_ERROR_START + 0x0)

/* conffile error */
#define JF_ERR_CONFFILE_ERROR_START (JF_ERR_CONFFILE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_CONFFILE_INVALID_FLAGS (JF_ERR_CONFFILE_ERROR_START + 0x0)
#define JF_ERR_CONFFILE_LINE_TOO_LONG (JF_ERR_CONFFILE_ERROR_START + 0x1)
#define JF_ERR_CONFFILE_UNKNOWN_ENTRY (JF_ERR_CONFFILE_ERROR_START + 0x2)
#define JF_ERR_CONFFILE_INVALID_ENTRY (JF_ERR_CONFFILE_ERROR_START + 0x3)
#define JF_ERR_CONFFILE_ILLEGAL_SYNTAX (JF_ERR_CONFFILE_ERROR_START + 0x4)
#define JF_ERR_CONFFILE_MISSING_SECTION_NAME (JF_ERR_CONFFILE_ERROR_START + 0x5)

/* host error */
#define JF_ERR_HOST_ERROR_START (JF_ERR_HOST_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define xxx (JF_ERR_HOST_ERROR_START + 0x0)

/* menu error */
#define JF_ERR_MENU_ERROR_START (JF_ERR_MENU_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define xxx (JF_ERR_MENU_ERROR_START + 0x0)

/* process error */
#define JF_ERR_PROCESS_ERROR_START (JF_ERR_PROCESS_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_PROCESS_CREATED (JF_ERR_PROCESS_ERROR_START + 0x0)
#define JF_ERR_ALREADY_RUNNING (JF_ERR_PROCESS_ERROR_START + 0x1)
#define JF_ERR_FAIL_CREATE_PROCESS (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_GET_CWD (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)
#define JF_ERR_FAIL_SET_CWD (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x4)
#define JF_ERR_FAIL_TERMINATE_PROCESS (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x5)
#define JF_ERR_FAIL_WAIT_PROCESS_TERMINATION (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x6)
#define JF_ERR_FAIL_INIT_SOCKET (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x7)
#define JF_ERR_FAIL_FINI_SOCKET (JF_ERR_PROCESS_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x8)


/* thread error */
#define JF_ERR_THREAD_ERROR_START (JF_ERR_THREAD_ERROR << JF_ERR_CODE_MODULE_SHIFT)
#define JF_ERR_FAIL_CREATE_THREAD (JF_ERR_THREAD_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_STOP_THREAD (JF_ERR_THREAD_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_WAIT_THREAD_TERMINATION (JF_ERR_THREAD_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_TERMINATE_THREAD (JF_ERR_THREAD_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)

/* sharedmemory */
#define JF_ERR_SHAREDMEMORY_ERROR_START (JF_ERR_SHAREDMEMORY_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INVALID_SHAREDMEMORY_ID (JF_ERR_SHAREDMEMORY_ERROR_START + 0x0)

#define JF_ERR_FAIL_CREATE_SHAREDMEMORY (JF_ERR_SHAREDMEMORY_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_SET_SHAREDMEMORY_ATTR (JF_ERR_SHAREDMEMORY_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_ATTACH_SHAREDMEMORY (JF_ERR_SHAREDMEMORY_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_DETACH_SHAREDMEMORY (JF_ERR_SHAREDMEMORY_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)
#define JF_ERR_FAIL_DESTROY_SHAREDMEMORY (JF_ERR_SHAREDMEMORY_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x4)

/* time error */
#define JF_ERR_TIME_ERROR_START (JF_ERR_TIME_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_GET_TIME (JF_ERR_TIME_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_GET_CLOCK_TIME (JF_ERR_TIME_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)

/* date error */
#define JF_ERR_DATE_ERROR_START (JF_ERR_DATE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

/* sem error */
#define JF_ERR_SEM_ERROR_START (JF_ERR_SEM_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_CREATE_SEM (JF_ERR_SEM_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_DESTROY_SEM (JF_ERR_SEM_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_ACQUIRE_SEM (JF_ERR_SEM_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_RELEASE_SEM (JF_ERR_SEM_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)

/* dynlib error */
#define JF_ERR_DYNLIB_ERROR_START (JF_ERR_DYNLIB_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_LOAD_DYNLIB (JF_ERR_DYNLIB_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_FREE_DYNLIB (JF_ERR_DYNLIB_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_GET_SYMBOL_ADDR (JF_ERR_DYNLIB_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)

/* attask error */
#define JF_ERR_ATTASK_ERROR_START (JF_ERR_ATTASK_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_ATTASK_ITEM_NOT_FOUND (JF_ERR_ATTASK_ERROR_START + 0x0)

/* bitarray error */
#define JF_ERR_BITARRAY_ERROR_START (JF_ERR_BITARRAY_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define xxx (JF_ERR_BITARRAY_ERROR_START + 0x0)

/* hsm error */
#define JF_ERR_HSM_ERROR_START (JF_ERR_HSM_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_HSM_STATE_NOT_FOUND (JF_ERR_HSM_ERROR_START + 0x0)

/* rwlock error */
#define JF_ERR_RWLOCK_ERROR_START (JF_ERR_RWLOCK_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_CREATE_RWLOCK (JF_ERR_RWLOCK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_DESTROY_RWLOCK (JF_ERR_RWLOCK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_ACQUIRE_RWLOCK (JF_ERR_RWLOCK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_RELEASE_RWLOCK (JF_ERR_RWLOCK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)

/* user error */
#define JF_ERR_USER_ERROR_START (JF_ERR_USER_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_GET_USER_INFO (JF_ERR_USER_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)

/* ptree error */
#define JF_ERR_PTREE_ERROR_START (JF_ERR_PTREE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_MAX_PTREE_NODE_FOUND (JF_ERR_PTREE_ERROR_START + 0x0)
#define JF_ERR_PTREE_NODE_NOT_FOUND (JF_ERR_PTREE_ERROR_START + 0x1)
#define JF_ERR_PTREE_NODE_ATTR_NOT_FOUND (JF_ERR_PTREE_ERROR_START + 0x2)

/* logger error */
#define JF_ERR_LOGGER_ERROR_START (JF_ERR_LOGGER_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_CALLER_NAME_TOO_LONG (JF_ERR_LOGGER_ERROR_START + 0x0)

/* archive error */
#define JF_ERR_ARCHIVE_ERROR_START (JF_ERR_ARCHIVE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_ARCHIVE_OPEN_FAIL (JF_ERR_ARCHIVE_ERROR_START + 0x0)
#define JF_ERR_INVALID_FILE_TYPE (JF_ERR_ARCHIVE_ERROR_START + 0x1)
#define JF_ERR_REACH_MAX_DIR_DEPTH (JF_ERR_ARCHIVE_ERROR_START + 0x2)
#define JF_ERR_ARCHIVE_CORRUPTED (JF_ERR_ARCHIVE_ERROR_START + 0x3)
#define JF_ERR_UNRECOGNIZED_FILE_TYPE (JF_ERR_ARCHIVE_ERROR_START + 0x4)

/* network error */
#define JF_ERR_NETWORK_ERROR_START (JF_ERR_NETWORK_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_INIT_NETWORK_LIB (JF_ERR_NETWORK_ERROR_START + 0x0)
#define JF_ERR_SOCKET_PEER_CLOSED (JF_ERR_NETWORK_ERROR_START + 0x1)
#define JF_ERR_SOCKET_ALREADY_CLOSED (JF_ERR_NETWORK_ERROR_START + 0x2)
#define JF_ERR_ASOCKET_IN_USE (JF_ERR_NETWORK_ERROR_START + 0x3)
#define JF_ERR_HOST_NOT_FOUND (JF_ERR_NETWORK_ERROR_START + 0x4)
#define JF_ERR_HOST_NO_ADDRESS (JF_ERR_NETWORK_ERROR_START + 0x5)
#define JF_ERR_NAME_SERVER_NO_RECOVERY (JF_ERR_NETWORK_ERROR_START + 0x6)
#define JF_ERR_RESOLVE_TRY_AGAIN (JF_ERR_NETWORK_ERROR_START + 0x7)
#define JF_ERR_CREATE_SECURE_SOCKET_ERROR (JF_ERR_NETWORK_ERROR_START + 0x8)
#define JF_ERR_SECURE_SOCKET_NEGOTIATE_ERROR (JF_ERR_NETWORK_ERROR_START + 0x9)
#define JF_ERR_SECURE_SOCKET_SHUTDOWN_ERROR (JF_ERR_NETWORK_ERROR_START + 0xA)
#define JF_ERR_SECURE_SOCKET_NOT_NEGOTIATED (JF_ERR_NETWORK_ERROR_START + 0xB)
#define JF_ERR_UTIMER_ITEM_NOT_FOUND (JF_ERR_NETWORK_ERROR_START + 0xC)
#define JF_ERR_SOCKET_CONNECTION_NOT_SETUP (JF_ERR_NETWORK_ERROR_START + 0xD)
#define JF_ERR_SOCKET_LOCAL_CLOSED (JF_ERR_NETWORK_ERROR_START + 0xE)
#define JF_ERR_SOCKET_POOL_EMPTY (JF_ERR_NETWORK_ERROR_START + 0xF)

#define JF_ERR_FAIL_CREATE_SOCKET (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_BIND_SOCKET (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_SEND_ICMP (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_RECEIVE_ICMP (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)
#define JF_ERR_FAIL_IOCTL_SOCKET (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x4)
#define JF_ERR_FAIL_CLOSE_SOCKET (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x5)
#define JF_ERR_FAIL_GET_ADAPTER_INFO (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x6)
#define JF_ERR_FAIL_SEND_DATA (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x7)
#define JF_ERR_FAIL_RECV_DATA (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x8)
#define JF_ERR_FAIL_INITIATE_CONNECTION (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x9)
#define JF_ERR_FAIL_ACCEPT_CONNECTION (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xA)
#define JF_ERR_FAIL_ENABLE_BROADCAST (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xB)
#define JF_ERR_FAIL_JOIN_MULTICAST_GROUP (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xC)
#define JF_ERR_FAIL_CREATE_SOCKET_PAIR (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xD)
#define JF_ERR_FAIL_LISTEN_ON_SOCKET (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xE)
#define JF_ERR_SELECT_ERROR (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0xF)
#define JF_ERR_FAIL_RESOLVE_HOST (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x10)
#define JF_ERR_FAIL_GET_SOCKET_NAME (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x11)
#define JF_ERR_FAIL_GET_SOCKET_OPT (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x12)
#define JF_ERR_FAIL_SET_SOCKET_OPT (JF_ERR_NETWORK_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x13)

/* encrypt error */
#define JF_ERR_ENCRYPT_ERROR_START (JF_ERR_ENCRYPT_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INVALID_ENCRYPT_KEY (JF_ERR_ENCRYPT_ERROR_START + 0x0)
#define JF_ERR_ENCRYPT_STRING_ERROR (JF_ERR_ENCRYPT_ERROR_START + 0x1)
#define JF_ERR_DECRYPT_STRING_ERROR (JF_ERR_ENCRYPT_ERROR_START + 0x2)
#define JF_ERR_INVALID_DECRYPT_KEY (JF_ERR_ENCRYPT_ERROR_START + 0x3)

/* encode error */
#define JF_ERR_ENCODE_ERROR_START (JF_ERR_ENCODE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_BUILD_HUFFMAN_TREE (JF_ERR_ENCODE_ERROR_START + 0x0)

/* clieng error */
#define JF_ERR_CLIENG_ERROR_START (JF_ERR_CLIENG_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_CMD_NAME_TOO_LONG (JF_ERR_CLIENG_ERROR_START + 0x0)
#define JF_ERR_CMD_ALREADY_EXIST (JF_ERR_CLIENG_ERROR_START + 0x1)
#define JF_ERR_CLIENG_PROMPT_TOO_LONG (JF_ERR_CLIENG_ERROR_START + 0x2)
#define JF_ERR_INVALID_ITEM_FLAG (JF_ERR_CLIENG_ERROR_START + 0x3)
#define JF_ERR_INVALID_CMD_GROUP (JF_ERR_CLIENG_ERROR_START + 0x4)
#define JF_ERR_INVALID_COMMAND (JF_ERR_CLIENG_ERROR_START + 0x5)
#define JF_ERR_CMD_TOO_LONG (JF_ERR_CLIENG_ERROR_START + 0x6)
#define JF_ERR_BLANK_CMD (JF_ERR_CLIENG_ERROR_START + 0x7)
#define JF_ERR_INVALID_COUNT (JF_ERR_CLIENG_ERROR_START + 0x8)
#define JF_ERR_COMMENT_CMD (JF_ERR_CLIENG_ERROR_START + 0x9)
#define JF_ERR_LINE_TOO_LONG (JF_ERR_CLIENG_ERROR_START + 0xA)

/* string error */
#define JF_ERR_STRINGPARSE_ERROR_START (JF_ERR_STRINGPARSE_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INVALID_STRING (JF_ERR_STRINGPARSE_ERROR_START + 0x0)
#define JF_ERR_INVALID_SIZE (JF_ERR_STRINGPARSE_ERROR_START + 0x1)
#define JF_ERR_INVALID_IP (JF_ERR_STRINGPARSE_ERROR_START + 0x2)
#define JF_ERR_INVALID_IP_ADDR_TYPE (JF_ERR_STRINGPARSE_ERROR_START + 0x3)
#define JF_ERR_NULL_IP_ADDRESS (JF_ERR_STRINGPARSE_ERROR_START + 0x4)
#define JF_ERR_INVALID_DATE (JF_ERR_STRINGPARSE_ERROR_START + 0x5)
#define JF_ERR_INVALID_TIME (JF_ERR_STRINGPARSE_ERROR_START + 0x6)
#define JF_ERR_INVALID_ALIAS (JF_ERR_STRINGPARSE_ERROR_START + 0x7)
#define JF_ERR_INVALID_INTEGER (JF_ERR_STRINGPARSE_ERROR_START + 0x8)
#define JF_ERR_INVALID_USER_NAME (JF_ERR_STRINGPARSE_ERROR_START + 0x9)
#define JF_ERR_INTEGER_OUT_OF_RANGE (JF_ERR_STRINGPARSE_ERROR_START + 0xA)
#define JF_ERR_SUBSTRING_NOT_FOUND (JF_ERR_STRINGPARSE_ERROR_START + 0xB)
#define JF_ERR_INVALID_FLOAT (JF_ERR_STRINGPARSE_ERROR_START + 0xC)

#define JF_ERR_INVALID_SETTING (JF_ERR_STRINGPARSE_ERROR_START + 0x300)
#define JF_ERR_SETTING_TOO_LONG (JF_ERR_STRINGPARSE_ERROR_START + 0x301)
#define JF_ERR_MISSING_QUOTE (JF_ERR_STRINGPARSE_ERROR_START + 0x302)
#define JF_ERR_SETTING_EMPTY (JF_ERR_STRINGPARSE_ERROR_START + 0x303)

/* files error */
#define JF_ERR_FILES_ERROR_START (JF_ERR_FILES_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_NOT_A_DIR (JF_ERR_FILES_ERROR_START + 0x0)
#define JF_ERR_DIR_ALREADY_EXIST (JF_ERR_FILES_ERROR_START + 0x1)

#define JF_ERR_END_OF_FILE (JF_ERR_FILES_ERROR_START + 0x50)
#define JF_ERR_FILE_ACCESS_VIOLOATION (JF_ERR_FILES_ERROR_START + 0x51)
#define JF_ERR_FILE_PATH_TOO_LONG (JF_ERR_FILES_ERROR_START + 0x52)

#define JF_ERR_DIR_ENTRY_NOT_FOUND (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x0)
#define JF_ERR_FAIL_CREATE_DIR (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x1)
#define JF_ERR_FAIL_REMOVE_DIR (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x2)
#define JF_ERR_FAIL_GET_ENTRY (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x3)
#define JF_ERR_FAIL_OPEN_DIR (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x4)

#define JF_ERR_FAIL_CREATE_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x50)
#define JF_ERR_FAIL_OPEN_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x51)
#define JF_ERR_FAIL_READ_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x52)
#define JF_ERR_FAIL_WRITE_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x53)
#define JF_ERR_FAIL_STAT_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x54)
#define JF_ERR_FAIL_SEEK_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x55)
#define JF_ERR_FAIL_FLUSH_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x56)
#define JF_ERR_FAIL_LOCK_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x57)
#define JF_ERR_FAIL_UNLOCK_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x58)
#define JF_ERR_FAIL_REMOVE_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x59)
#define JF_ERR_FAIL_CLOSE_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x60)
#define JF_ERR_FAIL_IOCTL_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x61)
#define JF_ERR_FAIL_RENAME_FILE (JF_ERR_FILES_ERROR_START + JF_ERR_CODE_FLAG_SYSTEM + 0x629)

/* uuid error */
#define JF_ERR_UUID_ERROR_START (JF_ERR_UUID_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INVALID_UUID_FORMAT (JF_ERR_UUID_ERROR_START + 0x0)

/* xmlparser error */
#define JF_ERR_XMLPARSER_ERROR_START (JF_ERR_XMLPARSER_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INCOMPLETE_XML_DOCUMENT (JF_ERR_XMLPARSER_ERROR_START + 0x0)
#define JF_ERR_ILLEGAL_XML_CLOSE_TAG (JF_ERR_XMLPARSER_ERROR_START + 0x1)
#define JF_ERR_UNMATCHED_XML_CLOSE_TAG (JF_ERR_XMLPARSER_ERROR_START + 0x2)
#define JF_ERR_CORRUPTED_XML_DOCUMENT (JF_ERR_XMLPARSER_ERROR_START + 0x3)
#define JF_ERR_INVALID_XML_DECLARATION (JF_ERR_XMLPARSER_ERROR_START + 0x4)
#define JF_ERR_XML_PTREE_NOT_BUILT (JF_ERR_XMLPARSER_ERROR_START + 0x5)
#define JF_ERR_INVALID_XML_FILE (JF_ERR_XMLPARSER_ERROR_START + 0x6)
#define JF_ERR_NOT_UNIQUE_XML_ROOT_ELEMENT (JF_ERR_XMLPARSER_ERROR_START + 0x7)
#define JF_ERR_XML_NODE_NOT_FOUND (JF_ERR_XMLPARSER_ERROR_START + 0x8)
#define JF_ERR_INVALID_XML_ATTRIBUTE (JF_ERR_XMLPARSER_ERROR_START + 0x9)

/* httpparser error */
#define JF_ERR_HTTPPARSER_ERROR_START (JF_ERR_HTTPPARSER_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_HTTP_STATUS_NOT_OK (JF_ERR_HTTPPARSER_ERROR_START + 0x0)
#define JF_ERR_CORRUPTED_HTTP_MSG (JF_ERR_HTTPPARSER_ERROR_START + 0x1)
#define JF_ERR_INVALID_HTTP_URI (JF_ERR_HTTPPARSER_ERROR_START + 0x2)
#define JF_ERR_INVALID_HTTP_HEADER_LINE (JF_ERR_HTTPPARSER_ERROR_START + 0x3)
#define JF_ERR_INVALID_HTTP_HEADER_START_LINE (JF_ERR_HTTPPARSER_ERROR_START + 0x4)
#define JF_ERR_HTTP_HEADER_NOT_FOUND (JF_ERR_HTTPPARSER_ERROR_START + 0x5)
#define JF_ERR_CORRUPTED_HTTP_CHUNK_DATA (JF_ERR_HTTPPARSER_ERROR_START + 0x6)

/* webclient error */
#define JF_ERR_WEBCLIENT_ERROR_START (JF_ERR_WEBCLIENT_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define xxx (JF_ERR_WEBCLIENT_ERROR_START + 0x0)

/* cghash error */
#define JF_ERR_CGHASH_ERROR_START (JF_ERR_CGHASH_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_SHA1_STATE_ERROR (JF_ERR_CGHASH_ERROR_START + 0x0)

/* cgmac error */
#define JF_ERR_CGMAC_ERROR_START (JF_ERR_CGMAC_ERROR << JF_ERR_CODE_MODULE_SHIFT)

//#define JF_ERR_ (JF_ERR_CGMAC_ERROR_START + 0x0)

/* prng error */
#define JF_ERR_PRNG_ERROR_START (JF_ERR_PRNG_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_PRNG_NOT_SEEDED (JF_ERR_PRNG_ERROR_START + 0x0)
#define JF_ERR_FAIL_GET_RANDOM_DATA (JF_ERR_PRNG_ERROR_START + 0x1)

/* jiukun error*/
#define JF_ERR_JIUKUN_ERROR_START (JF_ERR_JIUKUN_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_FAIL_DESTROY_JIUKUN_CACHE (JF_ERR_JIUKUN_ERROR_START + 0x0)
#define JF_ERR_FAIL_CREATE_JIUKUN_CACHE (JF_ERR_JIUKUN_ERROR_START + 0x1)
#define JF_ERR_JIUKUN_CACHE_GROW_NOT_ALLOW (JF_ERR_JIUKUN_ERROR_START + 0x2)
#define JF_ERR_FAIL_GROW_JIUKUN_CACHE (JF_ERR_JIUKUN_ERROR_START + 0x3)
#define JF_ERR_UNSUPPORTED_MEMORY_SIZE (JF_ERR_JIUKUN_ERROR_START + 0x4)
#define JF_ERR_FAIL_ALLOC_JIUKUN_PAGE (JF_ERR_JIUKUN_ERROR_START + 0x5)
#define JF_ERR_INVALID_JIUKUN_PAGE_ORDER (JF_ERR_JIUKUN_ERROR_START + 0x6)
#define JF_ERR_JIUKUN_OUT_OF_MEMORY (JF_ERR_JIUKUN_ERROR_START + 0x7)
#define JF_ERR_FAIL_REAP_JIUKUN (JF_ERR_JIUKUN_ERROR_START + 0x8)
#define JF_ERR_JIUKUN_FREE_UNALLOCATED (JF_ERR_JIUKUN_ERROR_START + 0x9)
#define JF_ERR_JIUKUN_DOUBLE_FREE (JF_ERR_JIUKUN_ERROR_START + 0xA)
#define JF_ERR_JIUKUN_BAD_POINTER (JF_ERR_JIUKUN_ERROR_START + 0xB)
#define JF_ERR_JIUKUN_MEMORY_LEAK (JF_ERR_JIUKUN_ERROR_START + 0xC)
#define JF_ERR_JIUKUN_MEMORY_CORRUPTED (JF_ERR_JIUKUN_ERROR_START + 0xD)
#define JF_ERR_JIUKUN_MEMORY_OUT_OF_BOUND (JF_ERR_JIUKUN_ERROR_START + 0xE)

/* ifmgmt error */
#define JF_ERR_IFMGMT_ERROR_START (JF_ERR_IFMGMT_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_ETHERNET_ADAPTER_NOT_FOUND (JF_ERR_IFMGMT_ERROR_START + 0x0)
#define JF_ERR_INVALID_MAC_ADDR (JF_ERR_IFMGMT_ERROR_START + 0x1)

/* matrix error */
#define JF_ERR_MATRIX_ERROR_START (JF_ERR_MATRIX_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_MATRIX_SINGULAR (JF_ERR_MATRIX_ERROR_START + 0x0)

/* persistency error */
#define JF_ERR_PERSISTENCY_ERROR_START (JF_ERR_PERSISTENCY_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_SQL_EVAL_ERROR (JF_ERR_PERSISTENCY_ERROR_START + 0x0)
#define JF_ERR_PERSISTENCY_INIT_ERROR (JF_ERR_PERSISTENCY_ERROR_START + 0x1)
#define JF_ERR_SQL_COMPILE_ERROR (JF_ERR_PERSISTENCY_ERROR_START + 0x2)

/* servmgmt error */
#define JF_ERR_SERVMGMT_ERROR_START (JF_ERR_SERVMGMT_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_SERV_NOT_FOUND (JF_ERR_SERVMGMT_ERROR_START + 0x0)

/* dispatcher error */
#define JF_ERR_DISPATCHER_ERROR_START (JF_ERR_DISPATCHER_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_INVALID_DISPATCHER_SERVICE_CONFIG (JF_ERR_DISPATCHER_ERROR_START + 0x0)
#define JF_ERR_DISPATCHER_UNAUTHORIZED_USER (JF_ERR_DISPATCHER_ERROR_START + 0x1)
#define JF_ERR_MSG_NOT_IN_PUBLISHED_LIST (JF_ERR_DISPATCHER_ERROR_START + 0x2)
#define JF_ERR_MSG_QUEUE_FULL (JF_ERR_DISPATCHER_ERROR_START + 0x3)
#define JF_ERR_MSG_PARTIAL_SENT (JF_ERR_DISPATCHER_ERROR_START + 0x4)

/* configmgr error */
#define JF_ERR_CONFIGMGR_ERROR_START (JF_ERR_CONFIGMGR_ERROR << JF_ERR_CODE_MODULE_SHIFT)
#define JF_ERR_INVALID_CONFIG (JF_ERR_CONFIGMGR_ERROR_START + 0x0)

/* cli error */
#define JF_ERR_CLI_ERROR_START (JF_ERR_CLI_ERROR << JF_ERR_CODE_MODULE_SHIFT)

#define JF_ERR_LOGOUT_REQUIRED (JF_ERR_CLI_ERROR_START + 0x0)

#define JF_ERR_MORE_CANCELED (JF_ERR_CLI_ERROR_START + 0x2)
#define JF_ERR_CLI_TOO_MANY_OPTIONS (JF_ERR_CLI_ERROR_START + 0x3)
#define JF_ERR_CLI_TERMINATED (JF_ERR_CLI_ERROR_START + 0x4)

#define JF_ERR_INVALID_ACTION (JF_ERR_CLI_ERROR_START + 0x10)
#define JF_ERR_ACTION_NOT_APPLY (JF_ERR_CLI_ERROR_START + 0x11)
#define JF_ERR_INVALID_OPTION (JF_ERR_CLI_ERROR_START + 0x12)

/** Vendor specific error.
 */
#define JF_ERR_VENDOR_SPEC_ERROR_START (JF_ERR_VENDOR_SPEC_ERROR << JF_ERR_CODE_MODULE_SHIFT)

/** Maximum vendor specific error code, tune this value if more error codes are required.
 */
#define JF_ERR_MAX_VENDOR_SPEC_ERROR    (200)

/* --- functional routines ---------------------------------------------------------------------- */

/** Get the description of the specified error code.
 *
 *  @param u32ErrCode [in] The specified error code.
 *
 *  @return The error message.
 */
LOGGERAPI olchar_t * LOGGERCALL jf_err_getDescription(u32 u32ErrCode);

/** Get the description of the specified error code, the description is returned in argument.
 *
 *  @param u32Err [in] The specified error code.
 *  @param pstrBuf [in] The buffer for the description.
 *  @param sBuf [in] The buffer size.
 *
 *  @return Void.
 */
LOGGERAPI void LOGGERCALL jf_err_getMsg(u32 u32Err, olchar_t * pstrBuf, olsize_t sBuf);

/** Add description for the vendor specific error code.
 *
 *  @param u32Err [in] The error code to be added.
 *  @param pstrDesc [in] The description for the error code.
 *
 *  @return The error code.
 */
LOGGERAPI u32 LOGGERCALL jf_err_addCode(u32 u32Err, olchar_t * pstrDesc);


#endif /*JIUFENG_ERR_H*/

/*------------------------------------------------------------------------------------------------*/




