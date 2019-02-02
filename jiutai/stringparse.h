/**
 *  @file stringparse.h
 *
 *  @brief String parse header file, provide some functional routine for
 *   string manipulation
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
    /** Token */
    olchar_t * prf_pstrData;
    /** Length of the token */
    olsize_t prf_sData;
    /** Next field */
    struct parse_result_field * prf_pprfNext;
} parse_result_field_t;

/** Parse result
 */
typedef struct parse_result
{
    /** First result */
    parse_result_field_t *pr_pprfFirst;
    /** Last result */
    parse_result_field_t *pr_pprfLast;
    /** Numbers of results */
    u32 pr_u32NumOfResult;
} parse_result_t;

/* --- functional routines ------------------------------------------------- */

/*basic string parse*/

/** Parse a string into a linked list of tokens.
 *
 *  @param ppResult [out] the parse result returned
 *  @param pstrBuf [in] The buffer to parse 
 *  @param sOffset [in] The offset of the buffer to start parsing 
 *  @param sBuf [in] The length of the buffer to parse 
 *  @param pstrDelimiter [in] The delimiter 
 *  @param sDelimiter [in] The length of the delimiter 
 * 
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 *
 *  @note differs from parseStringAdv, this method does not ignore
 *   characters contained within quotation marks, whereas parseStringAdv does.
 */
STRINGPARSEAPI u32 STRINGPARSECALL parseString(
    parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter);


/** Parses a string into a linked list of tokens. Ignore characters contained
 *  within quotation marks. The quotation is " or '.
 *
 *  @param ppResult [out] the parse result returned
 *  @param pstrBuf [in] The buffer to parse 
 *  @param sOffset [in] The offset of the buffer to start parsing 
 *  @param sBuf [in] The size of the buffer to parse 
 *  @param pstrDelimiter [in] The delimiter 
 *  @param sDelimiter [in] The length of the delimiter 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 *
 *  @note differs from parseString, this method ignores characters
 *   contained within quotation marks, whereas parseString does not.
 */
STRINGPARSEAPI u32 STRINGPARSECALL parseStringAdv(
    parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter);

/** Frees resources associated with the list of tokens returned from parseString
 *  and parseStringAdv.
 *
 *  @param ppResult [in] The list of tokens to free 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL destroyParseResult(
    parse_result_t ** ppResult);

/** Remove the blank space(s) from the left and the right of the string
 *
 *  @param pstrDest [out] the output string after removing the blank space(s)
 *  @param pstrSource [in] the input string to be removed the blank space(s)
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI void STRINGPARSECALL skipBlank(
    olchar_t * pstrDest, const olchar_t * pstrSource);

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

/** Removes all leading and tailing blankspaces, and replaces all multiple
 *  occurences of blank spaces with a single one.       \n
 *  eg:                                                 \n
 *      "  this   is  a   test string"                  \n
 *  =>    "this is a test string"                       \n
 *
 *  @param pstr [in/out] the string that should be trimmed.
 *
 *  @return void
 */
STRINGPARSEAPI void STRINGPARSECALL trimBlankOfString(olchar_t * pstr);

/** break string to line with width, the line terminator is at the best
 *  suitable position. 
 *
 *  @param pstr [in/out] the string that should be wrapped.
 *  @param sWidth [in] the maximal column count of the wrapped string.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 breakStringToLine(olchar_t * pstr, olsize_t sWidth);

/** ppstrLoc is a pointer to the beginning of the substring, or NULL if
 *  the substring is not found
 **/
STRINGPARSEAPI u32 STRINGPARSECALL locateSubString(
    const olchar_t * pstr, const olchar_t * pstrSub, olchar_t ** ppstrLoc);

/** Replaces the first occurence of needle in the string src with the string
 *  subst. If no occurence of needle could be found in src, NULL is returned,
 *  otherwise the starting index of needle inside src. src needs to be
 *  big enough to store the resulting string.
 *
 *  @param pstrSrc [in] the string that should be modified.
 *  @param sBuf [in] the size of the buffer containing the source string
 *  @param pstrNeedle [in] the pattern that should be replaced.
 *  @param pstrSubst [in] the pattern that should be used for replacing.
 *
 *  @return the occurence of needle
 *  @retval NULL if no occurence of needle could be found in src
 */
STRINGPARSEAPI olchar_t * STRINGPARSECALL replaceString(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle,
    olchar_t * pstrSubst);

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

/** Reads a long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param size [in] the length of the string 
 *  @param numeric [in] the long value extracted from the string 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, long * numeric);

/** Reads an unsigned long value from a string
 *
 *  @param pstrInteger [in] the string to read from 
 *  @param size [in] the length of the string 
 *  @param numeric [in] the unsigned long value extracted from the string 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric);

STRINGPARSEAPI u32 STRINGPARSECALL getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value);

STRINGPARSEAPI u32 STRINGPARSECALL getS64FromString(
    const olchar_t * pstrInteger, const olsize_t size, s64 * ps64Value);

/** Read binary from string
 *
 *  @param pstr [in] the string to read from 
 *  @param size [in] the length of the string 
 *  @param pu8Binary [out] the buffer for the binary
 *  @param psBinary [in/out] the length of the buffer and the length of binary
 *   is returned
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary,
    olsize_t * psBinary);

/** Get the size accordign to the size stirng. The size string will be the
 *  format of "xxxx.xxGB|MB|KB|B"
 *
 *  @param pstrSize [in] the size string;
 *  @param pu64Size [out] the size in bytes to be returned
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL getSizeFromString(
    const olchar_t * pstrSize, u64 * pu64Size);

/** Get the time from the string with the format hour:minute:second like
 *  15:23:58
 */
STRINGPARSEAPI u32 STRINGPARSECALL getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin,
    olint_t * pSec);

/** Get date from the string with the format year/month/date like 2005/10/20
 */
STRINGPARSEAPI u32 STRINGPARSECALL getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

/** Get date from the string with the format year-month-date like 2005-10-20
 */
STRINGPARSEAPI u32 STRINGPARSECALL getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

STRINGPARSEAPI u32 STRINGPARSECALL getMACAddressFromString(
    const olchar_t * pstrMACString, u8 * pu8Value);

/** Convert the string to byte hex
 *  return the number of bytes hex converted from the string
 */
STRINGPARSEAPI olsize_t STRINGPARSECALL getHexFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Hex, olsize_t sHexLen);

STRINGPARSEAPI u32 STRINGPARSECALL getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue);

STRINGPARSEAPI u32 STRINGPARSECALL getDoubleFromString(
	const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue);

/*setting parse*/

/** Process the command line id list to form a unary array of ids.
 * 
 *  @param pstrIdList [in] a id list form command line, ended with '\0'.
 *  @param pids [out] the id array to be returned
 *  @param psId [in/out] as in param, the expected count of ids;
 *   as out param, the actually inputed count of ids from the command line.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note pids is a buffer can at least hold *psId ids.
 *  @note if pstrIdList = "2,5,9~11", after processing
 *  @note pu32Ids[5] = {2,5,9,10,11}, * psId = 5
 */
STRINGPARSEAPI u32 STRINGPARSECALL processIdList(
    const olchar_t * pstrIdList, olid_t * pids, olsize_t * psId);

/** Retrive settings from the string array
 * 
 *  @param pstrArray [in] the string array returned by processSetting()
 *  @param sArray [in] number of element in the string array
 *  @param pstrName [in] setting name
 *  @param pstrValue [out] setting value
 *  @param sValue [in] length of value string
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note processSettings() should be called before
 */
STRINGPARSEAPI u32 STRINGPARSECALL retrieveSettings(
    olchar_t * pstrArray[], olsize_t sArray,
    const olchar_t * pstrName, olchar_t * pstrValue, olsize_t sValue);

STRINGPARSEAPI u32 STRINGPARSECALL retrieveSettingsEnable(
    olchar_t * pstrValue, boolean_t * pbEnable);

/** Validate the setting.
 * 
 *  @param pstrNameArray [in] the tag name array
 *  @param sNameArray [in] number of element in the tag name array
 *  @param pstrArray [in] the string array returned by processSetting()
 *  @param sArray [in] number of element in the string array
 *  @param piArray [out] the index of the invalid setting
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note processSettings() should be called before
 */
STRINGPARSEAPI u32 STRINGPARSECALL validateSettings(
    olchar_t * pstrNameArray[], olsize_t sNameArray,
    olchar_t * pstrArray[], olsize_t sArray, olindex_t * piArray);

/** Break down the strings pointed by pstrSettings to an array pointing to 
 *  each setting.                                                  \n
 *  Eg. pstrSetting = "tag1=value1\0tag2 = value2\0tag3=value3"    \n
 *   After processing,                                             \n
 *      pstrArray[0] = "tag1=value1"                               \n
 *      pstrArray[1] = "tag2 = value2"                             \n
 *      pstrArray[2] = "tag3=value3"                               \n
 *      *psArray = 3                                               \n
 *
 *  @param pu8Settings [in] the setting string
 *  @param sSettings [in] the size of the string
 *  @param pstrStrArray [out] the string array
 *  @param psArray [in/out] number of element in the string array
 * 
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note for iSCSI keywords process, the tag is seperated by '\0'
 *  @note psArray specifies the array size. If numbers of tags are more
 *   than array size, only those tags are returned.
 */
STRINGPARSEAPI u32 STRINGPARSECALL processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings,
    olchar_t * pstrStrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue);

/** Break down the strings pointed by pstrSettings to an array pointing to 
 *  each setting.                                                    \n
 *  Eg. pstrSetting = "tag1=value1, tag2 = value2, tag3=value3"      \n
 *   After processing,                                               \n
 *      pstrArray[0] = "tag1=value1"                                 \n
 *      pstrArray[1] = "tag2 = value2"                               \n
 *      pstrArray[2] = "tag3=value3"                                 \n
 *      *psArray = 3                                                 \n
 *
 *  @param pstrSettings [in] the setting string
 *  @param pstrArray [out] the string array
 *  @param psArray [out] number of element in the string array
 * 
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note the tag is seperated by ',' 
 */
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


