/**
 *  @file jf_uuid.h
 *
 *  @brief UUID (Universally Unique Identifier) header file, provide some
 *   functional routine to generate UUID (RFC 4122)
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_uuid library
 *  @note Link with prng library, call initPrng() to init the library
 *  @note Link with cghash library
 *  
 */

#ifndef JIUFENG_UUID_H
#define JIUFENG_UUID_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

#undef UUIDAPI
#undef UUIDCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_UUID_DLL)
        #define UUIDAPI  __declspec(dllexport)
        #define UUIDCALL
    #else
        #define UUIDAPI
        #define UUIDCALL __cdecl
    #endif
#else
    #define UUIDAPI
    #define UUIDCALL
#endif

/* --- constant definitions ------------------------------------------------ */

/** UUID with bin format, bit / bytes
 */
#define JF_UUID_LEN_BIN    (128 / 8)

/** UUID with string format, bit / nibbles + hyphens + trailing '\0'
 */
#define JF_UUID_LEN_STR    (128 / 4 + 4 + 1)

/** UUID with hex format, bit / nibbles + trailing '\0'
 */
#define JF_UUID_LEN_HEX    (128 / 4)

/** UUID with single integer value representation
 *  int(log(10, exp(2, 128) - 1) + 1) digits
 */
#define JF_UUID_LEN_SIV    (39)

/* --- data structures ----------------------------------------------------- */

/** UUID version number
 */
typedef enum
{
    JF_UUID_VER_1 = 1, /**< time-based version */
    JF_UUID_VER_3 = 3, /**< name-based version, uses MD5 hashing */
    JF_UUID_VER_4,     /**< randomly or pseudo-randomly generated versio */
    JF_UUID_VER_5,     /**< name-based version, uses SHA1 hashing */
} jf_uuid_ver_t;

/** UUID format
 */
typedef enum
{
    JF_UUID_FMT_BIN = 0, /**< bin format */
    JF_UUID_FMT_STR,     /**< string format */
    JF_UUID_FMT_HEX,     /**< hex format */
    JF_UUID_FMT_SIV,     /**< single integer value representation */
} jf_uuid_fmt_t;

typedef struct
{
    /** Use random number for the MAC address and set the multicast bit */
    boolean_t jup_bMulticastMac;
    u8 jup_u8Reserved[7];
    /** UUID format */
    jf_uuid_fmt_t jup_ufFmt;
    u8 jup_u8Reserved2[4];
    /** Name, null-terminated string */
    olchar_t * jup_pstrName;
    /** Name space UUID, MUST BE JF_UUID_LEN_BIN length */
    u8 * jup_pu8NameSpace;
} jf_uuid_param_t;

/* --- functional routines ------------------------------------------------- */

UUIDAPI u32 UUIDCALL jf_uuid_get(
    u8 * pu8Uuid, u32 u32Len, jf_uuid_ver_t version, jf_uuid_param_t * pjup);

#endif /*JIUFENG_UUID_H*/

/*---------------------------------------------------------------------------*/


