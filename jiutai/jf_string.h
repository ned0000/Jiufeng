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

#undef STRINGAPI
#undef STRINGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_STRING_DLL)
        #define STRINGAPI  __declspec(dllexport)
        #define STRINGCALL
    #else
        #define STRINGAPI
        #define STRINGCALL __cdecl
    #endif
#else
    #define STRINGAPI
    #define STRINGCALL
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
STRINGAPI u32 STRINGCALL jf_string_parse(
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
 *  <tr><th>Source String      <th>Delimiter   <th>Number of Fields <th>Fields              </tr>
 *  <tr><td>"<My name is">" adv>"    <td>">"   <td>2   <td>"<My name is">" adv", ""         </tr>
 *  <tr><td>"<My name is'>' adv>"    <td>">"   <td>2   <td>"<My name is'>' adv", ""         </tr>
 *  <tr><td>">My name is'>' adv>"    <td>">"   <td>3   <td>"", "My name is'>' adv", ""      </tr>
 *  <tr><td>""My > name is"> adv>"   <td>">"   <td>3   <td>""My > name is"", " adv", ""     </tr>
 *  <tr><td>"'My name > is'> adv>"   <td>">"   <td>3   <td>"'My name > is'", " adv", ""     </tr>
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
 */
STRINGAPI u32 STRINGCALL jf_string_parseAdv(
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
STRINGAPI u32 STRINGCALL jf_string_destroyParseResult(jf_string_parse_result_t ** ppResult);

/** Remove the blank spaces from the left and the right of the string.
 *
 *  @param pstrDest [out] The output string after removing the blank spaces.
 *  @param pstrSource [in] The input string to be removed the blank spaces.
 *
 *  @return Void.
 */
STRINGAPI void STRINGCALL jf_string_skipBlank(olchar_t * pstrDest, const olchar_t * pstrSource);

/** Check if the line is blank.
 *
 *  @note
 *  -# A line with all spaces is treated as blank line.
 *  -# A line started with '\\n' or '\\t' is treated as blank line.
 *
 *  @param pstrLine [in] The line to be checked.
 *
 *  @return The status of blank line.
 *  @retval TRUE It's a blank line.
 *  @retval FALSE It's not a blank line.
 */
STRINGAPI boolean_t STRINGCALL jf_string_isBlankLine(const olchar_t * pstrLine);

/** Duplicate the source string.
 *
 *  @note
 *  -# The terminating null byte '\0' is duplicated also.
 *
 *  @param ppstrDest [out] The destination string.
 *  @param pstrSource [in] The source string.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of jiukun memory.
 */
STRINGAPI u32 STRINGCALL jf_string_duplicate(olchar_t ** ppstrDest, const olchar_t * pstrSource);

/** Duplicate the source string with length.
 *
 *  @note
 *  -# The terminating null byte '\0' is always appended to the destination string.
 *
 *  @param ppstrDest [out] The destination string.
 *  @param pstrSource [in] The source string.
 *  @param sSource [in] The length of the source string.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of jiukun memory.
 */
STRINGAPI u32 STRINGCALL jf_string_duplicateWithLen(
    olchar_t ** ppstrDest, const olchar_t * pstrSource, const olsize_t sSource);

/** Free the string duplicated.
 *
 *  @param ppstrStr [in/out] The string to be freed.
 *
 *  @return The error status.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_free(olchar_t ** ppstrStr);

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
STRINGAPI void STRINGCALL jf_string_filterComment(
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
STRINGAPI void STRINGCALL jf_string_lower(olchar_t * pstr);

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
STRINGAPI void STRINGCALL jf_string_upper(olchar_t * pstr);

/** Remove the leading space of the string.
 *
 *  @note
 *  -# Move the string if there are leading spaces.
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
STRINGAPI void STRINGCALL jf_string_removeLeadingSpace(olchar_t * pstr);

/** Remove the traling space of the string.
 *
 *  @note
 *  -# The string is changed as '\0' is added.
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
STRINGAPI void STRINGCALL jf_string_removeTailingSpace(olchar_t * pstr);

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
STRINGAPI void STRINGCALL jf_string_trimBlank(olchar_t * pstr);

/** Break string to line with width, the line terminator is at the best suitable position. 
 *
 *  @note
 *  -# Only space will be replaced with line terminator.
 *
 *  @param pstr [in/out] The string that should be wrapped.
 *  @param sWidth [in] The maximal column count of the wrapped string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_breakToLine(olchar_t * pstr, olsize_t sWidth);

/** Locate sub string.
 *
 *  @param pstr [in] The string to be located.
 *  @param pstrSub [in] The sub string.
 *  @param ppstrLoc [out] Pointer to the beginning of the substring, or NULL if the substring is
 *   not found.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_SUBSTRING_NOT_FOUND Sub-string is not found.
 */
STRINGAPI u32 STRINGCALL jf_string_locateSubString(
    const olchar_t * pstr, const olchar_t * pstrSub, olchar_t ** ppstrLoc);

/** Replaces the first occurence of needle in the source string with the substitute string.
 *
 *  @note
 *  -# Source string needs to be big enough to store the resulting string.
 *  -# If no occurence of needle could be found in source string, NULL is returned, otherwise return
 *   the starting index of needle inside source string.
 *
 *  @param pstrSrc [in] The string that should be modified.
 *  @param sBuf [in] The size of the buffer containing the source string.
 *  @param pstrNeedle [in] the pattern that should be replaced.
 *  @param pstrSubst [in] the pattern that should be used for replacing.
 *
 *  @return The occurence of needle.
 *  @retval NULL If no occurence of needle could be found in source string.
 */
STRINGAPI olchar_t * STRINGCALL jf_string_replace(
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
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringPositive(const boolean_t bPositive);

/** Get the enable/disable status.
 *
 *  @param bEnable [in] The enable status.
 *
 *  @return The string of the status.
 *  @retval Enabled Enabled.
 *  @retval Disabled Disabled.
 */
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringEnable(const boolean_t bEnable);

/** Get WWN(world wide name).
 *
 *  @param pstrWwn [out] The string to return.
 *  @param sWwn [in] The size of the string buffer.
 *  @param u64WWN [in] The u64 WWN.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getStringWWN(
    olchar_t * pstrWwn, olsize_t sWwn, const u64 u64WWN);

/** Get the version string in the format of "v.vv.vvvv.vv".
 *
 *  @param pstrVersion [out] The version string to be returned.
 *  @param sVersion [in] Size of the string buffer.
 *  @param u8Major [in] The major version number.
 *  @param u8Minor [in] The minor version number.
 *  @param u32OEMCode [in] The OEM code.
 *  @param u8BuildNo [in] The build number.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getStringVersion(
    olchar_t * pstrVersion, olsize_t sVersion, const u8 u8Major, const u8 u8Minor,
    const u32 u32OEMCode, const u8 u8BuildNo);

/** Get string of u64 integer.
 *
 *  @param pstrInteger [out] The string to be returned.
 *  @param sInteger [in] Size of the string buffer.
 *  @param u64Integer [in] The integer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getStringU64(
    olchar_t * pstrInteger, olsize_t sInteger, const u64 u64Integer);

/** Get the string of true/false.
 *
 *  @param bTrue [in] The true status.
 *
 *  @return The string of the status.
 *  @retval True True.
 *  @retval False False.
 */
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringTrue(const boolean_t bTrue);

/** Get the string of unknown.
 *
 *  @return The string "Unknown".
 */
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringUnknown(void);

/** Get the string of not applicable "N/A".
 *
 *  @return The string "N/A".
 */
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringNotApplicable(void);

/** Get the string of not supported.
 *
 *  @return The string "Not Supported".
 */
STRINGAPI const olchar_t * STRINGCALL jf_string_getStringNotSupported(void);

/** Get the string of MAC Address.
 *
 *  @param pstrMacAddr [out] The string buffer where the MAC address string will return.
 *  @param sMacAddr [in] Size of the string.
 *  @param pu8Mac [in] The MAC addr information.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getStringMacAddress(
    olchar_t * pstrMacAddr, olsize_t sMacAddr, const u8 * pu8Mac);

/** Get the size string in TB, GB, MB, KB and B.
 *
 *  @note
 *  -# The size is calculated with 1024 byte.
 *  -# The size string will be the format of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>".
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param sStrSize [in] Size of the string.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getByteStringSize(
    olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size);

/** Get the size string in TB, GB, MB, KB and B.
 *
 *  @note
 *  -# The size is calculated with 1024 byte.
 *  -# The size string will be the format of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>".
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param sStrSize [in] Size of the string.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getByteStringSizeMax(
    olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size);

/** Get the size string based 1000 in TB, GB, MB, KB and B.
 *
 *  @note
 *  -# The size is calculated with 1000 byte.
 *  -# The size string will be the format of "<xxxx|xxxx.x|xxxx.xx><TB|GB|MB|KB|B>".
 *
 *  @param pstrSize [out] The size string to be returned.
 *  @param sStrSize [in] Size of the string.
 *  @param u64Size [in] The size in bytes.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getByteStringSize1000Based(
    olchar_t * pstrSize, olsize_t sStrSize, const u64 u64Size);

/*string validation*/

/** Validate the alias string.
 *
 *  @note
 *  -# The alias can contain digit, alphabet, space and underscore, but not started with space.
 *
 *  @param pstrAlias [in] The alias string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_validateStringAlias(const olchar_t * pstrAlias);

/** Validate the user name string.
 *
 *  @note
 *  -# The username can contain digit, alphabet and underscore.
 *
 *  @param pstrUserName [in] The user name string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_USER_NAME Invalid user name.
 */
STRINGAPI u32 STRINGCALL jf_string_validateStringUsername(const olchar_t * pstrUserName);

/** Validate the hex string.
 *
 *  @note
 *  -# The hex string can contain digit, alphabet "a~f" or "A~F".
 *
 *  @param pstrHex [in] The hex string.
 *  @param sHex [in] Size of the hex string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_STRING Invalid string.
 */
STRINGAPI u32 STRINGCALL jf_string_validateHexString(
    const olchar_t * pstrHex, const olsize_t sHex);

/** Validate the integer string.
 *
 *  @note
 *  -# The interger string can contain digit.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param sInteger [in] Size of the integer string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_STRING Invalid string.
 */
STRINGAPI u32 STRINGCALL jf_string_validateIntegerString(
    const olchar_t * pstrInteger, const olsize_t sInteger);

/** Validate the float string.
 *
 *  @note
 *  -# The interger string can contain digit, '-', 'e' and '.'.
 *
 *  @param pstrFloat [in] The integer string.
 *  @param sFloat [in] Size of the float string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_STRING Invalid string.
 */
STRINGAPI u32 STRINGCALL jf_string_validateFloatString(
    const olchar_t * pstrFloat, const olsize_t sFloat);

/*string scan*/

/** Get unsigned char from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu8Value [out] The unsigned char returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getU8FromString(
    const olchar_t * pstrInteger, const olsize_t size, u8 * pu8Value);

/** Get unsigned shor from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu16Value [out] The unsigned short returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getU16FromString(
    const olchar_t * pstrInteger, const olsize_t size, u16 * pu16Value);

/** Get signed integer from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param ps32Value [out] The signed integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getS32FromString(
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
 *  @param pstrHex [in] The integer hex string.
 *  @param size [in] The size of the string.
 *  @param ps32Value [out] The signed integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getS32FromHexString(
    const olchar_t * pstrHex, const olsize_t size, s32 * ps32Value);

/** Get unsigned integer from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu32Value [out] The unsigned integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getU32FromString(
    const olchar_t * pstrInteger, const olsize_t size, u32 * pu32Value);

/** Get long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param numeric [in] The long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getLongFromString(
    const olchar_t * pstrInteger, const olsize_t size, long * numeric);

/** Get unsigned long value from a string.
 *
 *  @param pstrInteger [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param numeric [in] The unsigned long value extracted from the string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getUlongFromString(
    const olchar_t * pstrInteger, const olsize_t size, unsigned long * numeric);

/** Read unsigned long long integer from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param pu64Value [out] The unsigned long long integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getU64FromString(
    const olchar_t * pstrInteger, const olsize_t size, u64 * pu64Value);

/** Read signed long long integer from string.
 *
 *  @param pstrInteger [in] The integer string.
 *  @param size [in] The size of the string.
 *  @param ps64Value [out] The signed long long integer returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getS64FromString(
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
STRINGAPI u32 STRINGCALL jf_string_getBinaryFromString(
    const olchar_t * pstr, const olsize_t size, u8 * pu8Binary, olsize_t * psBinary);

/** Get boolean from string.
 *
 *  @note
 *  -# The string can be "yes" or "no", "enabled" or "disabled", "true" or "false", the string is
 *   not case sensitive.
 *
 *  @param pstr [in] The string to read from.
 *  @param size [in] The length of the string.
 *  @param pbValue [out] The boolean returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_PARAM The string is invalid.
 */
STRINGAPI u32 STRINGCALL jf_string_getBooleanFromString(
    const olchar_t * pstr, const olsize_t size, boolean_t * pbValue);

/** Get the size from the byte string. The size string will be the format of "xxxx.xxTB|GB|MB|KB|B".
 *
 *  @param pstr [in] The byte string.
 *  @param size [in] The length of the string. 
 *  @param pu64Size [out] The size in bytes to be returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSizeFromByteString(
    const olchar_t * pstr, const olsize_t size, u64 * pu64Size);

/** Get MAC address from string.
 *
 *  @param pstrMacString [in] The string contains MAC address.
 *  @param pu8Value [out] The MAC address to be returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getMacAddressFromString(
    const olchar_t * pstrMacString, u8 * pu8Value);

/** Get float from string.
 *
 *  @param pstrFloat [in] The float string.
 *  @param size [in] The size of the string.
 *  @param pflValue [out] The float returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getFloatFromString(
    const olchar_t * pstrFloat, const olsize_t size, olfloat_t * pflValue);

/** Get double from string.
 *
 *  @param pstrDouble [in] The string contains the long floating-point number.
 *  @param size [in] The size of the string.
 *  @param pdbValue [out] The double returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getDoubleFromString(
	const olchar_t * pstrDouble, const olsize_t size, oldouble_t * pdbValue);

/** Get a unary array of ids from the string.
 * 
 *  @note
 *  -# The id array will only save specified number of id, other id are discarded.
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
STRINGAPI u32 STRINGCALL jf_string_getIdListFromString(
    const olchar_t * pstrIdList, olid_t * pids, olsize_t * psId);

/*setting parse*/

/** Break down the strings to an array pointing to each setting.
 *
 *  @note
 *  -# The setting is seperated by ','.
 *  -# Each setting has format with "tag=value"
 *  -# If numbers of tags are more than array size, only those tags with array size are returned.
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
STRINGAPI u32 STRINGCALL jf_string_processSettings(
    olchar_t * pstrSettings, olchar_t * pstrArray[], olsize_t * psArray);

/** Break down the strings to an array pointing to each keyword setting.
 *
 *  @note
 *  -# The keyword setting is seperated by '\0'.
 *  -# Each keyword setting has format with "tag=value"
 *  -# If numbers of tags are more than array size, only those tags with array size are returned.
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
STRINGAPI u32 STRINGCALL jf_string_processKeywordSettings(
    u8 * pu8Settings, olsize_t sSettings, olchar_t * pstrStrArray[], olsize_t * psArray);

/** Process setting string with format "name=value", the string is null-terminated.
 *
 *  @note
 *  -# After process, the string is modified. '\0' is added to the end of name.
 *
 *  @param pstrSetting [in] The setting string.
 *  @param ppstrName [out] The name of the setting string.
 *  @param ppstrValue [out] The value of the setting string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_processSettingString(
    olchar_t * pstrSetting, olchar_t ** ppstrName, olchar_t ** ppstrValue);

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
STRINGAPI u32 STRINGCALL jf_string_retrieveSettings(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrName, olchar_t * pstrValue,
    olsize_t sValue);

/** Validate the setting.
 * 
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
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
STRINGAPI u32 STRINGCALL jf_string_validateSettings(
    olchar_t * pstrNameArray[], olsize_t sNameArray, olchar_t * pstrArray[], olsize_t sArray,
    olindex_t * piArray);

/** Get value of the setting from the setting array.
 *
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
 *  -# Default value is returned incase there are any errors.
 *
 *  @param pstrArray [in] The setting string array.
 *  @param sArray [in] Number of element in the setting string array.
 *  @param pstrSettingName [in] Name of the setting.
 *  @param u32DefaultValue [in] Default value incase there is error.
 *  @param pu32Value [out] The setting value.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSettingsU32(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const u32 u32DefaultValue, u32 * pu32Value);

/** Get value of the setting from the setting array.
 *
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
 *  -# Default value is returned incase there are any errors.
 *
 *  @param pstrArray [in] The setting string array.
 *  @param sArray [in] Number of element in the setting string array.
 *  @param pstrSettingName [in] Name of the setting.
 *  @param u64DefaultValue [in] Default value incase there is error.
 *  @param pu64Value [out] The setting value.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSettingsU64(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const u64 u64DefaultValue, u64 * pu64Value);

/** Get value of the setting from the setting array.
 *
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
 *  -# Default value is returned incase there are any errors.
 *
 *  @param pstrArray [in] The setting string array.
 *  @param sArray [in] Number of element in the setting string array.
 *  @param pstrSettingName [in] Name of the setting.
 *  @param bDefaultValue [in] Default value incase there is error.
 *  @param pbValue [out] The setting value.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSettingsBoolean(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const boolean_t bDefaultValue, boolean_t * pbValue);

/** Get value of the setting from the setting array.
 *
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
 *  -# Default value is returned incase there are any errors.
 *
 *  @param pstrArray [in] The setting string array.
 *  @param sArray [in] Number of element in the setting string array.
 *  @param pstrSettingName [in] Name of the setting.
 *  @param pstrDefaultValue [in] Default value incase there is error.
 *  @param pstrValue [out] The setting value buffer.
 *  @param sValue [out] The size of the setting value buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSettingsString(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const olchar_t * pstrDefaultValue, olchar_t * pstrValue, olsize_t sValue);

/** Get value of the setting from the setting array.
 *
 *  @note
 *  -# The setting string array is a result of function jf_string_processSettings() or
 *   jf_string_processKeywordSettings().
 *  -# Default value is returned incase there are any errors.
 *
 *  @param pstrArray [in] The setting string array.
 *  @param sArray [in] Number of element in the setting string array.
 *  @param pstrSettingName [in] Name of the setting.
 *  @param dbDefaultValue [in] Default value incase there is error.
 *  @param pdbValue [out] The setting value.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
STRINGAPI u32 STRINGCALL jf_string_getSettingsDouble(
    olchar_t * pstrArray[], olsize_t sArray, const olchar_t * pstrSettingName,
    const oldouble_t dbDefaultValue, oldouble_t * pdbValue);

#endif /*JIUFENG_STRING_H*/

/*------------------------------------------------------------------------------------------------*/
