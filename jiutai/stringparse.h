/**
 *  @file stringparse.h
 *
 *  @brief string parse header file
 *     provide some functional routine to parse string
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_STRINGPARSE_H
#define JIUFENG_STRINGPARSE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"

#undef STRINGPARSEAPI
#undef STRINGPARSECALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_STRINGPARSE_DLL)
        #define STRINGPARSEAPI  __declspec(dllexport)
        #define STRINGPARSECALL
    #else
        #define STRINGPARSEAPI
        #define STRINGPARSECALL __cdecl
    #endif
#else
    #define STRINGPARSEAPI
    #define STRINGPARSECALL
#endif

/* --- constant definitions ------------------------------------------------ */

#define STRING_UNKNOWN  "Unknown"

/* --- data structures ----------------------------------------------------- */
/** Parse result field
 */
typedef struct parse_result_field
{
    /**< token */
    olchar_t * prf_pstrData;
    /**< Length of the token */
    olsize_t prf_sData;
    /**< next field */
    struct parse_result_field * prf_pprfNext;
} parse_result_field_t;

/** Parse result
 */
typedef struct parse_result
{
    /**< first result */
    parse_result_field_t *pr_pprfFirst;
    /**< last result */
    parse_result_field_t *pr_pprfLast;
    /**< numbers of results */
    u32 pr_u32NumOfResult;
} parse_result_t;

/* --- functional routines ------------------------------------------------- */

/*basic string parse*/

/// Parses a string into a linked list of tokens.
STRINGPARSEAPI u32 STRINGPARSECALL parseString(
    parse_result_t ** ppResult,
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, olchar_t * pstrDelimiter,
    olsize_t sDelimiter);

/// Parses a string into a linked list of tokens.
STRINGPARSEAPI u32 STRINGPARSECALL parseStringAdv(
    parse_result_t ** ppResult,
    olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf, olchar_t * pstrDelimiter,
    olsize_t sDelimiter);

/// Frees resources associated with the list of tokens returned 
/// from ILibParseString and ILibParseStringAdv.
STRINGPARSEAPI u32 STRINGPARSECALL destroyParseResult(
    parse_result_t ** ppResult);

STRINGPARSEAPI void STRINGPARSECALL skipBlank(olchar_t * pstrDest,
    const olchar_t * pstrSource);

STRINGPARSEAPI boolean_t STRINGPARSECALL isBlankLine(const olchar_t * pstrLine);

STRINGPARSEAPI u32 STRINGPARSECALL dupString(
    olchar_t ** ppstrDest, const olchar_t * pstrSource);

STRINGPARSEAPI u32 STRINGPARSECALL dupStringWithLen(
    olchar_t ** ppstrDest, const olchar_t * pu8Source, const olsize_t sSource);

STRINGPARSEAPI u32 STRINGPARSECALL freeString(olchar_t ** ppstrStr);

STRINGPARSEAPI void STRINGPARSECALL filterComment(
    olchar_t * pstrDest, const olchar_t * pstrComment);

STRINGPARSEAPI void STRINGPARSECALL lowerString(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL upperString(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL removeLeadingSpace(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL removeTailingSpace(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL trimBlankOfString(olchar_t * pstr);

/* *ppstrLoc is a pointer to the beginning of the substring, or NULL if
  the substring is not found*/
STRINGPARSEAPI u32 STRINGPARSECALL locateSubString(const olchar_t * pstr,
    const olchar_t * pstrSub, olchar_t ** ppstrLoc);

STRINGPARSEAPI olchar_t * STRINGPARSECALL replaceString(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle, olchar_t * pstrSubst);

/*string print*/

STRINGPARSEAPI const olchar_t * STRINGPARSECALL getStringPositive(
    const boolean_t bPositive);

STRINGPARSEAPI const olchar_t * STRINGPARSECALL getStringEnable(
    const boolean_t bEnable);

STRINGPARSEAPI const olchar_t * STRINGPARSECALL getStringTrue(
    const boolean_t bTrue);

STRINGPARSEAPI const olchar_t * STRINGPARSECALL getStringNotApplicable(void);

STRINGPARSEAPI const olchar_t * STRINGPARSECALL getStringNotSupported(void);

STRINGPARSEAPI void STRINGPARSECALL getStringMACAddress(
    olchar_t * pstrMACAddr, const u8 * pu8MAC);

STRINGPARSEAPI void STRINGPARSECALL getStringSize(
    olchar_t * pstrSize, const u64 u64Size);

STRINGPARSEAPI void STRINGPARSECALL getStringSizeMax(
    olchar_t * pstrSize, const u64 u64Size);

STRINGPARSEAPI void STRINGPARSECALL getStringSize1000Based(
    olchar_t * pstrSize, const u64 u64Size);

STRINGPARSEAPI olsize_t STRINGPARSECALL getStringHex(
    olchar_t * pstr, olsize_t size, const u8 * pu8Hex, const olsize_t sHex);

STRINGPARSEAPI void STRINGPARSECALL getStringTimePeriod(
    olchar_t * pstrTime, const u32 u32Period);

STRINGPARSEAPI u32 STRINGPARSECALL getStringLocalTime(
    olchar_t * pstrTime, const time_t tTime);

STRINGPARSEAPI u32 STRINGPARSECALL getStringUTCTime(
    olchar_t * pstrTime, const time_t tTime);

/*print date with format mm dd, yyyy*/
STRINGPARSEAPI void STRINGPARSECALL getStringDate(
    olchar_t * pstrDate, const olint_t year, const olint_t mon, const olint_t day);

/*print date with format yyyy-mm-dd*/
STRINGPARSEAPI void STRINGPARSECALL getStringDate2(
    olchar_t * pstrDate, const olint_t year, const olint_t mon, const olint_t day);

/*print date with format yyyy-mm-dd*/
STRINGPARSEAPI void STRINGPARSECALL getStringDate2ForDaysFrom1970(
    olchar_t * pstrDate, const olint_t nDays);

/*string validation*/

STRINGPARSEAPI u32 STRINGPARSECALL validateStringAlias(const olchar_t * pstrAlias);

STRINGPARSEAPI u32 STRINGPARSECALL validateStringUsername(
    const olchar_t * pstrUserName);

STRINGPARSEAPI u32 STRINGPARSECALL validateHexString(
    const olchar_t * pstrHex, const olsize_t sHex);

STRINGPARSEAPI u32 STRINGPARSECALL validateIpAddr(
    const olchar_t * pstrIp, const olchar_t u8AddrType);

STRINGPARSEAPI u32 STRINGPARSECALL validateIntegerString(
    const olchar_t * pstrInteger, const olsize_t size);

STRINGPARSEAPI u32 STRINGPARSECALL validateFloatString(
    const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

STRINGPARSEAPI u32 STRINGPARSECALL getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value);

STRINGPARSEAPI u32 STRINGPARSECALL getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value);

STRINGPARSEAPI u32 STRINGPARSECALL getS32FromString(
    const olchar_t * pstrInteger, const olsize_t size, s32 * ps32Value);

STRINGPARSEAPI u32 STRINGPARSECALL getS32FromHexString(
    const olchar_t * pstrHex, const olsize_t size, s32 * ps32Value);

STRINGPARSEAPI u32 STRINGPARSECALL getU32FromString(
    const olchar_t * pstrInteger, const olsize_t size, u32 * pu32Value);

STRINGPARSEAPI u32 STRINGPARSECALL getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, long * numeric);

STRINGPARSEAPI u32 STRINGPARSECALL getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric);

STRINGPARSEAPI u32 STRINGPARSECALL getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value);

STRINGPARSEAPI u32 STRINGPARSECALL getS64FromString(
    const olchar_t * pstrInteger, const olsize_t size, s64 * ps64Value);

STRINGPARSEAPI u32 STRINGPARSECALL getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary, olsize_t * psBinary);

STRINGPARSEAPI u32 STRINGPARSECALL getSizeFromString(
    const olchar_t * pstrSize, u64 * pu64Size);

/*get the time from the string with the format hour:minute:second like 15:23:58
 */
STRINGPARSEAPI u32 STRINGPARSECALL getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin, olint_t * pSec);

/* get date from the string with the format year/month/date like 2005/10/20
 */
STRINGPARSEAPI u32 STRINGPARSECALL getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

/* get date from the string with the format year-month-date like 2005-10-20
 */
STRINGPARSEAPI u32 STRINGPARSECALL getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

STRINGPARSEAPI u32 STRINGPARSECALL getMACAddressFromString(
    const olchar_t * pstrMACString, u8 * pu8Value);

/*convert the string to byte hex
  return the number of bytes hex converted from the string */
STRINGPARSEAPI olsize_t STRINGPARSECALL getHexFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Hex, olsize_t sHexLen);

STRINGPARSEAPI u32 STRINGPARSECALL getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue);

STRINGPARSEAPI u32 STRINGPARSECALL getDoubleFromString(
	const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue);

/*setting parse*/

STRINGPARSEAPI u32 STRINGPARSECALL retrieveSettingsEnable(
    olchar_t * pstrValue, boolean_t * pbEnable);

STRINGPARSEAPI u32 STRINGPARSECALL validateSettings(
    olchar_t * pstrNameArray[], olsize_t sNameArray,
    olchar_t * pstrArray[], olsize_t sArray, olindex_t * piArray);

STRINGPARSEAPI u32 STRINGPARSECALL processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings,
    olchar_t * pstrStrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue);

STRINGPARSEAPI u32 STRINGPARSECALL processSettings(
    olchar_t * pstrSettings, olchar_t * pstrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL getSettingsU32(
    olchar_t * pstrArray[], olsize_t sArray,
    const olchar_t * pstrSettingName, const u32 u32DefaultValue, u32 * pu32Value);

STRINGPARSEAPI u32 STRINGPARSECALL getSettingsBoolean(
    olchar_t * pstrArray[], olsize_t sArray,
    const olchar_t * pstrSettingName, const boolean_t bDefaultValue,
    boolean_t * pbValue);

STRINGPARSEAPI u32 STRINGPARSECALL getSettingsString(
    olchar_t * pstrArray[], olsize_t sArray,
    const olchar_t * pstrSettingName, const olchar_t * pstrDefaultValue,
    olchar_t * pstrValue, olsize_t sValue);


#endif /*JIUFENG_STRINGPARSE_H*/

/*---------------------------------------------------------------------------*/


