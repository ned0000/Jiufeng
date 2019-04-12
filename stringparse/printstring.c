/**
 *  @file printstring.c
 *
 *  @brief routines for printing string
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <time.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_string.h"
#include "jf_err.h"
#include "jf_date.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

const olchar_t * jf_string_getStringNotApplicable(void)
{
    return "N/A";
}

const olchar_t * jf_string_getStringNotSupported(void)
{
    return "Not Supported";
}

void jf_string_getStringWWN(olchar_t * pstrWwn, const u64 u64WWN)
{
    u8 * pByte;

    assert(pstrWwn != NULL);

    pByte = (u8 *)&u64WWN;
    ol_sprintf(
        pstrWwn, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
        pByte[0], pByte[1], pByte[2], pByte[3], pByte[4], pByte[5], pByte[6], pByte[7]);
}

void jf_string_getStringU64Integer(olchar_t * pstrInteger, const u64 u64Integer)
{
    assert(pstrInteger != NULL);
#ifdef WINDOWS
    ol_sprintf(pstrInteger, "%I64u", u64Integer);
#else
    ol_sprintf(pstrInteger, "%lld", u64Integer);
#endif
}

void jf_string_getStringMACAddress(olchar_t * pstrMACAddr, const u8 * pu8MAC)
{
    ol_sprintf(
        pstrMACAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
        pu8MAC[0], pu8MAC[1], pu8MAC[2], pu8MAC[3], pu8MAC[4], pu8MAC[5]);
}

#ifdef WINDOWS
    #define ONE_TEGABYTE    0x10000000000i64
#else
    #define ONE_TEGABYTE    0x10000000000LL
#endif
#define ONE_GIGABYTE    0x40000000
#define ONE_MEGABYTE    0x100000
#define ONE_KILOBYTE    0x400

void jf_string_getStringSize(olchar_t * pstrSize, const u64 u64Size)
{
    u64 u64Size1, u64Mod1;

    u64Size1 = u64Size;
    u64Mod1 = 0;

    if (u64Size <= 1)
    {
        ol_sprintf(pstrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)1024)
    {
        ol_sprintf(pstrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE)
    {
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE);
        u64Mod1 = ((u64Mod1 * 100) + (ONE_KILOBYTE / 2)) / 1024;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
        {
             u64Size1 += 1;
             ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE)
    {
        u64Size1 = u64Size / ONE_MEGABYTE;
        u64Mod1 = u64Size % ONE_MEGABYTE;
        u64Mod1 = (((u64Mod1 * 100) + (ONE_MEGABYTE / 2)) / ONE_MEGABYTE);/* adjust rounding */
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE)
    {
        u64Size1 = u64Size / ONE_GIGABYTE;
        u64Mod1 = u64Size % ONE_GIGABYTE;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_GIGABYTE / 2)) / ONE_GIGABYTE;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else if (u64Mod1 == 100)
        {
             u64Size1 += 1;
             ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "GB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        u64Size1 = u64Size / ONE_TEGABYTE;
        u64Mod1 = u64Size % ONE_TEGABYTE;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_TEGABYTE / 2)) / ONE_TEGABYTE;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }
}

void jf_string_getStringSizeMax(olchar_t * pstrSize, const u64 u64Size)
{
    u64 u64Size1, u64Mod1;

    u64Size1 = u64Size;
    u64Mod1 = 0;

    if (u64Size <= 1)
    {
        ol_sprintf(pstrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)1024)
    {
        ol_sprintf(pstrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE)
    {
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE);
        u64Mod1 = ((u64Mod1 * 100) + (ONE_KILOBYTE / 2)) / 1024;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
        {
             u64Size1 += 1;
             ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE)
    {
        u64Size1 = u64Size / ONE_MEGABYTE;
        u64Mod1 = u64Size % ONE_MEGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_MEGABYTE;/* adjust rounding */
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE)
    {
        u64Size1 = u64Size / ONE_GIGABYTE;
        u64Mod1 = u64Size % ONE_GIGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_GIGABYTE;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        u64Size1 = u64Size / ONE_TEGABYTE;
        u64Mod1 = u64Size % ONE_TEGABYTE;
        u64Mod1 = (u64Mod1 * 100) / ONE_TEGABYTE;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }
}

#ifdef WINDOWS
#define ONE_TEGABYTE_1000_BASED 0xE8D4A51000i64
#else
#define ONE_TEGABYTE_1000_BASED 0xE8D4A51000LL
#endif
#define ONE_GIGABYTE_1000_BASED 0x3B9ACA00
#define ONE_MEGABYTE_1000_BASED 0xF4240
#define ONE_KILOBYTE_1000_BASED 0x3E8

void jf_string_getStringSize1000Based(olchar_t * pstrSize, const u64 u64Size)
{
    u64 u64Size1, u64Mod1;

    u64Size1 = u64Size;
    u64Mod1 = 0;

    if (u64Size <= 1)
    {
        ol_sprintf(pstrSize, "%dByte", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_KILOBYTE_1000_BASED)
    {
        ol_sprintf(pstrSize, "%uBytes", (u32)u64Size);
    }
    else if (u64Size < (u64)ONE_MEGABYTE_1000_BASED)
    {
        u64Size1 = u64Size / ((u64)ONE_KILOBYTE_1000_BASED);
        u64Mod1 = u64Size % ((u64)ONE_KILOBYTE_1000_BASED);
        u64Mod1 = ((u64Mod1 * 100) + (ONE_KILOBYTE_1000_BASED / 2)) / ONE_KILOBYTE_1000_BASED;
        if (u64Mod1 == 0 )
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "KB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "KB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "KB");
    }
    else if (u64Size < (u64)ONE_GIGABYTE_1000_BASED)
    {
        u64Size1 = u64Size / ONE_MEGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_MEGABYTE_1000_BASED;
        u64Mod1 = (((u64Mod1 * 100) + (ONE_MEGABYTE_1000_BASED / 2)) / ONE_MEGABYTE_1000_BASED);/* adjust rounding */
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "MB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "MB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "MB");
    }
    else if (u64Size < ONE_TEGABYTE_1000_BASED)
    {
        u64Size1 = u64Size / ONE_GIGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_GIGABYTE_1000_BASED;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_GIGABYTE_1000_BASED / 2)) / ONE_GIGABYTE_1000_BASED;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "GB");
        else if (u64Mod1 < 10)
            ol_sprintf(pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "GB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "GB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "GB");
    }
    else
    {
        u64Size1 = u64Size / ONE_TEGABYTE_1000_BASED;
        u64Mod1 = u64Size % ONE_TEGABYTE_1000_BASED;
        u64Mod1 = ((u64Mod1 * 100) + (ONE_TEGABYTE_1000_BASED / 2)) / ONE_TEGABYTE_1000_BASED;
        if (u64Mod1 == 0)
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        else if (u64Mod1 < 10)
            ol_sprintf( pstrSize, "%u.%s%u%s", (u32)u64Size1, "0", (u32)u64Mod1, "TB");
        else if (u64Mod1 == 100)
        {
            u64Size1 += 1;
            ol_sprintf(pstrSize, "%u%s", (u32)u64Size1, "TB");
        }
        else
            ol_sprintf(pstrSize, "%u.%u%s", (u32)u64Size1, (u32)u64Mod1, "TB");
    }
}

void jf_string_getStringVersion(
    olchar_t * pstrVersion, const u8 u8Major,
    const u8 u8Minor, const u32 u32OEMCode, const u8 u8BuildNo)
{
    ol_sprintf(
        pstrVersion, "%d.%02d.%04d.%02d", u8Major, u8Minor, u32OEMCode, u8BuildNo);
}

const olchar_t * jf_string_getStringEnable(const boolean_t bEnable)
{
    if (bEnable)
    {
        return "Enabled";
    }
    else
    {
        return "Disabled";
    }
}

const olchar_t * jf_string_getStringUnknown(void)
{
    return "Unknown";
}

const olchar_t * jf_string_getStringPositive(const boolean_t bPositive)
{
    if (bPositive)
    {
        return "Yes";
    }
    else
    {
        return "No";
    }
}

const olchar_t * jf_string_getStringTrue(const boolean_t bTrue)
{
    if (bTrue)
    {
        return "TRUE";
    }
    else
    {
        return "FALSE";
    }
}

/*---------------------------------------------------------------------------*/


