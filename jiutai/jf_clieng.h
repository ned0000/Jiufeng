/**
 *  @file jf_clieng.h
 *
 *  @brief Header file defines the interface of CLI Engine.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_clieng library.
 */

#ifndef JIUFENG_CLIENG_H
#define JIUFENG_CLIENG_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <time.h>

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

typedef void  jf_clieng_cmd_t;
typedef void  jf_clieng_cmd_set_t;

/** Maximum output line length
 */
#define JF_CLIENG_MAX_OUTPUT_LINE_LEN      (100)

#define JF_CLIENG_MAX_COMMAND_LINE_SIZE    (320)

#define JF_CLIENG_MAX_CLI_NAME_LEN         (32)

typedef u32 (* jf_clieng_fnPrintGreeting_t)(void * pMaster);
typedef u32 (* jf_clieng_fnPreEnterLoop_t)(void * pMaster);
typedef u32 (* jf_clieng_fnPostExitLoop_t)(void * pMaster);

typedef struct
{
    /**Max number of characters for one command.*/
    olsize_t jcip_sMaxCmdLine;
    /**Number of commands in command history buffer.*/
    olsize_t jcip_sCmdHistroyBuf;
    /**CLI name.*/
    olchar_t jcip_strCliName[JF_CLIENG_MAX_CLI_NAME_LEN];
    jf_clieng_fnPrintGreeting_t jcip_fnPrintGreeting;
    jf_clieng_fnPreEnterLoop_t jcip_fnPreEnterLoop;
    jf_clieng_fnPostExitLoop_t jcip_fnPostExitLoop;
    void * jcip_pMaster;
    /**New line string. e.g. "\n" or "\r\n".*/
    olchar_t * jcip_pstrNewLine;
    u32 jcip_u32MaxCmdSet;
    boolean_t jcip_bEnableScriptEngine;
    u8 jcip_u8Reserved[3];
    jf_filestream_t * jcip_pjfOutput;
    olchar_t  jcip_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_SIZE];
} jf_clieng_init_param_t;

typedef struct
{
    /*The length of caption must be smaller than the length.*/
    olchar_t * jcc_pstrCaption;
    u32 jcc_u32Len;
} jf_clieng_caption_t;

#define JF_CLIENG_CAP_FULL_LINE  (JF_CLIENG_MAX_OUTPUT_LINE_LEN)
#define JF_CLIENG_CAP_HALF_LINE  (JF_CLIENG_MAX_OUTPUT_LINE_LEN / 2)

typedef u32 (* jf_clieng_fnSetDefaultParam_t)(void * pMaster, void * pParam);
typedef u32 (* jf_clieng_fnParseCmd_t)(
    void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);
typedef u32 (* jf_clieng_fnProcessCmd_t)(void * pMaster, void * pParam);

/* --- functional routines ---------------------------------------------------------------------- */

CLIENGAPI u32 CLIENGCALL jf_clieng_init(jf_clieng_init_param_t * pParam);

/** Run CLI engine.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_run(void);

/** Stop CLI engine.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_stop(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_fini(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_newCmdSet(
    const olchar_t * pstrName, jf_clieng_cmd_set_t ** ppCmdSet);

CLIENGAPI u32 CLIENGCALL jf_clieng_newCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd);

CLIENGAPI u32 CLIENGCALL jf_clieng_addToCmdSet(
    jf_clieng_cmd_t * pCmd, jf_clieng_cmd_set_t * pCmdSet);

CLIENGAPI u32 CLIENGCALL jf_clieng_outputLine(const olchar_t * fmt, ...);

CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine(const olchar_t * line);

/** Output "line" with new line.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine2(const olchar_t * line);

CLIENGAPI u32 CLIENGCALL jf_clieng_reportNotApplicableOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL jf_clieng_reportInvalidOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL jf_clieng_setPrompt(const olchar_t * pstrPrompt);

/** Print the banner of the header bar for brief printing.
 *
 *  @param u32Len [in] The desired length of the banner in characters.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printBanner(u32 u32Len);

CLIENGAPI u32 CLIENGCALL jf_clieng_printBannerShift4(u32 u32Len);

/** Print the devider between two manageable elements for verbose printing.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printDivider(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_printDividerShift2(void);

/** Print the hex dump of a data buffer byte by byte.
 *
 *  @param pu8Buffer [in] The pointer to the data buffer.
 *  @param u32Len [in] The length of the data buffer in bytes.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHexDumpInByte(
    u8 * pu8Buffer, u32 u32Len);

/** Print the header bar according to the caption list for brief printing.
 *
 *  @param pjcc [in] The pointer to the caption list.
 *  @param u32Count [in] The count of the captions in the caption list.
 *
 *  @return The error code.
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHeader(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

CLIENGAPI u32 CLIENGCALL jf_clieng_printHeaderShift4(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

/** Print one attribute in a line for verbose printing.
 *
 *  @param pjcc [in] The pointer to the caption of the attribute.
 *  @param pstrValue [in] The value of the attribute.
 *
 *  @return The error code.
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
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printTwoHalfLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrLeft,
    const olchar_t *pstrRight);

/** Append brief column to the line.
 *
 *  @param pjcc [in] The pointer to the caption of the column.
 *  @param pstrLine [in/out] The line to append to.
 *  @param pstrColumn [in] The value of the column.
 */
CLIENGAPI void CLIENGCALL jf_clieng_appendBriefColumn(
    const jf_clieng_caption_t * pjcc, olchar_t * pstrLine,
    const olchar_t * pstrColumn);

#endif /*JIUFENG_CLIEN_H*/

/*------------------------------------------------------------------------------------------------*/

