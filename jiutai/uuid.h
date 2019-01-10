/**
 *  @file uuid.h
 *
 *  @brief UUID (Universally Unique Identifier) header file, provide some
 *   functional routine to generate UUID (RFC 4122)
 *
 *  @author Min Zhang
 *
 *  @note Link with prng library, call initPrng() to init the library
 *  @note Link with cghash library
 *  
 */

#ifndef JIUFENG_UUID_H
#define JIUFENG_UUID_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

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
#define UUID_LEN_BIN    (128 / 8)

/** UUID with string format, bit / nibbles + hyphens + trailing '\0'
 */
#define UUID_LEN_STR    (128 / 4 + 4 + 1)

/** UUID with hex format, bit / nibbles + trailing '\0'
 */
#define UUID_LEN_HEX    (128 / 4)

/** UUID with single integer value representation
 *  int(log(10, exp(2, 128) - 1) + 1) digits
 */
#define UUID_LEN_SIV    (39)

/* --- data structures ----------------------------------------------------- */

/** UUID version number
 */
typedef enum
{
    UUID_VER_1 = 1, /**< time-based version */
    UUID_VER_3 = 3, /**< name-based version, uses MD5 hashing */
    UUID_VER_4,     /**< randomly or pseudo-randomly generated versio */
    UUID_VER_5,     /**< name-based version, uses SHA1 hashing */
} uuid_ver_t;

/** UUID format
 */
typedef enum
{
    UUID_FMT_BIN = 0, /**< bin format */
    UUID_FMT_STR,     /**< string format */
    UUID_FMT_HEX,     /**< hex format */
    UUID_FMT_SIV,     /**< single integer value representation */
} uuid_fmt_t;

typedef struct
{
    /** Use random number for the MAC address and set the multicast bit */
    boolean_t up_bMulticastMac;
    u8 up_u8Reserved[7];
    /** UUID format */
    uuid_fmt_t up_ufFmt;
    u8 up_u8Reserved2[4];
    /** Name, null-terminated string */
    olchar_t * up_pstrName;
    /** Name space UUID, MUST BE UUID_LEN_BIN length */
    u8 * up_pu8NameSpace;
} uuid_param_t;

/* --- functional routines ------------------------------------------------- */
UUIDAPI u32 UUIDCALL getUuid(
    u8 * pu8Uuid, u32 u32Len, uuid_ver_t version, uuid_param_t * pup);

#endif /*JIUFENG_UUID_H*/

/*---------------------------------------------------------------------------*/


