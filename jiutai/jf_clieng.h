/**
 *  @file jf_clieng.h
 *
 *  @brief Header file defines the interface of CLI engine.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_clieng library.
 *  -# CLI engine support several command set, each command set contains a number of command. 
 */

#ifndef JIUFENG_CLIENG_H
#define JIUFENG_CLIENG_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_filestream.h"

#undef CLIENGAPI
#undef CLIENGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_CLIENG_DLL)
        #define CLIENGAPI  __declspec(dllexport)
        #define CLIENGCALL
    #else
        #define CLIENGAPI
        #define CLIENGCALL __cdecl
    #endif
#else
    #define CLIENGAPI
    #define CLIENGCALL
#endif

/* --- constants and data structures ------------------------------------------------------------ */

/** Maximum output line length.
 */
#define JF_CLIENG_MAX_OUTPUT_LINE_LEN                   (100)

/** Maximum command line length.
 */
#define JF_CLIENG_MAX_COMMAND_LINE_LEN                  (320)

/** Maximum CLI name length.
 */
#define JF_CLIENG_MAX_CLI_NAME_LEN                      (32)

/** Length of full line caption.
 */
#define JF_CLIENG_CAP_FULL_LINE                         (JF_CLIENG_MAX_OUTPUT_LINE_LEN)

/** Length of half line caption.
 */
#define JF_CLIENG_CAP_HALF_LINE                         (JF_CLIENG_MAX_OUTPUT_LINE_LEN / 2)

/** Maximum output buffer length.
 */
#define JF_CLIENG_MAX_OUTPUT_BUFFER_SIZE                (512)

/** Define the CLI command data type.
 */
typedef void  jf_clieng_cmd_t;

/** Define the CLI command set data type.
 */
typedef void  jf_clieng_cmd_set_t;

/** Callback function which is called to print greeting.
 */
typedef u32 (* jf_clieng_fnPrintGreeting_t)(void * pMaster);

/** Callback function which is called before entering loop.
 */
typedef u32 (* jf_clieng_fnPreEnterLoop_t)(void * pMaster);

/** Callback function which is called before exiting loop.
 */
typedef u32 (* jf_clieng_fnPostExitLoop_t)(void * pMaster);

/** Define the parameter for initializing CLI engine.
 */
typedef struct
{
    /**CLI name.*/
    olchar_t jcip_strCliName[JF_CLIENG_MAX_CLI_NAME_LEN];
    /**Callback function to print greeting.*/
    jf_clieng_fnPrintGreeting_t jcip_fnPrintGreeting;
    /**Callback function before entering loop.*/
    jf_clieng_fnPreEnterLoop_t jcip_fnPreEnterLoop;
    /**Callback function after entering loop.*/
    jf_clieng_fnPostExitLoop_t jcip_fnPostExitLoop;
    /**Master object, the argument of the callback function.*/
    void * jcip_pMaster;
    /**New line string. e.g. "\n" or "\r\n".*/
    olchar_t * jcip_pstrNewLine;
    /**Maximum number of command set.*/
    u32 jcip_u32MaxCmdSet;
    /**File stream for saving output.*/
    jf_filestream_t * jcip_pjfOutput;
    /**Enable script engine if it's TRUE.*/
    boolean_t jcip_bEnableScriptEngine;
    u8 jcip_u8Reserved[7];
    /**The command script engine.*/
    olchar_t  jcip_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_LEN];
} jf_clieng_init_param_t;

/** Define the CLI caption data type.
 */
typedef struct
{
    /**Caption name, it will be truncated if the name length is larger than the length specified.*/
    olchar_t * jcc_pstrCaption;
    /**Length of caption name.*/
    u32 jcc_u32Len;
} jf_clieng_caption_t;

/** Callback function to set default parameters of command.
 */
typedef u32 (* jf_clieng_fnSetDefaultParam_t)(void * pMaster, void * pParam);

/** Callback function to parse the parameters of command.
 */
typedef u32 (* jf_clieng_fnParseCmd_t)(
    void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);

/** Callback function to process command.
 */
typedef u32 (* jf_clieng_fnProcessCmd_t)(void * pMaster, void * pParam);

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the CLI engine library.
 *
 *  @param pParam [in] The parameter for initilizing the CLI engine library.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_init(jf_clieng_init_param_t * pParam);

/** Run CLI engine.
 *
 *  @note
 *  -# It will enter loop unless user quit the CLI engine.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_run(void);

/** Stop CLI engine.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_stop(void);

/** Finalize CLI engine.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_fini(void);

/** Create a command set.
 *
 *  @param pstrName [in] Name of the command set.
 *  @param ppCmdSet [out] The command set created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_newCmdSet(
    const olchar_t * pstrName, jf_clieng_cmd_set_t ** ppCmdSet);

/** Create a command.
 *
 *  @param pstrName [in] Name of the command.
 *  @param fnSetDefaultParam [in] Callback function to set the default parameters of command.
 *  @param fnParseCmd [in] Callback function to parse parameters of command.
 *  @param fnProcessCmd [in] Callback function to process command.
 *  @param pParam [in] Parameters of command.
 *  @param ppCmd [out] The command created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_newCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd);

/** Add a command to command set.
 *
 *  @note
 *  -# One command can be added to different command set.
 *
 *  @param pCmd [out] The command.
 *  @param pCmdSet [out] The command set.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_addToCmdSet(
    jf_clieng_cmd_t * pCmd, jf_clieng_cmd_set_t * pCmdSet);

/** Output a line.
 *
 *  @note
 *  -# This routine produces output according to the format string.
 *  -# Maximum buffer size is for the formatted output, defined as JF_CLIENG_MAX_OUTPUT_BUFFER_SIZE.
 *  -# The buffer is outputted line by line, more is supported.
 *
 *  @param fmt [in] The line format.
 *  @param ... [in] The input to the line format.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_outputLine(const olchar_t * fmt, ...);

/** Output a raw line.
 *
 *  @note
 *  -# This routine produces output with the raw line, not the formatted string.
 *  -# No limit with the line length.
 *  -# The buffer is outputted line by line, more is supported.
 *
 *  @param line [in] The raw line to output.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine(olchar_t * line);

/** Output a raw line without any process.
 *
 *  @note
 *  -# This routine produces output with the raw line, not the formatted string.
 *  -# No limit with the line length.
 *  -# The line is outputted without any process, more is not supported.
 *  -# Caller should add line feed to the string buffer by itself, otherwise the string is difficult
 *   to read.
 *
 *  @param line [in] The raw line to output.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine2(const olchar_t * line);

/** Report not applicable option of command.
 *
 *  @param nOpt [in] The option.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_reportNotApplicableOpt(const olint_t nOpt);

/** Report invalid option of command.
 *
 *  @param nOpt [in] The option.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_reportInvalidOpt(const olint_t nOpt);

/** Set prompt of CLI engine.
 *
 *  @param pstrPrompt [in] The prompt string.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_setPrompt(const olchar_t * pstrPrompt);

/** Print the banner of the header bar for brief printing.
 *
 *  @param u32Len [in] The desired length of the banner in characters.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printBanner(u32 u32Len);

/** Print the banner of the header bar with right shift 4 spaces.
 *
 *  @param u32Len [in] The desired length of the banner in characters.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printBannerShift4(u32 u32Len);

/** Print the devider between two manageable elements for verbose printing.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printDivider(void);

/** Print the devider between two manageable elements with right shift 2 spaces.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printDividerShift2(void);

/** Print the hex dump of a data buffer byte by byte.
 *
 *  @param pu8Buffer [in] The pointer to the data buffer.
 *  @param u32Len [in] The length of the data buffer in bytes.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHexDumpInByte(u8 * pu8Buffer, u32 u32Len);

/** Print the header bar according to the caption list for brief printing.
 *
 *  @param pjcc [in] The pointer to the caption list.
 *  @param u32Count [in] The count of the captions in the caption list.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHeader(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

/** Print the header bar according to the caption list with right shift 4 spaces.
 *
 *  @param pjcc [in] The pointer to the caption list.
 *  @param u32Count [in] The count of the captions in the caption list.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHeaderShift4(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

/** Print one attribute in a line for verbose printing.
 *
 *  @param pjcc [in] The pointer to the caption of the attribute.
 *  @param pstrValue [in] The value of the attribute.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printOneFullLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrValue);

/** Print two attributes in a line for verbose printing.
 *
 *  @param pjcc [in] The pointer to the captions of the two attributes.
 *  @param pstrLeft [in] The value of the attribute to be printed at the left.
 *  @param pstrRight [in] The value of the attribute at the right.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printTwoHalfLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrLeft, const olchar_t * pstrRight);

/** Append brief column to the line.
 *
 *  @param pjcc [in] The pointer to the caption of the column.
 *  @param pstrLine [in/out] The line to append to.
 *  @param pstrColumn [in] The value of the column.
 *
 *  @return Void.
 */
CLIENGAPI void CLIENGCALL jf_clieng_appendBriefColumn(
    const jf_clieng_caption_t * pjcc, olchar_t * pstrLine, const olchar_t * pstrColumn);

#endif /*JIUFENG_CLIEN_H*/

/*------------------------------------------------------------------------------------------------*/

