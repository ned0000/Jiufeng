/**
 *  @file jf_string.h
 *
 *  @brief String parse header file, provide some functional routine for string manipulation.
 *
 *  @author Min Zhang
 *
 *  @note 
 *  -# Routines declared in this file are included in jf_string library.
 *  -# Link with jf_jiukun library for memory allocation.
 *
 */

#ifndef JIUFENG_STRING_H
#define JIUFENG_STRING_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"

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

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_STRING_NOT_APPLICABLE              "N/A"

/* --- data structures -------------------------------------------------------------------------- */

/** Define the string parse result field data type.
 */
typedef struct jf_string_parse_result_field
{
    /**The string token.*/
    olchar_t * jsprf_pstrData;
    /**Length of the token.*/
    olsize_t jsprf_sData;
    /**Next parse result field.*/
    struct jf_string_parse_result_field * jsprf_pjsprfNext;
} jf_string_parse_result_field_t;

/** Define the string parse result data type.
 */
typedef struct jf_string_parse_result
{
    /**First result.*/
    jf_string_parse_result_field_t * jspr_pjsprfFirst;
    /**Last result.*/
    jf_string_parse_result_field_t * jspr_pjsprfLast;
    /**Numbers of results.*/
    u32 jspr_u32NumOfResult;
} jf_string_parse_result_t;

/* --- functional routines ---------------------------------------------------------------------- */

/*basic string parse*/

/** Parse a string into a linked list of tokens.
 *
 *  @note
 *  -# Differs from parseStringAdv, this method does not ignore characters contained within
 *     quotation marks, whereas parseStringAdv does.
 *  -# After parse, the buffer is kept unchanged.
 *
 *  @par Example
 *  <table>
 *  <tr><th>Source String      <th>delimiter   <th>Number of Fields <th>Fields               </tr>
 *  <tr><td>"<name>"           <td>"<"         <td>2   <td>"", "name>"                       </tr>
 *  <tr><td>"<name>"           <td>"="         <td>1   <td>"name>"                           </tr>
 *  <tr><td>"<name>"           <td>">"         <td>2   <td>"<name", ""                       </tr>
 *  <tr><td>"<name>"           <td>"me"        <td>2   <td>"<na", ">"                        </tr>
 *  <tr><td>"<name>"           <td>"no"        <td>1   <td>"<name>"                          </tr>
 *  <tr><td>"<My name is">" adv>" <td>">"      <td>3   <td>"<My name is"", "" adv>", ""      </tr>
 *  </table>
 *
 *  @param ppResult [out] the parse result returned.
 *  @param pstrBuf [in] The buffer to parse.
 *  @param sOffset [in] The offset of the buffer to start parsing.
 *  @param sBuf [in] The length of the buffer to parse.
 *  @param pstrDelimiter [in] The delimiter.
 *  @param sDelimiter [in] The length of the delimiter. 
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of meory.
 *
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_parse(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset, olsize_t sBuf,
    olchar_t * pstrDelimiter, olsize_t sDelimiter);


/** Parses a string into a linked list of tokens. Ignore characters contained within quotation
 *  marks.
 *
 *  @note
 *  -# The quotation mark is " or '.
 *  -# Differs from parseString, this method ignores characters contained within quotation marks,
 *     whereas parseString does not.
 *  -# After parse, the buffer is kept unchanged.
 *
 *  @par Example
 *  <table>
 *  <tr><th>Source String      <th>Delimiter   <th>Number of Fields <th>Fields               </tr>
 *  <tr><td>"<My name is">" adv>"    <td>">"   <td>2   <td>"<My name is">" adv", ""         </tr>
 *  <tr><td>"<My name is'>' adv>"    <td>">"   <td>2   <td>"<My name is'>' adv", ""         </tr>
 *  <tr><td>">My name is'>' adv>"    <td>">"   <td>3   <td>"", "My name is'>' adv", ""     </tr>
 *  </table>
 *
 *  @param ppResult [out] The parse result returned.
 *  @param pstrBuf [in] The buffer to parse.
 *  @param sOffset [in] The offset of the buffer to start parsing.
 *  @param sBuf [in] The size of the buffer to parse.
 *  @param pstrDelimiter [in] The delimiter.
 *  @param sDelimiter [in] The length of the delimiter. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 *
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_parseAdv(
    jf_string_parse_result_t ** ppResult, olchar_t * pstrBuf, olsize_t sOffset,
    olsize_t sBuf, olchar_t * pstrDelimiter, olsize_t sDelimiter);

/** Frees resources associated with the list of tokens returned from jf_string_parse() and
 *  jf_string_parseAdv().
 *
 *  @param ppResult [in] The list of tokens to free.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_destroyParseResult(
    jf_string_parse_result_t ** ppResult);

/** Remove the blank space(s) from the left and the right of the string.
 *
 *  @param pstrDest [out] The output string after removing the blank space(s).
 *  @param pstrSource [in] The input string to be removed the blank space(s).
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_skipBlank(
    olchar_t * pstrDest, const olchar_t * pstrSource);

/** Check if the line is blank.
 *
 *  @param pstrLine [in] The line to be checked.
 *
 *  @return The status of blank line.
 *  @retval TRUE It's a blank line.
 *  @retval FALSE It's not a blank line.
 */
STRINGPARSEAPI boolean_t STRINGPARSECALL jf_string_isBlankLine(const olchar_t * pstrLine);

/** Duplicate the source string.
 *
 *  @param ppstrDest [out] The destination string.
 *  @param pstrSource [in] The source string.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_duplicate(
    olchar_t ** ppstrDest, const olchar_t * pstrSource);

/** Duplicate the source string with length.
 *
 *  @param ppstrDest [out] The destination string.
 *  @param pstrSource [in] The source string.
 *  @param sSource [in] The length of the source string.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_duplicateWithLen(
    olchar_t ** ppstrDest, const olchar_t * pstrSource, const olsize_t sSource);

/** Free the string duplicated.
 *
 *  @param ppstrStr [in/out] The string to be freed.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_free(olchar_t ** ppstrStr);

/** Filter the string with comment.
 *
 *  @par Example
 *  @code
 *   pstrDest = "hello # comend", pstrComment = "#"
 *   after processing:
 *   pstrDest = "hello "
 *  @endcode
 *
 *  @param pstrDest [in/out] The string to be filtered.
 *  @param pstrComment [in] The string of comment.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_filterComment(
    olchar_t * pstrDest, const olchar_t * pstrComment);

/** Lower the string.
 *
 *  @par Example
 *  @code
 *   pstrDest = "Hello"
 *   after processing:
 *   pstrDest = "hello"
 *  @endcode
 *
 *  @param pstr [in/out] The string to be lowered.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_lower(olchar_t * pstr);

/** Upper the string.
 *
 *  @par Example
 *  @code
 *   pstrDest = "Hello"
 *   after processing:
 *   pstrDest = "HELLO"
 *  @endcode
 *
 *  @param pstr [in/out] The string to be uppered.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_upper(olchar_t * pstr);

/** Remove the leading space of the string.
 *
 *  @par Example
 *  @code
 *   pstrDest = "  hello "
 *   after processing:
 *   pstrDest = "hello "
 *  @endcode
 *
 *  @param pstr [in/out] The string to be operated.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_removeLeadingSpace(olchar_t * pstr);

/** Remove the traling space of the string.
 *
 *  @par Example
 *  @code
 *   pstrDest = "  hello "
 *   after processing:
 *   pstrDest = "  hello"
 *  @endcode
 *
 *  @param pstr [in/out] The string to be operated.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_removeTailingSpace(olchar_t * pstr);

/** Removes all leading and tailing blankspaces, and replaces all multiple occurences of blank
 *  spaces with a single one.
 *
 *  @par Example
 *  @code
 *   pstr = "  this   is  a   test string"
 *   after processing:
 *   pstrDest = "this is a test string"
 *  @endcode
 *
 *  @param pstr [in/out] The string that should be trimmed.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_trimBlank(olchar_t * pstr);

/** Break string to line with width, the line terminator is at the best suitable position. 
 *
 *  @param pstr [in/out] The string that should be wrapped.
 *  @param sWidth [in] The maximal column count of the wrapped string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_breakToLine(olchar_t * pstr, olsize_t sWidth);

/** Locate sub string.
 *
 *  @param pstr [in] The string to be located.
 *  @param pstrSub [in] The sub string.
 *  @param ppstrLoc [out] Pointer to the beginning of the substring, or NULL if the substring is
 *   not found.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_locateSubString(
    const olchar_t * pstr, const olchar_t * pstrSub, olchar_t ** ppstrLoc);

/** Replaces the first occurence of needle in the source string with the substitute string. If no
 *  occurence of needle could be found in source string, NULL is returned, otherwise return the
 *  starting index of needle inside source string.
 *
 *  @note
 *  -# Source string needs to be big enough to store the resulting string.
 *
 *  @param pstrSrc [in] The string that should be modified.
 *  @param sBuf [in] The size of the buffer containing the source string.
 *  @param pstrNeedle [in] the pattern that should be replaced.
 *  @param pstrSubst [in] the pattern that should be used for replacing.
 *
 *  @return The occurence of needle.
 *  @retval NULL If no occurence of needle could be found in source string.
 */
STRINGPARSEAPI olchar_t * STRINGPARSECALL jf_string_replace(
    olchar_t * pstrSrc, olsize_t sBuf, olchar_t * pstrNeedle, olchar_t * pstrSubst);

/*string print*/

/** Get the string of positive.
 *
 *  @param bPositive [in] The positive status.
 *
 *  @return The string of the positive.
 *  @retval Yes Positive.
 *  @retval No Negative.
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringPositive(
    const boolean_t bPositive);

/** Get the enable/disable status.
 *
 *  @param bEnable [in] The enable status.
 *
 *  @return The string of the status.
 *  @retval Enabled Enabled.
 *  @retval Disabled Disabled.
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringEnable(
    const boolean_t bEnable);

/** Get WWN(world wide name).
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big
 *     enough to avoid memory access violation.
 *
 *  @param pstrWwn [out] The string to return.
 *  @param u64WWN [in] The u64 WWN.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringWWN(
    olchar_t * pstrWwn, const u64 u64WWN);

/** Get the version string in the format of "v.vv.vvvv.vv".
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big
 *     enough to avoid memory access violation.
 *
 *  @param pstrVersion [out] The size string to be returned.
 *  @param u8Major [in] The major version number.
 *  @param u8Minor [in] The minor version number.
 *  @param u32OEMCode [in] The OEM code.
 *  @param u8BuildNo [in] The build number.
 *
 *  @return Void.
 */
void getStringVersion(
    olchar_t * pstrVersion, const u8 u8Major, const u8 u8Minor, const u32 u32OEMCode,
    const u8 u8BuildNo);

/** Get string of u64 integer.
 *
 *  @note This function does not check the size of the string buffer. Please make sure it is big
 *   enough to avoid memory access violation.
 *
 *  @param pstrInteger [out] The u64 string to be returned.
 *  @param u64Integer [in] The u64 integer.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringU64Integer(
    olchar_t * pstrInteger, const u64 u64Integer);

/** Get the string of true/false.
 *
 *  @param bTrue [in] The true status.
 *
 *  @return The string of the status.
 *  @retval TRUE True.
 *  @retval FALSE False.
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringTrue(
    const boolean_t bTrue);

/** Get the string of unknown.
 *
 *  @return The string "Unknown".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringUnknown(void);

/** Get the string of not applicable "N/A".
 *
 *  @return The string "N/A".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringNotApplicable(void);

/** Get the string of not supported.
 *
 *  @return The string "Not Supported".
 */
STRINGPARSEAPI const olchar_t * STRINGPARSECALL jf_string_getStringNotSupported(void);

/** Get the string of MAC Address.
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big enough
 *     to avoid memory access violation.
 *
 *  @param pstrMACAddr [out] The string buffer where the MAC address string will return.
 *  @param pu8MAC [in] The MAC addr information.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringMACAddress(
    olchar_t * pstrMACAddr, const u8 * pu8MAC);

/** Get the size string in GB, MB, KB and B. The size string will be the format of
 *  "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>".
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big enough
 *     to avoid memory access violation.
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSize(olchar_t * pstrSize, const u64 u64Size);

/** Get the size string in GB, MB, KB and B. The size string will be the format of
 *  "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>"
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big enough
 *     to avoid memory access violation.
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSizeMax(
    olchar_t * pstrSize, const u64 u64Size);

/** Get the size string based 1000 in GB, MB, KB and B. The size string will be the format of
 *  "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>".
 *
 *  @note
 *  -# This function does not check the size of the string buffer. Please make sure it is big
 *     enough to avoid memory access violation.
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return Void.
 */
STRINGPARSEAPI void STRINGPARSECALL jf_string_getStringSize1000Based(
    olchar_t * pstrSize, const u64 u64Size);

/*string validation*/

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateStringAlias(const olchar_t * pstrAlias);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateStringUsername(const olchar_t * pstrUserName);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateHexString(
    const olchar_t * pstrHex, const olsize_t sHex);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateIntegerString(
    const olchar_t * pstrInteger, const olsize_t size);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateFloatString(
    const olchar_t * pstrFloat, const olsize_t size);

/*string scan*/

/** Read unsigned char from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu8Value [out] The unsigned char returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value);

/** Read unsigned shor from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu16Value [out] The unsigned short returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value);

/** Read signed integer from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param ps32Value [out] The signed integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS32FromString(
    const olchar_t * pstrInteger, const olsize_t size, s32 * ps32Value);

/** Read signed integer from hex string.
 *
 *  @par Example
 *  @code
 *   pstrHex = "ab12"
 *   after processing:
 *   *ps32Value = 43794
 *  @endcode
 *
 *  @param pstrHex [out] The integer hex string.
 *  @param size [in] The size of the string.
 *  @param ps32Value [out] The signed integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS32FromHexString(
    const olchar_t * pstrHex, const olsize_t size, s32 * ps32Value);

/** Read unsigned integer from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu32Value [out] The unsigned integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU32FromString(
    const olchar_t * pstrInteger, const olsize_t size, u32 * pu32Value);

/** Reads a long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param numeric [in] The long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, long * numeric);

/** Reads an unsigned long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param numeric [in] The unsigned long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric);

/** Read unsigned long long integer from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu64Value [out] The unsigned long long integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value);

/** Read signed long long integer from string.
 *
 *  @param pstrInteger [out] The integer string.
 *  @param size [in] The size of the string.
 *  @param ps64Value [out] The signed long long integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getS64FromString(
    const olchar_t * pstrInteger, const olsize_t size, s64 * ps64Value);

/** Read binary from string.
 *
 *  @param pstr [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param pu8Binary [out] The buffer for the binary.
 *  @param psBinary [in/out] The length of the buffer and the length of binary is returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary, olsize_t * psBinary);

/** Get the size accordign to the size stirng. The size string will be the format of
 *  "xxxx.xxGB|MB|KB|B".
 *
 *  @param pstrSize [in] The size string.
 *  @param pu64Size [out] The size in bytes to be returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSizeFromString(
    const olchar_t * pstrSize, u64 * pu64Size);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getMACAddressFromString(
    const olchar_t * pstrMACString, u8 * pu8Value);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getDoubleFromString(
	const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue);

/*setting parse*/

/** Process the command line id list to form a unary array of ids.
 * 
 *  @note
 *  -# The id buffer will only save specified number of id, other id are discarded.
 *
 *  @par Example
 *  @code
 *   pstrIdList = "2,5,9~11"
 *   after processing:
 *   pids[5] = {2,5,9,10,11}, *psId = 5.
 *  @endcode
 *
 *  @param pstrIdList [in] An id list form command line, ended with '\0'.
 *  @param pids [out] The id array to be returned
 *  @param psId [in/out] As in parameter, the expected count of ids; As out parameter, the actually
 *   inputed count of ids from the command line.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processIdList(
    const olchar_t * pstrIdList, olid_t * pids, olsize_t * psId);

/** Retrive settings from the string array.
 * 
 *  @note jf_string_processSettings() should be called before.
 *
 *  @param pstrArray [in] The string array returned by jf_string_processSettings().
 *  @param sArray [in] Number of element in the string array.
 *  @param pstrName [in] Setting name.
 *  @param pstrValue [out] Setting value.
 *  @param sValue [in] Length of value string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_retrieveSettings(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrName, olchar_t * pstrValue,
    olsize_t sValue);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_retrieveSettingsEnable(
    olchar_t * pstrValue, boolean_t * pbEnable);

/** Validate the setting.
 * 
 *  @note
 *  -# jf_string_processSettings() should be called before.
 *
 *  @param pstrNameArray [in] The tag name array.
 *  @param sNameArray [in] Number of element in the tag name array.
 *  @param pstrArray [in] The string array returned by jf_string_processSettings().
 *  @param sArray [in] Number of element in the string array.
 *  @param piArray [out] The index of the invalid setting.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_validateSettings(
    olchar_t * pstrNameArray[], olsize_t sNameArray, olchar_t * pstrArray[], olsize_t sArray,
    olindex_t * piArray);

/** Break down the strings to an array pointing to each setting.
 *
 *  @note
 *  -# For iSCSI keywords process, the tag is seperated by '\0'.
 *  -# psArray specifies the array size. If numbers of tags are more
 *     than array size, only those tags are returned.
 *
 *  @par Example
 *  @code
 *  pstrSettings = "tag1=value1\0tag2 = value2\0tag3=value3"
 *  After processing:
 *      pstrArray[0] = "tag1=value1"
 *      pstrArray[1] = "tag2 = value2"
 *      pstrArray[2] = "tag3=value3"
 *      *psArray = 3
 *  @endcode
 *
 *  @param pu8Settings [in] The setting string.
 *  @param sSettings [in] The size of the string.
 *  @param pstrStrArray [out] The string array.
 *  @param psArray [in/out] Number of element in the string array.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *
 */
STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings, olchar_t * pstrStrArray[], olsize_t * psArray);

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue);

/** Break down the strings to an array pointing to each setting.
 *
 *  @note
 *  -# The tag is seperated by ','.
 *
 *  @par Example
 *  @code
 *   pstrSetting = "tag1=value1, tag2 = value2, tag3=value3"
 *   After processing:
 *      pstrArray[0] = "tag1=value1"
 *      pstrArray[1] = "tag2 = value2"
 *      pstrArray[2] = "tag3=value3"
 *      *psArray = 3
 *  @endcode
 *
 *  @param pstrSettings [in] The setting string.
 *  @param pstrArray [out] The string array.
 *  @param psArray [out] Number of element in the string array.
 * 
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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

STRINGPARSEAPI u32 STRINGPARSECALL jf_string_getSettingsDouble(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const oldouble_t dbDefaultValue, oldouble_t * pdbValue);

#endif /*JIUFENG_STRING_H*/

/*------------------------------------------------------------------------------------------------*/


