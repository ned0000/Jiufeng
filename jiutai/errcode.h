/**
 *  @file
 *
 *  @brief Error code header file. It provides the definitions of error codes
 *   and the related external routines.
 *
 *  @author Min Zhang
 *
 *  @note the error code is in following format
 *  @note     3                   2                   1                 
 *  @note   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  @note  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  @note  |      module   |S|   reserved  |     code 1    |   code 2      |
 *  @note  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  
 */

#ifndef JIUFENG_ERRCODE_H
#define JIUFENG_ERRCODE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "logger.h"

/* --- constant definitions ------------------------------------------------ */

/* Sys error code */
#define ERRCODE_FLAG_SYSTEM       (0x800000)

#define ERRCODE_MODULE_MASK       (0xFF000000)
#define ERRCODE_MODULE_SHIFT      (24)
#define ERRCODE_CODE_MASK         (0xFFFF)

/* error code definition */
/* no error */
#define OLERR_NO_ERROR            (0x0)

/* module definition */
#define OLERR_UNKNOWN_ERROR       (0x1)
#define OLERR_GENERIC_ERROR       (0x2)

/* jiutai error */
#define OLERR_JIUTAI_ERROR        (0x3)
#define OLERR_BASES_ERROR         (OLERR_JIUTAI_ERROR + 0x0)
#define OLERR_XMALLOC_ERROR       (OLERR_JIUTAI_ERROR + 0x1)
#define OLERR_ARRAY_ERROR         (OLERR_JIUTAI_ERROR + 0x2)
#define OLERR_SYNCMUTEX_ERROR     (OLERR_JIUTAI_ERROR + 0x3)
#define OLERR_RESPOOL_ERROR       (OLERR_JIUTAI_ERROR + 0x4)
#define OLERR_HASH_ERROR          (OLERR_JIUTAI_ERROR + 0x5)
#define OLERR_CONFFILE_ERROR      (OLERR_JIUTAI_ERROR + 0x6)
#define OLERR_HOSTINFO_ERROR      (OLERR_JIUTAI_ERROR + 0x7)
#define OLERR_MENU_ERROR          (OLERR_JIUTAI_ERROR + 0x8)
#define OLERR_PROCESS_ERROR       (OLERR_JIUTAI_ERROR + 0x9)
#define OLERR_CKSUM_ERROR         (OLERR_JIUTAI_ERROR + 0xA)
#define OLERR_SHAREDMEMORY_ERROR  (OLERR_JIUTAI_ERROR + 0xB)
#define OLERR_XTIME_ERROR         (OLERR_JIUTAI_ERROR + 0xC)
#define OLERR_RESOURCE_ERROR      (OLERR_JIUTAI_ERROR + 0xD)
#define OLERR_SYNCSEM_ERROR       (OLERR_JIUTAI_ERROR + 0xE)
#define OLERR_DYNLIB_ERROR        (OLERR_JIUTAI_ERROR + 0xF)
#define OLERR_OBSTACK_ERROR       (OLERR_JIUTAI_ERROR + 0x10)
#define OLERR_ATTASK_ERROR        (OLERR_JIUTAI_ERROR + 0x11)
#define OLERR_BITARRAY_ERROR      (OLERR_JIUTAI_ERROR + 0x12)
#define OLERR_RADIXTREE_ERROR     (OLERR_JIUTAI_ERROR + 0x13)
#define OLERR_SYNCRWLOCK_ERROR    (OLERR_JIUTAI_ERROR + 0x14)
#define OLERR_COMMINIT_ERROR      (OLERR_JIUTAI_ERROR + 0x15)

#define OLERR_STATIC_LIB_ERROR    (0x60)

#define OLERR_DYNAMIC_LIB_ERROR   (0x80)
#define OLERR_LOGGER_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x0)
#define OLERR_ARCHIVE_ERROR       (OLERR_DYNAMIC_LIB_ERROR + 0x1)
#define OLERR_SMTP_ERROR          (OLERR_DYNAMIC_LIB_ERROR + 0x2)
#define OLERR_NETWORK_ERROR       (OLERR_DYNAMIC_LIB_ERROR + 0x3)
#define OLERR_ENCRYPT_ERROR       (OLERR_DYNAMIC_LIB_ERROR + 0x4)
#define OLERR_ENCODE_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x5)
#define OLERR_NETSEND_ERROR       (OLERR_DYNAMIC_LIB_ERROR + 0x6)
#define OLERR_CLIENG_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x7)
#define OLERR_EVENT_ERROR         (OLERR_DYNAMIC_LIB_ERROR + 0x8)
#define OLERR_STRINGPARSE_ERROR   (OLERR_DYNAMIC_LIB_ERROR + 0x9)
#define OLERR_FILES_ERROR         (OLERR_DYNAMIC_LIB_ERROR + 0xA)
#define OLERR_UUID_ERROR          (OLERR_DYNAMIC_LIB_ERROR + 0xB)
#define OLERR_XMLPARSER_ERROR     (OLERR_DYNAMIC_LIB_ERROR + 0xC)
#define OLERR_HTTPPARSER_ERROR    (OLERR_DYNAMIC_LIB_ERROR + 0xD)
#define OLERR_WEBCLIENT_ERROR     (OLERR_DYNAMIC_LIB_ERROR + 0xE)
#define OLERR_WEBSERVER_ERROR     (OLERR_DYNAMIC_LIB_ERROR + 0xF)
#define OLERR_SLP_ERROR           (OLERR_DYNAMIC_LIB_ERROR + 0x10)
#define OLERR_BIGNUM_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x11)
#define OLERR_CGHASH_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x12)
#define OLERR_CGMAC_ERROR         (OLERR_DYNAMIC_LIB_ERROR + 0x13)
#define OLERR_PRNG_ERROR          (OLERR_DYNAMIC_LIB_ERROR + 0x14)
#define OLERR_JIUKUN_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x15)
#define OLERR_IFMGMT_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x16)
#define OLERR_SERVMGMT_ERROR      (OLERR_DYNAMIC_LIB_ERROR + 0x17)
#define OLERR_MATRIX_ERROR        (OLERR_DYNAMIC_LIB_ERROR + 0x18)
#define OLERR_PERSISTENCY_ERROR   (OLERR_DYNAMIC_LIB_ERROR + 0x19)

#define OLERR_SERVICE_ERROR       0xC0
#define OLERR_JIUHUA_ERROR        (OLERR_SERVICE_ERROR + 0x0)

#define OLERR_APPLICATION_ERROR   0xD0
#define OLERR_CLI_ERROR           (OLERR_APPLICATION_ERROR + 0x0)

#define OLERR_VENDOR_SPEC_ERROR   0xE0

/* generic errors */
#define OLERR_GENERIC_ERROR_START (OLERR_GENERIC_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_NOT_FOUND (OLERR_GENERIC_ERROR_START + 0x0)
#define OLERR_TIMEOUT (OLERR_GENERIC_ERROR_START + 0x1)
#define OLERR_NULL_POINTER (OLERR_GENERIC_ERROR_START + 0x2)
#define OLERR_FILE_NOT_FOUND (OLERR_GENERIC_ERROR_START + 0x3)
#define OLERR_NOT_SUPPORTED (OLERR_GENERIC_ERROR_START + 0x4)
#define OLERR_OPERATION_FAIL (OLERR_GENERIC_ERROR_START + 0x5)
#define OLERR_PROGRAM_ERROR (OLERR_GENERIC_ERROR_START + 0x6)
#define OLERR_BUFFER_TOO_SMALL (OLERR_GENERIC_ERROR_START + 0x7)
#define OLERR_WRONG_PRODUCT_TYPE (OLERR_GENERIC_ERROR_START + 0x8)
#define OLERR_NOT_IMPLEMENTED (OLERR_GENERIC_ERROR_START + 0x9)
#define OLERR_OUT_OF_RANGE (OLERR_GENERIC_ERROR_START + 0xA)
#define OLERR_INVALID_VERSION (OLERR_GENERIC_ERROR_START + 0xB)
#define OLERR_PENDING (OLERR_GENERIC_ERROR_START + 0xC)
#define OLERR_INVALID_DATA (OLERR_GENERIC_ERROR_START + 0xD)
#define OLERR_BUSY (OLERR_GENERIC_ERROR_START + 0xE)
#define OLERR_INVALID_PARAM (OLERR_GENERIC_ERROR_START + 0xF)
#define OLERR_MISSING_PARAM (OLERR_GENERIC_ERROR_START + 0x10)
#define OLERR_ALREADY_EXIST (OLERR_GENERIC_ERROR_START + 0x11)
#define OLERR_INVALID_FILE (OLERR_GENERIC_ERROR_START + 0x12)
#define OLERR_INVALID_INPUT (OLERR_GENERIC_ERROR_START + 0x13)
#define OLERR_SERVICE_UNAVAILABLE (OLERR_GENERIC_ERROR_START + 0x14)
#define OLERR_SERVICE_BUSY (OLERR_GENERIC_ERROR_START + 0x15)
#define OLERR_NOT_READY (OLERR_GENERIC_ERROR_START + 0x16)
#define OLERR_NOT_MATCH (OLERR_GENERIC_ERROR_START + 0x17)
#define OLERR_BUFFER_IS_FULL (OLERR_GENERIC_ERROR_START + 0x18)

/* bases error */
#define OLERR_BASES_ERROR_START (OLERR_BASES_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_CREATE_QUEUE (OLERR_BASES_ERROR_START + 0x0)
#define OLERR_END_OF_HASHTREE (OLERR_BASES_ERROR_START + 0x100)
#define OLERR_HASHTREE_ENTRY_NOT_FOUND (OLERR_BASES_ERROR_START + 0x101)

/* xmalloc error */
#define OLERR_XMALLOC_ERROR_START (OLERR_XMALLOC_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_OUT_OF_MEMORY (OLERR_XMALLOC_ERROR_START + 0x0)

/* array error */
#define OLERR_ARRAY_ERROR_START (OLERR_ARRAY_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_ARRAY_ERROR_START + 0x0)

/* synmutex error */
#define OLERR_SYNCMUTEX_ERROR_START (OLERR_SYNCMUTEX_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_CREATE_MUTEX (OLERR_SYNCMUTEX_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_DESTROY_MUTEX (OLERR_SYNCMUTEX_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_ACQUIRE_MUTEX (OLERR_SYNCMUTEX_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_RELEASE_MUTEX (OLERR_SYNCMUTEX_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)

/* respool error */
#define OLERR_RESPOOL_ERROR_START (OLERR_RESPOOL_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_RESOURCE_BUSY (OLERR_RESPOOL_ERROR_START + 0x0)
#define OLERR_REACH_MAX_RESOURCES (OLERR_RESPOOL_ERROR_START + 0x1)

/* hash error */
#define OLERR_HASH_ERROR_START (OLERR_HASH_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_HASH_ENTRY_NOT_FOUND (OLERR_HASH_ERROR_START + 0x0)

/* conffile error */
#define OLERR_CONFFILE_ERROR_START (OLERR_CONFFILE_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_CONFFILE_INVALID_FLAGS (OLERR_CONFFILE_ERROR_START + 0x0)
#define OLERR_CONFFILE_LINE_TOO_LONG (OLERR_CONFFILE_ERROR_START + 0x1)
#define OLERR_CONFFILE_UNKNOWN_ENTRY (OLERR_CONFFILE_ERROR_START + 0x2)
#define OLERR_CONFFILE_INVALID_ENTRY (OLERR_CONFFILE_ERROR_START + 0x3)
#define OLERR_CONFFILE_ILLEGAL_SYNTAX (OLERR_CONFFILE_ERROR_START + 0x4)
#define OLERR_CONFFILE_MISSING_SECTION_NAME (OLERR_CONFFILE_ERROR_START + 0x5)

/* hostinfo error */
#define OLERR_HOSTINFO_ERROR_START (OLERR_HOSTINFO_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_HOSTINFO_ERROR_START + 0x0)

/* menu error */
#define OLERR_MENU_ERROR_START (OLERR_MENU_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_MENU_ERROR_START + 0x0)

/* process error */
#define OLERR_PROCESS_ERROR_START (OLERR_PROCESS_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_PROCESS_CREATED (OLERR_PROCESS_ERROR_START + 0x0)
#define OLERR_ALREADY_RUNNING (OLERR_PROCESS_ERROR_START + 0x1)

#define OLERR_FAIL_CREATE_PROCESS (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_CREATE_THREAD (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_STOP_THREAD (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_WAIT_THREAD_TERMINATION (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)
#define OLERR_FAIL_TERMINATE_THREAD (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x4)
#define OLERR_FAIL_GET_CWD (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x5)
#define OLERR_FAIL_SET_CWD (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x6)
#define OLERR_FAIL_TERMINATE_PROCESS (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x7)
#define OLERR_FAIL_WAIT_PROCESS_TERMINATION (OLERR_PROCESS_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x8)

/* cksum error */
#define OLERR_CKSUM_ERROR_START (OLERR_CKSUM_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_CKSUM_ERROR_START + 0x0)

/* shared memory */
#define OLERR_SHAREDMEMORY_ERROR_START (OLERR_SHAREDMEMORY_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INVALID_SHAREDMEMORY_ID (OLERR_SHAREDMEMORY_ERROR_START + 0x0)

#define OLERR_FAIL_CREATE_SHAREDMEMORY (OLERR_SHAREDMEMORY_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_SET_SHAREDMEMORY_ATTR (OLERR_SHAREDMEMORY_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_ATTACH_SHAREDMEMORY (OLERR_SHAREDMEMORY_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_DETACH_SHAREDMEMORY (OLERR_SHAREDMEMORY_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)
#define OLERR_FAIL_DESTROY_SHAREDMEMORY (OLERR_SHAREDMEMORY_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x4)

/* xtime error */
#define OLERR_XTIME_ERROR_START (OLERR_XTIME_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_GET_TIME (OLERR_XTIME_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)

/* resource error */
#define OLERR_RESOURCE_ERROR_START (OLERR_RESOURCE_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_RESOURCE_ERROR_START + 0x0)

/* syncsem error */
#define OLERR_SYNCSEM_ERROR_START (OLERR_SYNCSEM_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_CREATE_SEM (OLERR_SYNCSEM_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_DESTROY_SEM (OLERR_SYNCSEM_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_ACQUIRE_SEM (OLERR_SYNCSEM_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_RELEASE_SEM (OLERR_SYNCSEM_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)

/* dynlib error */
#define OLERR_DYNLIB_ERROR_START (OLERR_DYNLIB_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_LOAD_DYNLIB (OLERR_DYNLIB_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_FREE_DYNLIB (OLERR_DYNLIB_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_GET_SYMBOL_ADDR (OLERR_DYNLIB_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)

/* obstack error */
#define OLERR_OBSTACK_ERROR_START (OLERR_OBSTACK_ERROR << ERRCODE_MODULE_SHIFT)

//#define OLERR_OBSTACK_ERROR (OLERR_OBSTACK_ERROR_START + 0x0)

/* attask error */
#define OLERR_ATTASK_ERROR_START (OLERR_ATTASK_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_ATTASK_ITEM_NOT_FOUND (OLERR_ATTASK_ERROR_START + 0x0)

/* bitarray error */
#define OLERR_BITARRAY_ERROR_START (OLERR_BITARRAY_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_BITARRAY_ERROR_START + 0x0)

/* radixtree error */
#define OLERR_RADIXTREE_ERROR_START (OLERR_RADIXTREE_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_RADIXTREE_ENTRY_EXIST (OLERR_RADIXTREE_ERROR_START + 0x0)

/* syncrwlock lock */
#define OLERR_SYNCRWLOCK_ERROR_START (OLERR_SYNCRWLOCK_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_CREATE_RWLOCK (OLERR_SYNCRWLOCK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_DESTROY_RWLOCK (OLERR_SYNCRWLOCK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_ACQUIRE_RWLOCK (OLERR_SYNCRWLOCK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_RELEASE_RWLOCK (OLERR_SYNCRWLOCK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)

/* comminit error */
#define OLERR_COMMINIT_ERROR_START (OLERR_COMMINIT_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_INIT_NET_LIB (OLERR_COMMINIT_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_FINI_NET_LIB (OLERR_COMMINIT_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)

/* logger error */
#define OLERR_LOGGER_ERROR_START (OLERR_LOGGER_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_CALLER_NAME_TOO_LONG (OLERR_LOGGER_ERROR_START + 0x0)

/* archive error */
#define OLERR_ARCHIVE_ERROR_START (OLERR_ARCHIVE_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_ARCHIVE_OPEN_FAIL (OLERR_ARCHIVE_ERROR_START + 0x0)
#define OLERR_INVALID_FILE_TYPE (OLERR_ARCHIVE_ERROR_START + 0x1)
#define OLERR_REACH_MAX_DIR_DEPTH (OLERR_ARCHIVE_ERROR_START + 0x2)
#define OLERR_ARCHIVE_CORRUPTED (OLERR_ARCHIVE_ERROR_START + 0x3)
#define OLERR_UNRECOGNIZED_FILE_TYPE (OLERR_ARCHIVE_ERROR_START + 0x4)

/* smtp error */
#define OLERR_SMTP_ERROR_START (OLERR_SMTP_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_SEND_EMAIL (OLERR_SMTP_ERROR_START + 0x0)
#define OLERR_EMAIL_SUBJECT_TOO_LONG (OLERR_SMTP_ERROR_START + 0x1)
#define OLERR_SMTP_NOT_WELCOME (OLERR_SMTP_ERROR_START + 0x2)
#define OLERR_INVALID_SMTP_MSG (OLERR_SMTP_ERROR_START + 0x3)
#define OLERR_SMTP_DENY_SENDER (OLERR_SMTP_ERROR_START + 0x4)
#define OLERR_SMTP_DENY_RECIPIENT (OLERR_SMTP_ERROR_START + 0x5)
#define OLERR_SMTP_FATAL_ERROR (OLERR_SMTP_ERROR_START + 0x6)
#define OLERR_INVALID_SMTP_RECIPIENTS (OLERR_SMTP_ERROR_START + 0x7)
#define OLERR_UNSUPPORTED_SMTP_AUTH_METHOD (OLERR_SMTP_ERROR_START + 0x8)
#define OLERR_INVALID_SMTP_AUTH_CHALLENGE (OLERR_SMTP_ERROR_START + 0x9)

/* network error */
#define OLERR_NETWORK_ERROR_START (OLERR_NETWORK_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_INIT_NETWORK_LIB (OLERR_NETWORK_ERROR_START + 0x0)
#define OLERR_SOCKET_PEER_CLOSED (OLERR_NETWORK_ERROR_START + 0x1)
#define OLERR_SOCKET_ALREADY_CLOSED (OLERR_NETWORK_ERROR_START + 0x2)
#define OLERR_ASOCKET_IN_USE (OLERR_NETWORK_ERROR_START + 0x3)
#define OLERR_HOST_NOT_FOUND (OLERR_NETWORK_ERROR_START + 0x4)
#define OLERR_HOST_NO_ADDRESS (OLERR_NETWORK_ERROR_START + 0x5)
#define OLERR_NAME_SERVER_NO_RECOVERY (OLERR_NETWORK_ERROR_START + 0x6)
#define OLERR_RESOLVE_TRY_AGAIN (OLERR_NETWORK_ERROR_START + 0x7)
#define OLERR_CREATE_SECURE_SOCKET_ERROR (OLERR_NETWORK_ERROR_START + 0x8)
#define OLERR_SECURE_SOCKET_NEGOTIATE_ERROR (OLERR_NETWORK_ERROR_START + 0x9)
#define OLERR_SECURE_SOCKET_SHUTDOWN_ERROR (OLERR_NETWORK_ERROR_START + 0xA)
#define OLERR_SECURE_SOCKET_NOT_NEGOTIATED (OLERR_NETWORK_ERROR_START + 0xB)
#define OLERR_UTIMER_ITEM_NOT_FOUND (OLERR_NETWORK_ERROR_START + 0xC)

#define OLERR_FAIL_CREATE_SOCKET (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_BIND_SOCKET (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_SEND_ICMP (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_RECEIVE_ICMP (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)
#define OLERR_FAIL_IOCTL_SOCKET (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x4)
#define OLERR_FAIL_CLOSE_SOCKET (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x5)
#define OLERR_FAIL_GET_ADAPTER_INFO (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x6)
#define OLERR_FAIL_SEND_DATA (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x7)
#define OLERR_FAIL_RECV_DATA (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x8)
#define OLERR_FAIL_INITIATE_CONNECTION (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x9)
#define OLERR_FAIL_ACCEPT_CONNECTION (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xA)
#define OLERR_FAIL_ENABLE_BROADCAST (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xB)
#define OLERR_FAIL_JOIN_MULTICAST_GROUP (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xC)
#define OLERR_FAIL_CREATE_SOCKET_PAIR (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xD)
#define OLERR_FAIL_LISTEN_ON_SOCKET (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xE)
#define OLERR_SELECT_ERROR (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0xF)
#define OLERR_FAIL_RESOLVE_HOST (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x10)
#define OLERR_FAIL_GET_SOCKET_NAME (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x11)
#define OLERR_FAIL_GET_SOCKET_OPT (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x12)
#define OLERR_FAIL_SET_SOCKET_OPT (OLERR_NETWORK_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x13)

/* encrypt error */
#define OLERR_ENCRYPT_ERROR_START (OLERR_ENCRYPT_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INVALID_ENCRYPT_KEY (OLERR_ENCRYPT_ERROR_START + 0x0)
#define OLERR_ENCRYPT_STRING_ERROR (OLERR_ENCRYPT_ERROR_START + 0x1)
#define OLERR_DECRYPT_STRING_ERROR (OLERR_ENCRYPT_ERROR_START + 0x2)
#define OLERR_INVALID_DECRYPT_KEY (OLERR_ENCRYPT_ERROR_START + 0x3)

/* encode error */
#define OLERR_ENCODE_ERROR_START (OLERR_ENCODE_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_BUILD_HUFFMAN_TREE (OLERR_ENCODE_ERROR_START + 0x0)

/* netsend error */
#define OLERR_NETSEND_ERROR_START (OLERR_NETSEND_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_SEND_NETSEND_MSG (OLERR_NETSEND_ERROR_START + 0x0)
#define OLERR_FAIL_QUERY_NETBIOS_NAME (OLERR_NETSEND_ERROR_START + 0x1)

/* clieng error */
#define OLERR_CLIENG_ERROR_START (OLERR_CLIENG_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_CMD_NAME_TOO_LONG (OLERR_CLIENG_ERROR_START + 0x0)
#define OLERR_CMD_ALREADY_EXIST (OLERR_CLIENG_ERROR_START + 0x1)
#define OLERR_CLIENG_PROMPT_TOO_LONG (OLERR_CLIENG_ERROR_START + 0x2)
#define OLERR_INVALID_ITEM_FLAG (OLERR_CLIENG_ERROR_START + 0x3)
#define OLERR_INVALID_CMD_GROUP (OLERR_CLIENG_ERROR_START + 0x4)
#define OLERR_INVALID_COMMAND (OLERR_CLIENG_ERROR_START + 0x5)
#define OLERR_CMD_TOO_LONG (OLERR_CLIENG_ERROR_START + 0x6)
#define OLERR_BLANK_CMD (OLERR_CLIENG_ERROR_START + 0x7)
#define OLERR_INVALID_COUNT (OLERR_CLIENG_ERROR_START + 0x8)
#define OLERR_COMMENT_CMD (OLERR_CLIENG_ERROR_START + 0x9)
#define OLERR_LINE_TOO_LONG (OLERR_CLIENG_ERROR_START + 0xA)

/* event */
#define OLERR_EVENT_ERROR_START (OLERR_EVENT_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_EVENT_NOT_FOUND (OLERR_EVENT_ERROR_START + 0)
#define OLERR_INVALID_EVENT_LOCATION (OLERR_EVENT_ERROR_START + 0x1)
#define OLERR_EVENT_ID_OUTOFRANGE (OLERR_EVENT_ERROR_START + 0x2)
#define OLERR_EVENT_EMPTY_STRING (OLERR_EVENT_ERROR_START + 0x3)
#define OLERR_EVENT_STRLENGTH_TOOLONG (OLERR_EVENT_ERROR_START + 0x4)
#define OLERR_EVENT_DUPLICATED (OLERR_EVENT_ERROR_START + 0x5)
#define OLERR_EVENT_TOTAL_EXCEED_MAX (OLERR_EVENT_ERROR_START + 0x6)
#define OLERR_EVENT_NEW_EVENT (OLERR_EVENT_ERROR_START + 0x7)
#define OLERR_EVENT_MISSING_EVENT (OLERR_EVENT_ERROR_START + 0x8)
#define OLERR_EVENT_DUPLICATE_SEVERITY (OLERR_EVENT_ERROR_START + 0x9)
#define OLERR_EVENT_DUPLICATE_CLASS (OLERR_EVENT_ERROR_START + 0xA)
#define OLERR_EVENT_DUPLICATE_LOGLOCATION (OLERR_EVENT_ERROR_START + 0xB)
#define OLERR_EVENT_LINE_BREAK (OLERR_EVENT_ERROR_START + 0xC)

/* string parse error */
#define OLERR_STRINGPARSE_ERROR_START (OLERR_STRINGPARSE_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INVALID_STRING (OLERR_STRINGPARSE_ERROR_START + 0x0)
#define OLERR_INVALID_SIZE (OLERR_STRINGPARSE_ERROR_START + 0x1)
#define OLERR_INVALID_IP (OLERR_STRINGPARSE_ERROR_START + 0x2)
#define OLERR_INVALID_IP_ADDR_TYPE (OLERR_STRINGPARSE_ERROR_START + 0x3)
#define OLERR_NULL_IP_ADDRESS (OLERR_STRINGPARSE_ERROR_START + 0x4)
#define OLERR_INVALID_DATE (OLERR_STRINGPARSE_ERROR_START + 0x5)
#define OLERR_INVALID_TIME (OLERR_STRINGPARSE_ERROR_START + 0x6)
#define OLERR_INVALID_ALIAS (OLERR_STRINGPARSE_ERROR_START + 0x7)
#define OLERR_INVALID_INTEGER (OLERR_STRINGPARSE_ERROR_START + 0x8)
#define OLERR_INVALID_USER_NAME (OLERR_STRINGPARSE_ERROR_START + 0x9)
#define OLERR_INTEGER_OUT_OF_RANGE (OLERR_STRINGPARSE_ERROR_START + 0xA)
#define OLERR_SUBSTRING_NOT_FOUND (OLERR_STRINGPARSE_ERROR_START + 0xB)
#define OLERR_INVALID_FLOAT (OLERR_STRINGPARSE_ERROR_START + 0xC)

#define OLERR_INVALID_SETTING (OLERR_STRINGPARSE_ERROR_START + 0x300)
#define OLERR_SETTING_TOO_LONG (OLERR_STRINGPARSE_ERROR_START + 0x301)
#define OLERR_MISSING_QUOTE (OLERR_STRINGPARSE_ERROR_START + 0x302)
#define OLERR_SETTING_EMPTY (OLERR_STRINGPARSE_ERROR_START + 0x303)

/* files error */
#define OLERR_FILES_ERROR_START (OLERR_FILES_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_NOT_A_DIR (OLERR_FILES_ERROR_START + 0x0)
#define OLERR_DIR_ALREADY_EXIST (OLERR_FILES_ERROR_START + 0x1)

#define OLERR_END_OF_FILE (OLERR_FILES_ERROR_START + 0x50)
#define OLERR_FILE_ACCESS_VIOLOATION (OLERR_FILES_ERROR_START + 0x51)
#define OLERR_FILE_PATH_TOO_LONG (OLERR_FILES_ERROR_START + 0x52)

#define OLERR_DIR_ENTRY_NOT_FOUND (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x0)
#define OLERR_FAIL_CREATE_DIR (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x1)
#define OLERR_FAIL_REMOVE_DIR (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x2)
#define OLERR_FAIL_GET_ENTRY (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x3)
#define OLERR_FAIL_OPEN_DIR (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x4)

#define OLERR_FAIL_CREATE_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x50)
#define OLERR_FAIL_OPEN_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x51)
#define OLERR_FAIL_READ_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x52)
#define OLERR_FAIL_WRITE_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x53)
#define OLERR_FAIL_STAT_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x54)
#define OLERR_FAIL_SEEK_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x55)
#define OLERR_FAIL_FLUSH_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x56)
#define OLERR_FAIL_LOCK_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x57)
#define OLERR_FAIL_UNLOCK_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x58)
#define OLERR_FAIL_REMOVE_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x59)
#define OLERR_FAIL_CLOSE_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x60)
#define OLERR_FAIL_IOCTL_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x61)
#define OLERR_FAIL_RENAME_FILE (OLERR_FILES_ERROR_START + ERRCODE_FLAG_SYSTEM + 0x629)

/* uuid error */
#define OLERR_UUID_ERROR_START (OLERR_UUID_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INVALID_UUID_FORMAT (OLERR_UUID_ERROR_START + 0x0)

/* xml parser error */
#define OLERR_XMLPARSER_ERROR_START (OLERR_XMLPARSER_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INCOMPLETE_XML (OLERR_XMLPARSER_ERROR_START + 0x0)
#define OLERR_ILLEGAL_CLOSE_TAG (OLERR_XMLPARSER_ERROR_START + 0x1)
#define OLERR_UNMATCHED_CLOSE_TAG (OLERR_XMLPARSER_ERROR_START + 0x2)
#define OLERR_CORRUPTED_XML_FILE (OLERR_XMLPARSER_ERROR_START + 0x3)
#define OLERR_INVALID_XML_DECLARATION (OLERR_XMLPARSER_ERROR_START + 0x4)

/* http parser error */
#define OLERR_HTTPPARSER_ERROR_START (OLERR_HTTPPARSER_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_HTTP_STATUS_NOT_OK (OLERR_HTTPPARSER_ERROR_START + 0x0)
#define OLERR_CORRUPTED_HTTP_MSG (OLERR_HTTPPARSER_ERROR_START + 0x1)
#define OLERR_INVALID_HTTP_URI (OLERR_HTTPPARSER_ERROR_START + 0x2)

/* webclient error */
#define OLERR_WEBCLIENT_ERROR_START (OLERR_WEBCLIENT_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_WEBCLIENT_ERROR_START + 0x0)

/* webserver error */
#define OLERR_WEBSERVER_ERROR_START (OLERR_WEBSERVER_ERROR << ERRCODE_MODULE_SHIFT)

//#define xxx (OLERR_WEBSERVER_ERROR_START + 0x0)

/* slp error */
#define OLERR_SLP_ERROR_START (OLERR_SLP_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_INVALID_SLP_MSG (OLERR_SLP_ERROR_START + 0x0)
#define OLERR_SLP_SERVICE_NOT_FOUND (OLERR_SLP_ERROR_START + 0x1)
#define OLERR_FAIL_SEND_SLP_MSG (OLERR_SLP_ERROR_START + 0x2)
#define OLERR_SLP_INVALID_SERV (OLERR_SLP_ERROR_START + 0x3)
#define OLERR_SLP_REACH_MAX_SERV (OLERR_SLP_ERROR_START + 0x3)

/* bignum error */
#define OLERR_BIGNUM_ERROR_START (OLERR_BIGNUM_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_CREATE_RAND_BIGNUM (OLERR_BIGNUM_ERROR_START + 0x0)

/* cghash error */
#define OLERR_CGHASH_ERROR_START (OLERR_CGHASH_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_SHA1_STATE_ERROR (OLERR_CGHASH_ERROR_START + 0x0)

/* cgmac error */
#define OLERR_CGMAC_ERROR_START (OLERR_CGMAC_ERROR << ERRCODE_MODULE_SHIFT)

//#define OLERR_ (OLERR_CGMAC_ERROR_START + 0x0)

/* prng error */
#define OLERR_PRNG_ERROR_START (OLERR_PRNG_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_PRNG_NOT_SEEDED (OLERR_PRNG_ERROR_START + 0x0)
#define OLERR_FAIL_GET_RANDOM_DATA (OLERR_PRNG_ERROR_START + 0x1)

/* jiukun error*/
#define OLERR_JIUKUN_ERROR_START (OLERR_JIUKUN_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_FAIL_DESTROY_JIUKUN_CACHE (OLERR_JIUKUN_ERROR_START + 0x0)
#define OLERR_FAIL_CREATE_JIUKUN_CACHE (OLERR_JIUKUN_ERROR_START + 0x1)
#define OLERR_JIUKUN_CACHE_GROW_NOT_ALLOW (OLERR_JIUKUN_ERROR_START + 0x2)
#define OLERR_FAIL_GROW_JIUKUN_CACHE (OLERR_JIUKUN_ERROR_START + 0x3)
#define OLERR_UNSUPPORTED_MEMORY_SIZE (OLERR_JIUKUN_ERROR_START + 0x4)
#define OLERR_FAIL_ALLOC_JIUKUN_PAGE (OLERR_JIUKUN_ERROR_START + 0x5)
#define OLERR_INVALID_JIUKUN_PAGE_ORDER (OLERR_JIUKUN_ERROR_START + 0x6)
#define OLERR_JIUKUN_OUT_OF_MEMORY (OLERR_JIUKUN_ERROR_START + 0x7)
#define OLERR_FAIL_REAP_JIUKUN (OLERR_JIUKUN_ERROR_START + 0x8)

/* ifmgmt error */
#define OLERR_IFMGMT_ERROR_START (OLERR_IFMGMT_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_ETHERNET_ADAPTER_NOT_FOUND (OLERR_IFMGMT_ERROR_START + 0x0)
#define OLERR_INVALID_MAC_ADDR (OLERR_IFMGMT_ERROR_START + 0x1)

/* servmgmt error */
#define OLERR_SERVMGMT_ERROR_START (OLERR_SERVMGMT_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_WAKEUP_SERV_NOT_FOUND (OLERR_SERVMGMT_ERROR_START + 0x0)

/* matrix error */
#define OLERR_MATRIX_ERROR_START (OLERR_MATRIX_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_MATRIX_SINGULAR (OLERR_MATRIX_ERROR_START + 0x0)

/* persistency error */
#define OLERR_PERSISTENCY_ERROR_START (OLERR_PERSISTENCY_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_SQL_EVAL_ERROR (OLERR_PERSISTENCY_ERROR_START + 0x0)
#define OLERR_PERSISTENCY_INIT_ERROR (OLERR_PERSISTENCY_ERROR_START + 0x1)
#define OLERR_SQL_COMPILE_ERROR (OLERR_PERSISTENCY_ERROR_START + 0x2)

/* cli error */
#define OLERR_CLI_ERROR_START (OLERR_CLI_ERROR << ERRCODE_MODULE_SHIFT)

#define OLERR_LOGOUT_REQUIRED (OLERR_CLI_ERROR_START + 0x0)

#define OLERR_MORE_CANCELED (OLERR_CLI_ERROR_START + 0x2)
#define OLERR_CLI_TOO_MANY_OPTIONS (OLERR_CLI_ERROR_START + 0x3)
#define OLERR_CLI_TERMINATED (OLERR_CLI_ERROR_START + 0x4)

#define OLERR_INVALID_ACTION (OLERR_CLI_ERROR_START + 0x10)
#define OLERR_ACTION_NOT_APPLY (OLERR_CLI_ERROR_START + 0x11)
#define OLERR_INVALID_OPTION (OLERR_CLI_ERROR_START + 0x12)

/* vendor specific error */
#define OLERR_VENDOR_SPEC_ERROR_START (OLERR_VENDOR_SPEC_ERROR << ERRCODE_MODULE_SHIFT)

/* maximum vendor specific error code,
   tune this value if more error codes are required*/
#define MAX_VENDOR_SPEC_ERROR  200

/* --- functional routines ------------------------------------------------- */

/** Get the description of the specified error code.
 *
 *  @param u32ErrCode [in] the specified error code
 *
 *  @return the error message.
 */
LOGGERAPI olchar_t * LOGGERCALL getErrorDescription(u32 u32ErrCode);

LOGGERAPI void LOGGERCALL getErrMsg(
    u32 u32Err, olchar_t * pstrBuf, olsize_t sBuf);

LOGGERAPI u32 LOGGERCALL addErrCode(u32 u32Err, olchar_t * pstrDesc);



#endif /*JIUFENG_ERRCODE_H*/

/*---------------------------------------------------------------------------*/




