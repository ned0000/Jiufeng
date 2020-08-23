/**
 *  @file jf_uuid.h
 *
 *  @brief UUID (Universally Unique Identifier) header file which provides some functional routine
 *   to generate UUID (RFC 4122).
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_uuid library.
 *  -# Link with jf_prng library to get random data.
 *  -# Link with jf_cghash library to use MD5 or SHA1 hash function.
 *  
 */

#ifndef JIUFENG_UUID_H
#define JIUFENG_UUID_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
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

/* --- constant definitions --------------------------------------------------------------------- */

/** UUID with bin format, bit / bytes.
 */
#define JF_UUID_LEN_BIN    (128 / 8)

/** UUID with string format, bit / nibbles + hyphens + trailing '\0'.
 */
#define JF_UUID_LEN_STR    (128 / 4 + 4 + 1)

/** UUID with hex format, bit / nibbles + trailing '\0'.
 */
#define JF_UUID_LEN_HEX    (128 / 4)

/** UUID with single integer value representation, int(log(10, exp(2, 128) - 1) + 1) digits.
 */
#define JF_UUID_LEN_SIV    (39)

/* --- data structures -------------------------------------------------------------------------- */

/** UUID version number.
 */
typedef enum
{
    /**Time-based version.*/
    JF_UUID_VER_1 = 1,
    /**Name-based version, uses MD5 hashing.*/
    JF_UUID_VER_3 = 3,
    /**Randomly or pseudo-randomly generated versio.*/
    JF_UUID_VER_4,
    /**Name-based version, uses SHA1 hashing.*/
    JF_UUID_VER_5,
} jf_uuid_ver_t;

/** UUID format.
 */
typedef enum
{
    /**Bin format.*/
    JF_UUID_FMT_BIN = 0,
    /**String format.*/
    JF_UUID_FMT_STR,
    /**Hex format.*/
    JF_UUID_FMT_HEX,
    /**Single integer value representation.*/
    JF_UUID_FMT_SIV,
} jf_uuid_fmt_t;

/** The parameter for getting UUID.
 */
typedef struct
{
    /**Use random number for the MAC address and set the multicast bit.*/
    boolean_t jup_bMulticastMac;
    u8 jup_u8Reserved[7];
    /**UUID format.*/
    jf_uuid_fmt_t jup_ufFmt;
    u8 jup_u8Reserved2[4];
    /**Name, null-terminated string.*/
    olchar_t * jup_pstrName;
    /**Name space UUID, MUST BE JF_UUID_LEN_BIN length.*/
    u8 * jup_pu8NameSpace;
} jf_uuid_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize UUID library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
UUIDAPI u32 UUIDCALL jf_uuid_init(void);

/** Finalize UUID library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
UUIDAPI u32 UUIDCALL jf_uuid_fini(void);

/** Get UUID.
 *
 *  @param pu8Uuid [out] The data buffer for the UUID.
 *  @param u32Len [in] Length of the buffer.
 *  @param version [in] UUID version.
 *  @param pjup [in] Parameter for generating UUID.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_NOT_INITIALIZED UUID library is not initialized.
 */
UUIDAPI u32 UUIDCALL jf_uuid_get(
    u8 * pu8Uuid, u32 u32Len, jf_uuid_ver_t version, jf_uuid_param_t * pjup);

#endif /*JIUFENG_UUID_H*/

/*------------------------------------------------------------------------------------------------*/
