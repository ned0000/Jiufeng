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

/* --- standard C lib header files ----------------------------------------- */
#include <stdarg.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "clieng.h"
#include "stringparse.h"

/* --- constant definitions ------------------------------------------------ */

#define MAX_OUTPUT_BUFFER_LEN    (512) 

/* --- data structures ----------------------------------------------------- */

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

typedef enum
{
    cit_unknown = 0,
    cit_line,
    cit_navigation_cmd,
    cit_param,
    cit_password,
    cit_anykey,
} clieng_input_type_t;

typedef enum
{
    cnc_nextCommand = 1,
    cnc_previousCommand,
} clieng_navigation_command_t;

typedef enum
{
    cot_unknown = 0,
    cot_text,
    cot_navigation,
} clieng_output_type_t;

typedef enum
{
    cgt_baudRate = 1,
    cgt_flowControl,
} clieng_gettty_type_t;

typedef enum
{
    cst_baudRate = 1,
    cst_flowControl,
} clieng_settty_type_t;

typedef struct
{
    olchar_t * cip_pstrNewLine;
    FILE * cip_fhOutput;
    u32 cip_u32Reserved[4];
} clieng_io_param_t;

/* --- functional routines ------------------------------------------------- */

/** Create the CLI input and output module.
 *
 *  @param pciop [in] the parameters to the input & output module
 *
 *  @return the error code
 */
u32 initCliengIo(clieng_io_param_t * pciop);

/** Destroy the input and output module.
 *
 *  @return the error code
 */
u32 finiCliengIo(void);

/** Get the specified input.
 *
 *  @param pcitInputType [in/out] as in, the desired input type. If it is
 *   cit_unknown, it means any type; as out, it is the returned input type.
 *  @param pstrInputBuf [out] the buffer for the input to be returned.
 *  @param psInputBuf [in/out] as in, it is the size of the input buffer;
 *   as out, it it the length of the input data.
 *  @param sPrompt [in] prompt length
 *
 *  @return the error code
 */
u32 engioInput(
    clieng_input_type_t * pcitInputType,
    olchar_t * pstrInputBuf, olsize_t * psInputBuf, olsize_t sPrompt);

/** Output the data and a new line
 *
 *  @param fmt [in] the format in which the data is to be output.
 *  @param ap [in] the parameters
 *
 *  @return the error code
 */
u32 voutputLine(const olchar_t * fmt, va_list ap);

u32 outputLine(const olchar_t * fmt, ...);

/** Output the data
 *
 *  @param fmt [in] the format in which the data is to be output.
 *
 ×  @return the error code
 */
u32 engioOutput(const olchar_t * fmt, ...);

/** Output a paragraph, substitute the '\n' with the new line
 *
 *  @param pstrParagraph [in] the paragraph to be output.
 *
 ×  @return the error code
 */
u32 outputParagraph(const olchar_t * pstrParagraph);

/** Switch on/off the special delimit
 *
 *  @param bSwitchOn [in] TRUE means to switch it on; FALSE means to switch it
 *   off.
 *
 ×  @return the error code
 */
u32 switchSpecialDelimit(boolean_t bSwitchOn);

/** Get the string of EOL (End of Line)
 *
 ×  @return the error code
 */
char * getEOL(void);

/** Get the string of delimit. When the special delimit is turned on, this
 *  function returns the string of the special delimit regardless of the desired
 *  total length, otherwise, it returns string of blank spaces. The number of
 *  blank spaces will depends on the desired total length. If the specified
 *  string is already longer than the total length, only one blank space will be
 *  returned.
 *
 *  @param sStr [in] the length of the string that needs delimit.
 *  @param sTotal [in] the desired total length of the string and the delimit
 *
 ×  @return the error code
 */
char * getDelimit(const olsize_t sStr, olsize_t sTotal);

void setMoreEnable(void);
void setMoreDisable(void);

u32 getInputKey(clieng_io_key_t * pcikKey, olchar_t * pstrKey);

#endif /*CLIENG_IO_H*/

/*---------------------------------------------------------------------------*/


