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

#define JF_STRING_UNKNOWN  "Unknown"

/* --- data structures ----------------------------------------------------- */
/** Parse result field
 */
typedef struct jf_string_parse_result_field
{
    /** Token */
    olchar_t * jsprf_pstrData;
    /** Length of the token */
    olsize_t jsprf_sData;
    /** Next field */
    struct jf_string_parse_result_field * jsprf_pjsprfNext;
} jf_string_parse_result_field_t;

/** Parse result
 */
typedef struct jf_string_parse_result
{
    /** First result */
    jf_string_parse_result_field_t * jspr_pjsprfFirst;
    /** Last result */
    jf_string_parse_result_field_t * jspr_pjsprfLast;
    /** Numbers of results */
    u32 jspr_u32NumOfResult;
} jf_string_parse_result_t;

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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_parse(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_parseAdv(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter);

/** Frees resources associated with the list of tokens returned from parseString
 *  and parseStringAdv.
 *
 *  @param ppResult [in] The list of tokens to free 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_destroyParseResult(
    jf_string_parse_result_t ** ppResult);

/** Remove the blank space(s) from the left and the right of the string
 *
 *  @param pstrDest [out] the output string after removing the blank space(s)
 *  @param pstrSource [in] the input string to be removed the blank space(s)
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_skipBlank(
    olchar_t * pstrDest, const olchar_t * pstrSource);

STRINGPARSEAPI boolean_t STRINGPARSECALL jf_string_isBlankLine(const olchar_t * pstrLine);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_duplicate(
    olchar_t ** ppstrDest, const olchar_t * pstrSource);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_duplicateWithLen(
    olchar_t ** ppstrDest, const olchar_t * pu8Source, const olsize_t sSource);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_free(olchar_t ** ppstrStr);

STRINGPARSEAPI void STRINGPARSECALL jf_string_filterComment(
    olchar_t * pstrDest, const olchar_t * pstrComment);

STRINGPARSEAPI void STRINGPARSECALL jf_string_lower(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL jf_string_upper(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL jf_string_removeLeadingSpace(olchar_t * pstr);

STRINGPARSEAPI void STRINGPARSECALL jf_string_removeTailingSpace(olchar_t * pstr);

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
STRINGPARSEAPI void STRINGPARSECALL jf_string_trimBlank(olchar_t * pstr);

/** break string to line with width, the line terminator is at the best
 *  suitable position. 
 *
 *  @param pstr [in/out] the string that should be wrapped.
 *  @param sWidth [in] the maximal column count of the wrapped string.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_breakToLine(
    olchar_t * pstr, olsize_t sWidth);

/** ppstrLoc is a pointer to the beginning of the substring, or NULL if
 *  the substring is not found
 **/
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_locateSubString(
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
STRINGPARSEAPI olchar_t * STRINGPARSECALL jf_string_replace(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle,
    olchar_t * pstrSubst);

/*string print*/

/** Get the string of positive
 *
 *  @param bPositive [in] the positive status

 *  @return the string of the positive. Either "Yes" or "No".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringPositive(
    const boolean_t bPositive);

/** Get the enable/disable status
 *
 *  @param bEnable [in] the enable status
 *
 *  @return the string of the status. Either "Enabled" or "Disabled".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringEnable(
    const boolean_t bEnable);

/** Get WWN(world wide name)
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrWwn [out] the string to return
 *  @param u64WWN [in] the u64 WWN 
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringWWN(
    olchar_t * pstrWwn, const u64 u64WWN);

/** Get the version string in the format of "v.vv.vvvv.vv".
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @parame pstrVersion [out] the size string to be returned;
 *  @parame u8Major [in] the major version number;
 *  @parame u8Minor [in] the minor version number;
 *  @parame u32OEMCode [in] the OEM code;
 *  @parame u8BuildNo [in] the build number;
 */
void getStringVersion(
    olchar_t * pstrVersion, const u8 u8Major,
    const u8 u8Minor, const u32 u32OEMCode, const u8 u8BuildNo);

/** Get string of u64 integer
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrInteger [out] the u64 string to be returned
 *  @param u64Integer [in] the u64 integer
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringU64Integer(
    olchar_t * pstrInteger, const u64 u64Integer);

/** Get the string of true/false
 *
 *  @param bTrue [in] the true status
 *
 *  @return the string of the status, either "TRUE" or "FALSE".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringTrue(
    const boolean_t bTrue);

/** Get the string of not applicable "N/A".
 *
 *  @return the string of not applicable.
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringNotApplicable(void);

/** Get the string of not supported
 *
 *  @return the string of not supported
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringNotSupported(void);

/** Get the string of MAC Address
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrMACAddr [out] the string buffer where the MAC address string will return
 *  @param pu8MAC [in] the MAC addr info
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringMACAddress(
    olchar_t * pstrMACAddr, const u8 * pu8MAC);

/** Get the size string in GB, MB, KB and B. The size string will be the format
 *  of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrSize [out] the size string to be returned;
 *  @param u64Size [in] the size in bytes
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSize(
    olchar_t * pstrSize, const u64 u64Size);

/** Get the size string in GB, MB, KB and B. The size string will be the format
 *  of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrSize [out] the size string to be returned
 *  @param u64Size [in] the size in bytes
 *
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSizeMax(
    olchar_t * pstrSize, const u64 u64Size);

/** Get the size string based 1000 in GB, MB, KB and B. The size string will
 *  be the format of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrSize [out] the size string to be returned;
 *  @param u64Size [in] the size in bytes
 *
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSize1000Based(
    olchar_t * pstrSize, const u64 u64Size);

STRINGPARSEAPI olsize_t STRINGPARSECALL jf_string_getStringHex(
    olchar_t * pstr, olsize_t size, const u8 * pu8Hex, const olsize_t sHex);

/** Get the string of time period in the format of "[# hr] [# min] [# sec]"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrTime [out] the string buffer where the period string will return
 *  @param u32Period [in] the time
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringTimePeriod(
    olchar_t * pstrTime, const u32 u32Period);

/** Get the string of time in the format of "hh:mm:ss <month> <day>, <year>".
 *  The time is local time
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrDate [out] the string buffer where the date string will return
 *  @param tTime [in] the time
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getStringLocalTime(
    olchar_t * pstrTime, const time_t tTime);

/** Get the string of time in the format of "hh:mm:ss <month> <day>, <year>".
 *  the time is UTC time
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrDate [out] the string buffer where the date string will return
 *  @param tTime [in] the time
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getStringUTCTime(
    olchar_t * pstrTime, const time_t tTime);

/** Get the string of date in the format of "<mon> dd, yyyy".
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrDate [out] the string buffer where the date string will return
 *  @param u8Month [in] the month of the year
 *  @param u8Day [in] the day of the month
 *  @param u16Year [in] the year
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringDate(
    olchar_t * pstrDate, const olint_t year, const olint_t mon, const olint_t day);

/** Get the string of date in the format of "yyyy-mm-dd"
 *
 *  @note This function does not check the size of the string buffer. Please make
 *   sure it is big enough to avoid memory access violation.
 *
 *  @param pstrDate [out] the string buffer where the date string will return
 *  @param u8Month [in] the month of the year
 *  @param u8Day [in] the day of the month
 *  @param u16Year [in] the year
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringDate2(
    olchar_t * pstrDate, const olint_t year, const olint_t mon, const olint_t day);

/*print date with format yyyy-mm-dd*/
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringDate2ForDaysFrom1970(
    olchar_t * pstrDate, const olint_t nDays);

/*string validation*/

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateStringAlias(
    const olchar_t * pstrAlias);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateStringUsername(
    const olchar_t * pstrUserName);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateHexString(
    const olchar_t * pstrHex, const olsize_t sHex);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateIntegerString(
    const olchar_t * pstrInteger, const olsize_t size);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateFloatString(
    const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS32FromString(
    const olchar_t * pstrInteger, const olsize_t size, s32 * ps32Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS32FromHexString(
    const olchar_t * pstrHex, const olsize_t size, s32 * ps32Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU32FromString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getLongFromString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS64FromString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getBinaryFromString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSizeFromString(
    const olchar_t * pstrSize, u64 * pu64Size);

/** Get the time from the string with the format hour:minute:second like
 *  15:23:58
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getTimeFromString(
    const olchar_t * pstrTimeString, olint_t * pHour, olint_t * pMin,
    olint_t * pSec);

/** Get date from the string with the format year/month/date like 2005/10/20
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getDateFromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

/** Get date from the string with the format year-month-date like 2005-10-20
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getDate2FromString(
    const olchar_t * pstrDate, olint_t * pYear, olint_t * pMon, olint_t * pDay);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getMACAddressFromString(
    const olchar_t * pstrMACString, u8 * pu8Value);

/** Convert the string to byte hex
 *  return the number of bytes hex converted from the string
 */
STRINGPARSEAPI olsize_t STRINGPARSECALL jf_string_getHexFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Hex, olsize_t sHexLen);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getDoubleFromString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processIdList(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_retrieveSettings(
    olchar_t * pstrArray[], olsize_t sArray,
    const olchar_t * pstrName, olchar_t * pstrValue, olsize_t sValue);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_retrieveSettingsEnable(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateSettings(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings,
    olchar_t * pstrStrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processSettingString(
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
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processSettings(
    olchar_t * pstrSettings, olchar_t * pstrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSettingsU32(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const u32 u32DefaultValue, u32 * pu32Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSettingsBoolean(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const boolean_t bDefaultValue, boolean_t * pbValue);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSettingsString(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const olchar_t * pstrDefaultValue, olchar_t * pstrValue, olsize_t sValue);


#endif /*JIUFENG_STRINGPARSE_H*/

/*---------------------------------------------------------------------------*/


