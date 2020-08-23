/**
 *  @file jf_basic.h
 *
 *  @brief Header file define the basic data type.
 *
 *  @author Min Zhang
 *  
 *  @note
 */

#ifndef JIUTAI_JF_BASIC_H
#define JIUTAI_JF_BASIC_H

/* --- standard C lib header files -------------------------------------------------------------- */

#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#if defined(WINDOWS)
    #include <Winsock2.h>
    #include <windows.h>
    #if _MSC_VER >= 1500 /*for VS2008*/
        #include <stdio.h>
        #include <tchar.h>
    #endif
#elif defined(LINUX)
    #include <pthread.h>
    #include <unistd.h>
    #include <getopt.h>
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include <stddef.h>
#else
    #error "Unsupported platform."
#endif

/* --- internal header files -------------------------------------------------------------------- */

#if defined(WINDOWS)

#elif defined(LINUX)

#else
    #error "Unsupported platform."
#endif

/* --- constants & data structures -------------------------------------------------------------- */

#define IN
#define OUT

typedef char                         olchar_t;
#define BITS_PER_OLCHAR              (8)
#define BYTES_PER_OLCHAR             (1)

typedef unsigned char                u8;
#define U8_MAX                       (0xFF)
#define BITS_PER_U8                  (8)
#define BYTES_PER_U8                 (1)

typedef signed char                  s8;
#define S8_MAX                       (0x7F)
#define BITS_PER_S8                  (8)
#define BYTES_PER_S8                 (1)

typedef unsigned short               u16;
#define U16_MAX                      (0xFFFF)
#define BITS_PER_U16                 (16)
#define BYTES_PER_U16                (2)

typedef signed short                 s16;
#define S16_MAX                      (0x7FFF)
#define BITS_PER_S16                 (16)
#define BYTES_PER_S16                (2)

typedef short                        olshort_t;
#define OLSHORT_MAX                  S16_MAX
#define BITS_PER_OLSHORT             BITS_PER_S16
#define BYTES_PER_OLSHORT            BYTES_PER_S16

typedef unsigned int                 u32;
#define U32_MAX                      (0xFFFFFFFF)
#define BITS_PER_U32                 (32)
#define BYTES_PER_U32                (4)

typedef signed int                   s32;
#define S32_MAX                      (0x7FFFFFFF)
#define BITS_PER_S32                 (32)
#define BYTES_PER_S32                (4)

typedef int                          olint_t;
#define OLINT_MAX                    S32_MAX
#define BITS_PER_OLINT               BITS_PER_S32
#define BYTES_PER_OLINT              BYTES_PER_S32

typedef unsigned long                ulong;
#ifndef ULONG_MAX
    #define ULONG_MAX                (0xFFFFFFFFUL)
#endif
#define BITS_PER_ULONG               (32)

typedef signed long                  slong;
#define SLONG_MAX                    (0x7FFFFFFFUL)
#define BITS_PER_SLONG               (32)

#if defined(LINUX)
    typedef unsigned long long       u64;
#elif defined(WINDOWS)
    typedef unsigned __int64         u64;
#endif
#define U64_MAX                      (0xFFFFFFFFFFFFFFFFULL)
#define BITS_PER_U64                 (64)
#define BYTES_PER_U64                (8)

#if defined(LINUX)
    typedef long long                s64;
#elif defined(WINDOWS)
    typedef __int64                  s64;
#endif
#define S64_MAX                      (0x7FFFFFFFFFFFFFFFULL)
#define BITS_PER_S64                 (64)
#define BYTES_PER_S64                (8)

typedef float                        olfloat_t;
#define BYTES_PER_OLFLOAT            (4)

typedef double                       oldouble_t;
#define BYTES_PER_OLDOUBLE           (8)

typedef s32                          olsize_t;
#define OLSIZE_MAX                   S32_MAX

typedef s32                          olssize_t;
#define OLSSIZE_MAX                  S32_MAX

typedef u8                           boolean_t;
#ifndef TRUE
    #define TRUE                     (0x1)
#endif
#ifndef FALSE
    #define FALSE                    (0x0)
#endif

#define BYTES_PER_POINTER            (sizeof(void *))

typedef s32                          olindex_t;
typedef s32                          olid_t;
typedef time_t                       oltime_t;

#if defined(WINDOWS)
    #define ol_memset                memset
    #define ol_memcpy                memcpy
    #define ol_memcmp                memcmp
    #define ol_memchr                memchr
    #define ol_memmove               memmove
    #define ol_bzero(s, n)           memset(s, 0, n)
    #define ol_strcmp                strcmp
    #define ol_strncmp               strncmp
    #define ol_strcasecmp            _stricmp
    #define ol_strncasecmp           _strnicmp
    #define ol_strtol                strtol
    #define ol_strtoul               strtoul
    #define ol_strtoull              _strtoui64
    #define ol_strtoll               _strtoi64
    #define ol_sprintf               sprintf
    #define ol_snprintf              snprintf
    #define ol_vsnprintf             vsnprintf
    #define ol_printf                printf
    #define ol_fprintf               fprintf
    #define ol_fflush                fflush
    #define ol_sscanf                sscanf
    #define ol_strcpy                strcpy
    #define ol_strncpy               strncpy
    #define ol_strcat                strcat
    #define ol_strncat               strncat
    #define ol_strlen                strlen
    #define ol_strchr                strchr
    #define ol_strerror              _strerror
    #define ol_random()              rand()
    #define ol_srand                 srand
    #define ol_htonl                 htonl
    #define ol_ntohl                 ntohl
    #define ol_time                  time
    #define ol_unlink                unlink
    #define ol_open                  open
    #define ol_close                 close
    #define ol_read                  read
    #define ol_write                 write
    #define ol_sleep(s)              Sleep(s * 1000)
    #define __attribute__(__X__)
#elif defined(LINUX)
    #define ol_memset                memset
    #define ol_memcpy                memcpy
    #define ol_memcmp                memcmp
    #define ol_memchr                memchr
    #define ol_memmove               memmove
    #define ol_bzero                 bzero
    #define ol_strcmp                strcmp
    #define ol_strncmp               strncmp
    #define ol_strcasecmp            strcasecmp
    #define ol_strncasecmp           strncasecmp
    #define ol_strtol                strtol
    #define ol_strtoul               strtoul
    #define ol_strtoull              strtoull
    #define ol_strtoll               strtoll
    #define ol_sprintf               sprintf
    #define ol_snprintf              snprintf
    #define ol_vsnprintf             vsnprintf
    #define ol_vsprintf              vsprintf
    #define ol_printf                printf
    #define ol_fprintf               fprintf
    #define ol_fflush                fflush
    #define ol_dprintf               dprintf
    #define ol_sscanf                sscanf
    #define ol_strcpy                strcpy
    #define ol_strncpy               strncpy
    #define ol_strcat                strcat
    #define ol_strncat               strncat
    #define ol_strlen                strlen
    #define ol_strchr                strchr
    #define ol_strerror              strerror
    #define ol_random()              random()
    #define ol_srand                 srand
    #define ol_htonl                 htonl
    #define ol_ntohl                 ntohl
    #define ol_time                  time
    #define ol_qsort                 qsort
    #define ol_unlink                unlink
    #define ol_open                  open
    #define ol_close                 close
    #define ol_read                  read
    #define ol_write                 write
    #define ol_sleep(s)              sleep(s)
#else
    #error "Unsupported platform."
#endif

#if defined(WINDOWS)
    #define PATH_SEPARATOR           '\\'
    #define LINE_TERMINATOR          "\r\n"
#elif defined(LINUX)
    #define PATH_SEPARATOR           '/'
    #define LINE_TERMINATOR          "\n"
#else
    #error "Unsupported platform."
#endif

#if defined(WINDOWS)
    #define inline                   __inline
#endif

/** Get the maximum value.
 */
#ifndef MAX
    #define MAX(a, b)                ((a) >= (b) ? (a) : (b))
#endif

/** Get the minimum value.
 */
#ifndef MIN
    #define MIN(a, b)                ((a) <= (b) ? (a) : (b))
#endif

/** Align value 'x' with 'a' to ceil.
 */
#ifndef ALIGN_CEIL
    #define ALIGN_CEIL(x, a)         (((x) + (a) - 1) & ~((a) - 1))
#endif

/** Align value 'x' with 'a' to floor.
 */
#ifndef ALIGN_FLOOR
    #define ALIGN_FLOOR(x, a)        ((x) & ~((a) - 1))
#endif

/** Get the absolute value.
 */
#ifndef ABS
    #define ABS(a)                   ((a) >= 0 ? (a) : (-(a)))
#endif

/** Get the size of the array.
 */
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(x)            (sizeof(x) / sizeof((x)[0]))
#endif

/* --- functional routines ---------------------------------------------------------------------- */


#endif /*JIUTAI_JF_BASIC_H*/

/*------------------------------------------------------------------------------------------------*/
