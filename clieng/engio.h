/**
 *  @file engio.h
 *
 *  @brief CLI engine input & output header file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef CLIENG_IO_H
#define CLIENG_IO_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_clieng.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the clieng io key.
 */
typedef enum
{
    cik_unknown = 0,
    cik_tab,
    cik_up,
    cik_down,
    cik_right,
    cik_left,
    cik_enter,
    cik_esc,
    cik_pgup,
    cik_pgdn,
    cik_space,
    cik_save,
    cik_restore,
    cik_return,
    cik_help,
    cik_backspace,
    cik_delete,
    cik_home,
    cik_end,
    cik_insert,
    cik_f1,
    cik_f2,
    cik_f3,
    cik_f4,
    cik_f5,
    cik_f6,
    cik_f7,
    cik_f8,
    cik_f9,
    cik_f10,
    cik_f11,
    cik_f12,
} clieng_io_key_t;

/** Define the input type.
 */
typedef enum
{
    cit_unknown = 0,
    cit_line,
    cit_navigation_cmd,
    cit_param,
    cit_password,
    cit_anykey,
} clieng_input_type_t;

/** Define the navigation command for command history.
 */
typedef enum
{
    cnc_nextCommand = 1,
    cnc_previousCommand,
} clieng_navigation_command_t;

/** Define the output type.
 */
typedef enum
{
    cot_unknown = 0,
    cot_text,
    cot_navigation,
} clieng_output_type_t;

/** Define the get tty type.
 */
typedef enum
{
    cgt_baudRate = 1,
    cgt_flowControl,
} clieng_gettty_type_t;

/** Define the set tty type.
 */
typedef enum
{
    cst_baudRate = 1,
    cst_flowControl,
} clieng_settty_type_t;

/** Define the parameters for initializing io module.
 */
typedef struct
{
    /**New line string.*/
    olchar_t * cip_pstrNewLine;
    jf_filestream_t * cip_pjfOutput;
    u32 cip_u32Reserved[4];
} clieng_io_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the CLI input and output module.
 *
 *  @param pciop [in] The parameters to the input & output module.
 *
 *  @return The error code.
 */
u32 initCliengIo(clieng_io_param_t * pciop);

/** Destroy the input and output module.
 *
 *  @return The error code.
 */
u32 finiCliengIo(void);

/** Get the specified input.
 *
 *  @param pcitInputType [in/out] as in, the desired input type. If it is cit_unknown, it means any
 *   type; as out, it is the returned input type.
 *  @param pstrInputBuf [out] the buffer for the input to be returned.
 *  @param psInputBuf [in/out] As in, it is the size of the input buffer; As out, it it the length
 *   of the input data.
 *  @param sPrompt [in] Prompt length.
 *
 *  @return The error code.
 */
u32 engioInput(
    clieng_input_type_t * pcitInputType, olchar_t * pstrInputBuf, olsize_t * psInputBuf,
    olsize_t sPrompt);

/** Output the data and a new line.
 *
 *  @param fmt [in] The format in which the data is to be output.
 *  @param ap [in] The parameters.
 *
 *  @return The error code.
 */
u32 voutputLine(const olchar_t * fmt, va_list ap);

/** Output the data and a new line.
 *
 *  @param fmt [in] The format in which the data is to be output.
 *  @param ... [in] The parameters.
 *
 *  @return The error code.
 */
u32 outputLine(const olchar_t * fmt, ...);

/** Output the data.
 *
 *  @param fmt [in] The format in which the data is to be output.
 *
 ×  @return The error code.
 */
u32 engioOutput(const olchar_t * fmt, ...);

/** Output a paragraph, substitute the '\n' with the new line.
 *
 *  @param pstrParagraph [in] The paragraph to be output.
 *
 ×  @return The error code.
 */
u32 outputParagraph(const olchar_t * pstrParagraph);

/** Switch on/off the special delimit.
 *
 *  @param bSwitchOn [in] TRUE means to switch it on; FALSE means to switch it off.
 *
 ×  @return The error code.
 */
u32 switchSpecialDelimit(boolean_t bSwitchOn);

/** Get the string of EOL (End of Line).
 *
 ×  @return The error code.
 */
olchar_t * getEOL(void);

/** Get the string of delimit.
 *
 *  @note
 *  -# When the special delimit is turned on, this function returns the string of the special
 *   delimit regardless of the desired total length (it means the colume is not aligned);
 *   otherwise, it returns string of blank spaces.
 *  -# The number of blank spaces depends on the desired total length. If the specified string
 *   is already longer than the total length, only one blank space will be returned.
 *
 *  @param sStr [in] The length of the string that needs delimit.
 *  @param sTotal [in] The desired total length of the string and the delimit.
 *
 ×  @return The error code.
 */
olchar_t * getDelimit(const olsize_t sStr, olsize_t sTotal);

/** Enable 'more' command.
 *
 *  @return Void.
 */
void setMoreEnable(void);

/** Disable 'more' command.
 *
 *  @return Void.
 */
void setMoreDisable(void);

/** Get a key from user.
 *
 *  @param pcikKey [out] The key from user.
 *  @param pstrKey [out] The key in char if it's unknown.
 *
 ×  @return The error code.
 */
u32 getInputKey(clieng_io_key_t * pcikKey, olchar_t * pstrKey);

#endif /*CLIENG_IO_H*/

/*------------------------------------------------------------------------------------------------*/
