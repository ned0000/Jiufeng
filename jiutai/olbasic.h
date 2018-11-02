/**
 *  @file olbasic.h
 *
 *  @brief basic types header file
 *
 *  @author Min Zhang
 *  
 *  @note
 */

#ifndef JIUTAI_OLBASIC_H
#define JIUTAI_OLBASIC_H

/* --- standard C lib header files ------------------------------------------ */
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#endif

/* --- internal header files ------------------------------------------------ */
#if defined(WINDOWS)
    #include "getopt.h"
    #define optarg getoptarg()
#elif defined(LINUX)

#endif

/* --- constants & data structures ------------------------------------------ */

#define IN
#define OUT

typedef unsigned char   u8;
#define U8_MAX          (0xFF)
#define BITS_PER_U8     (8)
typedef signed char     s8;

typedef unsigned short  u16;
#define U16_MAX         (0xFFFF)
#define BITS_PER_U16    (16)
#define BYTES_PER_U16   (2)
typedef signed short    s16;

typedef unsigned int    u32;
#define U32_MAX         (0xFFFFFFFF)
#define BITS_PER_U32    (32)
#define BYTES_PER_U32   (4)
typedef signed int      s32;
#define S32_MAX         (0x7FFFFFFF)

typedef unsigned long   ulong;
#ifndef ULONG_MAX
    #define ULONG_MAX   (0xFFFFFFFFUL)
#endif
#define ULONG_PER_U32   (32)

#if defined(LINUX)
    typedef unsigned long long  u64;
    typedef long long  s64;
#elif defined(WINDOWS)
    typedef unsigned __int64  u64;
    typedef __int64  s64;
#endif
#define U64_MAX  0xFFFFFFFFFFFFFFFFULL
#define BITS_PER_U64  64

#ifndef TRUE
    #define TRUE  0x1
#endif

#ifndef FALSE
    #define FALSE  0x0
#endif

typedef u8           boolean_t;
typedef s32          olindex_t;
typedef s32          olsize_t;
#define OLSIZE_MAX   S32_MAX
typedef s32          olid_t;
typedef char         olchar_t;
typedef short        olshort_t;
typedef int          olint_t;
typedef float        olfloat_t;
typedef double       oldouble_t;

#if defined(WINDOWS)
    #define ol_memset                memset
    #define ol_bzero                 bzero
    #define ol_strcmp                strcmp
    #define ol_strncmp               strncmp
    #define ol_strcasecmp            _stricmp
    #define ol_strncasecmp           _strnicmp
    #define ol_sprintf               _sprintf
    #define ol_snprintf              _snprintf
    #define ol_vsnprintf             _vsnprintf
    #define ol_printf                _printf
    #define ol_strcpy                _strcpy
    #define ol_strncpy               _strncpy
    #define ol_strcat                _strcat
    #define ol_strlen                _strlen
    #define ol_random()              rand()
    #define __attribute__(__X__)
#elif defined(LINUX)
    #define ol_memset                memset
    #define ol_bzero                 bzero
    #define ol_strcmp                strcmp
    #define ol_strncmp               strncmp
    #define ol_strcasecmp            strcasecmp
    #define ol_strncasecmp           strncasecmp
    #define ol_sprintf               sprintf
    #define ol_snprintf              snprintf
    #define ol_vsnprintf             vsnprintf
    #define ol_vsprintf              vsprintf
    #define ol_printf                printf
    #define ol_strcpy                strcpy
    #define ol_strncpy               strncpy
    #define ol_strcat                strcat
    #define ol_strlen                strlen
    #define ol_random()              random()
#endif

#if defined(WINDOWS)
    #define PATH_SEPARATOR        '\\'
    #define LINE_TERMINATOR       "\r\n"
#elif defined(LINUX)
    #define PATH_SEPARATOR        '/'
    #define LINE_TERMINATOR       "\n"
#endif

#if defined(WINDOWS)
    #define inline  __inline
#endif

#ifndef MAX
    #define MAX(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) <= (b) ? (a) : (b))
#endif

#ifndef ALIGN
    #define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

#ifndef ABS
    #define ABS(a) ((a) >= 0 ? (a) : (-(a)))
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* --- functional routines ------------------------------------------------- */
static inline int before(u32 seq1, u32 seq2)
{
    return (s32)(seq1-seq2) < 0;
}
#define after(seq2, seq1)   before(seq1, seq2)

/* is s2<=s1<=s3 ? */
static inline int between(u32 seq1, u32 seq2, u32 seq3)
{
    return seq3 - seq2 >= seq1 - seq2;
}

#endif /*JIUTAI_OLBASIC_H*/

/*---------------------------------------------------------------------------*/



